#pragma once

#include "system_utility.h"

#include "texture.h"
#include "pipeline_state.h"

//	マテリアル
//	☆ﾃｷﾄｰ('ω')ｰﾄｷﾃ☆
class material
{
public:
	struct parameter
	{
		enum class type : std::uint8_t
		{
			Unknown,
			Int,
			Int2,
			Int3,
			Int4,
			UInt,
			UInt2,
			UInt3,
			UInt4,
			Float,
			Float2,
			Float3,
			Float4,
			Float4x4,
		};
		type Type;
		union data
		{
			int Int;
			DirectX::XMINT2 Int2;
			DirectX::XMINT3 Int3;
			DirectX::XMINT4 Int4;
			UINT UInt;
			DirectX::XMUINT2 UInt2;
			DirectX::XMUINT3 UInt3;
			DirectX::XMUINT4 UInt4;
			float Float;
			DirectX::XMFLOAT2 Float2;
			DirectX::XMFLOAT3 Float3;
			DirectX::XMFLOAT4 Float4;
			DirectX::XMFLOAT4X4 Float4x4;
		} Data;
		template<class Archive>
		void serialize(Archive& archive, int version);
	};
	enum class flags : std::uint8_t
	{
		shadow_caster,						//	影を生成するマテリアルかどうか
		shadow_receiver,					//	影を受けるマテリアルかどうか
		two_sided,							//	両面描画フラグ
		indirect_lighting_off,				//	間接光の影響を強制的に受けなくさせる

		use_reverse_outline,				//	背面法によるアウトラインを使用するかどうか
		filter_off,							//	フィルター効果の適応フラグ
		normal_depth_outline_off,			//	深度/法線による法線描画フラグ
		color_line_off,						//	色によるライン描画フラグ
		noise_off,							//	英人特有の影部分に対してのノイズ描画フラグ
		color_postarize_off,				//	色のポスタライズ適応フラグ(ただし、カラーラインの方はポスタライズを元にして検知しているので注意)
		light_direction_ignore_y,			//	シェーディング時にライトのYを無視する
		contact_shadow_on,					//	コンタクトシャドウ適応フラグ
		camera_distance_translucent,		//	カメラからの距離に応じて透過するフラグ

		character_flag,						//	キャラクター識別フラグ(後で何かしら処理するかも？)

		//	最大32個まで
	};
	enum class domains : std::uint8_t
	{
		surface,		//	面描画
		postprocess,	//	ポストプロセス

		unknown,
	};
	enum class blend_modes : std::uint8_t
	{
		opaque,			//	不透明描画
		mask,			//	不透明描画+アルファマスク

		translucent,	//	半透明描画(現状UI用)
	};
	enum class surface_paths : std::uint8_t
	{
		depth_write_pass,	//	深度書き込みパス
		g_buffer_pass,		//	G-Bufferパス
		shadow_write_pass,	//	シャドウマップ用パス
	//	forward_pass,		//	フォワードレンダリング

		ui_pass,			//	UI用パス
	};
	enum class postprcess_paths : std::uint8_t
	{
		A,
		B,
	};
	struct pipeline_state
	{
		std::string name{ "" };
		pipeline_state_ptr value{ nullptr };
		template<class Archive>
		void serialize(Archive& archive, int version);
	};
	struct texture
	{
		bool is_file_path_or_key{false};
		std::string path{ "" };
		texture_ptr value{ nullptr };
		template<class Archive>
		void serialize(Archive& archive, int version);
	};
	struct physical_datas
	{
		float friction{ 0.5f };
		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	using pipeline_state_array = std::vector<pipeline_state>;
	using parameter_map = std::unordered_map<std::string, parameter>;
	using texture_map = std::unordered_map<std::string, texture>;

public:
	material();
	material(std::string_view name);
	~material();

	template<class Archive>
	void serialize(Archive& archive, int version);

	void duplicate(material& material) const;

	void build();

public:
	//	名前取得・設定
	void set_name(std::string_view name);
	std::string_view get_name() const;

	//	ドメイン設定
	domains get_domain() const;
	void set_domain(domains domain);

	//	ブレンドモード設定
	blend_modes get_blend_mode() const;
	void set_blend_mode(blend_modes blend_mode);

	//	ブレンドモード設定
	parameter_map& get_parameters();

	//	パイプラインステート取得・設定
	bool set_pipeline_state(surface_paths type, std::string_view pipeline_state_name) { return set_pipeline_state(domains::surface, static_cast<UINT>(type), pipeline_state_name); }
	bool set_pipeline_state(postprcess_paths type, std::string_view pipeline_state_name) { return set_pipeline_state(domains::postprocess, static_cast<UINT>(type), pipeline_state_name); }
	std::string_view get_pipeline_state_name(surface_paths type) const { return get_pipeline_state_name(domains::surface, static_cast<UINT>(type)); }
	std::string_view get_pipeline_state_name(postprcess_paths type) const { return get_pipeline_state_name(domains::postprocess, static_cast<UINT>(type)); }
	pipeline_state_ptr get_pipeline_state(surface_paths value) const { return get_pipeline_state(domains::surface, static_cast<UINT>(value)); }
	pipeline_state_ptr get_pipeline_state(postprcess_paths value) const { return get_pipeline_state(domains::postprocess, static_cast<UINT>(value)); }

	//	システムフラグ類設定
	bool is_flag(flags flag) const { return system_utility::is_bitflag(system_flags, 1 << static_cast<UINT>(flag)); }
	void set_flag(flags flag, bool value) { system_utility::set_bitflag(system_flags, 1 << static_cast<UINT>(flag), value); }

	//	アルファクリップの敷地設定
	float get_alpha_clip_threshold() const { return alpha_clip_threshold; }
	void set_alpha_clip_threshold(float value){ alpha_clip_threshold = value; }

	//	物理マテリアルの設定
	inline const physical_datas& get_physical_data() const { return physical_data; }
	void set_physical_data(const physical_datas& value) { physical_data = value; }

	//	パラメーター設定
	bool get_int(std::string_view key_name, int& value) const;
	bool get_int2(std::string_view key_name, DirectX::XMINT2& value) const;
	bool get_int3(std::string_view key_name, DirectX::XMINT3& value) const;
	bool get_int4(std::string_view key_name, DirectX::XMINT4& value) const;
	void set_int(std::string_view key_name, int value);
	void set_int2(std::string_view key_name, DirectX::XMINT2 value);
	void set_int3(std::string_view key_name, DirectX::XMINT3 value);
	void set_int4(std::string_view key_name, DirectX::XMINT4 value);

	void set_uint(std::string_view key_name, UINT value);
	void set_uint2(std::string_view key_name, DirectX::XMUINT2 value);
	void set_uint3(std::string_view key_name, DirectX::XMUINT3 value);
	void set_uint4(std::string_view key_name, DirectX::XMUINT4 value);
	bool get_uint(std::string_view key_name, UINT& value) const;
	bool get_uint2(std::string_view key_name, DirectX::XMUINT2& value) const;
	bool get_uint3(std::string_view key_name, DirectX::XMUINT3& value) const;
	bool get_uint4(std::string_view key_name, DirectX::XMUINT4& value) const;

	void set_float(std::string_view key_name, float value);
	void set_float2(std::string_view key_name, DirectX::XMFLOAT2 value);
	void set_float3(std::string_view key_name, DirectX::XMFLOAT3 value);
	void set_float4(std::string_view key_name, DirectX::XMFLOAT4 value);
	bool get_float(std::string_view key_name, float& value) const;
	bool get_float2(std::string_view key_name, DirectX::XMFLOAT2& value) const;
	bool get_float3(std::string_view key_name, DirectX::XMFLOAT3& value) const;
	bool get_float4(std::string_view key_name, DirectX::XMFLOAT4& value) const;

	void set_float4x4(std::string_view key_name, DirectX::XMFLOAT4X4 value);
	bool get_float4x4(std::string_view key_name, DirectX::XMFLOAT4X4& value) const;

	bool remove_parameter(std::string_view key_name);

	void set_texture(std::string_view key_name, texture_ptr value);
	bool get_texture(std::string_view key_name, texture_ptr& value) const;

private:
	template<typename enum_type>
	inline domains tag_domains(enum_type path){ (void)path; return domains::unknown; }
	template<>
	inline domains tag_domains<surface_paths>(surface_paths path){ (void)path; return domains::surface; }
	template<>
	inline domains tag_domains<postprcess_paths>(postprcess_paths path){ (void)path; return domains::postprocess; }

	bool set_pipeline_state(domains domain, UINT index, std::string_view pipeline_state_name);
	std::string_view get_pipeline_state_name(domains domain, UINT index) const;
	pipeline_state_ptr get_pipeline_state(domains domain, UINT index) const;

	void serialize_stream(std::stringstream& ostream) const;
	void deserialize_stream(std::stringstream& istream);

private:
	std::string name;
	domains domain;
	blend_modes blend_mode;

	physical_datas physical_data;

	UINT system_flags;
	float alpha_clip_threshold;

	pipeline_state_array pipeline_states;
	texture_map textures;
	parameter_map parameters;
};
