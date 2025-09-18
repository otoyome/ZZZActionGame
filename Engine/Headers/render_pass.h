#pragma once

#include "texture.h"

struct render_target_view
{
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> view;
	bool clear_color{true};
	FLOAT clear_color_value[4]{0.0f, 0.0f, 0.0f, 0.0f};

	render_target_view(){};
	render_target_view(Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain);
	render_target_view(texture_ptr& texture, D3D11_RENDER_TARGET_VIEW_DESC* desc = nullptr);
	~render_target_view() {};
};

struct depth_stencil_view
{
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> view;
	bool clear_depth{true};
	bool clear_stencil{true};
	FLOAT clear_depth_value{1.0f};
	UINT8 clear_stencil_value{0};

	depth_stencil_view(){};
	depth_stencil_view(texture_ptr& texture, D3D11_DEPTH_STENCIL_VIEW_DESC* desc = nullptr);
	~depth_stencil_view() {};
};

struct render_pass
{
	std::vector<render_target_view>	render_targets;
	depth_stencil_view depth_stencil;

	render_pass();
	~render_pass();

	void activate(ID3D11DeviceContext* context);
	void deactivate(ID3D11DeviceContext* context);
};
