#include "object/ui/hud/sp_bar_controller_object.h"

namespace
{
	enum ui_id
	{
		cover,
		base,
		main,
		numeric,
	};
}

sp_bar_controller_object::directions sp_bar_controller_object::get_animation_direction() const
{
	return animation_direction;
}

void sp_bar_controller_object::set_animation_direction(directions animation_direction)
{
	this->animation_direction = animation_direction;
}

sp_bar_controller_object::sp_bar_controller_object(const object_descs& object_desc)
	: game_object(object_desc.name)
{
	uis[ui_id::cover] = object_desc.bar_cover;
	uis[ui_id::base] = object_desc.bar_base;
	uis[ui_id::main] = object_desc.bar_main;
	uis[ui_id::numeric] = object_desc.value;
	for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
	{
		if (uis[index])
		{
			origin_rects[index] = uis[index]->get_screen_rect();
			origin_colors[index] = uis[index]->get_color();
		}
	}
}

sp_bar_controller_object::~sp_bar_controller_object()
{
}

void sp_bar_controller_object::fixed_update(float elapsed_time)
{
	auto character = character_ptr.lock();
	if (!character)
	{
		character_ptr = {};
		return;
	}

	{
		int value = character->get_special_power();
		int max_value = character->get_max_special_power();
		{
			float old_value_inner = main_value_inner;
			float f_value = math::lerp(main_value_inner, static_cast<float>(value), math::saturate(animation_speed));
			main_value_inner = f_value;

			float main_rate = main_value_inner / max_value;
			DirectX::XMFLOAT4 main_rect = uis[ui_id::main]->get_screen_rect();
			main_rect.z = main_rate * origin_rects[ui_id::main].z;
			if (animation_direction == directions::right)
			{
				main_rect.x = origin_rects[ui_id::main].x + (origin_rects[ui_id::main].z - main_rect.z);
			}
			uis[ui_id::main]->set_screen_rect(main_rect);
		}
		float value_rate = static_cast<float>(value) / max_value;

		DirectX::XMFLOAT4 changed_color = { 1, 1, 1, 1 };
		bool is_change_color = false;
		if (character->get_special_power() < character->get_use_special_power())
		{
			changed_color = { 0.6f, 0.6f, 0.6f, 1.0f };
			is_change_color = true;
		}
		if (uis[ui_id::main])
		{
			auto image = std::static_pointer_cast<ui_image>(uis[ui_id::main]);
			auto color = is_change_color ? changed_color : origin_colors[ui_id::main];
			color.w = image->get_color().w;
			image->set_color(color);
		}
	}
}

void sp_bar_controller_object::post_update(float elapsed_time)
{
	shake_desc.time -= elapsed_time;
	if (shake_desc.time <= 0.0f)
	{
		if (shake_desc.time > -10000.0f)
		{
			for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
			{
				if (uis[index])
				{
					auto rect = uis[index]->get_screen_rect();
					rect.x -= shake_offset.x;
					rect.y -= shake_offset.y;
					uis[index]->set_screen_rect(rect);
				}
			}
			shake_offset.x = 0;
			shake_offset.y = 0;
		}
		shake_desc.time = -10000.0f;
		return;
	}

	float direction_x = shake_desc.power * static_cast<float>(rand() % 3 - 1);
	float direction_y = shake_desc.power * static_cast<float>(rand() % 3 - 1);
	for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
	{
		if (uis[index])
		{
			auto rect = uis[index]->get_screen_rect();
			rect.x -= shake_offset.x;
			rect.y -= shake_offset.y;
			rect.x += direction_x;
			rect.y += direction_y;
			uis[index]->set_screen_rect(rect);
		}
	}
	shake_offset.x = direction_x;
	shake_offset.y = direction_y;
}

void sp_bar_controller_object::set_character(std::weak_ptr<character_object> character)
{
	this->character_ptr = character;
}

void sp_bar_controller_object::call_shake(const shake_descs& shake_desc)
{
	this->shake_desc = shake_desc;
	this->shake_desc.power = (this->shake_desc.power * 1.0f);
}

void sp_bar_controller_object::set_alpha(float alpha)
{
	for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
	{
		if (!uis[index])
			continue;

		DirectX::XMFLOAT4 color = origin_colors[index];
		color.w *= alpha;
		uis[index]->set_color(color);
	}
}



