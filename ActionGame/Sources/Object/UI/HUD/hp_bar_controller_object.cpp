#include "object/ui/hud/hp_bar_controller_object.h"

namespace
{
	enum ui_id
	{
		cover,
		base,
		main,
		sub,
	};
}

hp_bar_controller_object::directions hp_bar_controller_object::get_animation_direction() const
{
	return animation_direction;
}

void hp_bar_controller_object::set_animation_direction(directions animation_direction)
{
	this->animation_direction = animation_direction;
}

hp_bar_controller_object::hp_bar_controller_object(const object_descs& object_desc)
: game_object(object_desc.name)
{
	ui[ui_id::cover] = object_desc.bar_cover;
	ui[ui_id::base] = object_desc.bar_base;
	ui[ui_id::main] = object_desc.bar_main;
	ui[ui_id::sub] = object_desc.bar_sub;
	for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
	{
		if (ui[index])
		{
			origin_rects[index] = ui[index]->get_screen_rect();
			origin_colors[index] = ui[index]->get_color();
		}
	}
}

hp_bar_controller_object::~hp_bar_controller_object()
{
}

void hp_bar_controller_object::fixed_update(float elapsed_time)
{
	auto character = character_ptr.lock();
	if (!character)
	{
		character_ptr = {};
		return;
	}

	//	HP•Ï“®ŒŸ’m
	if (value != character->get_health())
	{
		main_value_inner = character->get_health();
		if (animation_state == animation_state::idle)
			sub_value_inner = this->value;
		value = character->get_health();
		animation_state = animation_state::main;
	}
	this->value = value;

	switch (animation_state)
	{
		case animation_state::main:
		{
			if (!ui[ui_id::main])
			{
				animation_state = animation_state::sub;
				break;
			}
			int old_value_inner = main_value_inner;
			main_value_inner = math::lerp(main_value_inner, value, math::saturate(animation_speed));
			if (abs(main_value_inner - old_value_inner) <= 10)
			{
				main_value_inner = value;
			}

			float main_rate = static_cast<float>(main_value_inner) / character->get_max_health();
			if (main_value_inner <= value)
			{
				main_value_inner = value;
				main_rate = static_cast<float>(value) / character->get_max_health();
				animation_state = animation_state::sub;
			}
			DirectX::XMFLOAT4 main_rect = ui[ui_id::main]->get_screen_rect();
			main_rect.z = main_rate * origin_rects[ui_id::main].z;
			if (animation_direction == directions::right)
			{
				main_rect.x = origin_rects[ui_id::main].x + (origin_rects[ui_id::main].z - main_rect.z);
			}
			ui[ui_id::main]->set_screen_rect(main_rect);
			break;
		}
		case animation_state::sub:
		{
			if (!ui[ui_id::sub])
			{
				animation_state = animation_state::idle;
				break;
			}
			int old_value_inner = sub_value_inner;
			sub_value_inner = math::lerp(sub_value_inner, value, math::saturate(animation_speed));
			if (abs(sub_value_inner - old_value_inner) <= 10)
			{
				main_value_inner = value;
			}

			float sub_rate = static_cast<float>(sub_value_inner) / character->get_max_health();
			if (sub_value_inner <= value)
			{
				sub_value_inner = value;
				sub_rate = static_cast<float>(value) / character->get_max_health();
				animation_state = animation_state::idle;
			}
			DirectX::XMFLOAT4 sub_rect = ui[ui_id::sub]->get_screen_rect();
			sub_rect.z = sub_rate * origin_rects[ui_id::sub].z;
			if (animation_direction == directions::right)
			{
				sub_rect.x = origin_rects[ui_id::sub].x + (origin_rects[ui_id::sub].z - sub_rect.z);
			}
			ui[ui_id::sub]->set_screen_rect(sub_rect);
			break;
		}
		case animation_state::idle:
		{
			if (ui[ui_id::main])
			{
				float main_rate = static_cast<float>(value) / character->get_max_health();
				DirectX::XMFLOAT4 main_rect = ui[ui_id::main]->get_screen_rect();
				main_rect.z = main_rate * origin_rects[ui_id::main].z;
				if (animation_direction == directions::right)
				{
					main_rect.x = origin_rects[ui_id::main].x + (origin_rects[ui_id::main].z - main_rect.z);
				}
				ui[ui_id::main]->set_screen_rect(main_rect);
			}
			if (ui[ui_id::sub])
			{
				float sub_rate = static_cast<float>(value) / character->get_max_health();
				DirectX::XMFLOAT4 sub_rect = ui[ui_id::sub]->get_screen_rect();
				sub_rect.z = sub_rate * origin_rects[ui_id::sub].z;
				if (animation_direction == directions::right)
				{
					sub_rect.x = origin_rects[ui_id::sub].x + (origin_rects[ui_id::sub].z - sub_rect.z);
				}
				ui[ui_id::sub]->set_screen_rect(sub_rect);
			}
			break;
		}
	}
}

void hp_bar_controller_object::post_update(float elapsed_time)
{
	shake_desc.time -= elapsed_time;
	if (shake_desc.time <= 0.0f)
	{
		if (shake_desc.time > -10000.0f)
		{
			for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
			{
				if (ui[index])
				{
					auto rect = ui[index]->get_screen_rect();
					rect.x -= shake_offset.x;
					rect.y -= shake_offset.y;
					ui[index]->set_screen_rect(rect);
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
		if (ui[index])
		{
			auto rect = ui[index]->get_screen_rect();
			rect.x -= shake_offset.x;
			rect.y -= shake_offset.y;
			rect.x += direction_x;
			rect.y += direction_y;
			ui[index]->set_screen_rect(rect);
		}
	}
	shake_offset.x = direction_x;
	shake_offset.y = direction_y;
}

void hp_bar_controller_object::set_character(std::weak_ptr<character_object> character)
{
	this->character_ptr = character;
}

#if 0
void hp_bar_controller_object::change_value(int value, bool animation)
{
	if (animation)
	{
		main_value_inner = this->value;
		if (animation_state == animation_state::idle)
			sub_value_inner = this->value;
		animation_state = animation_state::main;
	}
	else
	{
		main_value_inner = value;
		sub_value_inner = value;
		animation_state = animation_state::idle;
	}
	this->value = value;
}
#endif

void hp_bar_controller_object::call_shake(const shake_descs& shake_desc)
{
	this->shake_desc = shake_desc;
	this->shake_desc.power = (this->shake_desc.power * 1.0f);
}

void hp_bar_controller_object::set_alpha(float alpha)
{
	for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
	{
		if (!ui[index])
			continue;

		DirectX::XMFLOAT4 color = origin_colors[index];
		color.w *= alpha;
		ui[index]->set_color(color);
	}
}

