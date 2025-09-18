#include "object/ui/common/transition_fade_controller.h"

void transition_fade_controller::set_fade_object(std::weak_ptr<fade_controller_object> object)
{
	this->object = object;
}

bool transition_fade_controller::is_processing()
{
	auto obj = object.lock();
	if (obj)
	{
		return obj->is_processing();
	}

	return false;
}

bool transition_fade_controller::play(fade::directions direction, float fade_time, fade::modes mode)
{
	auto obj = object.lock();
	if (obj)
	{
		return obj->play(direction, fade_time, mode);
	}
	return false;
}

