#pragma once

#include "collision/collision.h"

//	ライトオブジェクト
class light_object : public game_object
{
	GAME_OBJECT_DECRLEAR(light_object, game_object)

public:
	light_ptr get_light() const;

	void set_flag(light::flags flag, bool value);
	bool is_flag(light::flags flag) const;

	void set_color(DirectX::XMFLOAT4 color);
	DirectX::XMFLOAT4 get_color() const;

	void set_position(DirectX::XMFLOAT3 position);
	DirectX::XMFLOAT3 get_position() const;

public:
	light_object(std::string_view name);
	~light_object() override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
	virtual void imgui_tree_inner_render() = 0;
#endif	//	defined(USE_IMGUI)

protected:
	void on_active() override;
	void on_deactive() override;

protected:
	light_ptr light;
};

//	平行光源オブジェクト
class directional_light_object : public light_object
{
	GAME_OBJECT_DECRLEAR(directional_light_object, light_object)

public:
	void set_rotation(const math::rotator& rotation);
	const math::rotator& get_rotation() const;

public:
	directional_light_object(std::string_view name);
	~directional_light_object() override;

#if defined(USE_IMGUI)
	void imgui_tree_inner_render() override;
#endif	//	defined(USE_IMGUI)
};

//	点光源オブジェクト
class point_light_object : public light_object
{
	GAME_OBJECT_DECRLEAR(point_light_object, light_object)

public:
	void set_range(float radius);
	float get_range() const;

public:
	point_light_object(std::string_view name);
	~point_light_object() override;

#if defined(USE_IMGUI)
	void imgui_tree_inner_render() override;
#endif	//	defined(USE_IMGUI)
};

//	スポットライトオブジェクト
class spot_light_object : public light_object
{
	GAME_OBJECT_DECRLEAR(spot_light_object, light_object)

public:
	void set_rotation(const math::rotator& rotation);
	const math::rotator& get_rotation() const;

	void set_range(float radius);
	float get_range() const;

	void set_inner_cone(float degree);
	float get_inner_cone() const;

	void set_outer_cone(float degree);
	float get_outer_cone() const;

public:
	spot_light_object(std::string_view name);
	~spot_light_object() override;

#if defined(USE_IMGUI)
	void imgui_tree_inner_render() override;
#endif	//	defined(USE_IMGUI)
};

//	ライトプローブオブジェクト
class light_probe_object : public game_object
{
	GAME_OBJECT_DECRLEAR(light_probe_object, game_object)

public:
	light_probe_ptr get_light_probe() const;

	void set_flag(light_probe::flags flag, bool value);
	bool is_flag(light_probe::flags flag) const;

	void set_position(DirectX::XMFLOAT3 position);
	DirectX::XMFLOAT3 get_position() const;

	void set_color(DirectX::XMFLOAT4 color);
	DirectX::XMFLOAT4 get_color() const;

	void set_diffuse_ibl_texture(texture_ptr texture);
	texture_ptr get_diffuse_ibl_texture() const;

	void set_specular_ibl_texture(texture_ptr texture);
	texture_ptr get_specular_ibl_texture() const;

	void set_lut_ggx_texture(texture_ptr texture);
	texture_ptr get_lut_ggx_texture() const;


public:
	light_probe_object(std::string_view name);
	~light_probe_object() override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
	virtual void imgui_tree_inner_render() = 0;
#endif	//	defined(USE_IMGUI)

protected:
	void on_active() override;
	void on_deactive() override;

protected:
	light_probe_ptr light_probe;
};

//	スカイライトオブジェクト
class sky_light_object : public light_probe_object
{
	GAME_OBJECT_DECRLEAR(sky_light_object, light_probe_object)

public:
	sky_light_object(std::string_view name);
	~sky_light_object() override;

#if defined(USE_IMGUI)
	void imgui_tree_inner_render() override;
#endif	//	defined(USE_IMGUI)
};

//	範囲制限有りライトプローブオブジェクト
class area_light_probe_object : public light_probe_object
{
	GAME_OBJECT_DECRLEAR(area_light_probe_object, light_probe_object)

public:
	void set_inner_range(DirectX::XMFLOAT3 range);
	DirectX::XMFLOAT3 get_inner_range() const;

	void set_outer_range(DirectX::XMFLOAT3 range);
	DirectX::XMFLOAT3 get_outer_range() const;

public:
	area_light_probe_object(std::string_view name);
	~area_light_probe_object() override;

#if defined(USE_IMGUI)
	void imgui_tree_inner_render() override;
#endif	//	defined(USE_IMGUI)
};

//	影ライトオブジェクト
class shadow_light_object : public game_object
{
	GAME_OBJECT_DECRLEAR(shadow_light_object, game_object)

public:
	shadow_light_ptr get_shadow_light() const;

	void set_flag(shadow_light::flags flag, bool value);
	bool is_flag(shadow_light::flags flag) const;

	void set_color(DirectX::XMFLOAT4 color);
	DirectX::XMFLOAT4 get_color() const;

	void set_rotation(const math::rotator& rotation);
	const math::rotator& get_rotation() const;

	void set_position(DirectX::XMFLOAT3 position);
	DirectX::XMFLOAT3 get_position() const;

	void set_range(float radius);
	float get_range() const;

	void set_cone(float degree);
	float get_cone() const;

public:
	shadow_light_object(std::string_view name);
	~shadow_light_object() override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

protected:
	void on_active() override;
	void on_deactive() override;

protected:
	shadow_light_ptr shadow_light;
};
