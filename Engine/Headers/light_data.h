#pragma once

#include <d3d11.h>
#include <directxmath.h>

#include <unordered_map>
#include <vector>

#include "singleton.h"

//	ライトの種別
enum class light_type
{
	directional,	//  平行光源
	point,			//  点光源
	spot,			//  スポットライト

	light_probe,	//	ライトプローブ
};

struct directional_lights
{
	DirectX::XMFLOAT4 direction{ 1.0f, -1.0f, -1.0f, 1.0f };
	DirectX::XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct point_lights
{
	DirectX::XMFLOAT4 position{ 0, 0, 0, 0 };
	DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };
	float range{ 0 };
	DirectX::XMFLOAT3 dummy;
};

struct spot_lights
{
	DirectX::XMFLOAT4 position{ 0, 0, 0, 0 };
	DirectX::XMFLOAT4 direction{ 0, 0, 1, 0 };
	DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };
	float range{ 0 };
	float innerCorn{ DirectX::XMConvertToRadians(30) };
	float outerCorn{ DirectX::XMConvertToRadians(45) };
	float dummy;
};

struct light_probes
{
	DirectX::XMFLOAT4 position{ 0, 0, 0, 0 };
	DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };
	DirectX::XMFLOAT4 inner_area{ 100, 100, 100, 0 };
	DirectX::XMFLOAT4 outer_area{ 200, 200, 200, 0 };
};

struct light_data
{
	//  共有の情報
	//  work_data[3].w は共有でライトの種別番号を入れておく

	//  平行光源の場合
	//  work_data[0] = direction
	//  work_data[1] = color
	//  work_data[2] = dummy
	//  work_data[3] = xyz = dummy, w = ライト種別

	//  点光源の場合
	//  work_data[0] = position
	//  work_data[1] = color
	//  work_data[2] = x = range, yzw = dummy
	//  work_data[3] = xyz = dummy, w = ライト種別

	//  スポットライトの場合
	//  work_data[0] = position
	//  work_data[1] = direction
	//  work_data[2] = color
	//  work_data[3] = x = range, y = inner cone, z = outer cone, w = ライト種別

	//	プローブの場合
	//  work_data[0] = xyz = position, w = dummy
	//  work_data[1] = color
	//  work_data[2] = xyz = inner area, w = dummy
	//  work_data[3] = xyz = outer area, w = ライト種別
	DirectX::XMFLOAT4 work_data[4];
};

namespace light_data_utility
{
	directional_lights convert_directional_lights(const light_data& data);

	point_lights convert_point_lights(const light_data& data);

	spot_lights convert_spot_lights(const light_data& data);

	light_probes convert_light_probes(const light_data& data);

	light_data convert_light_data(const directional_lights& light);

	light_data convert_light_data(const point_lights& light);

	light_data convert_light_data(const spot_lights& light);

	light_data convert_light_data(const light_probes& light_probe);

	light_type get_light_type(const light_data& data);

	DirectX::XMFLOAT3 get_position(const light_data& data);

	DirectX::XMFLOAT3 get_direction(const light_data& data);

	DirectX::XMFLOAT4 get_color(const light_data& data);

}	//	namespace light_data_utility
