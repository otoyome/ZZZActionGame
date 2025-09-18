#pragma once

#include "renderer.h"
#include "render_pass.h"
#include "scene_render_targets.h"
#include "geometric_primitive.h"

#include "collision/frustum_culling.h"

//	汎用のモデル描画機能持ちのレンダラー
class model_renderer : public renderer
{
public:
	model_renderer() : renderer() {}
	virtual ~model_renderer() {}

protected:
	void model_render(graphics_context& context, const std::vector<std::size_t>& mesh_indices, material::blend_modes blend_mode, material::surface_paths surface_path);
	void model_render(graphics_context& context, const std::vector<std::size_t>& mesh_indices, const collision::frustum_culling& frustum_culling, material::blend_modes blend_mode, material::surface_paths surface_path);
};

//	深度プリパス
class depth_prepass_renderer : public model_renderer
{
public:
	domain get_domain() const override { return domain::depth_prepass; }

public:
	depth_prepass_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~depth_prepass_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	std::shared_ptr<scene_render_targets> render_targets;
	render_pass rendering_pass;
	D3D11_VIEWPORT viewport;
};

//	GBuffer
class gbuffer_renderer : public model_renderer
{
public:
	domain get_domain() const override { return domain::g_buffer; }

public:
	gbuffer_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~gbuffer_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	std::shared_ptr<scene_render_targets> render_targets;
	render_pass rendering_pass;
	D3D11_VIEWPORT viewport;

	pipeline_state_ptr skybox_pipeline_states;
};

//	GBufferデカール
class gbuffer_decal_renderer : public renderer
{
public:
	domain get_domain() const override { return domain::g_buffer_decal; }

public:
	gbuffer_decal_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~gbuffer_decal_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	std::shared_ptr<scene_render_targets> render_targets;
	render_pass normal_depth_copy_rendering_pass;
	render_pass rendering_pass;
	D3D11_VIEWPORT viewport;

	std::unique_ptr<geometric_cube> cube;
};

//	ディファードライティング
class deffered_lighting_renderer : public model_renderer
{
	static constexpr UINT directional_shadow_map_count = 4;

public:
	domain get_domain() const override { return domain::deffered_lighting; }

public:
	deffered_lighting_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~deffered_lighting_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	void render_indirect_and_emissive(graphics_context& context);
	void render_directional(const light& light, graphics_context& context);
	void render_directional_shadow_map(const light& light, graphics_context& context);
	void render_point(const light& light, graphics_context& context);
	void render_point_shadow_map(const light& light, graphics_context& context);
	void render_spot(const light& light, graphics_context& context);
	void render_spot_shadow_map(const light& light, graphics_context& context);

	void render_shadow_light(const shadow_light& light, graphics_context& context);
	void render_shadow_light_shadow_map(const shadow_light& light, graphics_context& context);

	void resize_shadow_buffer(int width, int height);

	void activate_lighting_pass(graphics_context& context);
	void deactivate_lighting_pass(graphics_context& context);

private:
	std::shared_ptr<scene_render_targets> render_targets;
	bool lighting_pass_cleared;
	render_pass* use_lighting_pass;
	render_pass lighting_pass_clear;
	render_pass lighting_pass_load;
	D3D11_VIEWPORT lighting_pass_viewport;

	render_pass shadow_map_pass;
	common_pipeline_states::shadow_map_constant_buffer shadow_map_constant;
	D3D11_VIEWPORT shadow_map_pass_viewport;

	pipeline_state_ptr indirect_lighting_pipeline_state;
	pipeline_state_ptr emissive_lighting_pipeline_state;
	pipeline_state_ptr directional_lighting_pipeline_state;
	pipeline_state_ptr point_lighting_pipeline_state;
	pipeline_state_ptr spot_lighting_pipeline_state;
	pipeline_state_ptr shadow_light_pipeline_state;
};

//	ポストプロセス
class postprocess_renderer : public renderer
{
public:
	domain get_domain() const override { return domain::post_process; }

public:
	postprocess_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~postprocess_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	std::shared_ptr<scene_render_targets> render_targets;
	render_pass work_buffer_rendering_pass[2];
	D3D11_VIEWPORT work_buffer_viewport;
	render_pass scene_color_rendering_pass;
	D3D11_VIEWPORT scene_color_viewport;

	pipeline_state_ptr write_back;
	pipeline_state_ptr color_filter;
};

//	ファイナルパス
class finalpass_renderer : public renderer
{
public:
	domain get_domain() const override { return domain::final_pass; }

public:
	finalpass_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~finalpass_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	std::shared_ptr<scene_render_targets> render_targets;
	render_pass rendering_pass;
	D3D11_VIEWPORT viewport;

	pipeline_state_ptr convert_linear_to_srgb_pipeline_states;
};

//	エフェクト描画後ポストプロセス
class postprocess_after_effect_renderer : public renderer
{
public:
	domain get_domain() const override { return domain::post_process; }

public:
	postprocess_after_effect_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~postprocess_after_effect_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	std::shared_ptr<scene_render_targets> render_targets;
	render_pass work_buffer_rendering_pass[2];
	D3D11_VIEWPORT work_buffer_viewport;
	render_pass scene_color_rendering_pass;
	D3D11_VIEWPORT scene_color_viewport;

	pipeline_state_ptr write_back;
	pipeline_state_ptr color_filter;
};

#if defined(_DEBUG)
//	デバッグ描画パス
class debug_draw_pass_renderer : public renderer
{
public:
	domain get_domain() const override { return domain::debug_draw; }

public:
	debug_draw_pass_renderer(std::shared_ptr<scene_render_targets> render_targets);
	~debug_draw_pass_renderer() override;

	void render(graphics_context& context) override;

	void resize_buffer(int width, int height) override;

private:
	std::shared_ptr<scene_render_targets> render_targets;
	render_pass rendering_pass;
	D3D11_VIEWPORT viewport;

	pipeline_state_ptr debug_draw_line_pipeline_states;
};
#endif // defined(_DEBUG)
