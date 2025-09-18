/******************************************************************************************************************!

	@file	pmx_loader.cpp

*******************************************************************************************************************/
#include	"pmx_loader.h"

#include	<iostream>
#include	<locale> 
#include	<codecvt> 
#include	<cstdio>

namespace
{
	//	マジックナンバー
	constexpr	std::string_view	MagicNumber1_0 = "Pmx ";
	constexpr	std::string_view	MagicNumber2_0 = "PMX ";
	constexpr	std::string_view	ToonTextureFilePath = "Data/PMXToonTex/";
	constexpr	std::string_view	ToonEmptyTextureFilePath = "toon0.bmp";

	std::string	ConvertUtf8(const char* string, size_t length)
	{
		return	std::string(string);
	}
	std::string	ConvertUtf8(std::string_view string)
	{
		return	std::string(string);
	}

	int	MagicNumberCheck( pmx_header* header )
	{
		if (strcmp(header->magic_number, MagicNumber1_0.data()) == 0)
		{
			//	pmdらしいので未サポート
			return	pmx_version1_0;
		}
		if (strcmp(header->magic_number, MagicNumber2_0.data()) == 0)
		{
			if (header->version < 2.1f)
				return	pmx_version2_0;
			return	pmx_version2_1;
		}
		return	-1;
	}
};

pmx_loader::pmx_loader()
: error_code(pmx_error_code::unparsed)
, version(0)
, data_buffer(nullptr)
, data_buffer_size(0)
, incriment(0)

, index_data()
, name()
, vertices()
, indices()
, textures()
, materials()
, bones()
, morphs()
{
}

pmx_loader::~pmx_loader()
{
}

bool	pmx_loader::load(void* buffer, size_t buffer_size, std::string_view folder_path)
{
	error_code = pmx_error_code::unparsed;
	incriment = 0;
	data_buffer = static_cast<char*>(buffer);
	data_buffer_size = buffer_size;

	//	ヘッダー取得
	pmx_header* header = read_buffer<pmx_header>();

	//	マジックナンバーチェック
	version = MagicNumberCheck(header);
	if(version <= pmx_version1_0)
	{
		set_error_code(pmx_error_code::not_suppot);
		return	false;
	}

	//	インデックス情報取得
	{
		unsigned char byte_size = *read_buffer<unsigned char>();
		unsigned char* byte_array = read_buffer<unsigned char>(sizeof(unsigned char) * byte_size);
		memcpy_s(&index_data, sizeof(index_data), byte_array, byte_size);
	}

	//	モデル情報
	{
		char	model_name[512];
		char	comment[4096];
		read_string(model_name, ARRAYSIZE(model_name) );
		name	= ConvertUtf8(model_name);
		read_string(model_name, ARRAYSIZE(model_name));
		read_string(comment, ARRAYSIZE(comment));
		read_string(comment, ARRAYSIZE(comment));
	}

	//	頂点データリスト取得
	{
		vertices.resize(*read_buffer<unsigned int>());
		for( auto& it : vertices)
		{
			it.position = *read_buffer<DirectX::XMFLOAT3>();
			it.normal = *read_buffer<DirectX::XMFLOAT3>();
			it.texcoord = *read_buffer<DirectX::XMFLOAT2>();
			for (int jj = 0; jj < index_data.uv; jj++)
			{
				it.additional_uv[jj] = *read_buffer<DirectX::XMFLOAT4>();
			}
			it.skining_type = static_cast<pmx_skining_type>(*read_buffer<unsigned char>());
			it.bone_indices = {pmx_invalid_target_id, pmx_invalid_target_id , pmx_invalid_target_id , pmx_invalid_target_id };
			it.bone_weights = {};
			switch(it.skining_type)
			{
				case	pmx_skining_type::brdf1:
				{
					it.bone_indices.x = read_index(index_data.bone_index_size);
					it.bone_weights.x = 1.0f;
					break;
				}
				case	pmx_skining_type::brdf2:
				{
					it.bone_indices.x = read_index(index_data.bone_index_size);
					it.bone_indices.y = read_index(index_data.bone_index_size);
					it.bone_weights.x = *read_buffer<float>();
					it.bone_weights.y = 1.0f - it.bone_weights.x;
					break;
				}
				case	pmx_skining_type::brdf4:
				{
					it.bone_indices.x	= read_index(index_data.bone_index_size);
					it.bone_indices.y	= read_index(index_data.bone_index_size);
					it.bone_indices.z	= read_index(index_data.bone_index_size);
					it.bone_indices.w	= read_index(index_data.bone_index_size);
					it.bone_weights = *read_buffer<DirectX::XMFLOAT4>();
					break;
				}
				case	pmx_skining_type::sdef:
				{
					it.bone_indices.x = read_index(index_data.bone_index_size);
					it.bone_indices.y = read_index(index_data.bone_index_size);
					it.bone_weights.x = *read_buffer<float>();
					it.bone_weights.y = 1.0f - it.bone_weights.x;
					it.sdef[0] = *read_buffer<DirectX::XMFLOAT3>();
					it.sdef[1] = *read_buffer<DirectX::XMFLOAT3>();
					it.sdef[2] = *read_buffer<DirectX::XMFLOAT3>();
					break;
				}
				case	pmx_skining_type::qdef:
				{
					it.bone_indices.x = read_index(index_data.bone_index_size);
					it.bone_indices.y = read_index(index_data.bone_index_size);
					it.bone_indices.z = read_index(index_data.bone_index_size);
					it.bone_indices.w = read_index(index_data.bone_index_size);
					it.bone_weights = *read_buffer<DirectX::XMFLOAT4>();
					break;
				}
				default:
				{
					set_error_code(pmx_error_code::not_suppot);
					return	false;
				}
			}
			it.edge_value	= *read_buffer<float>();
		}
	}

	//	面取得
	{
		indices.resize(*read_buffer<unsigned int>());
		assert(indices.size() % 3 == 0);
		for (unsigned int index = 0; index < indices.size() / 3; ++index)
		{
			indices[index * 3 + 2] = read_index(index_data.vertex_index_size);
			indices[index * 3 + 1] = read_index(index_data.vertex_index_size);
			indices[index * 3 + 0] = read_index(index_data.vertex_index_size);
		}
	}

	//	テクスチャ
	{
		textures.resize(*read_buffer<unsigned int>());
		for (auto& texture_path : textures)
		{
			char	file_path[1024];
			read_string(file_path, ARRAYSIZE(file_path));
			texture_path = std::string(folder_path) + "/" + ConvertUtf8(file_path);
		}
	}

	//	マテリアル
	{
		materials.resize(*read_buffer<unsigned int>());
		for (auto& material : materials)
		{
			char	material_name[256], memo[4096];
			read_string(material_name, ARRAYSIZE(material_name));
			material.name					= ConvertUtf8(material_name);
			read_string(material_name, ARRAYSIZE(material_name));
			material.diffuse				= *read_buffer<DirectX::XMFLOAT4>();
			material.specular				= *read_buffer<DirectX::XMFLOAT3>();
			material.specularlity			= *read_buffer<float>();
			material.ambient				= *read_buffer<DirectX::XMFLOAT3>();
			material.draw_flag				= *read_buffer<unsigned char>();
			material.edge_color				= *read_buffer<DirectX::XMFLOAT4>();
			material.edge_size				= *read_buffer<float>();
			material.deffuse_texture_index	= read_index(index_data.texture_index_size);
			material.sphere_texture_index	= read_index(index_data.texture_index_size);
			material.sphere_mode			= *read_buffer<unsigned char>();
			material.toon_flag				= *read_buffer<unsigned char>();
			material.toon_texture_index	= material.toon_flag  ? *read_buffer<unsigned char>() : read_index(index_data.texture_index_size);
			read_string(memo, ARRAYSIZE(memo));
			material.index_count			= *read_buffer<unsigned int>();
		}
	}

	//	ボーン
	{
		bones.resize(*read_buffer<unsigned int>());
		for( auto& bone : bones)
		{
			char	bone_name[256];
			read_string(bone_name, ARRAYSIZE(bone_name));
			bone.name = ConvertUtf8(bone_name);
			read_string(bone_name, ARRAYSIZE(bone_name));
			bone.position		= *read_buffer<DirectX::XMFLOAT3>();
			bone.parent_index	= read_index(index_data.bone_index_size);
			bone.level			= *read_buffer<unsigned int>();
			bone.bone_flag		= *read_buffer<unsigned short>();
			if (bone.bone_flag & static_cast<int>(pmx_bone_bitflag::has_connection_bone))
			{
				bone.target_index = read_index(index_data.bone_index_size);
			}
			else
			{
				bone.target_index = pmx_invalid_target_id;
				bone.offset = *read_buffer<DirectX::XMFLOAT3>();
			}
			if (bone.bone_flag & (static_cast<int>(pmx_bone_bitflag::is_append_rotation) | static_cast<int>(pmx_bone_bitflag::is_append_translation)))
			{
				bone.append_parent_index = read_index(index_data.bone_index_size);
				bone.append_weight = *read_buffer<float>();
			}
			if (bone.bone_flag & static_cast<int>(pmx_bone_bitflag::is_ik_axis_lock))
			{
				bone.lock_axis_orientation = *read_buffer<DirectX::XMFLOAT3>();
			}
			if (bone.bone_flag & static_cast<int>(pmx_bone_bitflag::local_axis))
			{
				bone.local_axis_x_orientation = *read_buffer<DirectX::XMFLOAT3>();
				bone.local_axis_z_orientation = *read_buffer<DirectX::XMFLOAT3>();
			}
			if (bone.bone_flag & static_cast<int>(pmx_bone_bitflag::is_external_parent_deform))
			{
				bone.key = *read_buffer<unsigned short>(sizeof(unsigned int));
			}
			if (bone.bone_flag & static_cast<int>(pmx_bone_bitflag::is_ik))
			{
				bone.ik_target_bone_index = read_index(index_data.bone_index_size);
				bone.ik_loop = *read_buffer<unsigned int>();
				bone.ik_loop_angle_limit = *read_buffer<float>();
				bone.ik_link_list.resize(*read_buffer<unsigned int>());
				for (auto& ik_link : bone.ik_link_list)
				{
					ik_link.link_target = read_index(index_data.bone_index_size);
					ik_link.angle_lock = *read_buffer<unsigned char>();
					if (ik_link.angle_lock == 1)
					{
						ik_link.angle_min = *read_buffer<DirectX::XMFLOAT3>();
						ik_link.angle_max = *read_buffer<DirectX::XMFLOAT3>();
					}
				}
			}
		}
	}

	//	モーフ
	{
		morphs.resize(*read_buffer<unsigned int>());
		for (auto& morph : morphs)
		{
			char	morph_name[256];
			read_string(morph_name, ARRAYSIZE(morph_name));
			morph.name = ConvertUtf8(morph_name);
			read_string(morph_name, ARRAYSIZE(morph_name));
			morph.morph_category = static_cast<pmx_morph_category>(*read_buffer<unsigned char>());
			morph.morph_type = static_cast<pmx_morph_type>(*read_buffer<unsigned char>());
			switch (morph.morph_type)
			{
				case	pmx_morph_type::group:
				{
					morph.group_offset.resize(*read_buffer<unsigned int>());
					for(auto& offset : morph.group_offset)
					{
						offset.morph_index = read_index(index_data.morph_index_size);
						offset.morph_weight = *read_buffer<float>();
					}
					break;
				}
				case	pmx_morph_type::vertex:
				{
					unsigned int size = *read_buffer<unsigned int>();
					morph.vertex_offset.reserve(size);
					for (unsigned int index = 0; index < size; ++index)
					{
						unsigned int vertex_index = read_index(index_data.vertex_index_size);
						morph.vertex_offset.insert_or_assign(vertex_index, *read_buffer<DirectX::XMFLOAT3>());
					}
					break;
				}
				case	pmx_morph_type::bone:
				{
					morph.bone_offset.resize(*read_buffer<unsigned int>());
					for (auto& offset : morph.bone_offset)
					{
						offset.bone_index = read_index(index_data.bone_index_size);
						offset.translation = *read_buffer<DirectX::XMFLOAT3>();
						offset.rotation = *read_buffer<DirectX::XMFLOAT4>();
					}
					break;
				}
				case	pmx_morph_type::uv:
				case	pmx_morph_type::additional_uv1:
				case	pmx_morph_type::additional_uv2:
				case	pmx_morph_type::additional_uv3:
				case	pmx_morph_type::additional_uv4:
				{
					unsigned int offset_index = static_cast<unsigned int>(morph.morph_type) - static_cast<unsigned int>(pmx_morph_type::uv);
					unsigned int size = *read_buffer<unsigned int>();
					morph.uv_offset[offset_index].reserve(size);
					for (unsigned int index = 0; index < size; ++index)
					{
						unsigned int vertex_index = read_index(index_data.vertex_index_size);
						morph.uv_offset[offset_index].insert_or_assign(vertex_index, *read_buffer<DirectX::XMFLOAT4>());
					}
					break;
				}
				case	pmx_morph_type::matrial:
				{
					morph.material_offset.resize(*read_buffer<unsigned int>());
					for (auto& offset : morph.material_offset)
					{
						offset.material_index = read_index(index_data.material_index_size);
						offset.offset_operation = *read_buffer<unsigned char>();
						offset.diffuse = *read_buffer<DirectX::XMFLOAT4>();
						offset.specular = *read_buffer<DirectX::XMFLOAT3>();
						offset.specularlity = *read_buffer<float>();
						offset.ambient = *read_buffer<DirectX::XMFLOAT3>();
						offset.edge_color = *read_buffer<DirectX::XMFLOAT4>();
						offset.edge_size = *read_buffer<float>();
						offset.textureARGB = *read_buffer<DirectX::XMFLOAT4>();
						offset.shpereARGB = *read_buffer<DirectX::XMFLOAT4>();
						offset.toonARGB = *read_buffer<DirectX::XMFLOAT4>();
					}
					break;
				}
				case	pmx_morph_type::flip:
				{
					morph.flip_offset.resize(*read_buffer<unsigned int>());
					for (auto& offset : morph.flip_offset)
					{
						offset.morph_index = read_index(index_data.morph_index_size);
						offset.morph_value = *read_buffer<float>();
					}
					break;
				}
				case	pmx_morph_type::implus:
				{
					morph.implus_offset.resize(*read_buffer<unsigned int>());
					for (auto& offset : morph.implus_offset)
					{
						offset.rigidbody_index = read_index(index_data.rigidbody_index_size);
						offset.is_local = *read_buffer<unsigned char>();
						offset.velocity = *read_buffer<DirectX::XMFLOAT3>();
						offset.angular_torque = *read_buffer<DirectX::XMFLOAT3>();
					}
					break;
				}
				default:
				{
					set_error_code(pmx_error_code::not_suppot);
					return	false;
				}
			}
		}
	}

	set_error_code(pmx_error_code::succeeded);
	return	true;
}

unsigned int	pmx_loader::read_index(unsigned char size)
{
	switch(size)
	{
		case	1:
		{
			unsigned char	temp	= *read_buffer<unsigned char>();
			return	temp == 0xFF ? -1 : temp;
		}
		case	2:
		{
			unsigned short	temp = *read_buffer<unsigned short>();
			return	temp == 0xFFFF ? -1 : temp;
		}
		case	4:
		{
			return	*read_buffer<unsigned int>();
		}
		default:
		{
			assert( !"NotSupported" );
		}
	}
	return	-1;
}

void	pmx_loader::read_string( char* string, size_t string_length )
{
	//	最初に文字列のサイズが入っている
	int read_size = *read_buffer<int>();
	if (read_size == 0)
		return;
	
	SIZE_T	sSize;
	switch (index_data.encoding)
	{
		case	0:	//	UTF16
		{
			setlocale(LC_CTYPE, "jpn");
			WCHAR* work_string = read_buffer<WCHAR>(read_size);
			std::wstring	str(work_string, read_size / 2);
			work_string = const_cast<WCHAR*>(str.c_str());
			wcstombs_s(&sSize, string, string_length, work_string, read_size);
			break;
		}
		default:	//	UTF8
		{
			setlocale(LC_CTYPE, "jpn");
			WCHAR* work_string = read_buffer<WCHAR>(read_size);
			std::wstring	str(work_string, read_size / 2);
			work_string = const_cast<WCHAR*>(str.c_str());
			wcstombs_s(&sSize, string, string_length, work_string, read_size);
			break;
		}
	}
}

void	pmx_loader::set_error_code(pmx_error_code error_code)
{
	this->error_code = error_code;
}
