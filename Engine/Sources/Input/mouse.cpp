#include "input/mouse.h"

namespace
{
	static const int KeyMap[] =
	{
		VK_LBUTTON,
		VK_MBUTTON,
		VK_RBUTTON,
#if _WIN32_WINNT >= 0x0500
		VK_XBUTTON1,
		VK_XBUTTON2,
#endif  //  _WIN32_WINNT >= 0x0500
	};
}

input::state mouse::states::is_key_state(key_states state) const
{
	return key_state[static_cast<size_t>(state)];
}

void mouse::states::set_key_state(key_states state, input::state flag)
{
	key_state[static_cast<size_t>(state)] = flag;
}

mouse::states mouse::get_state() const
{
	states state;
	state.position = position_state;
	memcpy_s(state.key_state, sizeof(state.key_state), key_state, sizeof(key_state));
	return state;
}

void mouse::set_state(const states& state)
{
	this->position_state = state.position;
	memcpy_s(key_state, sizeof(key_state), state.key_state, sizeof(state.key_state));
}

DirectX::XMINT2 mouse::get_move_cursol() const
{
	return {move_vector.x, move_vector.y};
}

int mouse::get_wheel_value() const
{
	return position_state.wheel;
}

void mouse::set_center_lock(bool lock)
{
	this->window_center_lock = lock;
}

bool mouse::get_center_lock() const
{
	return window_center_lock;
}

void mouse::set_cursol_hide(bool hide)
{
	cursol_hide = hide;
}

bool mouse::get_cursol_hide() const
{
	return cursol_hide;
}

mouse::mouse(HWND hwnd)
: hwnd(hwnd)
, window_center_lock(false)
, cursol_hide(false)
, wheel_value(0)
, viewport_size()
, position_state()
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	viewport_size.x = rc.right - rc.left;
	viewport_size.y = rc.bottom - rc.top;
}

mouse::~mouse()
{
}

void mouse::set_viewport_size(const DirectX::XMINT2& size)
{
	viewport_size = size;
}

void mouse::update_state()
{
	position_states new_position_state;
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		UINT screen_width = rc.right - rc.left;
		UINT screen_height = rc.bottom - rc.top;
		bool input_key_state[static_cast<int>(key_states::max)];
		if (::GetForegroundWindow() == hwnd)
		{
			//	入力状況
			for (size_t index = 0; index < static_cast<size_t>(key_states::max); ++index)
			{
				input_key_state[index] = ::GetAsyncKeyState(KeyMap[index]) & 0x8000;
			}
			//	カーソル座標
			{
				POINT cursor;
				::GetCursorPos(&cursor);

				//	移動ベクトルを算出
				if (window_center_lock)
				{
					//	マウス座標を中心点に固定化
					::SetCursorPos(screen_width / 2, screen_height / 2);
					move_vector.x = cursor.x - screen_width / 2;
					move_vector.y = cursor.y - screen_height / 2;
				}

				//	座標算出
				::ScreenToClient(hwnd, &cursor);
				new_position_state.position.x = static_cast<LONG>(cursor.x / static_cast<float>(viewport_size.x) * static_cast<float>(screen_width));
				new_position_state.position.y = static_cast<LONG>(cursor.y / static_cast<float>(viewport_size.y) * static_cast<float>(screen_height));
				if (!window_center_lock)
				{
					move_vector.x = new_position_state.position.x - position_state.position.x;
					move_vector.y = new_position_state.position.y - position_state.position.y;
				}
			}

			//	ホイール
			{
				new_position_state.wheel = wheel_value;
				wheel_value = 0;
			}
		}
		else
		{
			//	テキトーに初期化しておく
			move_vector.x = 0;
			move_vector.y = 0;
			POINT cursor;
			::GetCursorPos(&cursor);
			memset(input_key_state, 0, sizeof(input_key_state));

			//	座標算出
			::ScreenToClient(hwnd, &cursor);
			new_position_state.position.x = static_cast<LONG>(cursor.x / static_cast<float>(viewport_size.x) * static_cast<float>(screen_width));
			new_position_state.position.y = static_cast<LONG>(cursor.y / static_cast<float>(viewport_size.y) * static_cast<float>(screen_height));
		}

		//	入力状況を保存
		for (size_t index = 0; index < static_cast<size_t>(key_states::max); ++index)
		{
			if (input_key_state[index])
			{
					
				if (key_state[index] == input::state::none || key_state[index] == input::state::release)
				{
					key_state[index] = input::state::down;
				}
				else if (key_state[index] == input::state::down)
				{
					key_state[index] = input::state::hold;
				}
			}
			else
			{
				if (key_state[index] == input::state::down || key_state[index] == input::state::hold)
				{
					key_state[index] = input::state::release;
				}
				else
				{
					key_state[index] = input::state::none;
				}
			}
		}
	}

	//	カーソル非表示化
	static int cursol_hide_oneshot = 0x10000000;
	static bool cursol_hide_inner = false;
	if (cursol_hide_oneshot == 0x10000000 || cursol_hide_inner != cursol_hide)
	{
		cursol_hide_oneshot = 0;
		cursol_hide_inner = cursol_hide;
		::ShowCursor(!cursol_hide);
	}

	position_state = new_position_state;
}

LRESULT mouse::window_proccess(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_MOUSEWHEEL:
		{
			wheel_value += GET_WHEEL_DELTA_WPARAM(wParam);
			return 0;
		}
#if 0
		case WM_MOUSEHWHEEL:
		{
			io.AddMouseWheelEvent(-(float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, 0.0f);
			return 0;
		}
#endif
	}
	return 0;
}
