#pragma once

namespace game_constants
{
	//	ブレイク演出時間
	constexpr float break_effect_time = 5.0f;
	//	ブレイク演出速度係数
	constexpr float break_effect_time_scale = 0.01f;

	//	ブレイク操作系
	//	ブレイク操作中とブレイク演出の方は分けて考えたいのでこの形に
	constexpr float stagger_wait_time = 4.0f;
	constexpr float stagger_input_wait_time = 1.0f;

	//	復活時無敵時間
	constexpr float regenerate_invisible_time = 3.0f;
}
