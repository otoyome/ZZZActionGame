#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include "singleton.h"
#include "system_utility.h"

#include "math_utility.h"

#include "input.h"

class mouse
{
	SINGLETON_DECREATE(mouse)

public:
	enum class key_states
	{
		left,
		middle,
		right,
		x_button_1,
		x_button_2,

		max
	};
	struct position_states
	{
		DirectX::XMINT2 position = { 0, 0 }; // センターロック時は中心点からの移動量が保存されている
		int wheel = 0;
	};

	struct states
	{
		position_states position;
		input::state key_state[static_cast<int>(key_states::max)] = {};

		input::state is_key_state(key_states state) const;
		void set_key_state(key_states state, input::state flag);
	};

public:
	states get_state() const;
	void set_state(const states& state);

	DirectX::XMINT2 get_move_cursol() const;
	int get_wheel_value() const;

	void set_center_lock(bool lock);
	bool get_center_lock() const;

	void set_cursol_hide(bool hide);
	bool get_cursol_hide() const;

private:
	mouse(HWND hwnd);
	~mouse();

public:
	static	inline	void	create_instance(HWND hwnd)
	{
		if (!instance_ptr)
		{
			instance_ptr = new mouse(hwnd);
		}
	}

	void set_viewport_size(const DirectX::XMINT2& size);

	void update_state();

	LRESULT window_proccess(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HWND hwnd;
	bool window_center_lock;
	bool cursol_hide;
	int wheel_value;
	DirectX::XMINT2 viewport_size;
	DirectX::XMINT2 move_vector;
	position_states position_state;
	input::state key_state[static_cast<int>(key_states::max)];
};

ENUM_ATTR_BITFLAG(mouse::key_states);
