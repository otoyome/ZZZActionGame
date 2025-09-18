#pragma once

#include "renderer.h"
#include "render_pass.h"
#include "scene_render_targets.h"

#include "graphics/effect_manager.h"

#include "EffekseerRendererDX11.h"

//	エフェクト描画
class effect_renderer : public renderer
{
	enum pass
	{
		linear_to_srgb,
		srgb_effect,
		srgb_to_linear,
	};

public:
	domain get_domain() const override {return domain::effect_rendering;}

public:
	effect_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~effect_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	std::shared_ptr<scene_render_targets> render_targets;
	render_pass rendering_pass[3];
	D3D11_VIEWPORT viewport;

	pipeline_state_ptr convert_linear_to_srgb_pipeline_states;
	pipeline_state_ptr convert_srgb_to_linear_pipeline_states;

	//	エフェクシア関係
	::Effekseer::Backend::GraphicsDeviceRef graphics_device;
	::EffekseerRenderer::RendererRef effekseer_renderer;
};
