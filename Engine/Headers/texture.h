#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "misc.h"
#include "high_resolution_timer.h"

#include <d3d11.h>
#include <wrl.h>
#include <cassert>

#include "system_utility.h"
#include "singleton.h"

class texture
{
public:
	enum class types
	{
		color,	//	カラーマップ
		normal,	//	法線マップ
	};

	enum class flags
	{
		srgb,		//	SRGB対応済み

		cube_map,	//	キューブマップ
	};

public:
	D3D11_RESOURCE_DIMENSION	get_dimension() const;
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& get_shader_resource_view() const;

	types get_type() const;
	void set_type(types type);

	//	ファイルパスであればtrue, キーであればfalse
	bool is_file_path_or_key() const;
	//	ファイルパスかキーを取得
	std::string_view get_file_path_or_key() const;
	void set_file_path(std::string_view file_path);
	void set_data_key(std::string_view key);

	bool is_flag(flags flag) const { return system_utility::is_bitflag(system_flags, 1 << static_cast<UINT>(flag)); }
	void set_flag(flags flag, bool value) { system_utility::set_bitflag(system_flags, 1 << static_cast<UINT>(flag), value); }

public:
	texture(D3D11_RESOURCE_DIMENSION dimension, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view);
	virtual ~texture(){}

protected:
	bool is_file;
	std::string file_path_or_key;
	types type;
	UINT system_flags;
	D3D11_RESOURCE_DIMENSION dimension;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
};

using texture_ptr = std::shared_ptr<texture>;

class texture_manager
{
	SINGLETON(texture_manager)

	using texture_map = std::unordered_map<std::string, std::weak_ptr<texture>>;

public:
	static constexpr std::string_view white_texture2d_name = "white_texture";
	static constexpr std::string_view black_texture2d_name = "black_texture";

public:
	texture_manager();
	~texture_manager();

	bool	generate_texture2d(texture_ptr& out_texture, std::string_view key, D3D11_TEXTURE2D_DESC desc, D3D11_SHADER_RESOURCE_VIEW_DESC* srv_desc = nullptr);
	bool	generate_texture2d(texture_ptr& out_texture, std::string_view key, D3D11_TEXTURE2D_DESC desc, DWORD init_value/*0xAABBGGRR*/, D3D11_SHADER_RESOURCE_VIEW_DESC* srv_desc = nullptr);
	bool	load_texture_from_file(texture_ptr& out_texture, std::string_view file_path);
	bool	load_texture_from_memory(texture_ptr& out_texture, std::string_view key, const void* data, size_t size);

	texture_ptr	search(std::string_view name);

private:
	texture_ptr	white_texture2d;
	texture_ptr	black_texture2d;
	texture_map	textures;
};

class texture2d : public texture
{
public:
	texture2d(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view);
	~texture2d() override;

	//	テクスチャの情報取得
	inline const D3D11_TEXTURE2D_DESC& get_desc() const { return desc; }

	//	テクスチャ幅取得
	inline int get_width() const { return desc.Width; }

	//	テクスチャ高さ取得
	inline int get_height() const { return desc.Height; }

	//	テクスチャ配列数取得
	inline int get_array_size() const { return desc.ArraySize; }

	//	キューブマップ判定
	inline bool is_cube() const { return system_utility::is_bitflag(desc.MiscFlags, D3D11_RESOURCE_MISC_TEXTURECUBE); }

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D>	resource;
	D3D11_TEXTURE2D_DESC					desc;
};

using texture2d_ptr = std::shared_ptr<texture2d>;
