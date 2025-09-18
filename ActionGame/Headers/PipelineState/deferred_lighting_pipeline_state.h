#pragma once

#include "common_pipeline_states.h"

//	遅延間接光源ライティングパス
class deferred_indirect_lighting_pipeline_state : public pipeline_state
{
public:
	deferred_indirect_lighting_pipeline_state();
	~deferred_indirect_lighting_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw(const graphics_context& context) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<pixel_shader> ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states;

	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> lcbv;
};

//	遅延自己発光ライティングパス
class deferred_emissive_lighting_pipeline_state : public pipeline_state
{
public:
	deferred_emissive_lighting_pipeline_state();
	~deferred_emissive_lighting_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw(const graphics_context& context) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<pixel_shader> ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states;
};

//	遅延平行光源ライティングパス
class deferred_directional_lighting_pipeline_state : public pipeline_state
{
public:
	deferred_directional_lighting_pipeline_state();
	~deferred_directional_lighting_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw(const graphics_context& context) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<pixel_shader> ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states;

	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> lcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> smcbv;
};

//	遅延点光源ライティングパス
class deferred_point_lighting_pipeline_state : public pipeline_state
{
public:
	deferred_point_lighting_pipeline_state();
	~deferred_point_lighting_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw(const graphics_context& context) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<pixel_shader> ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states;

	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> lcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> smcbv;
};

//	遅延スポットライティングパス
class deferred_spot_lighting_pipeline_state : public pipeline_state
{
public:
	deferred_spot_lighting_pipeline_state();
	~deferred_spot_lighting_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw(const graphics_context& context) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<pixel_shader> ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states;

	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> lcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> smcbv;
};

//	遅延影ライトパス
class deferred_shadow_light_pipeline_state : public pipeline_state
{
public:
	deferred_shadow_light_pipeline_state();
	~deferred_shadow_light_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw(const graphics_context& context) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<pixel_shader> ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states;

	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> lcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> smcbv;
};

//	スカイボックスパス
class skybox_pipeline_state : public pipeline_state
{
public:
	skybox_pipeline_state();
	~skybox_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw(const graphics_context& context) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<pixel_shader> ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states;

	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> lcbv;
};
