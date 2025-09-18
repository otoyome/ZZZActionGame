#include "object/ui/hud/break_hud_controller_object.h"

namespace
{
}

break_hud_controller_object::break_hud_controller_object(object_descs desc)
: game_object(desc.name)
, break_plate_base(desc.break_plate_base)
, break_cover(desc.break_cover)
, break_main(desc.break_main)
, break_key_text(desc.break_key_text)
, break_key_icon(desc.break_key_icon)

, direction(static_cast<fade::directions>(-1))
, fade_state(static_cast<fade::states>(-1))
, fade_animation_time(0)
, fade_animation_max_time(0)
{
	original_color[ui_ids::plate_base] = break_plate_base->get_color();
	original_color[ui_ids::cover] = break_cover->get_color();
	original_color[ui_ids::main] = break_main->get_color();
	original_main_rect = break_main->get_screen_rect();
	original_color[ui_ids::key_text] = break_key_text->get_color();
}

break_hud_controller_object::~break_hud_controller_object()
{
}

void break_hud_controller_object::post_update(float elapsed_time)
{
	if (fade_state == fade::states::processing)
	{
		fade_animation_time += elapsed_time;
		float rate = 0.0f;
		switch (direction)
		{
			case fade::directions::in:
			{
				math::compute_fade::in_calculator calculator({ 0.0f, fade_animation_max_time });
				rate = calculator.compute(fade_animation_time);
				break;
			}
			case fade::directions::out:
			{
				math::compute_fade::out_calculator calculator({ 0.0f, fade_animation_max_time });
				rate = calculator.compute(fade_animation_time);
				break;
			}
		}
		#define SET_COLOR(UI_ID) \
		{ \
			auto color = original_color[ui_ids::##UI_ID]; \
			color.w *= rate; \
			break_##UI_ID->set_color(color); \
		}
		SET_COLOR(plate_base);
		SET_COLOR(cover);
		SET_COLOR(main);
		SET_COLOR(key_text);
		#undef SET_COLOR

		break_key_icon->set_alpha(rate);

		if (fade_animation_max_time < fade_animation_time)
			fade_state = fade::states::idle;
	}

	{
		animation_time += elapsed_time;

		float rate = math::saturate(1.0f - animation_time / animation_max_time);
		DirectX::XMFLOAT4 main_rect = original_main_rect;
		main_rect.z = rate * original_main_rect.z;
		break_main->set_screen_rect(main_rect);
	}
}

void break_hud_controller_object::play_fade(fade::directions direction, float time)
{
	if (this->direction == direction)
		return;

	this->direction = direction;
	this->fade_state = fade::states::processing;
	this->fade_animation_time = 0;
	this->fade_animation_max_time = time;
}

void break_hud_controller_object::play(float time)
{
	animation_time = 0;
	animation_max_time = time;
}
