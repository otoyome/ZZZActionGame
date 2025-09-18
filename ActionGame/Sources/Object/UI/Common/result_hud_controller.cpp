#include "object/ui/common/result_hud_controller.h"

bool result_controller_object::is_processing()
{
	return state == fade::states::processing;
}

result_controller_object::result_controller_object(std::string_view name, object_descs desc)
: game_object(name)
, back_image(desc.back_image)
, front_image(desc.front_image)
, result_text(desc.result_text)
, time_text(desc.time_text)
, pless_click_text(desc.pless_click_text)
, state(fade::states::idle)
, direction(fade::directions::in)
, fade_time(0)
, total_time(0)
{
}

result_controller_object::~result_controller_object()
{
}

void result_controller_object::post_update(float elapsed_time)
{
	if (state == fade::states::idle)
		return;

	total_time += elapsed_time;
	float rate = 0.0f;
	if (total_time < fade_time)
	{
		switch (direction)
		{
			case fade::directions::in:
			{
				rate = math::saturate(1.0f - total_time / fade_time);
				break;
			}
			case fade::directions::out:
			{
				rate = math::saturate(total_time / fade_time);
				break;
			}
		}
	}
	else
	{
		switch (direction)
		{
			case fade::directions::in:	rate = 0.0f;	break;
			case fade::directions::out:	rate = 1.0f;	break;
		}
		state = fade::states::idle;
	}

	if (back_image)
		back_image->set_color({ 1, 1, 1, rate });
	if (front_image)
		front_image->set_color({ 1, 1, 1, rate });
	if (result_text)
		result_text->set_color({ 1, 1, 1, rate });
	if (time_text)
		time_text->set_color({ 1, 1, 1, rate });
	if (pless_click_text)
		pless_click_text->set_color({ 1, 1, 1, rate });
}

bool result_controller_object::play(fade::directions direction, float fade_time)
{
	if (is_processing())
		return false;

	state = fade::states::processing;
	total_time = 0;
	this->direction = direction;
	this->fade_time = fade_time;
	return true;
}

void result_controller_object::set_display_time(float time)
{
	if (time_text)
	{
		time_text->set_texts(system_utility::to_display_time_string(time));
	}
}

#if defined(USE_IMGUI)
void result_controller_object::imgui_render_core()
{
	base::imgui_render_core();

	ImGui::SliderFloat("time", &total_time, 0.0f, 1.0f);
	ImGui::SliderFloat("fade time", &fade_time, 0.0f, 1.0f);
}
#endif	//	defined(USE_IMGUI)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void result_hud_controller::set_fade_object(std::weak_ptr<result_controller_object> object)
{
	this->object = object;
}

bool result_hud_controller::is_processing()
{
	auto obj = object.lock();
	if (obj)
	{
		return obj->is_processing();
	}

	return false;
}

bool result_hud_controller::play(fade::directions direction, float fade_time)
{
	auto obj = object.lock();
	if (obj)
	{
		return obj->play(direction, fade_time);
	}
	return false;
}

void result_hud_controller::set_display_time(float time)
{
	auto obj = object.lock();
	if (obj)
	{
		obj->set_display_time(time);
	}
}

