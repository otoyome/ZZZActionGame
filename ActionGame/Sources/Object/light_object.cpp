#include "Object/light_object.h"

light_ptr light_object::get_light() const
{
	return light;
}

void light_object::set_flag(light::flags flag, bool value)
{
	light->set_flag(flag, value);
}

bool light_object::is_flag(light::flags flag) const
{
	return light->is_flag(flag);
}

void light_object::set_color(DirectX::XMFLOAT4 color)
{
	light->set_color(color);
}

DirectX::XMFLOAT4 light_object::get_color() const
{
	return light->get_color();
}

void light_object::set_position(DirectX::XMFLOAT3 position)
{
	light->set_position(position);
}

DirectX::XMFLOAT3 light_object::get_position() const
{
	return light->get_position();
}

light_object::light_object(std::string_view name)
: game_object(name)
{
}

light_object::~light_object()
{
}

#if defined(USE_IMGUI)
void light_object::imgui_render_core()
{
	imgui_tree_inner_render();
}
#endif	//	defined(USE_IMGUI)

void light_object::on_active()
{
	light->set_flag(light::flags::active, true);
}

void light_object::on_deactive()
{
	light->set_flag(light::flags::active, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void directional_light_object::set_rotation(const math::rotator& direction)
{
	light->set_rotation(direction);
}

const math::rotator& directional_light_object::get_rotation() const
{
	return light->get_rotation();
}

directional_light_object::directional_light_object(std::string_view name)
: light_object(name)
{
	light_manager::instance()->generate_light(light, light_type::directional);
	math::rotator rotation;
	rotation.set_parameter_from_direction({1, -1, -1});
	set_rotation(rotation);
	set_position({0, 0, 0});
}

directional_light_object::~directional_light_object()
{
}

#if defined(USE_IMGUI)
void directional_light_object::imgui_tree_inner_render()
{
	bool use_shadow = light->is_flag(light::flags::use_shadow);
	ImGui::Checkbox("use_shadow", &use_shadow);
	light->set_flag(light::flags::use_shadow, use_shadow);

	bool debug_csm_area_flag = light->is_flag(light::flags::debug_csm_area_flag);
	ImGui::Checkbox("debug_csm_area_flag", &debug_csm_area_flag);
	light->set_flag(light::flags::debug_csm_area_flag, debug_csm_area_flag);

	bool debug_visualize_ailiasing_error_flag = light->is_flag(light::flags::debug_visualize_ailiasing_error_flag);
	ImGui::Checkbox("debug_visualize_ailiasing_error_flag", &debug_visualize_ailiasing_error_flag);
	light->set_flag(light::flags::debug_visualize_ailiasing_error_flag, debug_visualize_ailiasing_error_flag);

	auto position = get_position();
	ImGui::DragFloat3("position", &position.x, 0.01f);
	set_position(position);

	auto rotation = get_rotation();
	ImGui::DragFloat3("rotation", &rotation.x, 0.01f);
	set_rotation(rotation);

	auto color = get_color();
	ImGui::ColorPicker3("color", &color.x);
	ImGui::DragFloat("intensity", &color.w);
	set_color(color);

	auto shadow_attenuation =  light->get_shadow_attenuation();
	ImGui::SliderFloat3("shadow_attenuation", &shadow_attenuation.x, 0.0f, 1.0f);
	light->set_shadow_attenuation(shadow_attenuation);

	auto pssm_split_weight = light->get_pssm_split_weight();
	ImGui::SliderFloat("pssm_split_weight", &pssm_split_weight, 0.0f, 1.0f, "%.6f");
	light->set_pssm_split_weight(pssm_split_weight);

	auto csm_distance_factor = light->get_csm_distance_factor();
	ImGui::SliderFloat("csm_distance_factor", &csm_distance_factor, 0.0f, 1.0f, "%.6f");
	light->set_csm_distance_factor(csm_distance_factor);

	ImGui::Separator();
	for (int index = 0; index < common_pipeline_states::shadow_map_max; ++index)
	{
		std::string text = "shadow_bias ";
		text += std::to_string(index);
		int id = 0xFF34FF00 + index;
		if (ImGui::TreeNode(text.data()))
		{
			ImGui::PushID(id);
			auto shadow_bias = light->get_shadow_bias(index);
			ImGui::SliderFloat("depth_bias", &shadow_bias.depth_bias, 0.0f, 1.0f, "%.6f");
			ImGui::SliderFloat("slope_depth_bias_scale", &shadow_bias.slope_depth_bias_scale, 0.0f, 1.0f, "%.6f");

			ImGui::SliderFloat("max_depth_bias", &shadow_bias.max_depth_bias, 0.0f, 1.0f, "%.6f");
			ImGui::SliderFloat("slope_depth_bias", &shadow_bias.slope_depth_bias, 0.0f, 5.0f, "%.6f");
			ImGui::SliderFloat("max_slope_depth_bias", &shadow_bias.max_slope_depth_bias, 0.0f, 5.0f, "%.6f");
			ImGui::SliderFloat("sigma_factor", &shadow_bias.sigma_factor, 0.0f, 20.0f, "%.6f");
			light->set_shadow_bias(index, shadow_bias);
			ImGui::PopID();
			ImGui::TreePop();
		}
	}
}
#endif	//	defined(USE_IMGUI)

////////////////////////////////////////////////////////////////////////////////////////////////
void point_light_object::set_range(float radius)
{
	light->set_range(radius);
}

float point_light_object::get_range() const
{
	return light->get_range();
}

point_light_object::point_light_object(std::string_view name)
: light_object(name)
{
	light_manager::instance()->generate_light(light, light_type::point);
	set_position({0, 0, 0});
	set_range(10.0f);
}

point_light_object::~point_light_object()
{
}

#if defined(USE_IMGUI)
void point_light_object::imgui_tree_inner_render()
{
	bool use_shadow = light->is_flag(light::flags::use_shadow);
	ImGui::Checkbox("use_shadow", &use_shadow);
	light->set_flag(light::flags::use_shadow, use_shadow);

	auto position = get_position();
	ImGui::DragFloat3("position", &position.x, 0.01f);
	set_position(position);

	auto range = get_range();
	ImGui::DragFloat("range", &range, 0.01f);
	set_range(range);

	auto color = get_color();
	ImGui::ColorPicker3("color", &color.x);
	ImGui::DragFloat("intensity", &color.w);
	set_color(color);
}
#endif	//	defined(USE_IMGUI)

////////////////////////////////////////////////////////////////////////////////////////////////
void spot_light_object::set_rotation(const math::rotator& rotation)
{
	light->set_rotation(rotation);
}

const math::rotator& spot_light_object::get_rotation() const
{
	return light->get_rotation();
}

void spot_light_object::set_range(float radius)
{
	light->set_range(radius);
}

float spot_light_object::get_range() const
{
	return light->get_range();
}

void spot_light_object::set_inner_cone(float degree)
{
	light->set_inner_cone(DirectX::XMConvertToRadians(degree));
}

float spot_light_object::get_inner_cone() const
{
	return DirectX::XMConvertToDegrees(light->get_inner_cone());
}

void spot_light_object::set_outer_cone(float degree)
{
	light->set_outer_cone(DirectX::XMConvertToRadians(degree));
}

float spot_light_object::get_outer_cone() const
{
	return DirectX::XMConvertToDegrees(light->get_outer_cone());
}

spot_light_object::spot_light_object(std::string_view name)
: light_object(name)
{
	light_manager::instance()->generate_light(light, light_type::spot);
	set_position({0, 0, 0});
	set_range(10.0f);
	set_inner_cone(45.0f);
	set_outer_cone(60.0f);
}

spot_light_object::~spot_light_object()
{
}

#if defined(USE_IMGUI)
void spot_light_object::imgui_tree_inner_render()
{
	bool use_shadow = is_flag(light::flags::use_shadow);
	ImGui::Checkbox("use_shadow", &use_shadow);
	set_flag(light::flags::use_shadow, use_shadow);

	auto position = get_position();
	ImGui::DragFloat3("position", &position.x, 0.01f);
	set_position(position);

	auto rotation = get_rotation();
	ImGui::DragFloat3("rotation", &rotation.x, 0.01f);
	set_rotation(rotation);

	auto range = get_range();
	ImGui::DragFloat("range", &range, 0.01f);
	set_range(range);

	auto inner_cone = get_inner_cone();
	ImGui::DragFloat("inner_cone", &inner_cone, 0.01f);
	set_inner_cone(inner_cone);

	auto outer_cone = get_outer_cone();
	ImGui::DragFloat("outer_cone", &outer_cone, 0.01f);
	set_outer_cone(outer_cone);

	auto color = get_color();
	ImGui::ColorPicker3("color", &color.x);
	ImGui::DragFloat("intensity", &color.w);
	set_color(color);
}
#endif	//	defined(USE_IMGUI)

////////////////////////////////////////////////////////////////////////////////////////////////
light_probe_ptr light_probe_object::get_light_probe() const
{
	return light_probe;
}

void light_probe_object::set_flag(light_probe::flags flag, bool value)
{
	light_probe->set_flag(flag, value);
}

bool light_probe_object::is_flag(light_probe::flags flag) const
{
	return light_probe->is_flag(flag);
}

void light_probe_object::set_color(DirectX::XMFLOAT4 color)
{
	light_probe->set_color(color);
}

DirectX::XMFLOAT4 light_probe_object::get_color() const
{
	return light_probe->get_color();
}

void light_probe_object::set_position(DirectX::XMFLOAT3 position)
{
	light_probe->set_position(position);
}

DirectX::XMFLOAT3 light_probe_object::get_position() const
{
	return light_probe->get_position();
}

void light_probe_object::set_diffuse_ibl_texture(texture_ptr texture)
{
	light_probe->set_diffuse_ibl_texture(texture);
}

texture_ptr light_probe_object::get_diffuse_ibl_texture() const
{
	return light_probe->get_diffuse_ibl_texture();
}

void light_probe_object::set_specular_ibl_texture(texture_ptr texture)
{
	light_probe->set_specular_ibl_texture(texture);
}

texture_ptr light_probe_object::get_specular_ibl_texture() const
{
	return light_probe->get_specular_ibl_texture();
}

void light_probe_object::set_lut_ggx_texture(texture_ptr texture)
{
	light_probe->set_lut_ggx_texture(texture);
}

texture_ptr light_probe_object::get_lut_ggx_texture() const
{
	return light_probe->get_lut_ggx_texture();
}

light_probe_object::light_probe_object(std::string_view name)
: game_object(name)
{
}

light_probe_object::~light_probe_object()
{
}

#if defined(USE_IMGUI)
void light_probe_object::imgui_render_core()
{
	auto color = get_color();
	ImGui::ColorPicker3("color", &color.x);
	ImGui::DragFloat("intensity", &color.w);
	set_color(color);

	auto position = get_position();
	ImGui::DragFloat3("position", &position.x, 0.01f);
	set_position(position);

	imgui_tree_inner_render();
}
#endif	//	defined(USE_IMGUI)

void light_probe_object::on_active()
{
	light_probe->set_flag(light_probe::flags::active, true);
}

void light_probe_object::on_deactive()
{
	light_probe->set_flag(light_probe::flags::active, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////
sky_light_object::sky_light_object(std::string_view name)
: light_probe_object(name)
{
	light_manager::instance()->generate_light_probe(light_probe);
	light_probe->set_flag(light_probe::flags::unbind, true);
	set_position({0, 0, 0});
	set_position({90.0f, 90.0f, 90.0f});
}

sky_light_object::~sky_light_object()
{
}

#if defined(USE_IMGUI)
void sky_light_object::imgui_tree_inner_render()
{
}
#endif	//	defined(USE_IMGUI)

////////////////////////////////////////////////////////////////////////////////////////////////
void area_light_probe_object::set_inner_range(DirectX::XMFLOAT3 range)
{
	light_probe->set_inner_range(range);
}

DirectX::XMFLOAT3 area_light_probe_object::get_inner_range() const
{
	return light_probe->get_inner_range();
}

void area_light_probe_object::set_outer_range(DirectX::XMFLOAT3 range)
{
	light_probe->set_outer_range(range);
}

DirectX::XMFLOAT3 area_light_probe_object::get_outer_range() const
{
	return light_probe->get_outer_range();
}

area_light_probe_object::area_light_probe_object(std::string_view name)
: light_probe_object(name)
{
	light_manager::instance()->generate_light_probe(light_probe);
	light_probe->set_flag(light_probe::flags::unbind, false);
	set_position({0, 0, 0});
	set_position({90.0f, 90.0f, 90.0f});
	set_outer_range({100.0f, 100.0f, 100.0f});
}

area_light_probe_object::~area_light_probe_object()
{
}

#if defined(USE_IMGUI)
void area_light_probe_object::imgui_tree_inner_render()
{
	auto inner_range = get_inner_range();
	ImGui::DragFloat3("inner_range", &inner_range.x, 0.01f);
	set_inner_range(inner_range);

	auto outer_range = get_outer_range();
	ImGui::DragFloat3("outer_range", &outer_range.x, 0.01f);
	set_outer_range(outer_range);
}
#endif	//	defined(USE_IMGUI)

////////////////////////////////////////////////////////////////////////////////////////////////
shadow_light_ptr shadow_light_object::get_shadow_light() const
{
	return shadow_light;
}

void shadow_light_object::set_flag(shadow_light::flags flag, bool value)
{
	shadow_light->set_flag(flag, value);
}

bool shadow_light_object::is_flag(shadow_light::flags flag) const
{
	return shadow_light->is_flag(flag);
}

void shadow_light_object::set_color(DirectX::XMFLOAT4 color)
{
	shadow_light->set_color(color);
}

DirectX::XMFLOAT4 shadow_light_object::get_color() const
{
	return shadow_light->get_color();
}

void shadow_light_object::set_rotation(const math::rotator& rotation)
{
	shadow_light->set_rotation(rotation);
}

const math::rotator& shadow_light_object::get_rotation() const
{
	return shadow_light->get_rotation();
}

void shadow_light_object::set_position(DirectX::XMFLOAT3 position)
{
	shadow_light->set_position(position);
}

DirectX::XMFLOAT3 shadow_light_object::get_position() const
{
	return shadow_light->get_position();
}

void shadow_light_object::set_range(float radius)
{
	shadow_light->set_range(radius);
}

float shadow_light_object::get_range() const
{
	return shadow_light->get_range();
}

void shadow_light_object::set_cone(float degree)
{
	shadow_light->set_cone(DirectX::XMConvertToRadians(degree));
}

float shadow_light_object::get_cone() const
{
	return DirectX::XMConvertToDegrees(shadow_light->get_cone());
}

shadow_light_object::shadow_light_object(std::string_view name)
: game_object(name)
{
	light_manager::instance()->generate_shadow_light(shadow_light);
	set_position({0, 0, 0});
	set_range(10.0f);
}

shadow_light_object::~shadow_light_object()
{
}

#if defined(USE_IMGUI)
void shadow_light_object::imgui_render_core()
{

	auto position = get_position();
	ImGui::DragFloat3("position", &position.x, 0.01f);
	set_position(position);

	auto rotation = get_rotation();
	ImGui::DragFloat3("rotation", &rotation.x, 0.01f);
	set_rotation(rotation);

	auto range = get_range();
	ImGui::DragFloat("range", &range, 0.01f);
	set_range(range);

	auto cone = get_cone();
	ImGui::DragFloat("cone", &cone, 0.01f);
	set_cone(cone);

	auto contact_shadow = shadow_light->get_contact_shadow();
	ImGui::SliderFloat("contact_shadow", &contact_shadow, 0.0f, 1.0f);
	shadow_light->set_contact_shadow(contact_shadow);

	auto color =  shadow_light->get_color();
	ImGui::SliderFloat3("shadow_color", &color.x, 0.0f, 1.0f);
	shadow_light->set_color(color);

	auto shadow_bias = shadow_light->get_shadow_bias();
	if (ImGui::SliderFloat("shadow_bias", &shadow_bias, 0.0f, 0.01f))
	{
		shadow_light->set_shadow_bias(shadow_bias);
	}
}
#endif	//	defined(USE_IMGUI)

void shadow_light_object::on_active()
{
	shadow_light->set_flag(shadow_light::flags::active, true);
}

void shadow_light_object::on_deactive()
{
	shadow_light->set_flag(shadow_light::flags::active, false);
}
