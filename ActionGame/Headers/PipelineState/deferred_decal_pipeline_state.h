#pragma once

#include "common_pipeline_states.h"

namespace decal_material_parameter_name
{
	static constexpr std::string_view base_color = "base_color";
	static constexpr std::string_view base_texture = "base_texture";
	static constexpr std::string_view decal_direction = "decal_direction";
	static constexpr std::string_view decal_inverse_transform = "decal_inverse_transform";
}	//	namespace decal_material_parameter_name

//	遅延デカールプリミティブステンシル書き込みパス
class deferred_decal_stencil_write_pipeline_state : public pipeline_state
{
public:
	deferred_decal_stencil_write_pipeline_state();
	~deferred_decal_stencil_write_pipeline_state() override;

	void begin(const graphics_context& context) override;
	void set_parameters(const graphics_context& context, const material& material) override;
	void draw_indexed(const graphics_context& context, const buffer_datas& buffer_data) override;
	void end(const graphics_context& context) override;

private:
	std::shared_ptr<vertex_shader> vs;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states;

	Microsoft::WRL::ComPtr<ID3D11Buffer> scbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ocbv;
};

//	遅延デカールプリミティブパス
class deferred_decal_primitive_pipeline_state : public pipeline_state
{
public:
	deferred_decal_primitive_pipeline_state();
	~deferred_decal_primitive_pipeline_state() override;

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
	Microsoft::WRL::ComPtr<ID3D11Buffer> ocbv;
	Microsoft::WRL::ComPtr<ID3D11Buffer> gbdcbv;

	DirectX::XMFLOAT4 base_color;
};

//	遅延デカールプリミティブパス
class deferred_decal_fullscreen_pipeline_state : public pipeline_state
{
public:
	deferred_decal_fullscreen_pipeline_state();
	~deferred_decal_fullscreen_pipeline_state() override;

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
	Microsoft::WRL::ComPtr<ID3D11Buffer> gbdcbv;

	DirectX::XMFLOAT4 base_color;
};
