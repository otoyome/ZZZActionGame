#include "Renderer/scene_renderer.h"

#include "model_instance.h"

#include "PipelineState/deferred_lighting_pipeline_state.h"
#include "PipelineState/fullscreen_quad_pipeline_state.h"
#include "PipelineState/debug_draw_line_pipeline_state.h"

#include "PipelineState/post_process_pipeline_state.h"

#include "PipelineState/toon_rendering_pipeline_states.h"

namespace
{
	float convert_linear_depth_to_projection_depth(float depth, float z_buffer_parameteres_y)
	{
		float z_buffer_parameteres_x = 1.0f - z_buffer_parameteres_y;
		return (1.0f / depth - z_buffer_parameteres_y) / z_buffer_parameteres_x;
	};
}

void model_renderer::model_render(graphics_context& context, const std::vector<std::size_t>& mesh_indices, material::blend_modes blend_mode, material::surface_paths surface_path)
{
	//	モデル類描画
	pipeline_state_ptr current_pipeline_state;
	for (auto& draw_index : mesh_indices)
	{
		auto& draw_data = context.mesh_container.draw_datas[draw_index];
		auto& material_data = context.mesh_container.material_datas[draw_data.material_index];

		//	ドメイン&ブレンドモードチェック
		if(material_data.material->get_domain() != material::domains::surface || material_data.material->get_blend_mode() != blend_mode)
			continue;

		if (auto pipeline_state = material_data.material->get_pipeline_state(surface_path))
		{
			if (pipeline_state != current_pipeline_state)
			{
				if (current_pipeline_state)
					current_pipeline_state->end(context);

				current_pipeline_state = pipeline_state;
				current_pipeline_state->begin(context);
			}
			current_pipeline_state->set_parameters(context, *material_data.material);

			auto& buffer_data = context.mesh_container.buffer_datas[draw_data.buffer_index];
			current_pipeline_state->draw_indexed(context, buffer_data);
		}
	}
	if (current_pipeline_state)
		current_pipeline_state->end(context);
}

void model_renderer::model_render(graphics_context& context, const std::vector<std::size_t>& mesh_indices, const collision::frustum_culling& frustum_culling, material::blend_modes blend_mode, material::surface_paths surface_path)
{
	//	モデル類描画
	pipeline_state_ptr current_pipeline_state;
	for (auto& draw_index : mesh_indices)
	{
		auto& draw_data = context.mesh_container.draw_datas[draw_index];
		auto& buffer_data = context.mesh_container.buffer_datas[draw_data.buffer_index];
		auto& material_data = context.mesh_container.material_datas[draw_data.material_index];

		//	ドメイン&ブレンドモードチェック
		if (material_data.material->get_domain() != material::domains::surface || material_data.material->get_blend_mode() != blend_mode)
			continue;

		//	AABB判定
		if (!frustum_culling.to_aabb(buffer_data.bounds_min, buffer_data.bounds_max))
			continue;

		if (auto pipeline_state = material_data.material->get_pipeline_state(surface_path))
		{
			if (pipeline_state != current_pipeline_state)
			{
				if (current_pipeline_state)
					current_pipeline_state->end(context);

				current_pipeline_state = pipeline_state;
				current_pipeline_state->begin(context);
			}
			current_pipeline_state->set_parameters(context, *material_data.material);

			current_pipeline_state->draw_indexed(context, buffer_data);
		}
	}
	if (current_pipeline_state)
		current_pipeline_state->end(context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
depth_prepass_renderer::depth_prepass_renderer(std::shared_ptr<scene_render_targets> render_targets)
: model_renderer()
, render_targets(render_targets)
, viewport({0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
{
	//	バッファ生成
	texture2d_ptr texture = std::static_pointer_cast<texture2d>(render_targets->scene_color_texture);
	if (texture)
	{
		resize_buffer(texture->get_desc().Width, texture->get_desc().Height);
	}
}

depth_prepass_renderer::~depth_prepass_renderer()
{
}

void depth_prepass_renderer::render(graphics_context& context)
{
	//	描画先設定
	rendering_pass.activate(context.device_context);

	//	ビューポートの設定
	context.device_context->RSSetViewports(1, &viewport);

	//	モデル類描画
	model_render(context, context.mesh_container.color_datas, material::blend_modes::opaque, material::surface_paths::depth_write_pass);
	model_render(context, context.mesh_container.color_datas, material::blend_modes::mask, material::surface_paths::depth_write_pass);

	rendering_pass.deactivate(context.device_context);
}

void depth_prepass_renderer::resize_buffer(int width, int height)
{
	if (viewport.Width == static_cast<float>(width) && viewport.Height == static_cast<float>(height))
		return;

	//	描画パス生成
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;
	rendering_pass.depth_stencil = depth_stencil_view(render_targets->scene_depth_stencil_texture, &depth_stencil_view_desc);

	// ビューポートの設定
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
gbuffer_renderer::gbuffer_renderer(std::shared_ptr<scene_render_targets> render_targets)
: model_renderer()
, render_targets(render_targets)
, viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
{
	//	バッファ生成
	texture2d_ptr texture = std::static_pointer_cast<texture2d>(render_targets->scene_color_texture);
	if (texture)
	{
		resize_buffer(texture->get_desc().Width, texture->get_desc().Height);
	}

	//	シェーダー準備
//	pipeline_state_manager::instance()->register_pipeline_state<skybox_pipeline_state>("skybox", &skybox_pipeline_states);
	pipeline_state_manager::instance()->register_pipeline_state<toon_skybox_pipeline_state>("skybox", &skybox_pipeline_states);
}

gbuffer_renderer::~gbuffer_renderer()
{
}

void gbuffer_renderer::render(graphics_context& context)
{
	//	描画先設定
	rendering_pass.activate(context.device_context);

	//	ビューポートの設定
	context.device_context->RSSetViewports(1, &viewport);

	//	モデル類描画
	model_render(context, context.mesh_container.color_datas, material::blend_modes::opaque, material::surface_paths::g_buffer_pass);
	model_render(context, context.mesh_container.color_datas, material::blend_modes::mask, material::surface_paths::g_buffer_pass);

	//	スカイライト処理
	for (const auto& data : context.light_container.light_probes)
	{
		const light_probe* light_probe = data.get();
		if (!light_probe->is_flag(light_probe::flags::active) || !light_probe->is_flag(light_probe::flags::unbind))
		{
			continue;
		}

		//	IBLテクスチャ設定
		context.diffuse_ibl_texture = light_probe->get_diffuse_ibl_texture();
		context.specluar_ibl_texture = light_probe->get_specular_ibl_texture();
		context.lut_ggx_texture = light_probe->get_lut_ggx_texture();

		//	間接光の情報を設定
		light_probes probe;
		probe.position.x = light_probe->get_position().x;
		probe.position.y = light_probe->get_position().y;
		probe.position.z = light_probe->get_position().z;
		probe.inner_area.x = light_probe->get_inner_range().x;
		probe.inner_area.y = light_probe->get_inner_range().y;
		probe.inner_area.z = light_probe->get_inner_range().z;
		probe.outer_area.x = light_probe->get_outer_range().x;
		probe.outer_area.y = light_probe->get_outer_range().y;
		probe.outer_area.z = light_probe->get_outer_range().z;
		probe.color = light_probe->get_color();
		context.light_constant = {};
		context.light_constant.light_data = light_data_utility::convert_light_data(probe);
		context.light_constant.light_attribute = light_probe->get_attribute();

		//	全画面描画
		skybox_pipeline_states->begin(context);
		skybox_pipeline_states->draw(context);
		skybox_pipeline_states->end(context);
		break;
	}

	rendering_pass.deactivate(context.device_context);
}

void gbuffer_renderer::resize_buffer(int width, int height)
{
	if (viewport.Width == static_cast<float>(width) && viewport.Height == static_cast<float>(height))
		return;

	//	GBufferパス生成
	{
		FLOAT clear_color[][4] =
		{
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },

			{ 0.0f, 0.0f, 0.0f, 0.0f },
		};
		for (int i = 0; i < static_cast<int>(GBufferId::Max); ++i)
		{
			rendering_pass.render_targets.emplace_back(render_targets->g_buffer_textures[i]);
			auto& rtv = rendering_pass.render_targets.back();
			rtv.clear_color_value[0] = clear_color[i][0];
			rtv.clear_color_value[1] = clear_color[i][1];
			rtv.clear_color_value[2] = clear_color[i][2];
			rtv.clear_color_value[3] = clear_color[i][3];
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
		depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Texture2D.MipSlice = 0;
		rendering_pass.depth_stencil = depth_stencil_view(render_targets->scene_depth_stencil_texture, &depth_stencil_view_desc);
		rendering_pass.depth_stencil.clear_depth = false;
	}

	// ビューポートの設定
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
gbuffer_decal_renderer::gbuffer_decal_renderer(std::shared_ptr<scene_render_targets> render_targets)
: renderer()
, render_targets(render_targets)
, viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
{
	//	バッファ生成
	texture2d_ptr texture = std::static_pointer_cast<texture2d>(render_targets->scene_color_texture);
	if (texture)
	{
		resize_buffer(texture->get_desc().Width, texture->get_desc().Height);
	}

	//	書き込み用デフォルトプリミティブ
	cube = std::make_unique<geometric_cube>(render_data::instance()->get_device().Get());
}

gbuffer_decal_renderer::~gbuffer_decal_renderer()
{
}

void gbuffer_decal_renderer::render(graphics_context& context)
{
	//	法線/深度コピー
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> destination, source;
		render_targets->work_buffer_rgba32float_texture->get_shader_resource_view()->GetResource(destination.ReleaseAndGetAddressOf());
		render_targets->g_buffer_textures[static_cast<UINT>(GBufferId::Normal_Depth)]->get_shader_resource_view()->GetResource(source.ReleaseAndGetAddressOf());
		context.device_context->CopyResource(destination.Get(), source.Get());
	}

	//	デカール
	{
		//	描画先設定
		rendering_pass.activate(context.device_context);

		//	ビューポートの設定
		context.device_context->RSSetViewports(1, &viewport);

		//	カメラ球体
		collision::bounding_sphere sphere;
		{
			DirectX::XMFLOAT3 center;
			center.x = context.scene_constant.camera_position.x;
			center.y = context.scene_constant.camera_position.y;
			center.z = context.scene_constant.camera_position.z;
			sphere = collision::bounding_sphere(center, context.scene_constant.camera_clip_distance.x * 1.01f);
		}

		//	必要なテクスチャを設定
		{
			context.g_buffer_textures[static_cast<UINT>(GBufferId::Normal_Depth)] = render_targets->work_buffer_rgba32float_texture;
			context.g_buffer_textures[static_cast<UINT>(GBufferId::UniqueData)] = render_targets->g_buffer_textures[static_cast<UINT>(GBufferId::UniqueData)];
		}

		//	デカール描画
		for (auto& decal : context.decal_container.decal_datas)
		{
			//	バッファ情報生成
			buffer_datas data;
			{
				data.vertex_buffer_views = &cube->get_vertex_buffer_views();
				data.index_buffer_view = &cube->get_index_buffer_view();
				DirectX::XMMATRIX W = DirectX::XMMatrixScaling(decal.scale.x, decal.scale.y, decal.scale.z)
					* DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&decal.rotation.convert_quaternion()))
					* DirectX::XMMatrixTranslation(decal.position.x, decal.position.y, decal.position.z);
				data.transform_matrices.emplace_back();
				DirectX::XMStoreFloat4x4(&data.transform_matrices.back(), W);
			}

			//	ステンシル書き込み
			if (auto pipeline_state = decal.material.material->get_pipeline_state(material::surface_paths::depth_write_pass))
			{
				pipeline_state->begin(context);
				pipeline_state->set_parameters(context, *decal.material.material);

				pipeline_state->draw_indexed(context, data);

				pipeline_state->end(context);
			}

			//	デカール書き込み
			collision::bounding_box box = collision::bounding_box(decal.scale, decal.rotation, decal.position);
			if (sphere.intersection(&box))
			{
				if (auto pipeline_state = decal.material.material->get_pipeline_state(material::surface_paths::shadow_write_pass))
				{
					pipeline_state->begin(context);
					pipeline_state->set_parameters(context, *decal.material.material);

					pipeline_state->draw(context);

					pipeline_state->end(context);
				}
			}
			else
			{
				if (auto pipeline_state = decal.material.material->get_pipeline_state(material::surface_paths::g_buffer_pass))
				{
					pipeline_state->begin(context);
					pipeline_state->set_parameters(context, *decal.material.material);

					pipeline_state->draw_indexed(context, data);

					pipeline_state->end(context);
				}
			}
		}

		rendering_pass.deactivate(context.device_context);
	}

	context.g_buffer_textures[static_cast<UINT>(GBufferId::Normal_Depth)] = nullptr;
}

void gbuffer_decal_renderer::resize_buffer(int width, int height)
{
	if (viewport.Width == static_cast<float>(width) && viewport.Height == static_cast<float>(height))
		return;

	//	GBufferパス生成
	{
#if 01
		{
			rendering_pass.render_targets.emplace_back(render_targets->g_buffer_textures[static_cast<int>(GBufferId::BaseColor)]);
			auto& rtv = rendering_pass.render_targets.back();
			rtv.clear_color = false;
		}
		{
			rendering_pass.render_targets.emplace_back(render_targets->g_buffer_textures[static_cast<int>(GBufferId::Normal_Depth)]);
			auto& rtv = rendering_pass.render_targets.back();
			rtv.clear_color = false;
		}
#else
		FLOAT clear_color[][4] =
		{
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },

			{ 0.0f, 0.0f, 0.0f, 0.0f },
		};
		for (int i = 0; i < static_cast<int>(GBufferId::Max); ++i)
		{
			rendering_pass.render_targets.emplace_back(render_targets->g_buffer_textures[i]);
			auto& rtv = rendering_pass.render_targets.back();
			rtv.clear_color = false;
		}
#endif
		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
		depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Texture2D.MipSlice = 0;
		rendering_pass.depth_stencil = depth_stencil_view(render_targets->scene_depth_stencil_texture, &depth_stencil_view_desc);
		rendering_pass.depth_stencil.clear_depth = false;
		rendering_pass.depth_stencil.clear_stencil = true;
	}

	// ビューポートの設定
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
deffered_lighting_renderer::deffered_lighting_renderer(std::shared_ptr<scene_render_targets> render_targets)
: model_renderer()
, render_targets(render_targets)
, use_lighting_pass(nullptr)
, lighting_pass_viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
, shadow_map_pass_viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
{
	//	バッファ生成
	texture2d_ptr texture;
	if (texture = std::static_pointer_cast<texture2d>(render_targets->scene_color_texture))
	{
		resize_buffer(texture->get_desc().Width, texture->get_desc().Height);
	}
	//	シャドウマップ生成
	if (texture = std::static_pointer_cast<texture2d>(render_targets->shadowmap_depth_texture))
	{
		resize_shadow_buffer(texture->get_desc().Width, texture->get_desc().Height);
	}

	//	パイプラインステート生成
	{
		//	ディファードレンダリング系シェーダー準備
		CONST char* shader_names[] =
		{
			"deferred_indirect_lighting",
			"deferred_emissive_lighting",
			"deferred_directional_lighting",
			"deferred_point_lighting",
			"deferred_spot_lighting",
			"deferred_shadow_light",
		};
		pipeline_state_manager::instance()->register_pipeline_state<deferred_indirect_lighting_pipeline_state>(shader_names[0], &indirect_lighting_pipeline_state);
		pipeline_state_manager::instance()->register_pipeline_state<deferred_emissive_lighting_pipeline_state>(shader_names[1], &emissive_lighting_pipeline_state);
		pipeline_state_manager::instance()->register_pipeline_state<deferred_directional_lighting_pipeline_state>(shader_names[2], &directional_lighting_pipeline_state);
		pipeline_state_manager::instance()->register_pipeline_state<deferred_point_lighting_pipeline_state>(shader_names[3], &point_lighting_pipeline_state);
		pipeline_state_manager::instance()->register_pipeline_state<deferred_spot_lighting_pipeline_state>(shader_names[4], &spot_lighting_pipeline_state);
		pipeline_state_manager::instance()->register_pipeline_state<deferred_shadow_light_pipeline_state>(shader_names[5], &shadow_light_pipeline_state);
	}
}

deffered_lighting_renderer::~deffered_lighting_renderer()
{
}

void deffered_lighting_renderer::render(graphics_context& context)
{
	//	初回設定時はクリア処理を行う
	lighting_pass_cleared = false;

	//	G-Bufferテクスチャ設定
	{
		for (UINT index = 0; index < static_cast<UINT>(GBufferId::Max); ++index)
		{
			context.g_buffer_textures[index]	= render_targets->g_buffer_textures[index];
		}
	}

	//	間接光&自己発光
	render_indirect_and_emissive(context);

	//	ライティング処理
	for (const auto& data : context.light_container.lights)
	{
		const light* light = data.get();
		if (!light->is_flag(light::flags::active))
			continue;

		switch (light->get_light_type())
		{
			case light_type::directional:
			{
				//	シャドウマップ生成
				if (light->is_flag(light::flags::use_shadow))
				{
					render_directional_shadow_map(*light, context);
				}

				//	ライティング処理
				render_directional(*light, context);
				break;
			}
			case light_type::point:
			{
				//	シャドウマップ生成
				if (light->is_flag(light::flags::use_shadow))
				{
					render_point_shadow_map(*light, context);
				}

				//	ライティング処理
				render_point(*light, context);
				break;
			}
			case light_type::spot:
			{
				//	シャドウマップ生成
				if (light->is_flag(light::flags::use_shadow))
				{
					render_spot_shadow_map(*light, context);
				}

				//	ライティング処理
				render_spot(*light, context);
				break;
			}
		}
	}

	//	シャドウライトパス
	for (const auto& data : context.light_container.shadow_lights)
	{
		const shadow_light* light = data.get();
		if (!light->is_flag(shadow_light::flags::active))
			continue;

		//	シャドウマップ生成
		render_shadow_light_shadow_map(*light, context);

		//	ライティング処理
		render_shadow_light(*light, context);
	}

	//	SSR
	{
	}
}

void deffered_lighting_renderer::render_indirect_and_emissive(graphics_context& context)
{
	//	AOバッファ生成
	{
	}

	//	描画先設定
	activate_lighting_pass(context);

	//	自己発光
	{
		emissive_lighting_pipeline_state->begin(context);
		emissive_lighting_pipeline_state->draw(context);
		emissive_lighting_pipeline_state->end(context);
	}

	//	間接光
	for (const auto& data : context.light_container.light_probes)
	{
		const light_probe* light_probe = data.get();
		if (!light_probe->is_flag(light_probe::flags::active))
			continue;

		//	IBLテクスチャ設定
		context.diffuse_ibl_texture = light_probe->get_diffuse_ibl_texture();
		context.specluar_ibl_texture = light_probe->get_specular_ibl_texture();
		context.lut_ggx_texture = light_probe->get_lut_ggx_texture();

		//	間接光の情報を設定
		light_probes probe;
		probe.position.x = light_probe->get_position().x;
		probe.position.y = light_probe->get_position().y;
		probe.position.z = light_probe->get_position().z;
		probe.inner_area.x = light_probe->get_inner_range().x;
		probe.inner_area.y = light_probe->get_inner_range().y;
		probe.inner_area.z = light_probe->get_inner_range().z;
		probe.outer_area.x = light_probe->get_outer_range().x;
		probe.outer_area.y = light_probe->get_outer_range().y;
		probe.outer_area.z = light_probe->get_outer_range().z;
		probe.color = light_probe->get_color();
		context.light_constant = {};
		context.light_constant.light_data = light_data_utility::convert_light_data(probe);
		context.light_constant.light_attribute = light_probe->get_attribute();

		if (light_probe->is_flag(light_probe::flags::unbind))
		{
			//	全画面描画
			indirect_lighting_pipeline_state->begin(context);
			indirect_lighting_pipeline_state->draw(context);
			indirect_lighting_pipeline_state->end(context);
		}
		else
		{
			//	範囲制限をかける
			indirect_lighting_pipeline_state;
		}
	}

	deactivate_lighting_pass(context);
}

void deffered_lighting_renderer::render_directional(const light& light, graphics_context& context)
{
	//	描画先設定
	activate_lighting_pass(context);
	{
		context.light_constant.light_data = light.convert_light_data();
		context.light_constant.light_attribute = light.get_attribute();
		context.shadow_map_texture = render_targets->shadowmap_depth_texture;
		context.shadow_map_constant = shadow_map_constant;

		directional_lighting_pipeline_state->begin(context);
		directional_lighting_pipeline_state->draw(context);
		directional_lighting_pipeline_state->end(context);
	}
	deactivate_lighting_pass(context);
}

void deffered_lighting_renderer::render_directional_shadow_map(const light& light, graphics_context& context)
{
	shadow_map_pass.activate(context.device_context);

	DirectX::XMFLOAT4 rotation = light.get_rotation().convert_quaternion();
	DirectX::XMFLOAT3 position = light.get_position();
	DirectX::XMMATRIX CameraTransformMatrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&context.scene_constant.view_transform));
	DirectX::XMMATRIX LightTransformMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation)) * 
												DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMMATRIX InverseProjectionTransform;
	InverseProjectionTransform = DirectX::XMLoadFloat4x4(&context.scene_constant.inverse_projection_transform);

	DirectX::XMVECTOR LightRight, LightUp, LightFront, LightPosition;
	LightRight = LightTransformMatrix.r[0];
	LightUp = LightTransformMatrix.r[1];
	LightFront = LightTransformMatrix.r[2];
	LightPosition = LightTransformMatrix.r[3];

	//	クリップ距離を算出
	float near_clip, far_clip;
	{
		DirectX::XMVECTOR NearPosition, FarPosition;
		NearPosition = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), InverseProjectionTransform);
		FarPosition = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), InverseProjectionTransform);
		near_clip = DirectX::XMVectorGetZ(NearPosition);
		far_clip = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(FarPosition, NearPosition)));
	}

	//	CSM用に遠クリップ面に対して距離制限をかける
	float projection_near_clip, projection_far_clip;
	{
		projection_near_clip = 0.0f;
		projection_far_clip = convert_linear_depth_to_projection_depth(light.get_csm_distance_factor(), far_clip / near_clip);

		//	クリップ距離を再算出
		DirectX::XMVECTOR NearPosition, FarPosition;
		NearPosition = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0.0f, 0.0f, projection_near_clip, 0.0f), InverseProjectionTransform);
		FarPosition = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0.0f, 0.0f, projection_far_clip, 0.0f), InverseProjectionTransform);
		far_clip = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(FarPosition, NearPosition)));
	}

	//	シャドウマップ情報算出
	shadow_map_constant.shadow_map_count = directional_shadow_map_count;
	shadow_map_constant.shadow_attenuation = light.get_shadow_attenuation();
	shadow_map_constant.shadow_mask = 0x00000001;
	{
		DirectX::XMMATRIX InverseViewProjectionTransform;
		InverseViewProjectionTransform = DirectX::XMLoadFloat4x4(&context.scene_constant.inverse_view_projection_transform);
		float top_min_distance, top_max_distance;
		//	視錐台を囲むライト空間上の行列を生成
		DirectX::XMMATRIX LightViewMatrix, LightProjectionMatrix;
		{
			//	AABBを囲む球体を算出
			DirectX::XMVECTOR Center;
			float radius;
			{
				DirectX::XMVECTOR FrustumPoints[] =
				{
					DirectX::XMVectorSet(-1.0f, -1.0f, projection_near_clip, 1.0f),
					DirectX::XMVectorSet(+1.0f, -1.0f, projection_near_clip, 1.0f),
					DirectX::XMVectorSet(+1.0f, +1.0f, projection_near_clip, 1.0f),
					DirectX::XMVectorSet(-1.0f, +1.0f, projection_near_clip, 1.0f),

					DirectX::XMVectorSet(-1.0f, -1.0f, projection_far_clip, 1.0f),
					DirectX::XMVectorSet(+1.0f, -1.0f, projection_far_clip, 1.0f),
					DirectX::XMVectorSet(+1.0f, +1.0f, projection_far_clip, 1.0f),
					DirectX::XMVectorSet(-1.0f, +1.0f, projection_far_clip, 1.0f),
				};

				for (auto& Position : FrustumPoints)
				{
					Position = DirectX::XMVector3TransformCoord(Position, InverseViewProjectionTransform);
				}
				DirectX::XMVECTOR BoundsMin = DirectX::XMVectorSet(+FLT_MAX, +FLT_MAX, +FLT_MAX, 0),
					BoundsMax = DirectX::XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 0);
				for (auto& Position : FrustumPoints)
				{
					BoundsMin = DirectX::XMVectorMin(BoundsMin, Position);
					BoundsMax = DirectX::XMVectorMax(BoundsMax, Position);
				}
				DirectX::XMVECTOR Vector = DirectX::XMVectorScale(DirectX::XMVectorSubtract(BoundsMax, BoundsMin), 0.5f);
				Center = DirectX::XMVectorAdd(BoundsMin, Vector);
				radius = DirectX::XMVectorGetX(DirectX::XMVector3Length(Vector));
			}

			//	視線行列算出
			{
				LightPosition = DirectX::XMVectorSubtract(Center, DirectX::XMVectorScale(LightFront, radius));
				LightTransformMatrix.r[0] = LightRight;
				LightTransformMatrix.r[1] = LightUp;
				LightTransformMatrix.r[2] = LightFront;
				LightTransformMatrix.r[3] = LightPosition;
				LightTransformMatrix.r[0] = DirectX::XMVectorSetW(LightTransformMatrix.r[0], 0.0f);
				LightTransformMatrix.r[1] = DirectX::XMVectorSetW(LightTransformMatrix.r[1], 0.0f);
				LightTransformMatrix.r[2] = DirectX::XMVectorSetW(LightTransformMatrix.r[2], 0.0f);
				LightTransformMatrix.r[3] = DirectX::XMVectorSetW(LightTransformMatrix.r[3], 1.0f);
				LightViewMatrix = DirectX::XMMatrixInverse(nullptr, LightTransformMatrix);
			}

			//	投影行列生成
			{
				radius *= 1.01f;
				top_min_distance = 0.0f;
				top_max_distance = top_min_distance + radius * 2.0f;
				LightProjectionMatrix = DirectX::XMMatrixOrthographicOffCenterLH(-radius, +radius, -radius, +radius, top_min_distance, top_max_distance);
			}
		}

		//	PSSM(面白分割距離算出)
		std::vector<float> split_distance;
		split_distance.resize(directional_shadow_map_count + 1);
		{
			float split_weight = light.get_pssm_split_weight();
			float weight = math::lerp(0.1f, 0.9f, split_weight);
			float divN = far_clip / near_clip, subN = far_clip - near_clip;

			//	距離算出
			UINT index = 0;
			split_distance[index]	= near_clip;
			for (++index; index < directional_shadow_map_count; ++index)
			{
				float spliit = static_cast<float>(index) / static_cast<float>(directional_shadow_map_count);
				float fLog = near_clip * pow(divN, spliit);
				float fUni = near_clip + subN * spliit;
				split_distance[index]	= fLog * weight + fUni * ( 1.0f - weight);
			}
			split_distance[index]	= far_clip;
		}

		//	視錐台を距離で分割してライトビュープロジェクション行列を生成
		for (UINT index = 0; index < directional_shadow_map_count; ++index)
		{
			//	AABBを囲む球体を算出
			DirectX::XMVECTOR Center;
			float radius;
			{
				//	分割距離単位での視錐台生成
				float n_clip, f_clip;
				n_clip = convert_linear_depth_to_projection_depth(split_distance[index + 0] / far_clip, far_clip / near_clip);
				f_clip = convert_linear_depth_to_projection_depth(split_distance[index + 1] / far_clip, far_clip / near_clip);

				DirectX::XMVECTOR FrustumPoints[8] =
				{
					DirectX::XMVectorSet(-1.0f, -1.0f, n_clip, 1.0f),
					DirectX::XMVectorSet(+1.0f, -1.0f, n_clip, 1.0f),
					DirectX::XMVectorSet(+1.0f, +1.0f, n_clip, 1.0f),
					DirectX::XMVectorSet(-1.0f, +1.0f, n_clip, 1.0f),

					DirectX::XMVectorSet(-1.0f, -1.0f, f_clip, 1.0f),
					DirectX::XMVectorSet(+1.0f, -1.0f, f_clip, 1.0f),
					DirectX::XMVectorSet(+1.0f, +1.0f, f_clip, 1.0f),
					DirectX::XMVectorSet(-1.0f, +1.0f, f_clip, 1.0f),
				};

				for (auto& Position : FrustumPoints)
				{
					Position = DirectX::XMVector3TransformCoord(Position, InverseViewProjectionTransform);
				}

				DirectX::XMVECTOR	BoundsMin = DirectX::XMVectorSet(+FLT_MAX, +FLT_MAX, +FLT_MAX, 0),
									BoundsMax = DirectX::XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 0);
				for (auto& Position : FrustumPoints)
				{
					BoundsMin = DirectX::XMVectorMin(BoundsMin, Position);
					BoundsMax = DirectX::XMVectorMax(BoundsMax, Position);
				}
				DirectX::XMVECTOR Vector = DirectX::XMVectorScale(DirectX::XMVectorSubtract(BoundsMax, BoundsMin), 0.5f);
				Center = DirectX::XMVectorAdd(BoundsMin, Vector);
				radius = DirectX::XMVectorGetX(DirectX::XMVector3Length(Vector));
			}

			//	各行列調整
			if (index == directional_shadow_map_count - 1)
			{
				//	最終行列は全体表示にしておく
				DirectX::XMStoreFloat4x4(&shadow_map_constant.light_view_projection_transform[index], LightViewMatrix * LightProjectionMatrix);
			}
			else
			{
				float min_distance = top_min_distance;
				float max_distance = top_max_distance;

				float X = static_cast<float>(static_cast<int>(ceilf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(Center, LightRight)) * shadow_map_pass_viewport.Width / radius ) * radius / shadow_map_pass_viewport.Width ) );
				float Y = static_cast<float>(static_cast<int>(ceilf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(Center,    LightUp)) * shadow_map_pass_viewport.Height / radius ) * radius / shadow_map_pass_viewport.Height ) );
				float Z = DirectX::XMVectorGetX(DirectX::XMVector3Dot(Center, LightFront));
				Center = DirectX::XMVectorAdd(DirectX::XMVectorAdd(DirectX::XMVectorScale(LightRight, X), DirectX::XMVectorScale(LightUp, Y)), DirectX::XMVectorScale(LightFront, Z));

				DirectX::XMVECTOR Origin = DirectX::XMVectorSubtract(Center, DirectX::XMVectorScale(LightFront, (max_distance - min_distance) * 0.5f + radius));
				LightTransformMatrix.r[0] = LightRight;
				LightTransformMatrix.r[1] = LightUp;
				LightTransformMatrix.r[2] = LightFront;
				LightTransformMatrix.r[3] = Origin;
				LightTransformMatrix.r[0] = DirectX::XMVectorSetW(LightTransformMatrix.r[0], 0.0f);
				LightTransformMatrix.r[1] = DirectX::XMVectorSetW(LightTransformMatrix.r[1], 0.0f);
				LightTransformMatrix.r[2] = DirectX::XMVectorSetW(LightTransformMatrix.r[2], 0.0f);
				LightTransformMatrix.r[3] = DirectX::XMVectorSetW(LightTransformMatrix.r[3], 1.0f);
				DirectX::XMMATRIX LightViewMatrix = DirectX::XMMatrixInverse(nullptr, LightTransformMatrix);

				//	投影行列調整
				radius *= 1.1f;
				DirectX::XMMATRIX LightProjectionMatrix = DirectX::XMMatrixOrthographicOffCenterLH(-radius, +radius, -radius, +radius, min_distance, max_distance + radius);
				DirectX::XMStoreFloat4x4(&shadow_map_constant.light_view_projection_transform[index], LightViewMatrix * LightProjectionMatrix);
			}

			{
				float near_clip = split_distance[index + 0];
				float far_clip = split_distance[index + 1];
				shadow_map_constant.shadow_split_distance[index].x = near_clip;
				shadow_map_constant.shadow_split_distance[index].y = near_clip;
				shadow_map_constant.shadow_split_distance[index].z = far_clip;
				shadow_map_constant.shadow_split_distance[index].w = far_clip;

				//	バイアス納入
				auto& bias_data = light.get_shadow_bias(index);
				float depth_bias, slope_depth_bias;
				depth_bias = bias_data.max_depth_bias / (far_clip - near_clip);
				depth_bias *= (far_clip - near_clip) / directional_shadow_map_count;
				depth_bias *= 0.5f;
				depth_bias *= bias_data.depth_bias;
				slope_depth_bias	= bias_data.slope_depth_bias_scale * bias_data.slope_depth_bias;
				shadow_map_constant.shadow_bias[index].x = std::max(0.0f, depth_bias);
				shadow_map_constant.shadow_bias[index].y = std::max(0.0f, depth_bias * slope_depth_bias);
				shadow_map_constant.shadow_bias[index].z = bias_data.max_depth_bias;
				shadow_map_constant.shadow_bias[index].w = 1.0f / bias_data.sigma_factor; // 分散値
			}
		}
	}

	//	定数バッファ情報設定
	context.shadow_map_constant = shadow_map_constant;

	//	ビューポートの設定
	context.device_context->RSSetViewports(1, &shadow_map_pass_viewport);

	//	モデル類描画
	collision::frustum_culling frustum_culling;
	{
		auto view_projection = shadow_map_constant.light_view_projection_transform[directional_shadow_map_count - 1];
		frustum_culling.create_planes(view_projection);
		frustum_culling.create_frustum_corners(DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&view_projection)));
	}
	model_render(context, context.mesh_container.shadow_datas, frustum_culling, material::blend_modes::opaque, material::surface_paths::shadow_write_pass);
	model_render(context, context.mesh_container.shadow_datas, frustum_culling, material::blend_modes::mask, material::surface_paths::shadow_write_pass);

	shadow_map_pass.deactivate(context.device_context);
}

void deffered_lighting_renderer::render_point(const light& light, graphics_context& context)
{
	//	描画先設定
	activate_lighting_pass(context);
	{
		context.light_constant.light_data = light.convert_light_data();
		context.light_constant.light_attribute = light.get_attribute();
		context.shadow_map_texture = render_targets->shadowmap_depth_texture;
		context.shadow_map_constant = shadow_map_constant;

		point_lighting_pipeline_state->begin(context);
		point_lighting_pipeline_state->draw(context);
		point_lighting_pipeline_state->end(context);
	}
	deactivate_lighting_pass(context);
}

void deffered_lighting_renderer::render_point_shadow_map(const light& light, graphics_context& context)
{
}

void deffered_lighting_renderer::render_spot(const light& light, graphics_context& context)
{
	//	描画先設定
	activate_lighting_pass(context);
	{
		context.light_constant.light_data = light.convert_light_data();
		context.light_constant.light_attribute = light.get_attribute();
		context.shadow_map_texture = render_targets->shadowmap_depth_texture;
		context.shadow_map_constant = shadow_map_constant;

		spot_lighting_pipeline_state->begin(context);
		spot_lighting_pipeline_state->draw(context);
		spot_lighting_pipeline_state->end(context);
	}
	deactivate_lighting_pass(context);
}

void deffered_lighting_renderer::render_spot_shadow_map(const light& light, graphics_context& context)
{
	auto reserve_scene_constant = context.scene_constant;
	shadow_map_pass.activate(context.device_context);

	DirectX::XMFLOAT4 rotation = light.get_rotation().convert_quaternion();
	DirectX::XMFLOAT3 position = light.get_position();
	DirectX::XMMATRIX TransformMatrix, ViewMatrix, ProjectionMatrix;
	TransformMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation))
					* DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	ViewMatrix = DirectX::XMMatrixInverse(nullptr, TransformMatrix);
	ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(light.get_outer_cone() * 2.0f, 1.0f, 0.01f, light.get_range());

	//	定数バッファ情報設定
	shadow_map_constant.shadow_map_count = 1;
	shadow_map_constant.shadow_attenuation = light.get_shadow_attenuation();
	shadow_map_constant.shadow_split_distance[0].x = -1.0f;
	shadow_map_constant.shadow_split_distance[0].y = -1.0f;
	shadow_map_constant.shadow_split_distance[0].z = FLT_MAX;
	shadow_map_constant.shadow_split_distance[0].w = FLT_MAX;

	//	バイアス納入
	{
		//	ライトの範囲で処理
		float near_clip = 0.1f;
		float far_clip = near_clip + light.get_range();

		auto& bias_data = light.get_shadow_bias(0);
		float depth_bias, slope_depth_bias;
		depth_bias = bias_data.max_depth_bias / (far_clip - near_clip);
		depth_bias *= (far_clip - near_clip) / directional_shadow_map_count;
		depth_bias *= 0.5f;
		depth_bias *= bias_data.depth_bias;
		slope_depth_bias	= bias_data.slope_depth_bias_scale * bias_data.slope_depth_bias;
		shadow_map_constant.shadow_bias[0].x = std::max(0.0f, depth_bias);
		shadow_map_constant.shadow_bias[0].y = std::max(0.0f, depth_bias * slope_depth_bias);
		shadow_map_constant.shadow_bias[0].z = bias_data.max_depth_bias;
		shadow_map_constant.shadow_bias[0].w = bias_data.sigma_factor; // 分散値
	}

	shadow_map_constant.shadow_mask = 0x00000001;
	DirectX::XMStoreFloat4x4(&shadow_map_constant.light_view_projection_transform[0], ViewMatrix * ProjectionMatrix);
	context.shadow_map_constant = shadow_map_constant;

	//	ビューポートの設定
	context.device_context->RSSetViewports(1, &shadow_map_pass_viewport);

	//	モデル類描画
	collision::frustum_culling frustum_culling;
	{
		frustum_culling.create_planes(shadow_map_constant.light_view_projection_transform[0]);
		frustum_culling.create_frustum_corners(DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&context.scene_constant.view_projection_transform)));
	}
	model_render(context, context.mesh_container.shadow_datas, frustum_culling, material::blend_modes::opaque, material::surface_paths::shadow_write_pass);
	model_render(context, context.mesh_container.shadow_datas, frustum_culling, material::blend_modes::mask, material::surface_paths::shadow_write_pass);

	shadow_map_pass.deactivate(context.device_context);
	context.scene_constant = reserve_scene_constant;
}

void deffered_lighting_renderer::render_shadow_light(const shadow_light& light, graphics_context& context)
{
	//	描画先設定
	activate_lighting_pass(context);
	{
		spot_lights spot_light;
		spot_light.position.x = light.get_position().x;
		spot_light.position.y = light.get_position().y;
		spot_light.position.z = light.get_position().z;
		spot_light.position.w = 1.0f;

		spot_light.direction.x = light.get_rotation().get_front().x;
		spot_light.direction.y = light.get_rotation().get_front().y;
		spot_light.direction.z = light.get_rotation().get_front().z;
		spot_light.direction.w = 0.0f;

		spot_light.color = light.get_color();

		spot_light.range = light.get_range();
		spot_light.innerCorn = light.get_cone();
		spot_light.outerCorn = light.get_cone();

		context.light_constant.light_data = light_data_utility::convert_light_data(spot_light);
		context.light_constant.light_attribute = light.get_attribute();
		context.light_constant.contact_shadow = light.get_contact_shadow();
		context.shadow_map_texture = render_targets->shadowmap_depth_texture;
		context.shadow_map_constant = shadow_map_constant;

		shadow_light_pipeline_state->begin(context);
		shadow_light_pipeline_state->draw(context);
		shadow_light_pipeline_state->end(context);
	}
	deactivate_lighting_pass(context);
}

void deffered_lighting_renderer::render_shadow_light_shadow_map(const shadow_light& light, graphics_context& context)
{
	auto reserve_scene_constant = context.scene_constant;
	shadow_map_pass.activate(context.device_context);

	DirectX::XMFLOAT4 rotation = light.get_rotation().convert_quaternion();
	DirectX::XMFLOAT3 position = light.get_position();
	DirectX::XMMATRIX TransformMatrix, ViewMatrix, ProjectionMatrix;
	TransformMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation))
					* DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	ViewMatrix = DirectX::XMMatrixInverse(nullptr, TransformMatrix);
	ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(light.get_cone() * 2.0f, 1.0f, 0.01f, light.get_range());

	//	定数バッファ情報設定
	shadow_map_constant.shadow_map_count = 1;
	shadow_map_constant.shadow_attenuation.x = light.get_color().x;
	shadow_map_constant.shadow_attenuation.y = light.get_color().y;
	shadow_map_constant.shadow_attenuation.z = light.get_color().z;
	shadow_map_constant.shadow_bias[0].x =  light.get_shadow_bias();
	shadow_map_constant.shadow_split_distance[0].x = -1.0f;
	shadow_map_constant.shadow_split_distance[0].y = -1.0f;
	shadow_map_constant.shadow_split_distance[0].z = FLT_MAX;
	shadow_map_constant.shadow_split_distance[0].w = FLT_MAX;
	shadow_map_constant.shadow_mask = light.get_shadow_mask();
	DirectX::XMStoreFloat4x4(&shadow_map_constant.light_view_projection_transform[0], ViewMatrix * ProjectionMatrix);
	context.shadow_map_constant = shadow_map_constant;

	//	ビューポートの設定
	context.device_context->RSSetViewports(1, &shadow_map_pass_viewport);

	//	モデル類描画
	collision::frustum_culling frustum_culling;
	{
		frustum_culling.create_planes(shadow_map_constant.light_view_projection_transform[0]);
		frustum_culling.create_frustum_corners(DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&context.scene_constant.view_projection_transform)));
	}
	model_render(context, context.mesh_container.shadow_datas, frustum_culling, material::blend_modes::opaque, material::surface_paths::shadow_write_pass);
	model_render(context, context.mesh_container.shadow_datas, frustum_culling, material::blend_modes::mask, material::surface_paths::shadow_write_pass);

	shadow_map_pass.deactivate(context.device_context);
	context.scene_constant = reserve_scene_constant;
}

void deffered_lighting_renderer::resize_buffer(int width, int height)
{
	if (lighting_pass_viewport.Width == static_cast<float>(width) && lighting_pass_viewport.Height == static_cast<float>(height))
		return;

	//	ライティングパス生成
	lighting_pass_clear.render_targets.emplace_back(render_targets->scene_color_texture);
	lighting_pass_clear.render_targets.back().clear_color_value[0] = 0.0f;
	lighting_pass_clear.render_targets.back().clear_color_value[1] = 0.0f;
	lighting_pass_clear.render_targets.back().clear_color_value[2] = 0.0f;
	lighting_pass_clear.render_targets.back().clear_color_value[3] = 1.0f;

	lighting_pass_load.render_targets.emplace_back(render_targets->scene_color_texture);
	lighting_pass_load.render_targets.back().clear_color = false;
	lighting_pass_load.render_targets.back().clear_color_value[0] = 0.0f;
	lighting_pass_load.render_targets.back().clear_color_value[1] = 0.0f;
	lighting_pass_load.render_targets.back().clear_color_value[2] = 0.0f;
	lighting_pass_load.render_targets.back().clear_color_value[3] = 1.0f;

	// ビューポートの設定
	lighting_pass_viewport.TopLeftX = 0.0f;
	lighting_pass_viewport.TopLeftY = 0.0f;
	lighting_pass_viewport.Width = static_cast<float>(width);
	lighting_pass_viewport.Height = static_cast<float>(height);
	lighting_pass_viewport.MinDepth = 0.0f;
	lighting_pass_viewport.MaxDepth = 1.0f;
}

void deffered_lighting_renderer::resize_shadow_buffer(int width, int height)
{
	if (shadow_map_pass_viewport.Width == static_cast<float>(width) && shadow_map_pass_viewport.Height == static_cast<float>(height))
		return;

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	render_targets->shadowmap_depth_texture->get_shader_resource_view()->GetDesc(&srv_desc);
	//	シャドウマップパス生成
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depth_stencil_view_desc.Texture2DArray.ArraySize = srv_desc.Texture2DArray.ArraySize;
	depth_stencil_view_desc.Texture2DArray.FirstArraySlice = 0;
	depth_stencil_view_desc.Texture2DArray.MipSlice = 0;
	shadow_map_pass.depth_stencil = depth_stencil_view(render_targets->shadowmap_depth_texture, &depth_stencil_view_desc);

	// ビューポートの設定
	shadow_map_pass_viewport.TopLeftX = 0.0f;
	shadow_map_pass_viewport.TopLeftY = 0.0f;
	shadow_map_pass_viewport.Width = static_cast<float>(width);
	shadow_map_pass_viewport.Height = static_cast<float>(height);
	shadow_map_pass_viewport.MinDepth = 0.0f;
	shadow_map_pass_viewport.MaxDepth = 1.0f;
}

void deffered_lighting_renderer::activate_lighting_pass(graphics_context& context)
{
	//	ライティングパス設定
	if (lighting_pass_cleared)
	{
		use_lighting_pass = &lighting_pass_load;
	}
	else
	{
		lighting_pass_cleared = true;
		use_lighting_pass = &lighting_pass_clear;
	}
	use_lighting_pass->activate(context.device_context);

	//	ビューポートの設定
	context.device_context->RSSetViewports(1, &lighting_pass_viewport);
}

void deffered_lighting_renderer::deactivate_lighting_pass(graphics_context& context)
{
	assert(use_lighting_pass != nullptr);
	use_lighting_pass->deactivate(context.device_context);
	use_lighting_pass = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
postprocess_renderer::postprocess_renderer(std::shared_ptr<scene_render_targets> render_targets)
: renderer()
, render_targets(render_targets)
, work_buffer_rendering_pass()
, work_buffer_viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
, scene_color_rendering_pass()
, scene_color_viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
{
	resize_buffer(0, 0);

	//	パイプラインステート生成
	pipeline_state_manager::instance()->register_pipeline_state<write_back_pipeline_state>("write_back", &write_back);
	pipeline_state_manager::instance()->register_pipeline_state<color_filter_pre_effect_pipeline_state>("color_filter_pre_effect", &color_filter);
}

postprocess_renderer::~postprocess_renderer()
{
}

void postprocess_renderer::render(graphics_context& context)
{
	//	ポストプロセスボリュームが無ければ適応しない
	if (context.post_process_volume_container.post_process_volumes.empty())
		return;

	//	ポストプロセス情報設定
	//	とりあえず適当に
	context.post_process_data = context.post_process_volume_container.post_process_volumes[0]->get_post_process_data();

	bool buffer_index = 0;

	//	カラーフィルター
	{
		//	描画先設定
		work_buffer_rendering_pass[buffer_index].activate(context.device_context);

		//	ビューポートの設定
		context.device_context->RSSetViewports(1, &work_buffer_viewport);

		//	シーン描画
		context.scene_texture = render_targets->scene_color_texture;

		color_filter->begin(context);
		color_filter->draw(context);
		color_filter->end(context);

		work_buffer_rendering_pass[buffer_index].deactivate(context.device_context);
		buffer_index = !buffer_index;
	}

	//	書き戻し
	{
		//	描画先設定
		scene_color_rendering_pass.activate(context.device_context);

		//	ビューポートの設定
		context.device_context->RSSetViewports(1, &scene_color_viewport);

		//	シーン描画
		context.scene_texture = render_targets->work_buffer_scene_textures[!buffer_index];

		write_back->begin(context);
		write_back->draw(context);
		write_back->end(context);

		scene_color_rendering_pass.deactivate(context.device_context);
		buffer_index = !buffer_index;
	}
}

void postprocess_renderer::resize_buffer(int width, int height)
{
	DXGI_SWAP_CHAIN_DESC desc;
	render_data::instance()->get_swap_chain()->GetDesc(&desc);

	// ビューポートの設定
	work_buffer_rendering_pass[0].render_targets.emplace_back(render_targets->work_buffer_scene_textures[0]);
	work_buffer_rendering_pass[1].render_targets.emplace_back(render_targets->work_buffer_scene_textures[1]);
	work_buffer_viewport.TopLeftX = 0.0f;
	work_buffer_viewport.TopLeftY = 0.0f;
	work_buffer_viewport.Width = static_cast<float>(desc.BufferDesc.Width);
	work_buffer_viewport.Height = static_cast<float>(desc.BufferDesc.Height);
	work_buffer_viewport.MinDepth = 0.0f;
	work_buffer_viewport.MaxDepth = 1.0f;

	// ビューポートの設定
	scene_color_rendering_pass.render_targets.emplace_back(render_targets->scene_color_texture);
	scene_color_viewport.TopLeftX = 0.0f;
	scene_color_viewport.TopLeftY = 0.0f;
	scene_color_viewport.Width = static_cast<float>(desc.BufferDesc.Width);
	scene_color_viewport.Height = static_cast<float>(desc.BufferDesc.Height);
	scene_color_viewport.MinDepth = 0.0f;
	scene_color_viewport.MaxDepth = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	間にエフェクト関係

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
postprocess_after_effect_renderer::postprocess_after_effect_renderer(std::shared_ptr<scene_render_targets> render_targets)
	: renderer()
	, render_targets(render_targets)
	, work_buffer_rendering_pass()
	, work_buffer_viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
	, scene_color_rendering_pass()
	, scene_color_viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
{
	resize_buffer(0, 0);

	//	パイプラインステート生成
	pipeline_state_manager::instance()->register_pipeline_state<write_back_pipeline_state>("write_back", &write_back);
	pipeline_state_manager::instance()->register_pipeline_state<color_filter_after_effect_pipeline_state>("color_filter_after_effect", &color_filter);
}

postprocess_after_effect_renderer::~postprocess_after_effect_renderer()
{
}

void postprocess_after_effect_renderer::render(graphics_context& context)
{
	//	ポストプロセスボリュームが無ければ適応しない
	if (context.post_process_volume_container.post_process_volumes.empty())
		return;

	//	ポストプロセス情報設定
	//	とりあえず適当に
	context.post_process_data = context.post_process_volume_container.post_process_volumes[0]->get_post_process_data();

	bool buffer_index = 0;
	//	通常カラーフィルター
	{
		//	描画先設定
		work_buffer_rendering_pass[buffer_index].activate(context.device_context);

		//	ビューポートの設定
		context.device_context->RSSetViewports(1, &work_buffer_viewport);

		//	シーン描画
		context.scene_texture = render_targets->scene_color_texture;

		color_filter->begin(context);
		color_filter->draw(context);
		color_filter->end(context);

		work_buffer_rendering_pass[buffer_index].deactivate(context.device_context);
		buffer_index = !buffer_index;
	}

	//	書き戻し
	{
		//	描画先設定
		scene_color_rendering_pass.activate(context.device_context);

		//	ビューポートの設定
		context.device_context->RSSetViewports(1, &scene_color_viewport);

		//	シーン描画
		context.scene_texture = render_targets->work_buffer_scene_textures[!buffer_index];

		write_back->begin(context);
		write_back->draw(context);
		write_back->end(context);

		scene_color_rendering_pass.deactivate(context.device_context);
		buffer_index = !buffer_index;
	}
}

void postprocess_after_effect_renderer::resize_buffer(int width, int height)
{
	DXGI_SWAP_CHAIN_DESC desc;
	render_data::instance()->get_swap_chain()->GetDesc(&desc);

	// ビューポートの設定
	work_buffer_rendering_pass[0].render_targets.emplace_back(render_targets->work_buffer_scene_textures[0]);
	work_buffer_rendering_pass[1].render_targets.emplace_back(render_targets->work_buffer_scene_textures[1]);
	work_buffer_viewport.TopLeftX = 0.0f;
	work_buffer_viewport.TopLeftY = 0.0f;
	work_buffer_viewport.Width = static_cast<float>(desc.BufferDesc.Width);
	work_buffer_viewport.Height = static_cast<float>(desc.BufferDesc.Height);
	work_buffer_viewport.MinDepth = 0.0f;
	work_buffer_viewport.MaxDepth = 1.0f;

	// ビューポートの設定
	scene_color_rendering_pass.render_targets.emplace_back(render_targets->scene_color_texture);
	scene_color_viewport.TopLeftX = 0.0f;
	scene_color_viewport.TopLeftY = 0.0f;
	scene_color_viewport.Width = static_cast<float>(desc.BufferDesc.Width);
	scene_color_viewport.Height = static_cast<float>(desc.BufferDesc.Height);
	scene_color_viewport.MinDepth = 0.0f;
	scene_color_viewport.MaxDepth = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
finalpass_renderer::finalpass_renderer(std::shared_ptr<scene_render_targets> render_targets)
: renderer()
, render_targets(render_targets)
, viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
{
	resize_buffer(0, 0);

	//	パイプラインステート生成
	pipeline_state_manager::instance()->register_pipeline_state<convert_linear_to_srgb_pipeline_state>("convert_linear_to_srgb", &convert_linear_to_srgb_pipeline_states);
}

finalpass_renderer::~finalpass_renderer()
{
}

void finalpass_renderer::render(graphics_context& context)
{
	//	描画先設定
	rendering_pass.activate(context.device_context);

	//	ビューポートの設定
	context.device_context->RSSetViewports(1, &viewport);

	//	シーン描画
	context.scene_texture = render_targets->scene_color_texture;

	convert_linear_to_srgb_pipeline_states->begin(context);
	convert_linear_to_srgb_pipeline_states->draw(context);
	convert_linear_to_srgb_pipeline_states->end(context);

	rendering_pass.deactivate(context.device_context);
}

void finalpass_renderer::resize_buffer(int width, int height)
{
	rendering_pass.render_targets.emplace_back(render_data::instance()->get_swap_chain());

	DXGI_SWAP_CHAIN_DESC desc;
	render_data::instance()->get_swap_chain()->GetDesc(&desc);

	// ビューポートの設定
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(desc.BufferDesc.Width);
	viewport.Height = static_cast<float>(desc.BufferDesc.Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}


#if defined(_DEBUG)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
debug_draw_pass_renderer::debug_draw_pass_renderer(std::shared_ptr<scene_render_targets> render_targets)
: render_targets(render_targets)
{
	resize_buffer(0, 0);

	//	パイプラインステート生成
	pipeline_state_manager::instance()->register_pipeline_state<debug_draw_line_pipeline_state>("debug_draw_line_pipeline_state", &debug_draw_line_pipeline_states);
	
}

debug_draw_pass_renderer::~debug_draw_pass_renderer()
{
}

void debug_draw_pass_renderer::render(graphics_context& context)
{
	//	描画先設定
	rendering_pass.activate(context.device_context);

	//	ビューポートの設定
	context.device_context->RSSetViewports(1, &viewport);

	//	描画
	debug_draw_line_pipeline_states->begin(context);
	debug_draw_line_pipeline_states->draw_indexed(context, context.debug_draw_container.line_buffer_datas);
	debug_draw_line_pipeline_states->end(context);

	rendering_pass.deactivate(context.device_context);
}

void debug_draw_pass_renderer::resize_buffer(int width, int height)
{
	rendering_pass.render_targets.emplace_back(render_data::instance()->get_swap_chain());
	rendering_pass.render_targets.back().clear_color = false;
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;
	rendering_pass.depth_stencil = depth_stencil_view(render_targets->scene_depth_stencil_texture, &depth_stencil_view_desc);
	rendering_pass.depth_stencil.clear_depth = false;

	DXGI_SWAP_CHAIN_DESC desc;
	render_data::instance()->get_swap_chain()->GetDesc(&desc);

	// ビューポートの設定
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(desc.BufferDesc.Width);
	viewport.Height = static_cast<float>(desc.BufferDesc.Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}
#endif // defined(_DEBUG)
