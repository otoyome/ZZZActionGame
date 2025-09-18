#include "object/ui/hud/retry_hud_controller_object.h"

namespace
{
	constexpr std::string_view DAMAGE_CALLBACK = "DAMAGE_CALLBACK";
	constexpr std::string_view STAGGER_END_CALLBACK = "STAGGER_END_CALLBACK";
	constexpr std::string_view ENTER_SPECIAL_CALLBACK = "ENTER_SPECIAL_CALLBACK";
}

retry_hud_controller_object::retry_hud_controller_object(object_descs desc)
: game_object(desc.name)
, retry_text(desc.retry_text)
, regenerate_icon(desc.regenerate_icon)
, regenerate_text(desc.regenerate_text)

, dead_icon(desc.dead_icon)
, dead_text(desc.dead_text)

, direction(static_cast<fade::directions>(-1))
, fade_state(static_cast<fade::states>(-1))
, fade_animation_time(0)
, fade_animation_max_time(0)
{
	original_color[ui_ids::text_retry] = retry_text->get_color();
	original_color[ui_ids::text_regenerate] = regenerate_text->get_color();
	original_color[ui_ids::text_dead] = dead_text->get_color();
}

retry_hud_controller_object::~retry_hud_controller_object()
{
}

void retry_hud_controller_object::post_update(float elapsed_time)
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
		#define SET_COLOR(UI_ID, UI) \
		{ \
			auto color = original_color[ui_ids::##UI_ID]; \
			color.w *= rate; \
			UI->set_color(color); \
		}
		SET_COLOR(text_retry, retry_text);
		SET_COLOR(text_regenerate, regenerate_text);
		SET_COLOR(text_dead, dead_text);
		#undef SET_COLOR

		regenerate_icon->set_alpha(rate);
		dead_icon->set_alpha(rate);

		if (fade_animation_max_time < fade_animation_time)
			fade_state = fade::states::idle;
	}
}

void retry_hud_controller_object::play_fade(fade::directions direction, float time)
{
	if (this->direction == direction)
		return;

	this->direction = direction;
	this->fade_state = fade::states::processing;
	this->fade_animation_time = 0;
	this->fade_animation_max_time = time;
}

void retry_hud_controller_object::on_finalize()
{
}
