#include "object/ui/common/fade_controller_object.h"

bool fade_controller_object::is_processing()
{
	return state == fade::states::processing;
}

fade_controller_object::fade_controller_object(std::string_view name, std::shared_ptr<ui_image> image)
: game_object(name)
, image(image)
, state(fade::states::idle)
, direction(fade::directions::in)
, mode(fade::modes::black)
, fade_time(0)
, total_time(0)
{
}

fade_controller_object::~fade_controller_object()
{
}

void fade_controller_object::post_update(float elapsed_time)
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

	switch (mode)
	{
		case fade::modes::black:
		{
			image->set_color({ 0, 0, 0, rate });
			break;
		}
		case fade::modes::white:
		{
			image->set_color({ 1, 1, 1, rate });
			break;
		}
	}
}

bool fade_controller_object::play(fade::directions direction, float fade_time, fade::modes mode)
{
	if (is_processing())
		return false;

	state = fade::states::processing;
	total_time = 0;
	this->direction = direction;
	this->fade_time = fade_time;
	this->mode = mode;
	return true;
}

#if defined(USE_IMGUI)
void fade_controller_object::imgui_render_core()
{
	base::imgui_render_core();

	ImGui::SliderFloat("time", &total_time, 0.0f, 1.0f);
	ImGui::SliderFloat("fade time", &fade_time, 0.0f, 1.0f);
}
#endif	//	defined(USE_IMGUI)
