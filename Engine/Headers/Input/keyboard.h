#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include "singleton.h"
#include "system_utility.h"

#include "math_utility.h"

#include "input.h"

class keyboard
{
	SINGLETON(keyboard)

public:
	enum class key_states
	{
		a,
		b,
		c,
		d,
		e,
		f,
		g,
		h,
		i,
		j,
		k,
		l,
		m,
		n,
		o,
		p,
		q,
		r,
		s,
		t,
		u,
		v,
		w,
		x,
		y,
		z,

		shift,
		shiftl,
		shiftr,
		ctrl,
		ctrll,
		ctrlr,
		alt,
		space,

		f1,
		f2,
		f3,
		f4,
		f5,
		f6,
		f7,
		f8,
		f9,
		f10,
		f11,
		f12,

		max,
	};

	struct states
	{
		input::state key_state[static_cast<size_t>(key_states::max)];

		input::state is_key_state(key_states state) const;
		void set_key_state(key_states state, input::state flag);
	};

public:
	const keyboard::states& get_state() const;
	void set_state(const states& state);

private:
	keyboard();
	~keyboard();

public:
	void update_state();

private:
	states state;
};

ENUM_ATTR_BITFLAG(keyboard::key_states);
