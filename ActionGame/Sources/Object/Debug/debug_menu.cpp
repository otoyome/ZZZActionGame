#include "Object/Debug/debug_menu.h"

#if defined(_DEBUG)
debug_menu::debug_menu(std::shared_ptr<debug_camera> camera)
: game_object("debug_menu")
, camera(camera)
, execute(false)
{
	camera->set_active(false);
	restore_center_lock_mode = mouse::instance()->get_center_lock();
	restore_hide_cursol_mode = mouse::instance()->get_cursol_hide();
}

debug_menu::~debug_menu()
{
}

void debug_menu::debug_start_of_frame()
{
	//	“ü—Í‚ðã‘‚«‚·‚é
	auto keyboard_state = keyboard::instance()->get_state();
	if (keyboard_state.is_key_state(keyboard::key_states::f5) == input::state::down)
	{
		execute = !execute;
		if (execute)
		{
			mouse::instance()->set_center_lock(false);
			mouse::instance()->set_cursol_hide(false);
			camera->set_active(true);
			camera->on_execute();
		}
		else
		{
			mouse::instance()->set_center_lock(restore_center_lock_mode);
			mouse::instance()->set_cursol_hide(restore_hide_cursol_mode);
			camera->set_active(false);
		}
	}

	if (!execute)
	{
		restore_center_lock_mode = mouse::instance()->get_center_lock();
		restore_hide_cursol_mode = mouse::instance()->get_cursol_hide();
		camera->set_active(false);
		return;
	}
	else
	{
		{
			current_mouse_state = mouse::instance()->get_state();
			mouse::states state;
			mouse::instance()->set_state(state);
		}
		{
			current_keyboard_state = keyboard::instance()->get_state();
			auto state = current_keyboard_state;
			memset(state.key_state, 0, sizeof(state.key_state));
			keyboard::instance()->set_state(state);
		}
	}
}

void debug_menu::debug_pre_update(float elapsed_time)
{
	if (!execute)
		return;

	mouse::instance()->set_state(current_mouse_state);
	keyboard::instance()->set_state(current_keyboard_state);
}

#endif	//	defined(_DEBUG)

