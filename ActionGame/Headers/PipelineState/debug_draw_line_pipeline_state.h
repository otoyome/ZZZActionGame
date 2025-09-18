#pragma once

#include "common_pipeline_states.h"

//	G-Bufferê∂ê¨ÉpÉX
class debug_draw_line_pipeline_state : public pipeline_state
{
public:
	debug_draw_line_pipeline_state();
	~debug_draw_line_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw_indexed(const graphics_context& context, const buffer_datas& buffer_data) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;
	std::shared_ptr<pixel_shader> ps;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states;

	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
};
