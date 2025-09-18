#include "framework.h"

#include "shader.h"
#include "texture.h"
#include "font_data.h"
#include "camera.h"
#include "model_resource.h"
#include "model_instance.h"

#include "scenes/load_shader_scene.h"

#include "Renderer/scene_renderer.h"
#include "Renderer/effect_renderer.h"
#include "Renderer/ui_renderer.h"

#include "Object/object_manager.h"

#include "Object/decal_object.h"
#include "Object/ui/ui_object.h"

#if defined(USE_IMGUI)
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif // defined(USE_IMGUI)

#if defined(_DEBUG)
#include "Graphics/debug_primitive_collector.h"
#endif // defined(_DEBUG)

framework::framework(HWND hwnd) : hwnd(hwnd)
{
}

framework::~framework()
{
}

int framework::run()
{
	MSG msg{};

	if (!initialize())
	{
		return 0;
	}

#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().Fonts->AddFontFromFileTTF("Assets/Resources/Fonts/Noto_Sans_JP/NotoSansJP-VariableFont_wght.ttf", 24.0f, nullptr, glyphRangesJapanese);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(render_data::instance()->get_device().Get(), render_data::instance()->get_immediate_context().Get());
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
#endif

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			tictoc.tick();
			calculate_frame_stats();
			update(tictoc.time_interval());
			render(tictoc.time_interval());
		}
	}

#ifdef USE_IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	BOOL fullscreen{};
	render_data::instance()->get_swap_chain()->GetFullscreenState(&fullscreen, 0);
	if (fullscreen)
	{
		render_data::instance()->get_swap_chain()->SetFullscreenState(FALSE, 0);
	}

	return uninitialize() ? static_cast<int>(msg.wParam) : 0;
}

LRESULT framework::handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif

	if (mouse::instance()->window_proccess(hwnd, msg, wparam, lparam)) { return true; }

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);

		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
		{
			PostMessage(hwnd, WM_CLOSE, 0, 0);
		}
		break;
	case WM_ENTERSIZEMOVE:
		tictoc.stop();
		break;
	case WM_EXITSIZEMOVE:
		tictoc.start();
		break;
	case WM_MOUSEWHEEL:
		wheel = GET_WHEEL_DELTA_WPARAM(wparam);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

bool framework::initialize()
{
	HRESULT hr{ S_OK };

	render_data::create_instance(hwnd);
	mouse::create_instance(hwnd);
	keyboard::create_instance();
	texture_manager::create_instance();
	font_data_manager::create_instance();
	shader_manager::create_instance();
	pipeline_state_manager::create_instance();
	light_manager::create_instance();
	post_process_volume_manager::create_instance();
	camera_manager::create_instance();
	model_resource_manageer::create_instance();
	model_instance_maanger::create_instance();
#if defined(_DEBUG)
	debug_primitive_manager::create_instance();
#endif // defined(_DEBUG)
	collision_filter_manager::create_instance();
	object_manager::create_instance();
	scene_manager::create_instance();

	//	各種バッファ用意
	render_targets = std::make_shared<scene_render_targets>();
	if (!create_buffers())
		return false;

	//	当たり判定フィルター用意
	if (!create_collision_filters())
		return false;

	//	レンダラー登録
	{
		scene_renderers.emplace_back(std::make_unique<depth_prepass_renderer>(render_targets));
		scene_renderers.emplace_back(std::make_unique<gbuffer_renderer>(render_targets));
		scene_renderers.emplace_back(std::make_unique<gbuffer_decal_renderer>(render_targets));
		scene_renderers.emplace_back(std::make_unique<deffered_lighting_renderer>(render_targets));
		scene_renderers.emplace_back(std::make_unique<postprocess_renderer>(render_targets));
		scene_renderers.emplace_back(std::make_unique<effect_renderer>(render_targets));
		scene_renderers.emplace_back(std::make_unique<postprocess_after_effect_renderer>(render_targets));
		scene_renderers.emplace_back(std::make_unique<finalpass_renderer>(render_targets));

#if defined(_DEBUG)
		scene_renderers.emplace_back(std::make_unique<debug_draw_pass_renderer>(render_targets));
#endif // defined(_DEBUG)

		ui_renderers.emplace_back(std::make_unique<ui_renderer>(render_targets));
	}

	//	初回シーン登録
	scene_manager::instance()->change_scene<load_shader_scene>();
	return true;
}

void framework::update(float elapsed_time/*Elapsed seconds from last frame*/)
{
	//	入力関係の更新
	mouse::instance()->update_state();
	keyboard::instance()->update_state();

	//	シーン内の更新
	if (scene_manager::is_valid())
	{
		scene_manager::instance()->updates(elapsed_time);
	}

	//	オブジェクト更新
	if (object_manager::is_valid())
	{
		object_manager::instance()->updates(elapsed_time);
	}

	//	エフェクト更新
	if (effect_manager::is_valid())
	{
		effect_manager::instance()->updates(elapsed_time);
	}

#if defined(USE_IMGUI)
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (scene_manager::is_valid())
	{
		scene_manager::instance()->imgui_render();
	}

	//	オブジェクト情報表示
	object_manager::instance()->imgui_render();

	//	メイン
	if (ImGui::Begin("main"))
	{
		bool pause = object_manager::instance()->get_pause();
		ImGui::Checkbox("pause", &pause);
		object_manager::instance()->set_pause(pause);
		ImGui::SameLine();
		if (ImGui::Button("step"))
		{
			object_manager::instance()->called_step();
		}
	}
	ImGui::End();

	//	描画系情報
	if (ImGui::Begin("render infos"))
	{
		ImGui::Text("fixed_update_count : %d", object_manager::instance()->get_fixed_update_count());

#if defined(_DEBUG)
		bool draw_culling_mesh_aabb = model_instance_maanger::instance()->get_draw_culling_mesh_aabb();
		ImGui::Checkbox("draw_culling_mesh_aabb", &draw_culling_mesh_aabb);
		model_instance_maanger::instance()->set_draw_culling_mesh_aabb(draw_culling_mesh_aabb);

		bool draw_collision = object_manager::instance()->get_draw_collision();
		ImGui::Checkbox("draw_collision", &draw_collision);
		object_manager::instance()->set_draw_collision(draw_collision);

		int draw_mesh_count = static_cast<int>(model_instance_maanger::instance()->get_draw_mesh_count());
		ImGui::DragInt("draw_mesh_count", &draw_mesh_count);

		int draw_shadow_mesh_count = static_cast<int>(model_instance_maanger::instance()->get_draw_shadow_mesh_count());
		ImGui::DragInt("draw_shadow_mesh_count", &draw_shadow_mesh_count);

		int max_draw_mesh_count = static_cast<int>(model_instance_maanger::instance()->get_max_draw_mesh_count());
		ImGui::DragInt("max_draw_mesh_count", &max_draw_mesh_count);
#endif	//	defined(_DEBUG)

		ImGui::Checkbox("xz", &display_grid_xz); ImGui::SameLine();
		ImGui::Checkbox("xy", &display_grid_xy); ImGui::SameLine();
		ImGui::Checkbox("yz", &display_grid_yz);
		ImGui::DragInt("display_grid_length", &display_grid_length, 1.0f, 0, 1000);
	}
	ImGui::End();

	//	シーン変更
	imgui_scene_change();
#endif // defined(USE_IMGUI)
}

void framework::render(float elapsed_time/*Elapsed seconds from last frame*/)
{
	//	描画前処理
	if (object_manager::is_valid())
	{
		object_manager::instance()->pre_render();
	}

	//	描画
	{
		graphics_context context;
		context.device_context = render_data::instance()->get_immediate_context().Get();
		light_manager::instance()->get_container(context.light_container);
		post_process_volume_manager::instance()->get_container(context.post_process_volume_container);

#if defined(_DEBUG)
		if (display_grid_xz || display_grid_xy || display_grid_yz)
		{
			float grid_length = system_units::convert_m_to_unit(static_cast<float>(display_grid_length));
			for (int index = -display_grid_length; index <= display_grid_length; ++index)
			{
				float len = system_units::convert_m_to_unit(static_cast<float>(index));
				if (display_grid_xz)
				{
					DirectX::XMFLOAT4 line_color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1);
					PUSH_DEBUG_PRIMITIVE_LINE(DirectX::XMFLOAT3(len, 0.0f, -grid_length), DirectX::XMFLOAT3(len, 0.0f, +grid_length), line_color);
					PUSH_DEBUG_PRIMITIVE_LINE(DirectX::XMFLOAT3(-grid_length, 0.0f, len), DirectX::XMFLOAT3(+grid_length, 0.0f, len), line_color);
				}
				if (display_grid_xy)
				{
					DirectX::XMFLOAT4 line_color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1);
					PUSH_DEBUG_PRIMITIVE_LINE(DirectX::XMFLOAT3(len, -grid_length, 0.0f), DirectX::XMFLOAT3(len, +grid_length, 0.0f), line_color);
					PUSH_DEBUG_PRIMITIVE_LINE(DirectX::XMFLOAT3(-grid_length, len, 0.0f), DirectX::XMFLOAT3(+grid_length, len, 0.0f), line_color);
				}
				if (display_grid_yz)
				{
					DirectX::XMFLOAT4 line_color = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1);
					PUSH_DEBUG_PRIMITIVE_LINE(DirectX::XMFLOAT3(0.0f, len, -grid_length), DirectX::XMFLOAT3(0.0f, len, +grid_length), line_color);
					PUSH_DEBUG_PRIMITIVE_LINE(DirectX::XMFLOAT3(0.0f, -grid_length, len), DirectX::XMFLOAT3(0.0f, +grid_length, len), line_color);
				}
			}
		}
		debug_primitive_manager::instance()->get_container(context.debug_draw_container);
#endif // defined(_DEBUG)

		//	クリア処理
		{
			back_buffer_clear_rendering_pass.activate(context.device_context);
			back_buffer_clear_rendering_pass.deactivate(context.device_context);
		}

		static float total_time = 0;

		//	カメラ分描画(仮)
		camera_container camera_container;
		camera_manager::instance()->get_container(camera_container);
		for (auto& camera : camera_container.cameras)
		{
			//	シーン定数バッファ設定
			{
				context.scene_constant.time_data.x = total_time + elapsed_time;
				context.scene_constant.time_data.y = total_time;

				context.scene_constant.camera_position.x = camera->get_position().x;
				context.scene_constant.camera_position.y = camera->get_position().y;
				context.scene_constant.camera_position.z = camera->get_position().z;
				context.scene_constant.camera_position.w = 1.0f;

				context.scene_constant.camera_direction.x = camera->get_front().x;
				context.scene_constant.camera_direction.y = camera->get_front().y;
				context.scene_constant.camera_direction.z = camera->get_front().z;
				context.scene_constant.camera_direction.w = 0.0f;

				context.scene_constant.viewport_rect.x = camera->get_screen_rect().x;
				context.scene_constant.viewport_rect.y = camera->get_screen_rect().y;
				context.scene_constant.viewport_rect.z = camera->get_screen_rect().z;
				context.scene_constant.viewport_rect.w = camera->get_screen_rect().w;

				context.scene_constant.camera_clip_distance.x = camera->get_clip_distance().x;
				context.scene_constant.camera_clip_distance.y = camera->get_clip_distance().y;
				context.scene_constant.camera_clip_distance.z = camera->get_clip_distance().x * camera->get_clip_distance().y;
				context.scene_constant.camera_clip_distance.w = camera->get_clip_distance().y - camera->get_clip_distance().x;

				context.scene_constant.z_buffer_parameteres.y = camera->get_clip_distance().y / camera->get_clip_distance().x;
				context.scene_constant.z_buffer_parameteres.x = 1.0f - context.scene_constant.z_buffer_parameteres.y;
				context.scene_constant.z_buffer_parameteres.z = context.scene_constant.z_buffer_parameteres.x / camera->get_clip_distance().y;
				context.scene_constant.z_buffer_parameteres.w = context.scene_constant.z_buffer_parameteres.y / camera->get_clip_distance().y;

				context.scene_constant.view_transform = camera->get_view_matrix();
				context.scene_constant.projection_transform = camera->get_projection_matrix();
				context.scene_constant.view_projection_transform = camera->get_view_projection_matrix();

				DirectX::XMMATRIX View, Projection, ViewProjection;
				View = DirectX::XMLoadFloat4x4(&context.scene_constant.view_transform);
				Projection = DirectX::XMLoadFloat4x4(&context.scene_constant.projection_transform);
				ViewProjection = DirectX::XMLoadFloat4x4(&context.scene_constant.view_projection_transform);
				DirectX::XMStoreFloat4x4(&context.scene_constant.inverse_view_transform, DirectX::XMMatrixInverse(nullptr, View));
				DirectX::XMStoreFloat4x4(&context.scene_constant.inverse_projection_transform, DirectX::XMMatrixInverse(nullptr, Projection));
				DirectX::XMStoreFloat4x4(&context.scene_constant.inverse_view_projection_transform, DirectX::XMMatrixInverse(nullptr, ViewProjection));

				context.scene_constant.previous_view_projection_transform = camera->get_previous_view_projection_matrix();
			}

			//	モデル取得
			model_instance_maanger::instance()->get_container(context.mesh_container, camera->get_frustum_culling());

			//	デカールコンテナ生成
			{
				auto decals = object_manager::instance()->search_objects<decal_object>();
				context.decal_container.clear();
				for (auto& decal : decals)
				{
					if (!decal->is_active())
						continue;

					//	カリング
					collision::bounding_box box = collision::bounding_box(decal->get_scale(), decal->get_rotation(), decal->get_position());
					if (camera->get_frustum_culling().intersection(&box))
					{
						context.decal_container.decal_datas.emplace_back(decal->convert_decal_data());
					}
				}
			}

			//	描画
			for (auto& render : scene_renderers)
			{
				render->render(context);
			}

			//	カメラの視線投影行列保存
			camera->increment_previous_view_projection_matrix();
		}

		//	UIオブジェクトコンテナ生成
		{
			int sort_key = 0;
			context.sprite_container.sprite_datas.clear();

			auto ui_object_list = object_manager::instance()->search_objects<ui_object>();
			for (auto& ui_object : ui_object_list)
			{
				if (!ui_object->is_active())
					continue;

				//	スプライト納入
				sprite_datas sprite_data;
				sprite_data.sort_key = sort_key;
				sprite_data.sort_order = ui_object->get_sort_order();
				sprite_data.sprite = &ui_object->get_sprite();
				context.sprite_container.sprite_datas.emplace_back(sprite_data);
				++sort_key;
			}

			auto ui_objects_list = object_manager::instance()->search_objects<ui_objects>();
			for (auto& ui_object : ui_objects_list)
			{
				if (!ui_object->is_active())
					continue;

				//	スプライト納入
				for (auto& sprite : ui_object->get_sprites())
				{
					sprite_datas sprite_data;
					sprite_data.sort_key = sort_key;
					sprite_data.sort_order = ui_object->get_sort_order();
					sprite_data.sprite = &sprite;
					context.sprite_container.sprite_datas.emplace_back(sprite_data);
				}
				++sort_key;
			}
			//	オーダーでソート
			std::sort(context.sprite_container.sprite_datas.begin(),
				context.sprite_container.sprite_datas.end(),
				[](sprite_datas& l, sprite_datas& r)
				{
					if (l.sort_order < r.sort_order)
						return l.sort_key != r.sort_key;
					return false;
				});

			//	シーン定数バッファ設定
			//	最低限の情報だけ設定しておく
			{
				context.scene_constant.time_data.x = total_time + elapsed_time;
				context.scene_constant.time_data.y = total_time;

				auto tex2d = std::static_pointer_cast<texture2d>(render_targets->scene_color_texture);
				context.scene_constant.viewport_rect.x = 0;
				context.scene_constant.viewport_rect.y = 0;
				context.scene_constant.viewport_rect.z = static_cast<float>(tex2d->get_width());
				context.scene_constant.viewport_rect.w = static_cast<float>(tex2d->get_height());
			}
			//	描画
			for (auto& render : ui_renderers)
			{
				render->render(context);
			}
		}
		//	
		total_time += elapsed_time;
	}

	//	描画後処理
	if (object_manager::is_valid())
	{
		object_manager::instance()->post_render();
	}

#if defined(USE_IMGUI)
	//	ImGui描画
	{
		imgui_rendering_pass.activate(render_data::instance()->get_immediate_context().Get());
		render_data::instance()->get_immediate_context()->RSSetViewports(1, &imgui_viewport);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		imgui_rendering_pass.deactivate(render_data::instance()->get_immediate_context().Get());

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
#endif // defined(USE_IMGUI)

	//	プレゼント
	UINT sync_interval{ 0 };
	render_data::instance()->get_swap_chain()->Present(sync_interval, 0);
}

bool framework::uninitialize()
{
	scene_manager::delete_instance();
	if (object_manager::is_valid())
	{
		object_manager::instance()->finalize();
		object_manager::delete_instance();
	}
	collision_filter_manager::delete_instance();
#if defined(_DEBUG)
	debug_primitive_manager::delete_instance();
#endif // defined(_DEBUG)
	model_instance_maanger::delete_instance();
	model_resource_manageer::delete_instance();
	camera_manager::delete_instance();
	post_process_volume_manager::delete_instance();
	light_manager::delete_instance();
	pipeline_state_manager::delete_instance();
	shader_manager::delete_instance();
	font_data_manager::delete_instance();
	texture_manager::delete_instance();
	keyboard::delete_instance();
	mouse::delete_instance();
	render_data::delete_instance();

	return true;
}

void framework::calculate_frame_stats()
{
	if (++frames, (tictoc.time_stamp() - current_time) >= 1.0f)
	{
		float fps = static_cast<float>(frames);
		std::wostringstream outs;
		outs.precision(6);
		outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
		SetWindowTextW(hwnd, outs.str().c_str());

		frames = 0;
		previous_time = current_time;
		current_time += 1.0f;
	}
}

bool framework::create_buffers()
{
	int scene_width = SCREEN_WIDTH;
	int scene_height = SCREEN_HEIGHT;

	//	バックバッファクリア用
	{
		back_buffer_clear_rendering_pass.render_targets.emplace_back(render_data::instance()->get_swap_chain());
	}

	{
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		texture2d_desc.Width = scene_width;
		texture2d_desc.Height = scene_height;
		texture2d_desc.MipLevels = 1;
		texture2d_desc.ArraySize = 1;
		texture2d_desc.SampleDesc.Count = 1;
		texture2d_desc.SampleDesc.Quality = 0;
		texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
		texture2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		texture2d_desc.CPUAccessFlags = 0;
		texture2d_desc.MiscFlags = 0;

		//	シーンテクスチャ生成
		{
			texture2d_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			texture2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
			if (!texture_manager::instance()->generate_texture2d(render_targets->scene_color_texture, SceneColorName, texture2d_desc))
			{
				return false;
			}
		}

		//	深度ステンシルバッファを生成
		{
			texture2d_desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

			D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
			shader_resource_view_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
			shader_resource_view_desc.Texture2D.MipLevels = 1;
			if (!texture_manager::instance()->generate_texture2d(render_targets->scene_depth_stencil_texture, SceneDepthStencilName, texture2d_desc, &shader_resource_view_desc))
			{
				return false;
			}
		}

		//	GBuffer生成
		{
			texture2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
			DXGI_FORMAT formats[] =
			{
				DXGI_FORMAT_R8G8B8A8_UNORM,			//	BaseColor
				DXGI_FORMAT_R8G8B8A8_UNORM,			//	Emissive
				DXGI_FORMAT_R32G32B32A32_FLOAT,		//	Normal_Depth
				DXGI_FORMAT_R8G8B8A8_UNORM,			//	Parameters
				DXGI_FORMAT_R32G32B32A32_FLOAT,		//	Velocity
				DXGI_FORMAT_R32G32B32A32_UINT,		//	UniqueData

				DXGI_FORMAT_R8G8B8A8_UNORM,			//	ShadeColor
			};
			for (int i = 0; i < static_cast<int>(GBufferId::Max); ++i)
			{
				texture_ptr texture;
				texture2d_desc.Format = formats[i];
				if (texture_manager::instance()->generate_texture2d(texture, GBufferNames[i], texture2d_desc))
				{
					render_targets->g_buffer_textures.emplace_back(texture);
				}
			}
		}
	}

	//	シャドウマップ準備
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_buffer{};
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		texture2d_desc.Width = 2048;
		texture2d_desc.Height = 2048;
		texture2d_desc.MipLevels = 1;
		texture2d_desc.ArraySize = 6;
		texture2d_desc.Format = DXGI_FORMAT_R32_TYPELESS;
		texture2d_desc.SampleDesc.Count = 1;
		texture2d_desc.SampleDesc.Quality = 0;
		texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
		texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texture2d_desc.CPUAccessFlags = 0;
		texture2d_desc.MiscFlags = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
		shader_resource_view_desc.Format = DXGI_FORMAT_R32_FLOAT;
		shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		shader_resource_view_desc.Texture2DArray.MostDetailedMip = 0;
		shader_resource_view_desc.Texture2DArray.MipLevels = 1;
		shader_resource_view_desc.Texture2DArray.FirstArraySlice = 0;
		shader_resource_view_desc.Texture2DArray.ArraySize = texture2d_desc.ArraySize;

		if (!texture_manager::instance()->generate_texture2d(render_targets->shadowmap_depth_texture, ShadowmapName, texture2d_desc, &shader_resource_view_desc))
		{
			return false;
		}
	}

	//	ワークバッファ準備
	{
		D3D11_TEXTURE2D_DESC texture2d_desc{};
		texture2d_desc.Width = scene_width;
		texture2d_desc.Height = scene_height;
		texture2d_desc.MipLevels = 1;
		texture2d_desc.ArraySize = 1;
		texture2d_desc.SampleDesc.Count = 1;
		texture2d_desc.SampleDesc.Quality = 0;
		texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
		texture2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		texture2d_desc.CPUAccessFlags = 0;
		texture2d_desc.MiscFlags = 0;

		static constexpr std::string_view WorkColorName[2] =
		{
			"Work Scene Color1",
			"Work Scene Color2",
		};
		texture2d_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		for (size_t index = 0; index < 2; ++index)
		{
			if (!texture_manager::instance()->generate_texture2d(render_targets->work_buffer_scene_textures[index], WorkColorName[index], texture2d_desc))
			{
				return false;
			}
		}

		static constexpr std::string_view WorkRGBA32FloatName = "Work RGBA32 Float";
		texture2d_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (!texture_manager::instance()->generate_texture2d(render_targets->work_buffer_rgba32float_texture, WorkRGBA32FloatName, texture2d_desc))
		{
			return false;
		}
	}

#if defined(USE_IMGUI)
	//	ImGui
	{
		imgui_rendering_pass.render_targets.emplace_back(render_data::instance()->get_swap_chain());
		imgui_rendering_pass.render_targets.back().clear_color = false;

		DXGI_SWAP_CHAIN_DESC desc;
		render_data::instance()->get_swap_chain()->GetDesc(&desc);

		// ビューポートの設定
		imgui_viewport.TopLeftX = 0.0f;
		imgui_viewport.TopLeftY = 0.0f;
		imgui_viewport.Width = static_cast<float>(desc.BufferDesc.Width);
		imgui_viewport.Height = static_cast<float>(desc.BufferDesc.Height);
		imgui_viewport.MinDepth = 0.0f;
		imgui_viewport.MaxDepth = 1.0f;
	}
#endif // defined(USE_IMGUI)

	return true;
}

bool framework::create_collision_filters()
{
	//	プレイヤー用
	{
		std::shared_ptr<collision_filter> filter = std::make_shared<collision_filter>();
		filter->set_name("player");
		filter->set_category(collision_category::player);
		filter->set_flag(collision_category::player, false);
		filter->set_flag(collision_category::player_projectile, false);
		filter->set_flag(collision_category::enemy, true);
		filter->set_flag(collision_category::enemy_projectile, true);
		filter->set_flag(collision_category::stage, true);
		filter->set_flag(collision_category::camera, false);
		collision_filter_manager::instance()->register_filter(filter);
	}
	//	プレイヤー弾用
	{
		std::shared_ptr<collision_filter> filter = std::make_shared<collision_filter>();
		filter->set_name("player_projectile");
		filter->set_category(collision_category::player_projectile);
		filter->set_flag(collision_category::player, false);
		filter->set_flag(collision_category::player_projectile, false);
		filter->set_flag(collision_category::enemy, true);
		filter->set_flag(collision_category::enemy_projectile, false);
		filter->set_flag(collision_category::stage, true);
		filter->set_flag(collision_category::camera, false);
		collision_filter_manager::instance()->register_filter(filter);
	}
	//	エネミー用
	{
		std::shared_ptr<collision_filter> filter = std::make_shared<collision_filter>();
		filter->set_name("enemy");
		filter->set_category(collision_category::enemy);
		filter->set_flag(collision_category::player, true);
		filter->set_flag(collision_category::player_projectile, true);
		filter->set_flag(collision_category::enemy, false);
		filter->set_flag(collision_category::enemy_projectile, false);
		filter->set_flag(collision_category::stage, true);
		filter->set_flag(collision_category::camera, false);
		collision_filter_manager::instance()->register_filter(filter);
	}
	//	エネミー弾用
	{
		std::shared_ptr<collision_filter> filter = std::make_shared<collision_filter>();
		filter->set_name("enemy_projectile");
		filter->set_category(collision_category::enemy_projectile);
		filter->set_flag(collision_category::player, true);
		filter->set_flag(collision_category::player_projectile, false);
		filter->set_flag(collision_category::enemy, false);
		filter->set_flag(collision_category::enemy_projectile, false);
		filter->set_flag(collision_category::stage, true);
		filter->set_flag(collision_category::camera, false);
		collision_filter_manager::instance()->register_filter(filter);
	}
	//	ステージ用
	{
		std::shared_ptr<collision_filter> filter = std::make_shared<collision_filter>();
		filter->set_name("stage");
		filter->set_category(collision_category::stage);
		filter->set_flag(collision_category::player, true);
		filter->set_flag(collision_category::player_projectile, true);
		filter->set_flag(collision_category::enemy, true);
		filter->set_flag(collision_category::enemy_projectile, true);
		filter->set_flag(collision_category::stage, true);
		filter->set_flag(collision_category::camera, true);
		collision_filter_manager::instance()->register_filter(filter);
	}
	//	カメラ用
	{
		std::shared_ptr<collision_filter> filter = std::make_shared<collision_filter>();
		filter->set_name("camera");
		filter->set_category(collision_category::camera);
		filter->set_flag(collision_category::player, false);
		filter->set_flag(collision_category::player_projectile, false);
		filter->set_flag(collision_category::enemy, false);
		filter->set_flag(collision_category::enemy_projectile, false);
		filter->set_flag(collision_category::stage, true);
		filter->set_flag(collision_category::camera, false);
		collision_filter_manager::instance()->register_filter(filter);
	}
	//	非表示中用
	{
		std::shared_ptr<collision_filter> filter = std::make_shared<collision_filter>();
		filter->set_name("invisible");
		filter->set_category(collision_category::invisible);
		filter->set_flag(collision_category::player, false);
		filter->set_flag(collision_category::player_projectile, false);
		filter->set_flag(collision_category::enemy, false);
		filter->set_flag(collision_category::enemy_projectile, false);
		filter->set_flag(collision_category::stage, true);
		filter->set_flag(collision_category::camera, false);
		collision_filter_manager::instance()->register_filter(filter);
	}

	return true;
}
