#include "object/ui/hud/key_icon_controller_object.h"

namespace
{
	constexpr std::string_view key_icon_texture_path[] =
	{
		"Assets/Resources/texture/keyboard_E_click_icon.png",
		"Assets/Resources/texture/keyboard_Q_click_icon.png",
		"Assets/Resources/texture/keyboard_space_icon.png",
		"Assets/Resources/texture/mouse_left_click_icon.png",
		"Assets/Resources/texture/mouse_right_click_icon.png",
	};
}

key_icon_object::key_icon_object(const object_descs& object_desc)
: game_object(object_desc.name)
{
	icon = object_desc.icon;
	origin_color = object_desc.icon->get_color();

	set_key_icon_type(object_desc.icon_type);
}

key_icon_object::~key_icon_object()
{
}

void key_icon_object::set_alpha(float alpha)
{
	if (icon)
	{
		DirectX::XMFLOAT4 color = origin_color;
		color.w *= alpha;
		icon->set_color(color);
	}
}

void key_icon_object::set_key_icon_type(key_icon_type type)
{
	texture_ptr texture;
	if (texture_manager::instance()->load_texture_from_file(texture, key_icon_texture_path[static_cast<int>(type)]))
	{
		if (icon)
			icon->set_texture(texture);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
key_indicator_object::key_indicator_object(const object_descs& object_desc)
: game_object(object_desc.name)
, enabled(true)
, button(object_desc.button)
, key(object_desc.key)
{
}

key_indicator_object::~key_indicator_object()
{
}

void key_indicator_object::fixed_update(float elapsed_time)
{
}

void key_indicator_object::set_enabled(bool enabled)
{
	button->set_enabled(enabled);
//	key->set_e
}

void key_indicator_object::set_alpha(float alpha)
{
	button->set_alpha(alpha);
	key->set_alpha(alpha);
}

void key_indicator_object::press()
{
	button->press();
}




