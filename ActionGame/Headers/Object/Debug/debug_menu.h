#pragma once

#if defined(_DEBUG)
#include "camera.h"
#include "object/camera_object.h"
#include "object/debug/debug_camera.h"

class debug_menu : public game_object
{
public:
	debug_menu(std::shared_ptr<debug_camera> camera);
	~debug_menu();

	void debug_start_of_frame() override;
	void debug_pre_update(float elapsed_time) override;

private:
	std::shared_ptr<debug_camera> camera;
	bool execute;

	bool restore_center_lock_mode;
	bool restore_hide_cursol_mode;

	mouse::states current_mouse_state;
	keyboard::states current_keyboard_state;
};

#endif	//	defined(_DEBUG)
