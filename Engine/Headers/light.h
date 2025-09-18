#pragma once

#include "light_data.h"
#include "common_pipeline_states.h"
#include "texture.h"

#include "system_utility.h"
#include "math.h"
#include "math_utility.h"

//	ライト情報
class light
{
public:
	enum class flags
	{
		active,
		use_shadow,
		contact_shadow,

		debug_csm_area_flag = 20,
		debug_visualize_ailiasing_error_flag = 21,
	};

	struct	shadow_map_bias_data
	{
		float depth_bias = 0.5f;
		float slope_depth_bias_scale = 0.5f;

		float max_depth_bias = 0.1f;
		float slope_depth_bias = 3.0f;
		float max_slope_depth_bias = 1.0f;
		float sigma_factor = 5.0f;
	};

public:
	light(light_type type);
	~light();

	UINT get_attribute() const { return system_flags; }
	bool is_flag(flags flag) const { return system_utility::is_bitflag(system_flags, 1 << static_cast<UINT>(flag)); }
	void set_flag(flags flag, bool value){ system_utility::set_bitflag(system_flags, 1 << static_cast<UINT>(flag), value); }

	light_data convert_light_data() const;
	light_type get_light_type() const;
	DirectX::XMFLOAT3 get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);
	const math::rotator& get_rotation() const;
	void set_rotation(const math::rotator& rotation);
	const DirectX::XMFLOAT4& get_color() const;
	void set_color(const DirectX::XMFLOAT4& color);

	float get_range() const;
	void set_range(float range);
	float get_inner_cone() const;
	void set_inner_cone(float inner_cone);
	float get_outer_cone() const;
	void set_outer_cone(float outer_cone);

	//	影関係
	const DirectX::XMFLOAT3& get_shadow_attenuation() const;
	void set_shadow_attenuation(const DirectX::XMFLOAT3& attenuation);
	float get_csm_distance_factor() const;
	void set_csm_distance_factor(float factor);
	float get_pssm_split_weight() const;
	void set_pssm_split_weight(float weight);
	const shadow_map_bias_data& get_shadow_bias(int index) const;
	void set_shadow_bias(int index, const shadow_map_bias_data& bias_data);

private:
	UINT system_flags;

	//	ライトパラメーター
	light_type type;
	DirectX::XMFLOAT3 position;
	math::rotator rotation;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT3 parameter;

	//	影関係
	float csm_distance_factor; // 平行光源専用
	float pssm_split_weight; // 平行光源専用
	DirectX::XMFLOAT3 shadow_attenuation;
	shadow_map_bias_data bias_data[common_pipeline_states::shadow_map_max];
};
ENUM_ATTR_BITFLAG(light::flags);

//	ライトプローブ情報
class light_probe
{
public:
	enum class flags
	{
		active,	//	アクティブ状態かどうか
		unbind,	//	範囲制限をかけるかどうか(かけない場合は全画面)
	};

public:
	light_probe();
	~light_probe();

	UINT get_attribute() const { return system_flags; }
	bool is_flag(flags flag) const { return system_utility::is_bitflag(system_flags, 1 << static_cast<UINT>(flag)); }
	void set_flag(flags flag, bool value){ system_utility::set_bitflag(system_flags, 1 << static_cast<UINT>(flag), value); }
	texture_ptr get_diffuse_ibl_texture() const;
	texture_ptr get_specular_ibl_texture() const;
	texture_ptr get_lut_ggx_texture() const;
	void set_diffuse_ibl_texture(texture_ptr texture);
	void set_specular_ibl_texture(texture_ptr texture);
	void set_lut_ggx_texture(texture_ptr texture);

	DirectX::XMFLOAT3 get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);
	DirectX::XMFLOAT3 get_inner_range() const;
	void set_inner_range(const DirectX::XMFLOAT3& inner_range);
	DirectX::XMFLOAT3 get_outer_range() const;
	void set_outer_range(const DirectX::XMFLOAT3& outer_range);
	const DirectX::XMFLOAT4& get_color() const;
	void set_color(const DirectX::XMFLOAT4& color);

private:
	UINT system_flags;
	texture_ptr diffuse_texture;
	texture_ptr specular_texture;
	texture_ptr lut_ggx_texture;

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 inner_range;
	DirectX::XMFLOAT3 outer_range;
	DirectX::XMFLOAT4 color;
};
ENUM_ATTR_BITFLAG(light_probe::flags);

//	影専用ライト情報
class shadow_light
{
public:
	enum class types
	{
		point,
		spot,
	};
	enum class flags
	{
		active,	//	アクティブ状態かどうか
	};

public:
	shadow_light(types type);
	~shadow_light();

	UINT get_attribute() const { return system_flags; }
	bool is_flag(flags flag) const { return system_utility::is_bitflag(system_flags, 1 << static_cast<UINT>(flag)); }
	void set_flag(flags flag, bool value) { system_utility::set_bitflag(system_flags, 1 << static_cast<UINT>(flag), value); }

	types get_type() const;
	DirectX::XMFLOAT3 get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);
	const math::rotator& get_rotation() const;
	void set_rotation(const math::rotator& rotation);

	const DirectX::XMFLOAT4& get_color() const;
	void set_color(const DirectX::XMFLOAT4& color);

	float get_range() const;
	void set_range(float range);
	float get_cone() const;
	void set_cone(float cone);
	float get_shadow_bias() const;
	void set_shadow_bias(float bias);
	float get_contact_shadow() const;
	void set_contact_shadow(float contact_shadow);

	UINT get_shadow_mask() const;
	bool is_shadow_flag(common_pipeline_states::shadow_category flag) const { return system_utility::is_bitflag(mask, 1 << static_cast<UINT>(flag)); }
	void set_shadow_flag(common_pipeline_states::shadow_category flag, bool value) { system_utility::set_bitflag(mask, 1 << static_cast<UINT>(flag), value); }

private:
	types type;
	UINT system_flags;
	DirectX::XMFLOAT3 position;
	math::rotator rotation;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT3 parameter;
	float contact_shadow;
	UINT mask;
};
ENUM_ATTR_BITFLAG(shadow_light::flags);

using light_ptr = std::shared_ptr<light>;
using light_probe_ptr = std::shared_ptr<light_probe>;
using shadow_light_ptr = std::shared_ptr<shadow_light>;

//	ライトコンテナ
struct light_container
{
	std::vector<light_ptr> lights;
	std::vector<light_probe_ptr> light_probes;
	std::vector<shadow_light_ptr> shadow_lights;
};

//	ライト管理クラス
class light_manager
{
	SINGLETON(light_manager);

private:
	light_manager();
	~light_manager();

public:
	bool generate_light(light_ptr& out_light, light_type type);
	bool generate_light_probe(light_probe_ptr& out_light_probe);
	bool generate_shadow_light(shadow_light_ptr& out_shadow_light);
	void get_container(light_container& container);

private:
	std::vector<std::weak_ptr<light>> lights;
	std::vector<std::weak_ptr<light_probe>> light_probes;
	std::vector<std::weak_ptr<shadow_light>> shadow_lights;
};
