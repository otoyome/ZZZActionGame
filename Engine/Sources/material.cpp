#include "material.h"
#include "texture.h"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/unordered_map.hpp"

// CEREALバージョン定義
CEREAL_CLASS_VERSION(material::parameter, 1)
CEREAL_CLASS_VERSION(material::pipeline_state, 1)
CEREAL_CLASS_VERSION(material::texture, 1)
CEREAL_CLASS_VERSION(material::physical_datas, 1)
CEREAL_CLASS_VERSION(material, 1)

template<class Archive>
void material::parameter::serialize(Archive& archive, int version)
{
	archive(CEREAL_NVP(Type), CEREAL_NVP(Data.Float4x4));
}

template<class Archive>
void material::pipeline_state::serialize(Archive& archive, int version)
{
	archive(CEREAL_NVP(name));
}

template<class Archive>
void material::texture::serialize(Archive& archive, int version)
{
	archive(CEREAL_NVP(is_file_path_or_key), CEREAL_NVP(path));
}

template<class Archive>
void material::physical_datas::serialize(Archive& archive, int version)
{
	archive(CEREAL_NVP(friction));
}

material::material()
: name()
, domain(domains::unknown)
, blend_mode()
, pipeline_states()
, system_flags(0)
, alpha_clip_threshold(0.5f)
, physical_data()
, textures()
, parameters()
{
	set_flag(flags::shadow_caster, true);
	set_flag(flags::shadow_receiver, true);
}

material::material(std::string_view name)
: material()
{
	this->name = name;
}

material::~material()
{
}

template<class Archive>
void material::serialize(Archive& archive, int version)
{
	archive(CEREAL_NVP(name),
			CEREAL_NVP(domain),
			CEREAL_NVP(blend_mode),
			CEREAL_NVP(physical_data),
			CEREAL_NVP(pipeline_states),
			CEREAL_NVP(system_flags),
			CEREAL_NVP(alpha_clip_threshold),
			CEREAL_NVP(textures),
			CEREAL_NVP(parameters));
}

template<>
void material::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, int version)
{
	archive(CEREAL_NVP(name),
			CEREAL_NVP(domain),
			CEREAL_NVP(blend_mode),
			CEREAL_NVP(physical_data),
			CEREAL_NVP(pipeline_states),
			CEREAL_NVP(system_flags),
			CEREAL_NVP(alpha_clip_threshold),
			CEREAL_NVP(textures),
			CEREAL_NVP(parameters));
}

template<>
void material::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, int version)
{
	archive(CEREAL_NVP(name),
			CEREAL_NVP(domain),
			CEREAL_NVP(blend_mode),
			CEREAL_NVP(physical_data),
			CEREAL_NVP(pipeline_states),
			CEREAL_NVP(system_flags),
			CEREAL_NVP(alpha_clip_threshold),
			CEREAL_NVP(textures),
			CEREAL_NVP(parameters));
}

void material::duplicate(material& material) const
{
	std::stringstream iostream;
	serialize_stream(iostream);
	material.deserialize_stream(iostream);
	material.build();
}

void material::build()
{
	//	パイプラインステート登録
	for (auto& pipeline_state : pipeline_states)
	{
		if (pipeline_state_ptr state = pipeline_state_manager::instance()->search(pipeline_state.name))
		{
			pipeline_state.value = state;
		}
	}

	//	テクスチャ登録
	for (auto& texture : textures)
	{
		if (texture.second.is_file_path_or_key)
		{
			texture_manager::instance()->load_texture_from_file(texture.second.value, texture.second.path);
		}
		else
		{
			texture.second.value = texture_manager::instance()->search(texture.second.path);
		}
	}
}

void material::set_name(std::string_view name)
{
	this->name = std::string(name);
}

std::string_view material::get_name() const
{
	return name;
}

material::domains material::get_domain() const
{
	return domain;
}

void material::set_domain(material::domains domain)
{
	this->domain = domain;
	pipeline_states.resize(std::max(magic_enum::enum_count<surface_paths>(), magic_enum::enum_count<postprcess_paths>()));
}

material::blend_modes material::get_blend_mode() const
{
	return blend_mode;
}

void material::set_blend_mode(blend_modes blend_mode)
{
	this->blend_mode = blend_mode;
}

material::parameter_map& material::get_parameters()
{
	return parameters;
}

bool material::set_pipeline_state(domains domain, UINT index, std::string_view pipeline_state_name)
{
	//	ドメインが違えば登録させない
	if(this->domain != domain)
		return false;

	std::string name = std::string(pipeline_state_name);
	if (pipeline_states.size() >= index)
	{
		pipeline_states.resize(index + 1);
	}

	if (pipeline_states[index].name != name)
	{
		if (pipeline_state_ptr pipeline_state = pipeline_state_manager::instance()->search(name))
		{
			pipeline_states[index].name = name;
			pipeline_states[index].value = pipeline_state;
			return true;
		}
	}
	return false;
}

std::string_view material::get_pipeline_state_name(domains domain, UINT index) const
{
	if(this->domain != domain)
		return "";
	return pipeline_states[index].name;
}

pipeline_state_ptr material::get_pipeline_state(domains domain, UINT index) const
{
	if(this->domain != domain)
		return nullptr;

	return pipeline_states[index].value;
}

void material::set_int(std::string_view key_name, int value)
{
	parameter param;
	param.Type = parameter::type::Int;
	param.Data.Int = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

void material::set_int2(std::string_view key_name, DirectX::XMINT2 value)
{
	parameter param;
	param.Type = parameter::type::Int2;
	param.Data.Int2 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

void material::set_int3(std::string_view key_name, DirectX::XMINT3 value)
{
	parameter param;
	param.Type = parameter::type::Int3;
	param.Data.Int3 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

void material::set_int4(std::string_view key_name, DirectX::XMINT4 value)
{
	parameter param;
	param.Type = parameter::type::Int4;
	param.Data.Int4 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

bool material::get_int(std::string_view key_name, int& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::Int)
	{
		value = it->second.Data.Int;
		return true;
	}
	return false;
}

bool material::get_int2(std::string_view key_name, DirectX::XMINT2& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::Int2)
	{
		value = it->second.Data.Int2;
		return true;
	}
	return false;
}

bool material::get_int3(std::string_view key_name, DirectX::XMINT3& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::Int3)
	{
		value = it->second.Data.Int3;
		return true;
	}
	return false;
}

bool material::get_int4(std::string_view key_name, DirectX::XMINT4& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::Int4)
	{
		value = it->second.Data.Int4;
		return true;
	}
	return false;
}

void material::set_uint(std::string_view key_name, UINT value)
{
	parameter param;
	param.Type = parameter::type::UInt;
	param.Data.UInt = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

void material::set_uint2(std::string_view key_name, DirectX::XMUINT2 value)
{
	parameter param;
	param.Type = parameter::type::UInt2;
	param.Data.UInt2 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

void material::set_uint3(std::string_view key_name, DirectX::XMUINT3 value)
{
	parameter param;
	param.Type = parameter::type::UInt3;
	param.Data.UInt3 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

void material::set_uint4(std::string_view key_name, DirectX::XMUINT4 value)
{
	parameter param;
	param.Type = parameter::type::UInt4;
	param.Data.UInt4 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

bool material::get_uint(std::string_view key_name, UINT& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::UInt)
	{
		value = it->second.Data.UInt;
		return true;
	}
	return false;
}

bool material::get_uint2(std::string_view key_name, DirectX::XMUINT2& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::UInt2)
	{
		value = it->second.Data.UInt2;
		return true;
	}
	return false;
}

bool material::get_uint3(std::string_view key_name, DirectX::XMUINT3& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::UInt3)
	{
		value = it->second.Data.UInt3;
		return true;
	}
	return false;
}

bool material::get_uint4(std::string_view key_name, DirectX::XMUINT4& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::UInt4)
	{
		value = it->second.Data.UInt4;
		return true;
	}
	return false;
}

void material::set_float(std::string_view key_name, float value)
{
	parameter param;
	param.Type = parameter::type::Float;
	param.Data.Float = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

void material::set_float2(std::string_view key_name, DirectX::XMFLOAT2 value)
{
	parameter param;
	param.Type = parameter::type::Float2;
	param.Data.Float2 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

void material::set_float3(std::string_view key_name, DirectX::XMFLOAT3 value)
{
	parameter param;
	param.Type = parameter::type::Float3;
	param.Data.Float3 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

void material::set_float4(std::string_view key_name, DirectX::XMFLOAT4 value)
{
	parameter param;
	param.Type = parameter::type::Float4;
	param.Data.Float4 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

bool material::get_float(std::string_view key_name, float& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::Float)
	{
		value = it->second.Data.Float;
		return true;
	}
	return false;
}

bool material::get_float2(std::string_view key_name, DirectX::XMFLOAT2& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::Float2)
	{
		value = it->second.Data.Float2;
		return true;
	}
	return false;
}

bool material::get_float3(std::string_view key_name, DirectX::XMFLOAT3& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::Float3)
	{
		value = it->second.Data.Float3;
		return true;
	}
	return false;
}

bool material::get_float4(std::string_view key_name, DirectX::XMFLOAT4& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::Float4)
	{
		value = it->second.Data.Float4;
		return true;
	}
	return false;
}

void material::set_float4x4(std::string_view key_name, DirectX::XMFLOAT4X4 value)
{
	parameter param;
	param.Type = parameter::type::Float4x4;
	param.Data.Float4x4 = value;
	parameters.insert_or_assign(std::string(key_name), param);
}

bool material::get_float4x4(std::string_view key_name, DirectX::XMFLOAT4X4& value) const
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end() && it->second.Type == parameter::type::Float4x4)
	{
		value = it->second.Data.Float4x4;
		return true;
	}
	return false;
}

bool material::remove_parameter(std::string_view key_name)
{
	auto it = parameters.find(std::string(key_name));
	if (it != parameters.end())
	{
		parameters.erase(it);
		return true;
	}
	return false;
}

void material::set_texture(std::string_view key_name, texture_ptr value)
{
	texture tex;
	tex.is_file_path_or_key = value->is_file_path_or_key();
	tex.path = std::string(value->get_file_path_or_key());
	tex.value = value;
	textures.insert_or_assign(std::string(key_name), tex);
}

bool material::get_texture(std::string_view key_name, texture_ptr& value) const
{
	auto it = textures.find(std::string(key_name));
	if (it != textures.end())
	{
		value = it->second.value;
		return true;
	}
	return false;
}

void material::serialize_stream(std::stringstream& ostream) const
{
	cereal::JSONOutputArchive archive(ostream);

	try
	{
		archive(CEREAL_NVP(name),
				CEREAL_NVP(domain),
				CEREAL_NVP(blend_mode),
				CEREAL_NVP(physical_data),
				CEREAL_NVP(pipeline_states),
				CEREAL_NVP(system_flags),
				CEREAL_NVP(alpha_clip_threshold),
				CEREAL_NVP(textures),
				CEREAL_NVP(parameters));
	}
	catch (...)
	{
		OutputDebugStringA(name.data());
		assert(!"material serialize failed.\n");
		return;
	}
}

void material::deserialize_stream(std::stringstream& istream)
{
	cereal::JSONInputArchive archive(istream);

	try
	{
		archive(CEREAL_NVP(name),
				CEREAL_NVP(domain),
				CEREAL_NVP(blend_mode),
				CEREAL_NVP(physical_data),
				CEREAL_NVP(pipeline_states),
				CEREAL_NVP(system_flags),
				CEREAL_NVP(alpha_clip_threshold),
				CEREAL_NVP(textures),
				CEREAL_NVP(parameters));
	}
	catch (...)
	{
		OutputDebugStringA(name.data());
		assert(!"material deserialize failed.\n");
		return;
	}
}

