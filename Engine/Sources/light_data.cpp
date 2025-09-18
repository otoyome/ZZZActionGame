#include "light_data.h"

#include "render_data.h"

directional_lights light_data_utility::convert_directional_lights(const light_data& data)
{
	directional_lights light;
	light.direction = data.work_data[0];
	light.color = data.work_data[1];
	return light;
}

point_lights light_data_utility::convert_point_lights(const light_data& data)
{
	point_lights light;
	light.position = data.work_data[0];
	light.color = data.work_data[1];
	light.range = data.work_data[2].x;
	return light;
}

spot_lights light_data_utility::convert_spot_lights(const light_data& data)
{
	spot_lights light;
	light.position = data.work_data[0];
	light.direction = data.work_data[1];
	light.color = data.work_data[2];
	light.range = data.work_data[3].x;
	light.innerCorn = data.work_data[3].y;
	light.outerCorn = data.work_data[3].z;
	return light;
}

light_probes light_data_utility::convert_light_probes(const light_data& data)
{
	light_probes light_probe;
	light_probe.position = data.work_data[0];
	light_probe.color = data.work_data[1];
	light_probe.inner_area = data.work_data[2];
	light_probe.outer_area = data.work_data[3];
	return light_probe;
}

light_data light_data_utility::convert_light_data(const directional_lights& light)
{
	light_data data;
	data.work_data[0] = light.direction;
	data.work_data[1] = light.color;
	data.work_data[3].w = static_cast<int>(light_type::directional);
	return data;
}

light_data light_data_utility::convert_light_data(const point_lights& light)
{
	light_data data;
	data.work_data[0] = light.position;
	data.work_data[1] = light.color;
	data.work_data[2].x = light.range;
	data.work_data[3].w = static_cast<int>(light_type::point);
	return data;
}

light_data light_data_utility::convert_light_data(const spot_lights& light)
{
	light_data data;
	data.work_data[0] = light.position;
	data.work_data[1] = light.direction;
	data.work_data[2] = light.color;
	data.work_data[3].x = light.range;
	data.work_data[3].y = light.innerCorn;
	data.work_data[3].z = light.outerCorn;
	data.work_data[3].w = static_cast<int>(light_type::spot);
	return data;
}

light_data light_data_utility::convert_light_data(const light_probes& light_probe)
{
	light_data data;
	data.work_data[0] = light_probe.position;
	data.work_data[1] = light_probe.color;
	data.work_data[2] = light_probe.inner_area;
	data.work_data[3] = light_probe.outer_area;
	data.work_data[3].w = static_cast<int>(light_type::light_probe);
	return data;
}

light_type light_data_utility::get_light_type(const light_data& data)
{
	return static_cast<light_type>(data.work_data[3].w + 0.5f);
}

DirectX::XMFLOAT3 light_data_utility::get_position(const light_data& data)
{
	switch (get_light_type(data))
	{
	case light_type::directional: return  DirectX::XMFLOAT3(data.work_data[2].x, data.work_data[2].y, data.work_data[2].z);
	case light_type::point: return  DirectX::XMFLOAT3(data.work_data[0].x, data.work_data[0].y, data.work_data[0].z);
	case light_type::spot: return  DirectX::XMFLOAT3(data.work_data[0].x, data.work_data[0].y, data.work_data[0].z);
	}
	return DirectX::XMFLOAT3(0, 0, 0);
}

DirectX::XMFLOAT3 light_data_utility::get_direction(const light_data& data)
{
	switch (get_light_type(data))
	{
	case light_type::directional: return  DirectX::XMFLOAT3(data.work_data[0].x, data.work_data[0].y, data.work_data[0].z);
	case light_type::point: return  DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	case light_type::spot: return  DirectX::XMFLOAT3(data.work_data[1].x, data.work_data[1].y, data.work_data[1].z);
	}
	return DirectX::XMFLOAT3(0, 0, 0);
}

DirectX::XMFLOAT4 light_data_utility::get_color(const light_data& data)
{
	switch (get_light_type(data))
	{
	case light_type::directional: return  data.work_data[1];
	case light_type::point: return  data.work_data[1];
	case light_type::spot: return  data.work_data[2];
	}
	return DirectX::XMFLOAT4(0, 0, 0, 0);
}
