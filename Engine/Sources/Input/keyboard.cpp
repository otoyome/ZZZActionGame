#include "Input/keyboard.h"

namespace
{
	static const int KeyMap[] =
	{
		'A',
		'B',
		'C',
		'D',
		'E',
		'F',
		'G',
		'H',
		'I',
		'J',
		'K',
		'L',
		'M',
		'N',
		'O',
		'P',
		'Q',
		'R',
		'S',
		'T',
		'U',
		'V',
		'W',
		'X',
		'Y',
		'Z',

		VK_SHIFT,
		VK_LSHIFT,
		VK_RSHIFT,
		VK_CONTROL,
		VK_LCONTROL,
		VK_RCONTROL,
		VK_MENU,
		VK_SPACE,

		VK_F1,
		VK_F2,
		VK_F3,
		VK_F4,
		VK_F5,
		VK_F6,
		VK_F7,
		VK_F8,
		VK_F9,
		VK_F10,
		VK_F11,
		VK_F12,
	};
}

input::state keyboard::states::is_key_state(key_states state) const
{
	return key_state[static_cast<size_t>(state)];
}

void keyboard::states::set_key_state(key_states state, input::state flag)
{
	key_state[static_cast<size_t>(state)] = flag;
}

const keyboard::states& keyboard::get_state() const
{
	return state;
}

void keyboard::set_state(const states& state)
{
	this->state = state;
}


keyboard::keyboard()
{
}

keyboard::~keyboard()
{
}

void keyboard::update_state()
{
	BYTE input_data[256];
	if (!::GetKeyboardState(input_data))
	{
		memset(input_data, 0, sizeof(input_data));
	}
	for (size_t index = 0; index < static_cast<size_t>(key_states::max); ++index)
	{
		if (input_data[KeyMap[index]] & 0x80)
		{
			
			if (state.key_state[index] == input::state::none || state.key_state[index] == input::state::release)
			{
				state.key_state[index] = input::state::down;
			}
			else if (state.key_state[index] == input::state::down)
			{
				state.key_state[index] = input::state::hold;
			}
		}
		else
		{
			if (state.key_state[index] == input::state::down || state.key_state[index] == input::state::hold)
			{
				state.key_state[index] = input::state::release;
			}
			else
			{
				state.key_state[index] = input::state::none;
			}
		}
	}
}
