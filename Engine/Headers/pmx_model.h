#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

#include <vector>
#include <unordered_map>

#include "texture.h"

#include "pmx_loader.h"
#include "vmd_loader.h"


class pmx_model
{
	static constexpr size_t UseDefaultRamp = 1 << 16;
	static constexpr const char* DefaultShaderName = "pmx_default_shader";
	enum class plane_lock
	{
		free,
		lock,
		plane_x,
		plane_y,
		plane_z,
	};

	struct calculate_node_data
	{
		DirectX::XMFLOAT3 init_scale{ 1, 1, 1 };
		DirectX::XMFLOAT4 init_rotation{ 0, 0, 0, 1 };
		DirectX::XMFLOAT3 init_translation{ 0, 0, 0 };

		DirectX::XMFLOAT4X4 initialize_matrix;

		DirectX::XMFLOAT3 animation_scale{ 1, 1, 1 };
		DirectX::XMFLOAT4 animation_rotation{ 0, 0, 0, 1 };
		DirectX::XMFLOAT3 animation_translation{ 0, 0, 0 };

		DirectX::XMFLOAT4 append_rotation{ 0, 0, 0, 1 };
		DirectX::XMFLOAT3 append_translation{ 0, 0, 0 };

		DirectX::XMFLOAT4 ik_rotation{ 0, 0, 0, 1 };

		DirectX::XMFLOAT3 scale{ 1, 1, 1 };
		DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 };
		DirectX::XMFLOAT3 position{ 0, 0, 0 };

		DirectX::XMFLOAT4X4 locll_matrix{ 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
		DirectX::XMFLOAT4X4 global_matrix{ 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };

		void begin();
		void update_local_matrix();
		void update_global_matrix(DirectX::XMMATRIX parent_global_matrix);
	};

public:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;
		DirectX::XMFLOAT4 additional_uv[4];
		DirectX::XMUINT4  bone_indices;
		DirectX::XMFLOAT4 bone_weights;
	};

	struct subset
	{
		size_t start;
		size_t count;
		size_t material_index;
	};

	//  マテリアル属性(32ビットまで利用可能)
	enum material_attribute
	{
		indirect_lighting_off,				//	間接光の処理を適応させない

		reserve_1,							//	予約領域
		reserve_2,							//	予約領域
		reserve_3,							//	予約領域
		reserve_4,							//	予約領域
		reserve_5,							//	予約領域
		reserve_6,							//	予約領域
		reserve_7,							//	予約領域
		reserve_8,							//	予約領域
		reserve_9,							//	予約領域
		reserve_10,							//	予約領域
		reserve_11,							//	予約領域
		reserve_12,							//	予約領域
		reserve_13,							//	予約領域
		reserve_14,							//	予約領域
		reserve_15,							//	予約領域
		reserve_16,							//	予約領域
		reserve_17,							//	予約領域
		reserve_18,							//	予約領域
		reserve_19,							//	予約領域

		filter_off,							//	フィルター効果の適応フラグ
		normal_depth_outline_off,			//	深度/法線による法線描画フラグ
		color_line_off,						//	色によるライン描画フラグ
		noise_off,							//	英人特有の影部分に対してのノイズ描画フラグ
		color_postarize_off,				//	色のポスタライズ適応フラグ(ただし、カラーラインの方はポスタライズを元にして検知しているので注意)
		face_shadow_on,						//	顔用シャドウ処理適応フラグ
		flow_face_shadow_to_contact_shadow,	//	顔用シャドウをシェーディングからコンタクトシャドウに持っていく
		contact_shadow_off,					//	コンタクトシャドウ適応フラグ
		contact_drop_shadow_recever,		//	コンタクトシャドウのドロップシャドウブロッカーを元に影を出す
		contact_drop_shadow_caster,			//	コンタクトシャドウをドロップシャドウブロッカーフラグ

		reserve_30,							//	予約領域
		reserve_31,							//	予約領域
	};

	struct material
	{
		std::string name;
		std::string pipeline_state_name = DefaultShaderName;
		unsigned char draw_flag;
		union
		{
			//	もう既にデフォルトではなくなっているのでどこかで分離が必要
			struct
			{
				UINT diffuse_texture_index;
				UINT sphere_texture_index;
				UINT ramp_texture_index;
				UINT shadow_threshold_texture_index;
				DirectX::XMFLOAT4 diffuse_color;
				DirectX::XMFLOAT4 specular_color;
				DirectX::XMFLOAT4 edge_color;

				float edge_size;
				float alpha_clip_threshold;
				float alpha_dither_factor;

				//  ポストエフェクトの色差によるライン算出係数
				float color_diff_line_factor;		//	アウトライン係数
				float color_diff_line_threshold;	//	検知係数
				float color_line_adjust_saturate;	//  ライン彩度調整係数
				float color_line_adjust_brightness;	//  ライン明度調整係数

				//  色のポスタライズ数
				float color_posterize_factor;
				//  ポストエフェクトの法線/深度によるアウトライン算出係数
				float depth_normal_outline_factor;

				//	影閾値マップのオフセット
				DirectX::XMFLOAT2 shadow_threshold_texcoord_offset;
				float dummy_2;

				DirectX::XMFLOAT4 outline_adjust_parameter;
			} def;
		};

		UINT material_attribute = 0;	//	シェーダー側に引き渡すフラグ類

		bool IsFlags(UINT bit_flag)
		{
			return material_attribute & (1 << bit_flag);
		}
		void SetFlags(UINT bit_flag, bool flag)
		{
			if (flag)
				material_attribute |= (1 << bit_flag);
			else
				material_attribute &= ~(1 << bit_flag);
		}
	};

	struct node
	{
		friend class pmx_model;

		std::string name;
		size_t level = 0;
		size_t parent_node_index = pmx_invalid_target_id;
		size_t append_node_index = pmx_invalid_target_id;
		size_t ik_node_index = pmx_invalid_target_id;

		DirectX::XMFLOAT4X4 offset_matrix;
		DirectX::XMFLOAT4X4 local_matrix{ 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
		DirectX::XMFLOAT4X4 global_matrix{ 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };

		calculate_node_data calculate_data;
	};

	struct append_data
	{
		size_t source_index;
		size_t target_index;
		float weight;
		bool is_rotation;
		bool is_translation;
	};

	struct ik_link_data
	{
		size_t target_index;
		plane_lock lock = plane_lock::free;
		DirectX::XMFLOAT3 angle_max{ 1, 1, 1 };
		DirectX::XMFLOAT3 angle_min{ 1, 1, 1 };

		float plane_mode_angle = 0;
		DirectX::XMFLOAT3 prev_angle = {0, 0, 0};
	};

	struct ik_data
	{
		size_t source_index = -1;
		size_t target_index = -1;

		int loop;
		float angile_limit;
		size_t ik_link_start = -1;
		size_t ik_link_end = -1;

		bool enable;
	};
	struct node_animation
	{
		float time;
		//	MMDに拡縮は無い。
		DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 };
		DirectX::XMFLOAT3 translation{ 0, 0, 0 };
	};

	struct animation
	{
		//	MMDは30FPSを基準として作られている。
		std::string name;
		float end_time;
		std::vector<std::vector<node_animation>>	nodes;
	};

#define BONE_MAX 1000
	struct primitive_constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 previous_world;
		DirectX::XMINT4 texture_index;
		DirectX::XMFLOAT4 diffuse_color;
		DirectX::XMFLOAT4 specular_color;
		DirectX::XMFLOAT4 edge_color;

		float edge_size;
		float alpha_clip_threshold = 0.01f;
		float alpha_dither_factor = 0.5f;
		UINT material_attribute;

		//  ポストエフェクトの色差によるライン算出係数
		float color_diff_line_factor;
		float color_diff_line_threshold;
		float color_line_adjust_saturate;
		float color_line_adjust_brightness;

		//  色のポスタライズ数
		float color_posterize_factor;
		//  ポストエフェクトの法線/深度によるアウトライン算出係数
		float depth_normal_outline_factor;

		//	影閾値マップのオフセット(本来はテクスチャに合わせて作成するのでいらない。これはサンプルをそのまま使うための物。)
		DirectX::XMFLOAT2 shadow_threshold_texcoord_offset;

		DirectX::XMFLOAT4 outline_adjust_parameter;

		DirectX::XMFLOAT4X4 node_transforms[BONE_MAX];
	};

public:
	pmx_model(ID3D11Device* device, const std::string& filename);
	~pmx_model();

	struct node_animation_data
	{
		DirectX::XMFLOAT3 scale{1, 1, 1};
		DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 };
		DirectX::XMFLOAT3 translation{ 0, 0, 0 };
	};
	void compute_animation(std::string_view animation_name, float time, std::vector<node_animation_data>& node_datas);
	void update_transforms(std::vector<node_animation_data>& node_datas);
//	void render(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4X4& previous_world, std::string_view override_pipeline_stae = "", bool wire = false);

	bool add_vmd_animation(const char* motion_name, const char* filepath);
	bool add_vmd_animation(const char* motion_name, const vmd_loader& loader);
	UINT add_texture(ID3D11Device* device, const char* filepath);

	std::vector<std::string> get_animation_names();
	animation* search_animation(std::string_view name);
	node* search_node(std::string_view name);
	const std::vector<node>& get_nodes() const { return nodes; }
	const std::vector<subset>& get_subsets() const { return subsets; }
	const std::vector<vertex>& get_vertices() const { return vertices; }
	const std::vector<UINT>& get_indices() const { return indices; }
	const std::vector<material>& get_materials() const { return materials; }
	const std::vector<append_data>& get_append_datas() const { return append_datas; }
	const std::vector<ik_data>& get_ik_datas() const { return ik_datas; }
	const std::vector<ik_link_data>& get_ik_link_datas() const { return ik_link_datas; }
	const std::vector<std::string>& get_texture_names() const { return texture_names; }
	const std::vector<texture_ptr>& get_textures() const { return textures; }
	const std::unordered_map<std::string, animation>& get_animations() const { return animations; }

private:
	void update_ik(size_t node_index);
	void update_ik_plane(int iteration, ik_data& ik_data, ik_link_data& link_data, plane_lock axis);
	void update_node_transforms(size_t node_index);

	void update_append(size_t node_index);

private:
	std::vector<node> nodes;
	std::vector<subset> subsets;
	std::vector<size_t> levels;
	std::vector<vertex> vertices;
	std::vector<UINT> indices;
	std::vector<material> materials;
	std::vector<append_data> append_datas;
	std::vector<ik_data> ik_datas;
	std::vector<ik_link_data> ik_link_datas;

	std::unordered_map<std::string, animation> animations;

	std::vector<std::string> texture_names;
	std::vector<texture_ptr> textures;
	std::vector<texture_ptr> default_toon_textures;
	texture_ptr dummy_texture;
};
