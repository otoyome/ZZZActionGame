#include "light.h"
#include "texture.h"

light::light(light_type type)
: system_flags(0)
, type(type)
, position(0, 0, 0)
, rotation(0, 0, 0)
, color(1, 1, 1, 1)
, parameter(10.0f, DirectX::XMConvertToRadians(30.0f), DirectX::XMConvertToRadians(45.0f))
{
	set_flag(flags::active, true);
	set_flag(flags::use_shadow, true);

	csm_distance_factor = 0.5f;
	pssm_split_weight = 0.5f;
	shadow_attenuation = {0.2f, 0.2f, 0.2f};

	switch(type)
	{
		case light_type::directional:
		{
			bias_data[0].max_depth_bias = 0.01f;
			bias_data[0].sigma_factor = 0.5f;
			bias_data[1].max_depth_bias = 0.015f;
			bias_data[1].sigma_factor = 1.25f;
			bias_data[2].max_depth_bias = 0.018f;
			bias_data[2].sigma_factor = 20.0f;
			bias_data[3].max_depth_bias = 0.1f;
			bias_data[3].sigma_factor = 20.0f;
			break;
		}
		case light_type::point:
		{
			bias_data[0].max_depth_bias = 0.01f;
			bias_data[1].max_depth_bias = 0.01f;
			bias_data[2].max_depth_bias = 0.01f;
			bias_data[3].max_depth_bias = 0.01f;
			bias_data[4].max_depth_bias = 0.01f;
			bias_data[5].max_depth_bias = 0.01f;
			break;
		}
		case light_type::spot:
		{
			bias_data[0].max_depth_bias = 0.01f;
			bias_data[1].max_depth_bias = 0.01f;
			bias_data[2].max_depth_bias = 0.01f;
			bias_data[3].max_depth_bias = 0.01f;
			bias_data[4].max_depth_bias = 0.01f;
			bias_data[5].max_depth_bias = 0.01f;
			break;
		}
	}
}

light::~light()
{
}

light_data light::convert_light_data() const
{
	light_data data{};
	switch(type)
	{
		case light_type::directional:
		{
			directional_lights light;
			DirectX::XMFLOAT3 direction = rotation.get_front();
			light.direction.x = direction.x;
			light.direction.y = direction.y;
			light.direction.z = direction.z;
			light.direction.w = 0;
			light.color = color;
			data = light_data_utility::convert_light_data(light);
			break;
		}
		case light_type::point:
		{
			point_lights light;
			light.position.x = position.x;
			light.position.y = position.y;
			light.position.z = position.z;
			light.position.w = 1.0f;
			light.color = color;
			light.range = parameter.x;
			data = light_data_utility::convert_light_data(light);
			break;
		}
		case light_type::spot:
		{
			spot_lights light;
			DirectX::XMFLOAT3 direction = rotation.get_front();
			light.position.x = position.x;
			light.position.y = position.y;
			light.position.z = position.z;
			light.position.w = 1.0f;
			light.direction.x = direction.x;
			light.direction.y = direction.y;
			light.direction.z = direction.z;
			light.direction.w = 0;
			light.color = color;
			light.range = parameter.x;
			light.innerCorn = parameter.y;
			light.outerCorn = parameter.z;
			data = light_data_utility::convert_light_data(light);
			break;
		}
	}
	return data;
}

light_type light::get_light_type() const
{
	return type;
}

DirectX::XMFLOAT3 light::get_position() const
{
	return position;
}

void light::set_position(const DirectX::XMFLOAT3& position)
{
	this->position = position;
}

const math::rotator& light::get_rotation() const
{
	return rotation;
}

void light::set_rotation(const math::rotator& rotation)
{
	this->rotation = rotation;
}

const DirectX::XMFLOAT4& light::get_color() const
{
	return color;
}

void light::set_color(const DirectX::XMFLOAT4& color)
{
	this->color = color;
}

float light::get_range() const
{
	return parameter.x;
}

void light::set_range(float range)
{
	this->parameter.x = range;
}

float light::get_inner_cone() const
{
	return parameter.y;
}

void light::set_inner_cone(float inner_cone)
{
	this->parameter.y = inner_cone;
}

float light::get_outer_cone() const
{
	return parameter.z;
}

void light::set_outer_cone(float outer_cone)
{
	this->parameter.z = outer_cone;
}

const DirectX::XMFLOAT3& light::get_shadow_attenuation() const
{
	return shadow_attenuation;
}

void light::set_shadow_attenuation(const DirectX::XMFLOAT3& attenuation)
{
	this->shadow_attenuation = attenuation;
}

float light::get_csm_distance_factor() const
{
	return csm_distance_factor;
}

void light::set_csm_distance_factor(float factor)
{
	this->csm_distance_factor = factor;
}

float light::get_pssm_split_weight() const
{
	return pssm_split_weight;
}

void light::set_pssm_split_weight(float weight)
{
	this->pssm_split_weight = weight;
}

const light::shadow_map_bias_data& light::get_shadow_bias(int index) const
{
	return bias_data[index];
}

void light::set_shadow_bias(int index, const shadow_map_bias_data& bias_data)
{
	this->bias_data[index] = bias_data;
}

////////////////////////////////////////////////////////////////////////////////////////////////
shadow_light::shadow_light(types type)
: system_flags(0)
, type(type)
, position(0, 0, 0)
, rotation(0, 0, 0)
, color(0.2f, 0.2f, 0.2f, 1)
, parameter(300.0f, DirectX::XMConvertToRadians(20.0f), 0.00001f)
, contact_shadow(0.08f)
, mask(0)
{
	set_flag(flags::active, true);
}

shadow_light::~shadow_light()
{
}

shadow_light::types shadow_light::get_type() const
{
	return type;
}

DirectX::XMFLOAT3 shadow_light::get_position() const
{
	return position;
}

void shadow_light::set_position(const DirectX::XMFLOAT3& position)
{
	this->position = position;
}

const math::rotator& shadow_light::get_rotation() const
{
	return rotation;
}

void shadow_light::set_rotation(const math::rotator& rotation)
{
	this->rotation = rotation;
}

const DirectX::XMFLOAT4& shadow_light::get_color() const
{
	return color;
}

void shadow_light::set_color(const DirectX::XMFLOAT4& color)
{
	this->color = color;
}

float shadow_light::get_range() const
{
	return parameter.x;
}

void shadow_light::set_range(float range)
{
	this->parameter.x = range;
}

float shadow_light::get_cone() const
{
	return parameter.y;
}

void shadow_light::set_cone(float cone)
{
	this->parameter.y = cone;
}

float shadow_light::get_shadow_bias() const
{
	return this->parameter.z;
}

void shadow_light::set_shadow_bias(float bias)
{
	this->parameter.z = bias;
}

float shadow_light::get_contact_shadow() const
{
	return contact_shadow;
}

void shadow_light::set_contact_shadow(float contact_shadow)
{
	this->contact_shadow = contact_shadow;
}

UINT shadow_light::get_shadow_mask() const
{
	return mask;
}


////////////////////////////////////////////////////////////////////////////////////////////////
light_probe::light_probe()
: system_flags(0)
, diffuse_texture()
, specular_texture()
, lut_ggx_texture()
, position(0.0f, 0.0f, 0.0f)
, inner_range(50.0f, 50.0f, 50.0f)
, outer_range(100.0f, 100.0f, 100.0f)
, color(1, 1, 1, 1)
{
	set_flag(flags::active, true);
}

light_probe::~light_probe()
{
}

texture_ptr light_probe::get_diffuse_ibl_texture() const
{
	return diffuse_texture;
}

texture_ptr light_probe::get_specular_ibl_texture() const
{
	return specular_texture;
}

texture_ptr light_probe::get_lut_ggx_texture() const
{
	return lut_ggx_texture;
}

void light_probe::set_diffuse_ibl_texture(texture_ptr texture)
{
	diffuse_texture = texture;
}

void light_probe::set_specular_ibl_texture(texture_ptr texture)
{
	specular_texture = texture;
}

void light_probe::set_lut_ggx_texture(texture_ptr texture)
{
	lut_ggx_texture = texture;
}

DirectX::XMFLOAT3 light_probe::get_position() const
{
	return position;
}

void light_probe::set_position(const DirectX::XMFLOAT3& position)
{
	this->position = position;
}

DirectX::XMFLOAT3 light_probe::get_inner_range() const
{
	return inner_range;
}

void light_probe::set_inner_range(const DirectX::XMFLOAT3& inner_range)
{
	this->inner_range = inner_range;
}

DirectX::XMFLOAT3 light_probe::get_outer_range() const
{
	return outer_range;
}

void light_probe::set_outer_range(const DirectX::XMFLOAT3& outer_range)
{
	this->outer_range = outer_range;
}

const DirectX::XMFLOAT4& light_probe::get_color() const
{
	return color;
}

void light_probe::set_color(const DirectX::XMFLOAT4& color)
{
	this->color = color;
}

////////////////////////////////////////////////////////////////////////////////////////////////
light_manager::light_manager()
{
}

light_manager::~light_manager()
{
}

bool light_manager::generate_light(light_ptr& out_light, light_type type)
{
	out_light = std::make_shared<light>(type);
	lights.emplace_back(out_light);
	return true;
}

bool light_manager::generate_light_probe(light_probe_ptr& out_light_probe)
{
	out_light_probe = std::make_shared<light_probe>();
	light_probes.emplace_back(out_light_probe);
	return true;
}

bool light_manager::generate_shadow_light(shadow_light_ptr& out_shadow_light)
{
	out_shadow_light = std::make_shared<shadow_light>(shadow_light::types::spot);
	shadow_lights.emplace_back(out_shadow_light);
	return true;
}

void light_manager::get_container(light_container& container)
{
	//	ライト
	{
		container.lights.clear();
		auto it = lights.begin();
		for (; it != lights.end();)
		{
			if (it->expired())
			{
				it = lights.erase(it);
				continue;
			}
			container.lights.emplace_back(it->lock());
			++it;
		}
	}
	//	プローブ
	{
		container.light_probes.clear();
		auto it = light_probes.begin();
		for (; it != light_probes.end();)
		{
			if (it->expired())
			{
				it = light_probes.erase(it);
				continue;
			}
			container.light_probes.emplace_back(it->lock());
			++it;
		}
	}
	//	影ライト
	{
		container.shadow_lights.clear();
		auto it = shadow_lights.begin();
		for (; it != shadow_lights.end();)
		{
			if (it->expired())
			{
				it = shadow_lights.erase(it);
				continue;
			}
			container.shadow_lights.emplace_back(it->lock());
			++it;
		}
	}
}
