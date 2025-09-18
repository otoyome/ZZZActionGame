#include "Renderer/effect_renderer.h"
#include "PipelineState/fullscreen_quad_pipeline_state.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
effect_renderer::effect_renderer(std::shared_ptr<scene_render_targets> render_targets)
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

	//	エフェクトマネージャー生成
	effect_manager::create_instance();

	//	エフェクシア関係の準備
	auto& manager = effect_manager::instance()->get_effekseer_manager();
	auto& device = render_data::instance()->get_device();
	auto& device_context = render_data::instance()->get_immediate_context();

	graphics_device = ::EffekseerRendererDX11::CreateGraphicsDevice(device.Get(), device_context.Get());
	effekseer_renderer = ::EffekseerRendererDX11::Renderer::Create(graphics_device, effect_manager::get_effekseer_instance_max());

	manager->SetSpriteRenderer(effekseer_renderer->CreateSpriteRenderer());
	manager->SetRibbonRenderer(effekseer_renderer->CreateRibbonRenderer());
	manager->SetRingRenderer(effekseer_renderer->CreateRingRenderer());
	manager->SetTrackRenderer(effekseer_renderer->CreateTrackRenderer());
	manager->SetModelRenderer(effekseer_renderer->CreateModelRenderer());

	manager->SetTextureLoader(effekseer_renderer->CreateTextureLoader());
	manager->SetModelLoader(effekseer_renderer->CreateModelLoader());
	manager->SetMaterialLoader(effekseer_renderer->CreateMaterialLoader());
	manager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

	manager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);

	//	
	pipeline_state_manager::instance()->register_pipeline_state<convert_linear_to_srgb_pipeline_state>("convert_linear_to_srgb", &convert_linear_to_srgb_pipeline_states);
	pipeline_state_manager::instance()->register_pipeline_state<convert_srgb_to_linear_pipeline_state>("convert_srgb_to_linear", &convert_srgb_to_linear_pipeline_states);
}

effect_renderer::~effect_renderer()
{
	if (effect_manager::is_valid())
	{
		effect_manager::delete_instance();
	}
}

void effect_renderer::render(graphics_context& context)
{
	//	エフェクシアの関係でLinear空間＞sRGB空間に変換
	{
		//	描画先設定
		rendering_pass[pass::linear_to_srgb].activate(context.device_context);

		//	ビューポートの設定
		context.device_context->RSSetViewports(1, &viewport);

		//	シーン描画
		context.scene_texture = render_targets->scene_color_texture;

		convert_linear_to_srgb_pipeline_states->begin(context);
		convert_linear_to_srgb_pipeline_states->draw(context);
		convert_linear_to_srgb_pipeline_states->end(context);

		rendering_pass[pass::linear_to_srgb].deactivate(context.device_context);
	}

	//	sRGB空間エフェクト描画
	{
		//	描画先設定
		rendering_pass[pass::srgb_effect].activate(context.device_context);

		//	ビューポートの設定
		context.device_context->RSSetViewports(1, &viewport);

		//	エフェクト描画
		effekseer_renderer->SetTime(context.scene_constant.time_data.x);
		effekseer_renderer->SetCameraMatrix(effect_utility::to_matrix44(context.scene_constant.view_transform));
		effekseer_renderer->SetProjectionMatrix(effect_utility::to_matrix44(context.scene_constant.projection_transform));
		effekseer_renderer->BeginRendering();
		{
			Effekseer::Manager::DrawParameter draw_parameter;
			draw_parameter.ZNear = 0.0f;
			draw_parameter.ZFar = 1.0f;
			draw_parameter.ViewProjectionMatrix = effekseer_renderer->GetCameraProjectionMatrix();
			auto& effekseer_manager = effect_manager::instance()->get_effekseer_manager();
			effekseer_manager->Draw(draw_parameter);
		}
		effekseer_renderer->EndRendering();

		rendering_pass[pass::srgb_effect].deactivate(context.device_context);
	}

	//	sRGB空間＞Linear空間に変換
	{
		//	描画先設定
		rendering_pass[pass::srgb_to_linear].activate(context.device_context);

		//	ビューポートの設定
		context.device_context->RSSetViewports(1, &viewport);

		//	シーン描画
		context.scene_texture = render_targets->work_buffer_scene_textures[0];

		convert_srgb_to_linear_pipeline_states->begin(context);
		convert_srgb_to_linear_pipeline_states->draw(context);
		convert_srgb_to_linear_pipeline_states->end(context);

		rendering_pass[pass::srgb_to_linear].deactivate(context.device_context);
	}
}

void effect_renderer::resize_buffer(int width, int height)
{
	if (viewport.Width == static_cast<float>(width) && viewport.Height == static_cast<float>(height))
		return;

	//	sRGBに変換
	rendering_pass[pass::linear_to_srgb].render_targets.emplace_back(render_targets->work_buffer_scene_textures[0]);

	//	描画パス生成
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;
	rendering_pass[pass::srgb_effect].render_targets.emplace_back(render_targets->work_buffer_scene_textures[0]);
	rendering_pass[pass::srgb_effect].render_targets.back().clear_color = false;
	rendering_pass[pass::srgb_effect].depth_stencil = depth_stencil_view(render_targets->scene_depth_stencil_texture, &depth_stencil_view_desc);
	rendering_pass[pass::srgb_effect].depth_stencil.clear_depth = false;
	rendering_pass[pass::srgb_effect].depth_stencil.clear_stencil = false;

	//	linearに変換
	rendering_pass[pass::srgb_to_linear].render_targets.emplace_back(render_targets->scene_color_texture);

	// ビューポートの設定
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}
