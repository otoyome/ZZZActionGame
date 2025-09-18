#include "object/ui/ui_button.h"

DirectX::XMFLOAT4 ui_button::get_color(color_type type) const
{
	return color[static_cast<int>(type)];
}

void ui_button::set_color(color_type type, const DirectX::XMFLOAT4& color)
{
	this->color[static_cast<int>(type)] = color;
}

void ui_button::set_color(color_type type, const DirectX::XMFLOAT3& color)
{
	auto& c = this->color[static_cast<int>(type)];
	c.x = color.x;
	c.y = color.y;
	c.z = color.z;
}

void ui_button::set_alpha(float alpha)
{
	this->alpha = alpha;
}

bool ui_button::get_enabled()
{
	return enable;
}

void ui_button::set_enabled(bool enable)
{
	this->enable = enable;
}

ui_button::ui_button(std::string_view name, std::shared_ptr<ui_image> image)
: game_object(name)
, image(image)
{
	set_color(color_type::normal, {1, 1, 1, 1});
	set_color(color_type::pressed, {0.5f, 0.5f, 0.5f, 1});
	set_color(color_type::disabled, {0.5f, 0.5f, 0.5f, 1});
}

ui_button::~ui_button()
{
}

void ui_button::fixed_update(float elapsed_time)
{
	DirectX::XMFLOAT4 apply_color = image->get_color();
	if (enable)
	{
		apply_color = color[static_cast<int>(color_type::normal)];
	}
	else
	{
		apply_color = color[static_cast<int>(color_type::disabled)];
	}

	if (pressed)
	{
		if (math::equal(image->get_color(), color[static_cast<int>(color_type::pressed)]))
			pressed = false;
		apply_color = color[static_cast<int>(color_type::pressed)];
	}

	if (image)
	{
		apply_color.w *= alpha;
		DirectX::XMFLOAT4 col = math::lerp(image->get_color(), apply_color, fade_duration);
		image->set_color(col);
	}
}

void ui_button::press()
{
	pressed = true;
}

#if defined(USE_IMGUI)
void ui_button::imgui_render_core()
{
	base::imgui_render_core();

	ImGui::ColorEdit4(magic_enum::enum_name(color_type::normal).data(), &color[static_cast<int>(color_type::normal)].x);
	ImGui::ColorEdit4(magic_enum::enum_name(color_type::pressed).data(), &color[static_cast<int>(color_type::pressed)].x);
	ImGui::ColorEdit4(magic_enum::enum_name(color_type::disabled).data(), &color[static_cast<int>(color_type::disabled)].x);
}
#endif	//	defined(USE_IMGUI)

