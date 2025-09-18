#pragma once

#include "common_pipeline_states.h"

//	深度書き込みパス
class toon_depth_write_pipeline_state : public pipeline_state
{
public:
	toon_depth_write_pipeline_state();
	~toon_depth_write_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw_indexed(const graphics_context& context, const buffer_datas& buffer_data) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<geometry_shader> gs;
	std::shared_ptr<pixel_shader> ps;
	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ppcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mcbv;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states[2];
};

//	カスケードシャドウマップ用深度書き込みパス
class toon_csm_write_pipeline_state : public pipeline_state
{
public:
	toon_csm_write_pipeline_state();
	~toon_csm_write_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw_indexed(const graphics_context& context, const buffer_datas& buffer_data) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<geometry_shader> gs;
//	std::shared_ptr<pixel_shader> ps;
	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ppcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> smcbv;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states[2];

	bool render_shadow;
};

//	G-Buffer生成パス
class toon_gbuffer_pipeline_state : public pipeline_state
{
public:
	toon_gbuffer_pipeline_state();
	~toon_gbuffer_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw_indexed(const graphics_context& context, const buffer_datas& buffer_data) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<geometry_shader> gs;
	std::shared_ptr<pixel_shader> ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states[2];

	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ppcbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mcbv;
};

//	トゥーンスカイボックスパス
class toon_skybox_pipeline_state : public pipeline_state
{
public:
	toon_skybox_pipeline_state();
	~toon_skybox_pipeline_state() override;

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
