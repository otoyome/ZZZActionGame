#pragma once

#include "common_pipeline_states.h"

//	描き戻し
class write_back_pipeline_state : public pipeline_state
{
public:
	write_back_pipeline_state();
	~write_back_pipeline_state() override;

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

//	エフェクト描画前カラーフィルターパス
class color_filter_pre_effect_pipeline_state : public pipeline_state
{
public:
	color_filter_pre_effect_pipeline_state();
	~color_filter_pre_effect_pipeline_state() override;

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
};

//	エフェクト描画後カラーフィルターパス
class color_filter_after_effect_pipeline_state : public pipeline_state
{
	//	カラーフィルターポストプロセス定数バッファ
	struct color_filter_constant_buffer
	{
		postprocess::color_grading_data color_grading;
		postprocess::radial_blur_datas radial_blur;
		postprocess::separate_color_datas separate_color;
		postprocess::overwrite_text_datas overwrite_text;

		postprocess::concentrated_line_work_datas concentrated_line_work;
		postprocess::distortion_datas distortion_data;
		postprocess::sin_distortion_data sin_distortion_datas[2];

		postprocess::voronoi_distortion_data voronoi_distortion_data;
	};

public:
	color_filter_after_effect_pipeline_state();
	~color_filter_after_effect_pipeline_state() override;

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
	Microsoft::WRL::ComPtr<ID3D11Buffer> cgcbv;
};
