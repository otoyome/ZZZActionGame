#pragma once
#define NOMINMAX
#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <algorithm>

#include "misc.h"
#include "high_resolution_timer.h"

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

#include "singleton.h"

CONST LONG SCREEN_WIDTH{ 1600 };
CONST LONG SCREEN_HEIGHT{ 900 };
CONST BOOL FULLSCREEN{ FALSE };

class render_data
{
	SINGLETON_DECREATE(render_data)

public:
	static	inline	void	create_instance(CONST HWND hwnd)
	{
		if(!instance_ptr)
		{
			instance_ptr	= new render_data(hwnd);
		}
	}

public:
	enum class SAMPLER_STATE
	{
		POINT,
		LINEAR,
		ANISOTROPIC,
		POINT_CLAMP,
		LINEAR_CLAMP,
		ANISOTROPIC_CLAMP,
		LINEAR_BORDER_BLACK,
		LINEAR_BORDER_WHITE
	};

	enum class DEPTH_STATE
	{
		ZT_ON_ZW_ON,
		ZT_ON_ZW_OFF,
		ZT_OFF_ZW_ON,
		ZT_OFF_ZW_OFF
	};

	enum class BLEND_STATE
	{
		NONE,
		ALPHA,
		ADD,
		MULTIPLY,
		COLOR_ADD_ALPHA_ADD
	};

	enum class RASTER_STATE
	{
		SOLID,
		SOLID_CULL_FRONT,
		WIREFRAME,
		CULL_NONE,
		WIREFRAME_CULL_NONE
	};

public:
	CONST HWND get_hwnd() const { return hwnd; }
	Microsoft::WRL::ComPtr<ID3D11Device> get_device() const { return device; }
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> get_immediate_context() const { return immediate_context; }
	Microsoft::WRL::ComPtr<IDXGISwapChain> get_swap_chain() const { return swap_chain; }

	Microsoft::WRL::ComPtr<ID3D11SamplerState> get_sampler_states(SAMPLER_STATE index) const { return sampler_states[static_cast<int>(index)]; }

private:
	render_data(CONST HWND hwnd);
	~render_data();

private:
	CONST HWND hwnd;
	DirectX::XMUINT2 screen_size;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediate_context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_states[8];

};
