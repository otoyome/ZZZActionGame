#pragma once

#include "renderer.h"
#include "render_pass.h"
#include "scene_render_targets.h"
#include "geometric_primitive.h"

#include "collision/frustum_culling.h"

//	UIƒpƒX
class ui_renderer : public renderer
{
	static constexpr size_t vertex_max = 0xFFFF;
	static constexpr size_t index_max = 0x000FFFFF;

public:
	domain get_domain() const override { return domain::ui_rendering; }

public:
	ui_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~ui_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	void sprite_render(material::blend_modes blend_mode, graphics_context& context, const std::vector<std::unordered_map<semantic, buffer_view>>& vbvs, const std::vector<buffer_view>& ibvs);

private:
	std::shared_ptr<scene_render_targets> render_targets;
	render_pass rendering_pass;
	D3D11_VIEWPORT viewport;

	std::unordered_map<semantic, buffer_view> vertex_buffer_views;
	buffer_view index_buffer_view;
};
