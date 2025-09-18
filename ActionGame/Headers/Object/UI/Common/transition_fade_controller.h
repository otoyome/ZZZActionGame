#pragma once

#include "fade_controller_object.h"

//	シーン遷移用のフェードコントローラー
class transition_fade_controller
{
	SINGLETON(transition_fade_controller)

public:
	void set_fade_object(std::weak_ptr<fade_controller_object> object);

	bool is_processing();
	bool play(fade::directions direction, float fade_time, fade::modes mode);

public:
	std::weak_ptr<fade_controller_object> object;
};
