#include "texture.h"

#include "render_data.h"

#include <WICTextureLoader.h>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <sstream>
#include <iomanip>

#include <filesystem>
#include <DDSTextureLoader.h>

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/unordered_map.hpp"

// CEREALバージョン定義
CEREAL_CLASS_VERSION(texture, 1)

namespace
{
	bool	generate_texture2d_core(texture_ptr& out_texture, D3D11_TEXTURE2D_DESC desc, DWORD* init_value, D3D11_SHADER_RESOURCE_VIEW_DESC* srv_desc)
	{
		std::unique_ptr<DWORD[]> sysmem;
		if(init_value)
		{
			size_t texels = desc.Width * desc.Height;
			sysmem	= std::make_unique<DWORD[]>(texels);
			for (size_t i = 0; i < texels; ++i)
			{
				sysmem[i] = *init_value;
			}
		}

		HRESULT hr;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> resource;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
		if(!init_value)
		{
			hr = render_data::instance()->get_device()->CreateTexture2D(&desc, nullptr, &resource);
		}
		else
		{
			D3D11_SUBRESOURCE_DATA subresource_data{};
			subresource_data.pSysMem = sysmem.get();
			subresource_data.SysMemPitch = sizeof(DWORD) * desc.Width;
			subresource_data.SysMemSlicePitch = 0;
			hr = render_data::instance()->get_device()->CreateTexture2D(&desc, &subresource_data, &resource);
		}
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		if(FAILED(hr))
		{
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc{};
		if(srv_desc)
		{
			srvdesc	= *srv_desc;
		}
		else
		{
			srvdesc.Format = desc.Format;
			srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvdesc.Texture2D.MipLevels = 1;
		}
		hr = render_data::instance()->get_device()->CreateShaderResourceView(resource.Get(), &srvdesc, shader_resource_view.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		if(FAILED(hr))
		{
			return false;
		}

		out_texture	= std::make_shared<texture2d>(resource, shader_resource_view);
		return true;
	}
}

//	毎回定義するのが面倒なのでマクロにしておく
#define	RETURN_SEARCH_FILE(KEY_NAME)			\
auto	key_name	= std::string(KEY_NAME);	\
auto	it	= textures.find(key_name);			\
if (it != textures.end()){						\
	auto ptr = it->second.lock();				\
	if(ptr){									\
		out_texture = ptr;						\
		return	true;							\
	}											\
	textures.erase(it);							\
}

D3D11_RESOURCE_DIMENSION	texture::get_dimension() const
{
	return dimension;
}

const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& texture::get_shader_resource_view() const
{
	return shader_resource_view;
}

texture::types texture::get_type() const
{
	return type;
}

void texture::set_type(types type)
{
	this->type = type;
}

bool texture::is_file_path_or_key() const
{
	return this->is_file;
}

std::string_view texture::get_file_path_or_key() const
{
	return	this->file_path_or_key;
}

void texture::set_file_path(std::string_view file_path)
{
	this->is_file = true;
	this->file_path_or_key = file_path;
}

void texture::set_data_key(std::string_view key)
{
	this->is_file = false;
	this->file_path_or_key = key;
}

texture::texture(D3D11_RESOURCE_DIMENSION dimension, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view)
: is_file(false)
, file_path_or_key()
, type(types::color)
, system_flags(0)
, dimension(dimension)
, shader_resource_view(shader_resource_view)
{
	set_flag(flags::srgb, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
texture_manager::texture_manager()
{
	//	ダミーテクスチャ生成
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = 16;
		desc.Height = 16;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		generate_texture2d(white_texture2d, white_texture2d_name, desc, 0xFFFFFFFF);
		generate_texture2d(black_texture2d, black_texture2d_name, desc, 0xFF000000);
	}
}

texture_manager::~texture_manager()
{
}

bool	texture_manager::generate_texture2d(texture_ptr& out_texture, std::string_view key, D3D11_TEXTURE2D_DESC desc, D3D11_SHADER_RESOURCE_VIEW_DESC* srv_desc)
{
	//	存在すればそちらを渡す
	RETURN_SEARCH_FILE(key)

	if(generate_texture2d_core(out_texture, desc, nullptr, srv_desc))
	{
		out_texture->set_data_key(key);
		textures.insert(make_pair(key_name, out_texture));
		return true;
	}
	out_texture	= nullptr;
	return false;
}

bool	texture_manager::generate_texture2d(texture_ptr& out_texture, std::string_view key, D3D11_TEXTURE2D_DESC desc, DWORD init_value/*0xAABBGGRR*/, D3D11_SHADER_RESOURCE_VIEW_DESC* srv_desc)
{
	//	存在すればそちらを渡す
	RETURN_SEARCH_FILE(key)

	if(generate_texture2d_core(out_texture, desc, &init_value, srv_desc))
	{
		out_texture->set_data_key(key);
		textures.insert(make_pair(key_name, out_texture));
		return true;
	}
	out_texture	= nullptr;
	return false;
}

bool	texture_manager::load_texture_from_file(texture_ptr& out_texture, std::string_view file_path)
{
	//	存在すればそちらを渡す
	RETURN_SEARCH_FILE(file_path)

	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

	const std::filesystem::path path(file_path);
	hr = DirectX::CreateWICTextureFromFile(render_data::instance()->get_device().Get(), path.c_str(), resource.GetAddressOf(), shader_resource_view.GetAddressOf());
	if (FAILED(hr))
	{
		hr = DirectX::CreateDDSTextureFromFile(render_data::instance()->get_device().Get(), path.c_str(), resource.GetAddressOf(), shader_resource_view.GetAddressOf());
	}

	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	if(SUCCEEDED(hr))
	{
		D3D11_RESOURCE_DIMENSION dimension;
		resource->GetType(&dimension);
		switch(dimension)
		{
			case	D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				Microsoft::WRL::ComPtr<ID3D11Texture2D> resource_data;
				hr	= resource.As<ID3D11Texture2D>(&resource_data);
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
				if(SUCCEEDED(hr))
				{
					out_texture	= std::make_shared<texture2d>(resource_data, shader_resource_view);
					out_texture->set_file_path(file_path);
					textures.insert(make_pair(key_name, out_texture));
					return	true;
				}
				break;
			}
			//	とりあえず未対応
			case	D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			case	D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			default:
			{
				assert(!"[texture]unknown type.");
			}
		}
	}
	out_texture	= nullptr;
	return	false;
}

texture_ptr	texture_manager::search(std::string_view name)
{
	auto key_name = std::string(name);
	auto it = textures.find(key_name);
	if (it != textures.end())
	{
		auto ptr = it->second.lock();
		if (!ptr)
		{
			textures.erase(it);
			return	nullptr;
		}
		return	ptr;
	}

	return	nullptr;
}

bool texture_manager::load_texture_from_memory(texture_ptr& out_texture, std::string_view key, const void* data, size_t size)
{
	RETURN_SEARCH_FILE(key)

	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
	hr = DirectX::CreateDDSTextureFromMemory(render_data::instance()->get_device().Get(), reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), shader_resource_view.GetAddressOf());
	if (hr != S_OK)
	{
		hr = DirectX::CreateWICTextureFromMemory(render_data::instance()->get_device().Get(), reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), shader_resource_view.GetAddressOf());
	}

	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	if(SUCCEEDED(hr))
	{
		D3D11_RESOURCE_DIMENSION dimension;
		resource->GetType(&dimension);
		switch(dimension)
		{
			case	D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				Microsoft::WRL::ComPtr<ID3D11Texture2D> resource_data;
				hr	= resource.As<ID3D11Texture2D>(&resource_data);
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
				if(SUCCEEDED(hr))
				{
					out_texture	= std::make_shared<texture2d>(resource_data, shader_resource_view);
					textures.insert(make_pair(key_name, out_texture));
					return	true;
				}
				break;
			}
			//	とりあえず未対応
			case	D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			case	D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			default:
			{
				assert(!"[texture]unknown type.");
			}
		}
	}

	out_texture	= nullptr;
	return false;
}

texture2d::texture2d(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view)
: texture(D3D11_RESOURCE_DIMENSION_TEXTURE2D, shader_resource_view)
, resource(texture)
{
	texture->GetDesc(&desc);
}

texture2d::~texture2d()
{
}

#undef	RETURN_SEARCH_FILE