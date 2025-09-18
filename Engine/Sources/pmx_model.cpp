#include "pmx_model.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>

#include "misc.h"
#include "texture.h"
#include "shader.h"

#include <stdio.h>
#include <tchar.h>
#include <locale.h>
#include <iostream>
#include <string>
#include <windows.h>

namespace
{
	float normalize_angle(float angle)
	{
		float ret = angle;
		while (ret >= DirectX::XM_2PI)
		{
			ret -= DirectX::XM_2PI;
		}
		while (ret < 0)
		{
			ret += DirectX::XM_2PI;
		}

		return ret;
	}
	float difference_angle(float a, float b)
	{
		float diff = normalize_angle(a) - normalize_angle(b);
		if (diff > DirectX::XM_PI)
		{
			return diff - DirectX::XM_2PI;
		}
		else if (diff < -DirectX::XM_PI)
		{
			return diff + DirectX::XM_2PI;
		}
		return diff;
	}
	DirectX::XMFLOAT3 decompose(const DirectX::XMMATRIX& m, const DirectX::XMFLOAT3& before)
	{
		DirectX::XMFLOAT3 r;
		float sy = -m.r[0].m128_f32[2];
		const float e = 1.0e-6f;

		if ((1.0f - std::abs(sy)) < e)
		{
			r.y = std::asin(sy);
			float sx = std::sin(before.x);
			float sz = std::sin(before.z);
			if (std::abs(sx) < std::abs(sz))
			{
				float cx = std::cos(before.x);
				if (cx > 0)
				{
					r.x = 0;
					r.z = std::asin(-m.r[1].m128_f32[0]);
				}
				else
				{
					r.x = DirectX::XM_PI;
					r.z = std::asin(m.r[1].m128_f32[0]);
				}
			}
			else
			{
				float cz = std::cos(before.z);
				if (cz > 0)
				{
					r.z = 0;
					r.x = std::asin(-m.r[2].m128_f32[1]);
				}
				else
				{
					r.z = DirectX::XM_PI;
					r.x = std::asin(m.r[2].m128_f32[1]);
				}
			}
		}
		else
		{
			r.x = std::atan2(m.r[1].m128_f32[2], m.r[2].m128_f32[2]);
			r.y = std::asin(-m.r[0].m128_f32[2]);
			r.z = std::atan2(m.r[0].m128_f32[1], m.r[0].m128_f32[0]);
		}

		const float pi = DirectX::XM_PI;
		DirectX::XMFLOAT3 tests[] =
		{
			{ r.x + pi, pi - r.y, r.z + pi },
			{ r.x + pi, pi - r.y, r.z - pi },
			{ r.x + pi, -pi - r.y, r.z + pi },
			{ r.x + pi, -pi - r.y, r.z - pi },
			{ r.x - pi, pi - r.y, r.z + pi },
			{ r.x - pi, pi - r.y, r.z - pi },
			{ r.x - pi, -pi - r.y, r.z + pi },
			{ r.x - pi, -pi - r.y, r.z - pi },
		};

		float errX = std::abs(difference_angle(r.x, before.x));
		float errY = std::abs(difference_angle(r.y, before.y));
		float errZ = std::abs(difference_angle(r.z, before.z));
		float minErr = errX + errY + errZ;
		for (const auto test : tests)
		{
			float err = std::abs(difference_angle(test.x, before.x))
				+ std::abs(difference_angle(test.y, before.y))
				+ std::abs(difference_angle(test.z, before.z));
			if (err < minErr)
			{
				minErr = err;
				r = test;
			}
		}
		return r;
	}
	DirectX::XMFLOAT3 decompose(const DirectX::XMVECTOR& q, const DirectX::XMFLOAT3& before)
	{
		return decompose(DirectX::XMMatrixRotationQuaternion(q), before);
	}

	DirectX::XMFLOAT3 subtract_angle(const DirectX::XMFLOAT3& current_angle, const DirectX::XMFLOAT3& prev_angle)
	{
		DirectX::XMFLOAT3 angle;
		angle.x = normalize_angle(current_angle.x - prev_angle.x);
		angle.y = normalize_angle(current_angle.y - prev_angle.y);
		angle.z = normalize_angle(current_angle.z - prev_angle.z);
		return	angle;
	}
	DirectX::XMFLOAT3 additional_angle(const DirectX::XMFLOAT3& current_angle, const DirectX::XMFLOAT3& prev_angle)
	{
		DirectX::XMFLOAT3 angle;
		angle.x = normalize_angle(current_angle.x + prev_angle.x);
		angle.y = normalize_angle(current_angle.y + prev_angle.y);
		angle.z = normalize_angle(current_angle.z + prev_angle.z);
		return	angle;
	}
	DirectX::XMFLOAT3 clamp_angle(const DirectX::XMFLOAT3& current_angle, const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max)
	{
		DirectX::XMFLOAT3 angle;
		angle.x = std::clamp(current_angle.x, min.x, max.x);
		angle.y = std::clamp(current_angle.y, min.y, max.y);
		angle.z = std::clamp(current_angle.z, min.z, max.z);
		return	angle;
	}

	std::string_view DefaultRampTexturePath[] =
	{
		"Assets/Resources/toon/toon0.bmp",
		"Assets/Resources/toon/toon01.bmp",
		"Assets/Resources/toon/toon02.bmp",
		"Assets/Resources/toon/toon03.bmp",
		"Assets/Resources/toon/toon04.bmp",
		"Assets/Resources/toon/toon05.bmp",
		"Assets/Resources/toon/toon06.bmp",
		"Assets/Resources/toon/toon07.bmp",
		"Assets/Resources/toon/toon08.bmp",
		"Assets/Resources/toon/toon09.bmp",
		"Assets/Resources/toon/toon10.bmp",
	};

	std::wstring StringToWString(std::string oString)
	{
		// SJIS → wstring
		int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str()
			, -1, (wchar_t*)NULL, 0);

		// バッファの取得
		wchar_t* cpUCS2 = new wchar_t[iBufferSize];

		// SJIS → wstring
		MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2
			, iBufferSize);

		// stringの生成
		std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

		// バッファの破棄
		delete[] cpUCS2;

		// 変換結果を返す
		return(oRet);
	}

	//	MMDは30FPS基準で作られている
	static constexpr float normalized_frame = 30.0f;
}

void pmx_model::calculate_node_data::begin()
{
	animation_scale = { 1, 1, 1 };
	animation_rotation = { 0, 0, 0, 1 };
	animation_translation = { 0, 0, 0 };

	append_rotation = { 0, 0, 0, 1 };
	append_translation = { 0, 0, 0 };

	ik_rotation = { 0, 0, 0, 1 };

	scale = { 1, 1, 1 };
	rotation = { 0, 0, 0, 1 };
	position = { 0, 0, 0 };
}

void pmx_model::calculate_node_data::update_local_matrix()
{
	DirectX::XMVECTOR local_scale, local_rotation, local_position;
	local_scale = DirectX::XMLoadFloat3(&animation_scale);

	local_rotation	= DirectX::XMVectorSet( 0, 0, 0, 1 );
	local_rotation	= DirectX::XMQuaternionMultiply(local_rotation, DirectX::XMLoadFloat4(&append_rotation));
	local_rotation	= DirectX::XMQuaternionMultiply(local_rotation, DirectX::XMLoadFloat4(&animation_rotation));
	local_rotation	= DirectX::XMQuaternionMultiply(local_rotation, DirectX::XMLoadFloat4(&ik_rotation));

	local_position	= DirectX::XMLoadFloat3(&append_translation);
	local_position	= DirectX::XMVectorAdd(local_position, DirectX::XMLoadFloat3(&animation_translation));

	DirectX::XMMATRIX matrix;
	matrix	= DirectX::XMMatrixScalingFromVector(local_scale)
			* DirectX::XMMatrixRotationQuaternion(local_rotation)
			* DirectX::XMMatrixTranslationFromVector(local_position)
			* DirectX::XMLoadFloat4x4(&initialize_matrix);
	DirectX::XMMatrixDecompose(&local_scale, &local_rotation, &local_position, matrix);
	DirectX::XMStoreFloat3(&scale, local_scale);
	DirectX::XMStoreFloat4(&rotation, local_rotation);
	DirectX::XMStoreFloat3(&position, local_position);
	DirectX::XMStoreFloat4x4(&locll_matrix, matrix);
	global_matrix = locll_matrix;
}

void pmx_model::calculate_node_data::update_global_matrix(DirectX::XMMATRIX parent_global_matrix)
{
	DirectX::XMMATRIX matrix = DirectX::XMLoadFloat4x4(&global_matrix);
	DirectX::XMStoreFloat4x4(&global_matrix, matrix * parent_global_matrix);
}

#define	SWAP( a, b )	if(a > b){ decltype( a )	t	= b; b	= a; a	= t; }

///////////////////////////////////////////////////////////////////////////////////////////////////////
pmx_model::pmx_model(ID3D11Device* device, const std::string& filepath)
{
	//	ファイル読み込み
	pmx_loader loader;
	{
		std::filesystem::path path = filepath;
		std::string directory = path.parent_path().string();

		std::ifstream	read_file;
		read_file.open(path, std::ios::in | std::ios::binary);
		_ASSERT_EXPR_A(read_file, L"Failed to load file");

		std::string	file_buffer;
		file_buffer = std::string(std::istreambuf_iterator<char>(read_file), std::istreambuf_iterator<char>());

		bool succeeded = loader.load(file_buffer.data(), file_buffer.size(), directory);
		_ASSERT_EXPR_A(succeeded, L"Failed to load file");
		if (!succeeded)
			return;
	}

	//	テクスチャ読み込み
	{
		textures.reserve(loader.get_textures().size());
		for (auto& filename : loader.get_textures())
		{
			texture_ptr texture;
			if (texture_manager::instance()->load_texture_from_file(texture, filename))
			{
				texture_names.emplace_back(filename);
				textures.emplace_back(texture);
			}
		}
	}

	//	マテリアル/サブセット
	std::vector<subset> subsets;
	{
		materials.reserve(loader.get_materials().size());
		subsets.reserve(loader.get_materials().size());
		size_t index_count = 0;
		for (auto& m : loader.get_materials())
		{
			material wm;
			wm.name = m.name;
			wm.draw_flag = m.draw_flag;
			wm.def.diffuse_color = m.diffuse;
			wm.def.diffuse_texture_index = m.deffuse_texture_index;
			wm.def.sphere_texture_index = m.sphere_texture_index;
			wm.def.ramp_texture_index = m.toon_texture_index;
			if (m.toon_flag != 0)
			{
				wm.def.ramp_texture_index = m.toon_flag;
				wm.def.ramp_texture_index |= UseDefaultRamp;
			}
			wm.def.specular_color.x = m.specular.x;
			wm.def.specular_color.y = m.specular.y;
			wm.def.specular_color.z = m.specular.z;
			wm.def.specular_color.w = m.specularlity;
			wm.def.edge_color = m.edge_color;
			wm.def.edge_size = m.edge_size;
			{
				wm.def.alpha_clip_threshold = 0.01f;
				wm.def.alpha_dither_factor = 0.0f;
				wm.def.color_diff_line_factor = 1.0f;
				wm.def.color_diff_line_threshold = 0.005f;
				wm.def.color_line_adjust_saturate = 1.0f;
				wm.def.color_line_adjust_brightness = 1.0f;
				wm.def.color_posterize_factor = 24.0f;
				wm.def.depth_normal_outline_factor = 0.1f;
			}

			subset subset;
			subset.start = index_count;
			subset.count = m.index_count;
			subset.material_index = static_cast<size_t>(materials.size());

			subsets.emplace_back(subset);
			materials.emplace_back(wm);
			index_count += m.index_count;
		}
	}

	//	ノード
	{
		nodes.reserve(loader.get_bones().size());
		for (auto& b : loader.get_bones())
		{
			node wn;
			wn.name = b.name;
			wn.level = b.level;
			wn.parent_node_index = b.parent_index;
			wn.calculate_data.init_translation = b.position;

			//	階層
			{
				bool	is_find	= false;
				for(auto level : levels)
				{
					if( level == wn.level )
					{
						is_find	= true;
						break;
					}
				}
				if( !is_find )
				{
					levels.emplace_back(wn.level);
					std::sort(levels.begin(), levels.end());
				}
			}

			DirectX::XMMATRIX im;
			im = DirectX::XMMatrixTranslation(wn.calculate_data.init_translation.x, wn.calculate_data.init_translation.y, wn.calculate_data.init_translation.z);
			DirectX::XMStoreFloat4x4(&wn.calculate_data.initialize_matrix, im);
			DirectX::XMStoreFloat4x4(&wn.offset_matrix, DirectX::XMMatrixInverse(nullptr, im));

			//	Append
			if ((b.bone_flag & static_cast<unsigned short>(pmx_bone_bitflag::is_append_translation)) ||
				(b.bone_flag & static_cast<unsigned short>(pmx_bone_bitflag::is_append_rotation)))
			{
				wn.append_node_index = append_datas.size();
				append_data append;
				append.source_index = nodes.size();
				append.target_index = b.append_parent_index;
				append.weight = b.append_weight;
				append.is_rotation = (b.bone_flag & static_cast<unsigned short>(pmx_bone_bitflag::is_append_rotation));
				append.is_translation = (b.bone_flag & static_cast<unsigned short>(pmx_bone_bitflag::is_append_translation));
				append_datas.emplace_back(append);
			}

			//	IK
			if (b.bone_flag & static_cast<unsigned short>(pmx_bone_bitflag::is_ik))
			{
				ik_data ik;
				ik.source_index = nodes.size();
				ik.target_index = b.ik_target_bone_index;
				ik.loop = b.ik_loop;
				ik.angile_limit = b.ik_loop_angle_limit;
				ik.ik_link_start = ik_link_datas.size();
				bool is_insert = false;
				for (auto& il : b.ik_link_list)
				{
					if (b.ik_target_bone_index == ik.source_index || il.link_target == ik.source_index || b.ik_target_bone_index == il.link_target)
					{
						is_insert = false;
						break;
					}
					ik_link_data wil;
					wil.target_index = il.link_target;

					wil.angle_min = il.angle_min;
					wil.angle_max = il.angle_max;
					SWAP( wil.angle_min.x, wil.angle_max.x );
					SWAP( wil.angle_min.y, wil.angle_max.y );
					SWAP( wil.angle_min.z, wil.angle_max.z );

					if (il.angle_lock)
					{
						wil.lock = plane_lock::lock;
						if (wil.angle_min.x != wil.angle_max.x &&
							wil.angle_min.y == wil.angle_max.y &&
							wil.angle_min.z == wil.angle_max.z)
						{
							wil.lock = plane_lock::plane_x;
						}
						else if (wil.angle_min.x == wil.angle_max.x &&
								 wil.angle_min.y != wil.angle_max.y &&
								 wil.angle_min.z == wil.angle_max.z)
						{
							wil.lock = plane_lock::plane_y;
						}
						else if (wil.angle_min.x == wil.angle_max.x &&
								 wil.angle_min.y == wil.angle_max.y &&
								 wil.angle_min.z != wil.angle_max.z)
						{
							wil.lock = plane_lock::plane_z;
						}
					}

					is_insert = true;
					ik_link_datas.emplace_back(wil); 
				}
				if (is_insert)
				{
					wn.ik_node_index = static_cast<size_t>(ik_datas.size());
					ik.ik_link_end = static_cast<size_t>(ik_link_datas.size());
					ik_datas.emplace_back(ik);
				}
			}
			nodes.emplace_back(wn);
		}

		for (auto& n : nodes)
		{
			DirectX::XMMATRIX im = DirectX::XMLoadFloat4x4(&n.calculate_data.initialize_matrix);
			if (n.parent_node_index != pmx_invalid_target_id)
				im *= DirectX::XMLoadFloat4x4(&nodes[n.parent_node_index].offset_matrix);

			DirectX::XMVECTOR scale, rotation, translation;
			DirectX::XMMatrixDecompose(&scale, &rotation, &translation, im);
			DirectX::XMStoreFloat3(&n.calculate_data.init_scale, scale);
			DirectX::XMStoreFloat4(&n.calculate_data.init_rotation, DirectX::XMQuaternionNormalize(rotation));
			DirectX::XMStoreFloat3(&n.calculate_data.init_translation, translation);
			im	= DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&n.calculate_data.init_scale))
				* DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&n.calculate_data.init_rotation))
				* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&n.calculate_data.init_translation));
			DirectX::XMStoreFloat4x4(&n.calculate_data.initialize_matrix, im);
		}
	}

	//	メッシュ
	{
		vertices.reserve(loader.get_vertices().size());
		for (auto& v : loader.get_vertices())
		{
			vertex wv;
			wv.position = v.position;
			wv.normal = v.normal;
			//wv.color = { 1.0f,1.0f ,1.0f ,1.0f };
			wv.texcoord = v.texcoord;
			for (int ii = 0; ii < loader.get_index_data().uv; ++ii)
			{
				wv.additional_uv[ii] = v.additional_uv[ii];
			}
			wv.bone_indices = v.bone_indices;
			wv.bone_weights = v.bone_weights;
			vertices.emplace_back(wv);
		}
		this->subsets = subsets;
	}

	//	ノードソート
	{
		struct	sort_node
		{
			std::string	name;		//	名前
			size_t		order;		//	オーダー
			size_t		level;		//	変形階層
			size_t		old_index;	//	元番号
			size_t		new_index;	//	新番号
			node		row_node;	//	ノード情報
		};
		std::vector<sort_node>	sorted_nodes( nodes.size() );

		//	ソートオーダーを算出
		for( size_t ii = 0; ii < nodes.size(); ++ii )
		{
			sorted_nodes[ii].row_node	= nodes[ii];
			sorted_nodes[ii].name		= nodes[ii].name;
			sorted_nodes[ii].old_index	= ii;
			sorted_nodes[ii].order		= 0;
			sorted_nodes[ii].level		= nodes[ii].level;
			//	階層算出
			auto	node_pointer	= nodes.data() + ii;
			while( node_pointer->parent_node_index != pmx_invalid_target_id )
			{
				sorted_nodes[ii].order++;
				node_pointer	= nodes.data() + node_pointer->parent_node_index;
			}
		}

		//	オーダーソート
		std::sort( sorted_nodes.begin(), sorted_nodes.end(),
				   [&]( sort_node& right, sort_node& left )
					{
						if( right.level == left.level )
						{
							return	right.old_index < left.old_index;
						}
						return	right.level < left.level;
					} );

		//	新番号設定
		for( size_t ii = 0; ii < nodes.size(); ++ii )
		{
			sorted_nodes[ii].new_index	= ii;
			nodes[ii]	= sorted_nodes[ii].row_node;
		}

		//	元に戻す
		std::sort( sorted_nodes.begin(),
				   sorted_nodes.end(),
				   []( sort_node& right, sort_node& left )
					{
						return	right.old_index < left.old_index;
					} );

		//	頂点ノード再設定
		for( auto& it : vertices )
		{
			if( it.bone_indices.x != pmx_invalid_target_id )
			{
				it.bone_indices.x	= static_cast<UINT>(sorted_nodes[it.bone_indices.x].new_index);
			}
			if( it.bone_indices.y != pmx_invalid_target_id )
			{
				it.bone_indices.y	= static_cast<UINT>(sorted_nodes[it.bone_indices.y].new_index);
			}
			if( it.bone_indices.z != pmx_invalid_target_id )
			{
				it.bone_indices.z	= static_cast<UINT>(sorted_nodes[it.bone_indices.z].new_index);
			}
			if( it.bone_indices.w != pmx_invalid_target_id )
			{
				it.bone_indices.w	= static_cast<UINT>(sorted_nodes[it.bone_indices.w].new_index);
			}
		}

		indices.reserve(loader.get_indices().size());
		for(UINT i = 0; i < loader.get_indices().size(); i += 3)
		{
			indices.push_back(loader.get_indices()[i + 2]);
			indices.push_back(loader.get_indices()[i + 1]);
			indices.push_back(loader.get_indices()[i + 0]);
		}

		for( auto& it : nodes )
		{
			if( it.parent_node_index != pmx_invalid_target_id )
			{
				it.parent_node_index	= sorted_nodes[it.parent_node_index].new_index;
			}
		}

		for( auto& it : append_datas )
		{
			it.source_index	= sorted_nodes[it.source_index].new_index;
			it.target_index	= sorted_nodes[it.target_index].new_index;
		}

		for( auto& it : ik_datas )
		{
			it.source_index	= sorted_nodes[it.source_index].new_index;
			it.target_index	= sorted_nodes[it.target_index].new_index;
		}

		for( auto& it : ik_link_datas )
		{
			it.target_index	= sorted_nodes[it.target_index].new_index;
		}
	}

	//	ダミーテクスチャ生成
	dummy_texture	= texture_manager::instance()->search(texture_manager::white_texture2d_name);

	//	デフォルトランプテクスチャ生成
	{
		default_toon_textures.reserve(ARRAYSIZE(DefaultRampTexturePath));
		for (auto& filename : DefaultRampTexturePath)
		{
			texture_ptr texture;
			if (texture_manager::instance()->load_texture_from_file(texture, filename))
			{
				default_toon_textures.emplace_back(texture);
			}
		}
	}
}

pmx_model::~pmx_model()
{
}

void pmx_model::compute_animation(std::string_view animation_name, float time, std::vector<node_animation_data>& node_datas)
{
	if(node_datas.size() != nodes.size())
	{
		node_datas.resize(nodes.size());
	}

	//	アニメーションが存在しない場合は初期値を入れる
	const auto& it_animation = animations.find(std::string(animation_name));
	if (it_animation == animations.end())
	{
		for (size_t node_index = 0; node_index < nodes.size(); ++node_index)
		{
			auto& node = nodes[node_index];
			auto& node_data = node_datas[node_index];
			node_data.scale = node.calculate_data.init_scale;
			node_data.rotation = node.calculate_data.init_rotation;
			node_data.translation = node.calculate_data.init_translation;
		}
		return;
	}

	//	計算するために初期化
	for (auto& node : nodes)
	{
		node.calculate_data.begin();
	}

	//	アニメーション計算
	for (size_t node_index = 0; node_index < it_animation->second.nodes.size(); ++node_index)
	{
		auto& animation_node = it_animation->second.nodes[node_index];
		if (animation_node.empty())
			continue;

		size_t keyframe_max	= animation_node.size() - 1;
		if( keyframe_max >= animation_node.size() )
			continue;

		auto& node = nodes[node_index];

		size_t keyframe_index;
		for (keyframe_index = 0; keyframe_index < keyframe_max; ++keyframe_index)
		{
			const auto& keyframe0 = animation_node.at(keyframe_index + 0);
			const auto& keyframe1 = animation_node.at(keyframe_index + 1);
			if (!(keyframe0.time <= time && time < keyframe1.time))
				continue;

			float rate = (time - keyframe0.time) / (keyframe1.time - keyframe0.time);
			DirectX::XMVECTOR R0 = DirectX::XMLoadFloat4(&keyframe0.rotation);
			DirectX::XMVECTOR R1 = DirectX::XMLoadFloat4(&keyframe1.rotation);
			DirectX::XMVECTOR T0 = DirectX::XMLoadFloat3(&keyframe0.translation);
			DirectX::XMVECTOR T1 = DirectX::XMLoadFloat3(&keyframe1.translation);
			DirectX::XMStoreFloat4(&node.calculate_data.animation_rotation, DirectX::XMQuaternionNormalize(DirectX::XMQuaternionSlerp(R0, R1, rate)));
			DirectX::XMStoreFloat3(&node.calculate_data.animation_translation, DirectX::XMVectorLerp(T0, T1, rate));
			break;
		}

		if( keyframe_max == keyframe_index )
		{
			const auto& keyframe = animation_node.back();
			if (keyframe.time <= time)
			{
				node.calculate_data.animation_rotation = keyframe.rotation;
				node.calculate_data.animation_translation = keyframe.translation;
			}
		}
	}

	for(auto& ik_link_data : ik_link_datas)
	{
		ik_link_data.plane_mode_angle = 0;
		ik_link_data.prev_angle = {0, 0, 0};
	}

	//	ノード一旦更新
	for (size_t node_index = 0; node_index < nodes.size(); ++node_index)
	{
		auto& node = nodes[node_index];

		node.calculate_data.update_local_matrix();
		if (node.parent_node_index != pmx_invalid_target_id)
		{
			DirectX::XMMATRIX matrix;
			matrix = DirectX::XMLoadFloat4x4(&nodes[node.parent_node_index].calculate_data.global_matrix);
			node.calculate_data.update_global_matrix(matrix);
		}
	}

	//	その他計算
	for (size_t node_index = 0; node_index < nodes.size(); ++node_index)
	{
		update_append(node_index);

		update_ik(node_index);
	}

	//	最終更新
	for (size_t node_index = 0; node_index < nodes.size(); ++node_index)
	{
		auto& node = nodes[node_index];
		auto& node_data = node_datas[node_index];

		node.calculate_data.update_local_matrix();
		node_data.scale = node.calculate_data.scale;
		node_data.rotation = node.calculate_data.rotation;
		node_data.translation = node.calculate_data.position;
	}
}

void pmx_model::update_ik(size_t node_index)
{
	auto& source_node = nodes[node_index];
	if( source_node.ik_node_index == pmx_invalid_target_id )
		return;

	auto& ikdata = ik_datas[source_node.ik_node_index];

	//	CCD-IK
	if (!ikdata.enable)
		return;

	DirectX::XMVECTOR ik_target_position = DirectX::XMVectorSet(source_node.calculate_data.global_matrix._41, source_node.calculate_data.global_matrix._42, source_node.calculate_data.global_matrix._43, 1);

	int loop_max = ikdata.loop;
	for (int loop_count = 0; loop_count < loop_max; ++loop_count)
	{
		auto& target_node = nodes[ikdata.target_index];
		DirectX::XMVECTOR target_position = DirectX::XMVectorSet(target_node.calculate_data.global_matrix._41, target_node.calculate_data.global_matrix._42, target_node.calculate_data.global_matrix._43, 1);
		DirectX::XMVECTOR vector = DirectX::XMVectorSubtract(ik_target_position, target_position);
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(vector));
		if (distance <= 0.001f)
			break;

		for (size_t link_index = ikdata.ik_link_start; link_index < ikdata.ik_link_end; ++link_index)
		{
			auto& link_data = ik_link_datas[link_index];
			auto& link_node = nodes[link_data.target_index];

			//	平面の処理に変更
			if (link_data.lock != plane_lock::free && link_data.lock != plane_lock::lock)
			{
				update_ik_plane(loop_count, ikdata, link_data, link_data.lock);
			}
			//	平面処理以外
			else
			{
				DirectX::XMMATRIX inverse_link_global_matrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&link_node.calculate_data.global_matrix));

				DirectX::XMVECTOR effector_global_position = DirectX::XMVectorSet(target_node.calculate_data.global_matrix._41, target_node.calculate_data.global_matrix._42, target_node.calculate_data.global_matrix._43, 1);
				DirectX::XMVECTOR effector_local_position = DirectX::XMVector3TransformCoord(effector_global_position, inverse_link_global_matrix);
				DirectX::XMVECTOR target_local_position = DirectX::XMVector3TransformCoord(ik_target_position, inverse_link_global_matrix);

				DirectX::XMVECTOR effector_local_direction = DirectX::XMVector3Normalize(effector_local_position);
				DirectX::XMVECTOR target_local_direction = DirectX::XMVector3Normalize(target_local_position);
				float	dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(effector_local_direction, target_local_direction));
				float	angle = acosf(dot);
				DirectX::XMVECTOR axis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(effector_local_direction, target_local_direction));
				if (isnan(angle) || DirectX::XMVectorGetX(DirectX::XMVector3Dot(axis, axis)) <= 0.001f)
					continue;

				angle = min(angle, ikdata.angile_limit);

				DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationAxis(axis, angle);

				//	適応
				DirectX::XMVECTOR node_rotation = DirectX::XMLoadFloat4(&link_node.calculate_data.rotation);
				node_rotation = DirectX::XMQuaternionMultiply(rotation, node_rotation);
				node_rotation = DirectX::XMQuaternionNormalize(node_rotation);

				//	角度制限
				if (link_data.lock == plane_lock::lock)
				{
					DirectX::XMFLOAT3 angle = decompose(node_rotation, link_data.prev_angle);

					DirectX::XMFLOAT3 limit_angle = clamp_angle(angle, link_data.angle_min, link_data.angle_max);
					limit_angle = clamp_angle(subtract_angle(limit_angle, link_data.prev_angle),
											{-ikdata.angile_limit, -ikdata.angile_limit, -ikdata.angile_limit},
											{+ikdata.angile_limit, +ikdata.angile_limit, +ikdata.angile_limit});
					limit_angle = additional_angle(limit_angle, link_data.prev_angle);

					node_rotation = DirectX::XMQuaternionRotationRollPitchYaw(limit_angle.x, limit_angle.y, limit_angle.z);
					link_data.prev_angle = limit_angle;
				}

				//	IKノード情報に変換
				DirectX::XMVECTOR inverse_rotation, append_rotation, animation_rotation;
				append_rotation = DirectX::XMLoadFloat4(&link_node.calculate_data.append_rotation);
				animation_rotation = DirectX::XMLoadFloat4(&link_node.calculate_data.animation_rotation);
				inverse_rotation = DirectX::XMQuaternionInverse(DirectX::XMQuaternionMultiply(append_rotation, animation_rotation));
				node_rotation = DirectX::XMQuaternionMultiply(inverse_rotation, node_rotation);
				DirectX::XMStoreFloat4(&link_node.calculate_data.ik_rotation, DirectX::XMQuaternionNormalize(node_rotation));
			}

			//	グローバル行列再計算
			update_node_transforms(link_data.target_index);
		}
	}
}

void pmx_model::update_ik_plane(int iteration, ik_data& ik_data, ik_link_data& link_data, plane_lock axis)
{
	DirectX::XMVECTOR rotate_axis;
	float limit_min_angle = 0;
	float limit_max_angle = 0;

	switch (axis)
	{
		case plane_lock::plane_x:
		{
			limit_min_angle = link_data.angle_min.x;
			limit_max_angle = link_data.angle_max.x;
			rotate_axis = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
			break;
		}
		case plane_lock::plane_y:
		{
			limit_min_angle = link_data.angle_min.y;
			limit_max_angle = link_data.angle_max.y;
			rotate_axis = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			break;
		}
		case plane_lock::plane_z:
		{
			limit_min_angle = link_data.angle_min.z;
			limit_max_angle = link_data.angle_max.z;
			rotate_axis = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			break;
		}
	}
	node& source_node	= nodes[ik_data.source_index];
	node& target_node	= nodes[ik_data.target_index];
	node& link_node		= nodes[link_data.target_index];

	DirectX::XMVECTOR ik_target_position = DirectX::XMVectorSet(source_node.calculate_data.global_matrix._41, source_node.calculate_data.global_matrix._42, source_node.calculate_data.global_matrix._43, 1);
	DirectX::XMVECTOR target_position = DirectX::XMVectorSet(target_node.calculate_data.global_matrix._41, target_node.calculate_data.global_matrix._42, target_node.calculate_data.global_matrix._43, 1);

	DirectX::XMMATRIX inverse_link_global_matrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&link_node.calculate_data.global_matrix));

	DirectX::XMVECTOR effector_global_position = DirectX::XMVectorSet(target_node.calculate_data.global_matrix._41, target_node.calculate_data.global_matrix._42, target_node.calculate_data.global_matrix._43, 1);
	DirectX::XMVECTOR effector_local_position = DirectX::XMVector3TransformCoord(effector_global_position, inverse_link_global_matrix);
	DirectX::XMVECTOR target_local_position = DirectX::XMVector3TransformCoord(ik_target_position, inverse_link_global_matrix);

	DirectX::XMVECTOR effector_local_direction = DirectX::XMVector3Normalize(effector_local_position);
	DirectX::XMVECTOR target_local_direction = DirectX::XMVector3Normalize(target_local_position);

	float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(effector_local_direction, target_local_direction));
	float angle = std::acos(dot);
	if(isnan(angle))
		return;

	angle = std::clamp(angle, -ik_data.angile_limit, ik_data.angile_limit);

	//	回転方向を判断するため、両方の方向に回転させてみて適応角度を変える
	DirectX::XMVECTOR rotation1 = DirectX::XMQuaternionRotationAxis(rotate_axis, +angle);
	DirectX::XMVECTOR target_vector1 = DirectX::XMVector3Rotate(effector_local_direction, rotation1);
	float dot1 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(target_vector1, target_local_direction));

	DirectX::XMVECTOR rotation2 = DirectX::XMQuaternionRotationAxis(rotate_axis, -angle);
	DirectX::XMVECTOR target_vector2 = DirectX::XMVector3Rotate(effector_local_direction, rotation2);
	float dot2 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(target_vector2, target_local_direction));

	float new_angle = link_data.plane_mode_angle + ((dot1 > dot2) ? +angle : -angle);
	if (iteration == 0)
	{
		if (new_angle < limit_min_angle || new_angle > limit_max_angle)
		{
			if (-new_angle > limit_min_angle && -new_angle < limit_max_angle)
			{
				new_angle *= -1;
			}
			else
			{
				float halfRadian = (limit_min_angle + limit_max_angle) * 0.5f;
				if (abs(halfRadian - new_angle) > abs(halfRadian + new_angle))
				{
					new_angle *= -1;
				}
			}
		}
	}
	new_angle = std::clamp(new_angle, limit_min_angle, limit_max_angle);
	link_data.plane_mode_angle = new_angle;
	DirectX::XMVECTOR node_rotation = DirectX::XMQuaternionRotationAxis(rotate_axis, new_angle);

	//	IKノード情報に変換
	DirectX::XMVECTOR inverse_rotation, append_rotation, animation_rotation;
	append_rotation = DirectX::XMLoadFloat4(&link_node.calculate_data.append_rotation);
	animation_rotation = DirectX::XMLoadFloat4(&link_node.calculate_data.animation_rotation);
	inverse_rotation = DirectX::XMQuaternionInverse(DirectX::XMQuaternionMultiply(append_rotation, animation_rotation));
	node_rotation	= DirectX::XMQuaternionMultiply(inverse_rotation, node_rotation);
	DirectX::XMStoreFloat4(&link_node.calculate_data.ik_rotation, DirectX::XMQuaternionNormalize(node_rotation));
}

void pmx_model::update_append(size_t node_index)
{
	auto& node = nodes[node_index];
	if( node.append_node_index == pmx_invalid_target_id )
		return;

	auto& append = append_datas[node.append_node_index];
	auto& source_node = nodes[append.source_index];
	auto& target_node = nodes[append.target_index];

	bool is_update = false;
	if (append.is_translation)
	{
		DirectX::XMFLOAT3 translation;
		if( target_node.append_node_index != pmx_invalid_target_id )
		{
			translation	= target_node.calculate_data.append_translation;
		}
		else
		{
			translation	= target_node.calculate_data.animation_translation;
		}

		if (!DirectX::XMVector3Equal(DirectX::XMLoadFloat3(&source_node.calculate_data.append_translation), DirectX::XMLoadFloat3(&translation)))
		{
			source_node.calculate_data.append_translation.x	= translation.x * append.weight;
			source_node.calculate_data.append_translation.y	= translation.y * append.weight;
			source_node.calculate_data.append_translation.z	= translation.z * append.weight;
			is_update = true;
		}
	}
	if (append.is_rotation)
	{
		DirectX::XMVECTOR rotation = DirectX::XMVectorSet(0, 0, 0, 1);
		if( target_node.append_node_index != pmx_invalid_target_id )
		{
			rotation	= DirectX::XMLoadFloat4(&target_node.calculate_data.append_rotation);
		}
		else
		{
			rotation	= DirectX::XMQuaternionMultiply(rotation, DirectX::XMLoadFloat4(&target_node.calculate_data.append_rotation));
			rotation	= DirectX::XMQuaternionMultiply(rotation, DirectX::XMLoadFloat4(&target_node.calculate_data.animation_rotation));
		}

		rotation	= DirectX::XMQuaternionMultiply(rotation, DirectX::XMLoadFloat4(&target_node.calculate_data.ik_rotation));
		rotation	= DirectX::XMQuaternionMultiply(rotation, DirectX::XMLoadFloat4(&target_node.calculate_data.init_rotation));
		rotation	= DirectX::XMQuaternionNormalize(rotation);

		DirectX::XMVECTOR append_rotation = DirectX::XMLoadFloat4(&source_node.calculate_data.append_rotation);
		if (!DirectX::XMQuaternionEqual(append_rotation, rotation))
		{
			append_rotation = DirectX::XMQuaternionSlerp(DirectX::XMVectorSet(0, 0, 0, 1), rotation, append.weight);
			append_rotation = DirectX::XMQuaternionNormalize(append_rotation);
			DirectX::XMStoreFloat4(&source_node.calculate_data.append_rotation, append_rotation);
			is_update = true;
		}
	}

	if (!is_update)
		return;

	update_node_transforms(node_index);
}

void pmx_model::update_node_transforms(size_t node_index)
{
	auto& node = nodes[node_index];

	node.calculate_data.update_local_matrix();
	if (node.parent_node_index != pmx_invalid_target_id)
	{
		node.calculate_data.update_global_matrix(DirectX::XMLoadFloat4x4(&nodes[node.parent_node_index].calculate_data.global_matrix));
	}

	//	子供を更新していく
	for (int index = 0; index < nodes.size(); ++index)
	{
		auto& node = nodes[index];
		if(node.parent_node_index != node_index)
			continue;

		update_node_transforms(index);
	}
}

void pmx_model::update_transforms(std::vector<node_animation_data>& node_datas)
{
	//	ノード一旦更新
	for (size_t node_index = 0; node_index < nodes.size(); ++node_index)
	{
		auto& node = nodes[node_index];
		auto& node_data = node_datas[node_index];

		DirectX::XMMATRIX local, global;
		local	= DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&node_data.scale))
				* DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node_data.rotation))
				* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&node_data.translation));
		global = local;
		if (node.parent_node_index != pmx_invalid_target_id)
		{
			global = DirectX::XMLoadFloat4x4(&nodes[node.parent_node_index].global_matrix);
			global = DirectX::XMMatrixMultiply(local, global);
		}
		DirectX::XMStoreFloat4x4(&node.local_matrix, local);
		DirectX::XMStoreFloat4x4(&node.global_matrix, global);
	}
}

#if 0
void pmx_model::render(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4X4& previous_world, std::string_view override_pipeline_stae, bool wire)
{
	DirectX::XMFLOAT4X4 node_transforms[BONE_MAX]{};
	for (int index = 0; index < BONE_MAX && index < nodes.size(); ++index)
	{
		DirectX::XMMATRIX OffsetMatrix = DirectX::XMLoadFloat4x4(&nodes[index].offset_matrix);
		DirectX::XMMATRIX GlobalMatrix = DirectX::XMLoadFloat4x4(&nodes[index].global_matrix);
		DirectX::XMStoreFloat4x4(&node_transforms[index], OffsetMatrix * GlobalMatrix);
	}

	UINT stride = sizeof(vertex);
	UINT offset = 0;
	immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
	immediate_context->IASetIndexBuffer(index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//	パイプラインステートの上書き処理
	auto pipeline_state = pipeline_states.find(std::string(override_pipeline_stae));
	bool is_override = pipeline_state != pipeline_states.end();
	if (is_override)
	{
		immediate_context->VSSetShader(pipeline_state->second.vertex_shader.Get(), nullptr, 0);
		immediate_context->HSSetShader(pipeline_state->second.hull_shader.Get(), nullptr, 0);
		immediate_context->DSSetShader(pipeline_state->second.domain_shader.Get(), nullptr, 0);
		immediate_context->GSSetShader(pipeline_state->second.geometry_shader.Get(), nullptr, 0);
		immediate_context->PSSetShader(pipeline_state->second.pixel_shader.Get(), nullptr, 0);
		immediate_context->IASetInputLayout(pipeline_state->second.input_layout.Get());
		immediate_context->IASetPrimitiveTopology(pipeline_state->second.primitive_toporogy);
	}

	{
		std::string current_pipeline_state_name;
		for (auto& s : subsets)
		{
			//	マテリアルの情報を設定
			auto material = materials[s.material_index];

			//	シェーダー設定
			if (!is_override && current_pipeline_state_name != material.pipeline_state_name)
			{
				current_pipeline_state_name = material.pipeline_state_name;

				auto pipeline_state = pipeline_states.find(current_pipeline_state_name);
				if (pipeline_state == pipeline_states.end())
				{
					// シェーダーが見つからない場合はデフォルトを使用
					pipeline_state = pipeline_states.find(std::string(DefaultShaderName));
				}

				immediate_context->VSSetShader(pipeline_state->second.vertex_shader.Get(), nullptr, 0);
				immediate_context->HSSetShader(pipeline_state->second.hull_shader.Get(), nullptr, 0);
				immediate_context->DSSetShader(pipeline_state->second.domain_shader.Get(), nullptr, 0);
				immediate_context->GSSetShader(pipeline_state->second.geometry_shader.Get(), nullptr, 0);
				immediate_context->PSSetShader(pipeline_state->second.pixel_shader.Get(), nullptr, 0);
				immediate_context->IASetInputLayout(pipeline_state->second.input_layout.Get());
				immediate_context->IASetPrimitiveTopology(pipeline_state->second.primitive_toporogy);
			}

			//	デフォルト情報を設定
			{
				//	定数バッファを更新
				primitive_constants constant;
				constant.world = world;
				constant.previous_world = previous_world;
				constant.material_attribute = material.material_attribute;

				constant.texture_index.x = material.def.diffuse_texture_index == 0xFFFFFFFF ? -1 : static_cast<int>(material.def.diffuse_texture_index);
				constant.texture_index.y = material.def.sphere_texture_index == 0xFFFFFFFF ? -1 : static_cast<int>(material.def.sphere_texture_index);
				constant.texture_index.z = material.def.ramp_texture_index == 0xFFFFFFFF ? -1 : static_cast<int>(material.def.ramp_texture_index);
				constant.texture_index.w = material.def.shadow_threshold_texture_index == 0xFFFFFFFF ? -1 : static_cast<int>(material.def.shadow_threshold_texture_index);

				constant.diffuse_color = material.def.diffuse_color;
				constant.specular_color = material.def.specular_color;
				constant.edge_color = material.def.edge_color;

				constant.edge_size = material.def.edge_size;
				constant.alpha_clip_threshold = material.def.alpha_clip_threshold;
				constant.alpha_dither_factor = material.def.alpha_dither_factor;

				constant.color_diff_line_factor = material.def.color_diff_line_factor;
				constant.color_diff_line_threshold = material.def.color_diff_line_threshold;
				constant.color_line_adjust_saturate = material.def.color_line_adjust_saturate;
				constant.color_line_adjust_brightness = material.def.color_line_adjust_brightness;

				constant.color_posterize_factor = material.def.color_posterize_factor;
				constant.depth_normal_outline_factor = material.def.depth_normal_outline_factor;
				constant.shadow_threshold_texcoord_offset = material.def.shadow_threshold_texcoord_offset;

				memcpy_s(constant.node_transforms, sizeof(constant.node_transforms), node_transforms, sizeof(node_transforms));
				immediate_context->UpdateSubresource(primitive_cbuffer.Get(), 0, NULL, &constant, 0, 0);
				immediate_context->VSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->HSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->DSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->GSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->PSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());

				//	シェーダーリソースを設定
				ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr, nullptr, nullptr };
				if (material.def.diffuse_texture_index != 0xFFFFFFFF)
				{
					srvs[0] = textures[material.def.diffuse_texture_index]->get_shader_resource_view().Get();
				}
				if (material.def.sphere_texture_index != 0xFFFFFFFF)
				{
					srvs[1] = textures[material.def.sphere_texture_index]->get_shader_resource_view().Get();
				}
				if (material.def.ramp_texture_index != 0xFFFFFFFF)
				{
					if ((material.def.ramp_texture_index & UseDefaultRamp) == UseDefaultRamp)
					{
						size_t texture_index = (material.def.ramp_texture_index & ~UseDefaultRamp);
						srvs[2] = default_toon_textures[texture_index]->get_shader_resource_view().Get();
					}
					else
					{
						srvs[2] = textures[material.def.ramp_texture_index]->get_shader_resource_view().Get();
					}
				}
				else
				{
					//	0番を設定しておく
					srvs[2] = default_toon_textures[0]->get_shader_resource_view().Get();
				}
				if (material.def.shadow_threshold_texture_index != 0xFFFFFFFF)
				{
					srvs[3] = textures[material.def.shadow_threshold_texture_index]->get_shader_resource_view().Get();
				}
				immediate_context->VSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
				immediate_context->HSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
				immediate_context->DSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
				immediate_context->GSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
				immediate_context->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
				immediate_context->VSSetSamplers(0, 1, sampler_state.GetAddressOf());
				immediate_context->HSSetSamplers(0, 1, sampler_state.GetAddressOf());
				immediate_context->DSSetSamplers(0, 1, sampler_state.GetAddressOf());
				immediate_context->GSSetSamplers(0, 1, sampler_state.GetAddressOf());
				immediate_context->PSSetSamplers(0, 1, sampler_state.GetAddressOf());
			}

			//	両面描画等の設定
			{
				bool is_both_face = (material.draw_flag & static_cast<unsigned char>(pmx_material_draw_bitflag::cull_both)) == static_cast<unsigned char>(pmx_material_draw_bitflag::cull_both);
				immediate_context->RSSetState(rasterizer_states[(is_both_face ? 1 : 0) + wire * 2].Get());
			}
			immediate_context->DrawIndexed(static_cast<UINT>(s.count), static_cast<UINT>(s.start), 0);
		}
	}

	immediate_context->VSSetShader(nullptr, nullptr, 0);
	immediate_context->HSSetShader(nullptr, nullptr, 0);
	immediate_context->DSSetShader(nullptr, nullptr, 0);
	immediate_context->GSSetShader(nullptr, nullptr, 0);
	immediate_context->PSSetShader(nullptr, nullptr, 0);
	immediate_context->IASetInputLayout(nullptr);
}
#endif

bool pmx_model::add_vmd_animation(const char* motion_name, const char* filepath)
{
	//	ファイル読み込み
	vmd_loader loader;
	{
		std::filesystem::path path = filepath;
		std::string directory = path.parent_path().string();

		std::ifstream	read_file;
		read_file.open(path, std::ios::in | std::ios::binary);
		_ASSERT_EXPR_A(read_file, L"Failed to load file");

		std::string	file_buffer;
		file_buffer = std::string(std::istreambuf_iterator<char>(read_file), std::istreambuf_iterator<char>());

		bool succeeded = loader.load(file_buffer.data(), static_cast<unsigned int>(file_buffer.size()));
		_ASSERT_EXPR_A(succeeded, L"Failed to load file");
		if (!succeeded)
			return false;
	}
	return add_vmd_animation(motion_name, loader);
}

bool pmx_model::add_vmd_animation(const char* motion_name, const vmd_loader& loader)
{
	animation animation;
	animation.end_time = loader.get_end_frame() / normalized_frame;
	animation.name = motion_name;
	animation.nodes.resize(nodes.size());
	for (auto& bone_animation : loader.get_bone())
	{
		int node_index = pmx_invalid_target_id;
		for (int index = 0; index < nodes.size(); ++index)
		{
			if (nodes[index].name == bone_animation.name)
			{
				node_index = index;
				break;
			}
		}

		if (node_index == pmx_invalid_target_id)
			continue;

		auto& node = nodes[node_index];
		node_animation anim;
		anim.time = bone_animation.frame / normalized_frame;
		DirectX::XMMATRIX animation_matrix;
		animation_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&bone_animation.rotation))
						* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&bone_animation.position));
		DirectX::XMVECTOR scale, rotation, translation;
		DirectX::XMMatrixDecompose(&scale, &rotation, &translation, animation_matrix);
		DirectX::XMStoreFloat4(&anim.rotation, DirectX::XMQuaternionNormalize(rotation));
		DirectX::XMStoreFloat3(&anim.translation, translation);
		animation.nodes[node_index].emplace_back(anim);
	}
	//	ソート
	for (auto& animes : animation.nodes)
	{
		std::sort(animes.begin(), animes.end(), [&](const node_animation& l, const node_animation& r) {return l.time < r.time; });
	}
	animations.insert_or_assign(animation.name, animation);
	return true;
}

UINT pmx_model::add_texture(ID3D11Device* device, const char* filepath)
{
	//	テクスチャ読み込み
	texture_ptr texture;
	if (texture_manager::instance()->load_texture_from_file(texture, filepath))
	{
		textures.resize(textures.size() + 1);
		textures.back() = texture;
		return static_cast<UINT>(textures.size()) - 1;
	}
	return 0xFFFFFFFF;
}

std::vector<std::string> pmx_model::get_animation_names()
{
	std::vector<std::string> names;
	names.reserve(animations.size());
	for (auto& it : animations)
	{
		names.push_back(it.first);
	}
	return names;
}

pmx_model::animation* pmx_model::search_animation(std::string_view name)
{
	auto it	= animations.find(std::string(name));
	if(it == animations.end())
		return nullptr;

	return &it->second;
}

pmx_model::node* pmx_model::search_node(std::string_view name)
{
	auto it	= std::find_if(nodes.begin(), nodes.end(), [&](const node& l){ return l.name == std::string(name); });
	if(it == nodes.end())
		return nullptr;

	return &(*it);
}
