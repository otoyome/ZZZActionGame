#include "object/character/person/boss_seraphim_object.h"

#include "collision/capsule.h"

namespace setup_data
{
	constexpr std::string_view model_control_object_name = "ボス戦用セラフィム 制御";
	constexpr int health = 30000; // 体力
	constexpr int trunk = 7500; // 体幹
	constexpr float mass = 99999.0f; // 重さ
	constexpr float capsule_push_radius = system_units::convert_m_to_unit(0.4f); // 押し合い判定半径
	constexpr float capsule_hit_radius = system_units::convert_m_to_unit(0.2f); // 当たり判定半径
	constexpr float capsule_length = system_units::convert_m_to_unit(1.4f); // 当たり判定長さ
	constexpr DirectX::XMFLOAT3 capsule_offset = system_units::convert_m_to_unit({ 0.0f, 1.1f, 0.0f }); // 当たり判定長さオフセット

	constexpr float friction = 0.995f;	// 摩擦係数(ゴム靴？)

	constexpr float walk_accelaration = setup_data::mass * system_units::convert_m_to_unit(5.0f) * 2.0f; // 歩きの加速度(m/s2)
	constexpr float dash_accelaration = setup_data::mass * system_units::convert_m_to_unit(15.0f) * 2.0f; // ダッシュ加速度(m/s2)
	constexpr float hi_dash_accelaration = setup_data::mass * system_units::convert_m_to_unit(20.0f) * 2.0f; // ハイダッシュ加速度(m/s2)

	constexpr float walk_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(3.0f); // 歩きの減速度(m/s2)
	constexpr float dash_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(15.0f) * 2.0f; // ダッシュ減速度(m/s2)
	constexpr float hi_dash_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(20.0f) * 2.0f; // ハイダッシュ減速度(m/s2)
	constexpr float move_stop_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(24.0f) * 3.0f; // ブレーキング減速力(m/s2)
	constexpr float parrying_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(24.0f) * 3.0f; // ブレーキング減速力(m/s2)

	constexpr float walk_max_speed = system_units::convert_m_to_unit(2.0f); // 歩きの速度(m/s)
	constexpr float dash_max_speed = system_units::convert_m_to_unit(10.0f); // ダッシュ速度(m/s)
	constexpr float hi_dash_max_speed = system_units::convert_m_to_unit(15.0f); // ハイダッシュ速度(m/s)
}

namespace
{
	//	攻撃可能ステート
	struct attack_state_datas
	{
		std::string_view state_id; // 対応するステートId
		boss_seraphim_object::attack_id attack_id; // 対応する攻撃Id
		struct next_attack_data
		{
			float start;
			float end;
			boss_seraphim_object::attack_id id;
		};
		next_attack_data next_data[2]; // 移行する攻撃Id
	};
	static constexpr attack_state_datas onwards_attack_state[] =
	{
		{
			character_object::state_names[character_object::state_idle],
			boss_seraphim_object::attack_id::none,
			{
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_1},
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_3},
			}
		},
		{
			character_object::state_names[character_object::state_walk],
			boss_seraphim_object::attack_id::none,
			{
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_1},
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_1},
			}
		},
		{
			character_object::state_names[character_object::state_dash],
			boss_seraphim_object::attack_id::none,
			{
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_1},
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_1},
			}
		},
		{
			character_object::state_names[character_object::state_hi_dash],
			boss_seraphim_object::attack_id::none,
			{
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_1},
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_1},
			}
		},
		{
			character_object::state_names[character_object::state_move_stop],
			boss_seraphim_object::attack_id::none,
			{
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_1},
				{0.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_1},
			}
		},
		{
			character_object::state_names[character_object::state_parry],
			boss_seraphim_object::attack_id::none,
			{
				{15.0f / 30.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_parry_1},
				{15.0f / 30.0f, FLT_MAX, boss_seraphim_object::attack_id::attack_parry_2},
			}
		},
		/////////////////////////////////////////////////////////////////////////////////////////
		{
			character_object::state_names[character_object::state_attack],
			boss_seraphim_object::attack_id::attack_1,
			{
				{34.0f / 30.0f, 40.0f / 30.0f, boss_seraphim_object::attack_id::attack_2},
				{34.0f / 30.0f, 40.0f / 30.0f, boss_seraphim_object::attack_id::attack_2},
			}
		},
		{
			character_object::state_names[character_object::state_attack],
			boss_seraphim_object::attack_id::attack_2,
			{
				{26.0f / 30.0f, 31.0f / 30.0f, boss_seraphim_object::attack_id::attack_3},
				{26.0f / 30.0f, 31.0f / 30.0f, boss_seraphim_object::attack_id::attack_3},
			}
		},
	};

	//	attack_idに合わせた攻撃アニメーション名
	static constexpr std::string_view attack_animation_names[] =
	{
		"None",
		"Attack1",
		"Attack2",
		"Attack3",

		"SpecialAttack1",
		"SpecialAttack2-1",
	};

	//	attack_idに合わせた攻撃フレーム
	static constexpr float normal_attack_omen_play_flames[] =
	{
		FLT_MAX,
		10.0f / 30.0f,
		10.0f / 30.0f,
		8.0f / 30.0f,

		FLT_MAX,
		FLT_MAX,
	};

	//	ダメージアニメーション名
	static constexpr std::string_view damage_animation_names[] =
	{
		"DamageNormal",
		"DamageNormal",
		"DamageNormal",
		"DamageBlown",
		"DamageBlown",
		"Dead",
	};

	//	よろけアニメーション名
	static constexpr std::string_view stagger_animation_names[] =
	{
		"StaggerStart",
		"Stagger",
		"StaggerComplete",
	};

	//	攻撃情報
	struct attack_box_datas
	{
		DirectX::XMFLOAT2 time_range;
		character_object::attack_descs attack;
		character_object::bounding_descs bounds;
	};
	constexpr attack_box_datas attack_data_list[] =
	{
		//	None
		{
			{0.0f / 30.0f, 0.0f / 30.0f},
			{
				{0},
				{0, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				0.0f,
				0.0f,
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({0, 0, 0}), {0, 0, 0}, system_units::convert_m_to_unit({0, 0, 0})}
		},
		//	Attack1
		{
			{23.0f / 30.0f, 24.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{25.0f / 30.0f, 26.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{27.0f / 30.0f, 28.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{29.0f / 30.0f, 30.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{31.0f / 30.0f, 32.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{33.0f / 30.0f, 34.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		//	Attack2
		{
			{21.0f / 30.0f, 22.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{23.0f / 30.0f, 24.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{25.0f / 30.0f, 26.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{27.0f / 30.0f, 28.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{29.0f / 30.0f, 30.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{31.0f / 30.0f, 32.0f / 30.0f},
			{
				{100},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		//	Attack3
		{
			{17.0f / 30.0f, 20.0f / 30.0f},
			{
				{1200},
				{1200, true, 0.25f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(0.5f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({3.0f, 2.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 2.0f})}
		},
		//	ParryAttack1
		{
			{15.0f / 30.0f, 18.0f / 30.0f},
			{
				{500},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({0.5f, 2.5f, 5.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 2.5f, 5.0f})}
		},
		{
			{18.0f / 30.0f, 21.0f / 30.0f},
			{
				{500},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({0.5f, 2.5f, 5.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 2.5f, 5.0f})}
		},
		{
			{21.0f / 30.0f, 24.0f / 30.0f},
			{
				{500},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({0.5f, 2.5f, 5.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 2.5f, 5.0f})}
		},
		{
			{24.0f / 30.0f, 27.0f / 30.0f},
			{
				{500},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({0.5f, 2.5f, 5.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 2.5f, 5.0f})}
		},
		{
			{27.0f / 30.0f, 30.0f / 30.0f},
			{
				{500},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({0.5f, 2.5f, 5.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 2.5f, 5.0f})}
		},
		{
			{30.0f / 30.0f, 32.0f / 30.0f},
			{
				{500},
				{100, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(3.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({0.5f, 2.5f, 5.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 2.5f, 5.0f})}
		},
		//	ParryAttack2
		{
			{2.0f / 30.0f, 5.0f / 30.0f},
			{
				{2200},
				{400, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(4.0f),
				system_units::convert_m_to_unit(1.0f),
				true,
				system_units::convert_fps_to_sec(12.0f),
				system_units::convert_m_to_unit(0.15f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({5.0f, 5.0f, 5.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.0f, 0.0f})}
		},
		{
			{5.0f / 30.0f, 10.0f / 30.0f},
			{
				{2800},
				{600, true, 0.0f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(8.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(12.0f),
				system_units::convert_m_to_unit(0.5f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({5.0f, 5.0f, 5.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.0f, 0.0f})}
		},
	};

	//	攻撃ごとの攻撃情報の範囲
	constexpr std::pair<size_t, size_t> attack_data_range[] =
	{
		{ 0,  0}, // None
		{ 1,  6}, // Attack1
		{ 7, 12}, // Attack2
		{13, 13}, // Attack3
		{14, 19}, // AttackSpecial1
		{20, 21}, // AttackSpecial2-2
	};

	//	ステートごとの中断耐性
	struct damage_cancel_resist_state_datas
	{
		std::string_view state_id; // 対応するステートId
		boss_seraphim_object::attack_id attack_id; // 対応する攻撃Id
		DirectX::XMFLOAT2 range; // 範囲
		character_object::action_level action_level; // 対応する攻撃Id
	};
	constexpr damage_cancel_resist_state_datas damage_cancel_resist_state_data[] =
	{
		{
			character_object::state_names[character_object::state_entry],
			boss_seraphim_object::attack_id::none,
			{0.0f / 30.0f, FLT_MAX},
			character_object::action_level::level_5,
		},
		{
			character_object::state_names[character_object::state_idle],
			boss_seraphim_object::attack_id::none,
			{0.0f / 30.0f, FLT_MAX},
			character_object::action_level::level_2,
		},
		{
			character_object::state_names[character_object::state_walk],
			boss_seraphim_object::attack_id::none,
			{0.0f / 30.0f, FLT_MAX},
			character_object::action_level::level_2,
		},
		{
			character_object::state_names[character_object::state_dash],
			boss_seraphim_object::attack_id::none,
			{0.0f / 30.0f, FLT_MAX},
			character_object::action_level::level_2,
		},
		{
			character_object::state_names[character_object::state_hi_dash],
			boss_seraphim_object::attack_id::none,
			{0.0f / 30.0f, FLT_MAX},
			character_object::action_level::level_2,
		},
		{
			character_object::state_names[character_object::state_move_stop],
			boss_seraphim_object::attack_id::none,
			{0.0f / 30.0f, FLT_MAX},
			character_object::action_level::level_2,
		},
		{
			character_object::state_names[character_object::state_damage],
			boss_seraphim_object::attack_id::none,
			{0.0f / 30.0f, FLT_MAX},
			character_object::action_level::level_2,
		},
		{
			character_object::state_names[character_object::state_attack],
			boss_seraphim_object::attack_id::attack_1,
			{0.0f / 30.0f, 29.0f / 30.0f},
			character_object::action_level::level_3,
		},
		{
			character_object::state_names[character_object::state_attack],
			boss_seraphim_object::attack_id::attack_2,
			{0.0f / 30.0f, 17.0f / 30.0f},
			character_object::action_level::level_3,
		},
		{
			character_object::state_names[character_object::state_attack],
			boss_seraphim_object::attack_id::attack_3,
			{0.0f / 30.0f, 20.0f / 30.0f},
			character_object::action_level::level_5,
		},
	};

	//	
	constexpr float animation_epsilon_time = system_units::convert_fps_to_sec(1.0f);

	//	
	constexpr std::string_view face_texture_path = "Assets/Resources/Seraphim/face_small.png";
	constexpr DirectX::XMFLOAT4 face_texture_uv = {0.0f, 0.0f, 1.0f, 1.0f};

	//	カメラフォーカス位置補正
	constexpr DirectX::XMFLOAT3 focus_offset = system_units::convert_m_to_unit({0.0f, 1.6f, 0.0f});

	//	入場
	constexpr float entry_roar_animation_time = 55.0f / 30.0f;

	//	ダッシュ速度変更待機時間
	constexpr float change_dash_state_wait_time = system_units::convert_fps_to_sec(30.0f);

	//	パリィ系
	constexpr float parry_slow_time_scale = 0.2f;
	constexpr float parry_slow_time = system_units::convert_fps_to_sec(180.0f);
	constexpr float parry_self_slow_time = system_units::convert_fps_to_sec(60.0f);
	constexpr std::string_view parry_node_name = "右手首";

	//	パリィ攻撃系
	constexpr float parry_attack_2_move_time = system_units::convert_fps_to_sec(13.0f);
	constexpr float parry_attack_2_takedown_point_front = -system_units::convert_m_to_unit(1.5f);
	constexpr float parry_attack_2_takedown_point_right = +system_units::convert_m_to_unit(0.0f);
	constexpr float parry_attack_2_remain_velocity_power = 200.0f;
	constexpr float parry_attack2_breaking_time = 12.0f / 30.0f;
}

boss_seraphim_object::boss_seraphim_object(std::shared_ptr<model_object> character_model, std::string_view collision_filter_name)
: character_object(setup_data::model_control_object_name, character_model, collision_filter_name)
, sub_state(0)
, action_data()

, walk_accelaration(setup_data::walk_accelaration)
, dash_accelaration(setup_data::dash_accelaration)
, hi_dash_accelaration(setup_data::hi_dash_accelaration)

, walk_breaking_deceleration(setup_data::walk_breaking_deceleration)
, dash_breaking_deceleration(setup_data::dash_breaking_deceleration)
, hi_dash_breaking_deceleration(setup_data::hi_dash_breaking_deceleration)

, walk_max_speed(setup_data::walk_max_speed)
, dash_max_speed(setup_data::dash_max_speed)
, hi_dash_max_speed(setup_data::hi_dash_max_speed)

, move_stop_vector(0, 0)
, move_stop_max_speed(0.0f)

, current_attack_id(attack_id::none)
, attack_data_flags(false)
, play_omen_flags(false)
, attack_hit_flag(false)
{
	//	顔画像読込
	{
		texture_ptr face;
		texture_manager::instance()->load_texture_from_file(face, face_texture_path);
		set_face_texture(face);
		set_face_uv(face_texture_uv);
	}

	//	体力設定
	set_max_health(setup_data::health);
	set_health(setup_data::health);

	//	体幹設定
	set_max_trunk(setup_data::trunk);
	set_trunk(setup_data::trunk);

	//	押し合い判定の登録
	get_push_box()->set_filter(collision_filter_name);
	get_push_box()->set_bounding_object(std::make_shared<collision::bounding_capsule>());
	get_push_box()->set_local_scale({setup_data::capsule_push_radius, setup_data::capsule_length, 0.0f});
	get_push_box()->set_local_offset(setup_data::capsule_offset);
	get_push_box()->set_physical_data({setup_data::friction});
	get_push_box()->set_mass(setup_data::mass);

	//	喰らい判定
	bounding_datas bounding_data;
	bounding_data.desc.is_follow_self = true;
	bounding_data.desc.type = collision::bounding_object::types::capsule;
	bounding_data.desc.local_scale = { setup_data::capsule_hit_radius, setup_data::capsule_length, 0.0f };
	bounding_data.desc.local_offset = setup_data::capsule_offset;
	add_hit_box(bounding_data);

	//	ステップオフセットを腰当たりにしておく
	set_step_offset((setup_data::capsule_push_radius + setup_data::capsule_length) * 0.5f);

	//	ステート情報登録
	register_states();

	//	エフェクト準備
	register_effects();

	//	待機状態へ以降
	change_state(character_object::state_names[state_idle]);
}

boss_seraphim_object::~boss_seraphim_object()
{
}

void boss_seraphim_object::fixed_update(float elapsed_time)
{
	//	減速力初期化
	set_breaking_deceleration(walk_breaking_deceleration);

	//	基底の処理を呼び出す
	base::fixed_update(elapsed_time);

	//	中断耐性レベルを変更する
	//	※本来はアニメーション側等で設定できるようにしておきたい所
	{
		action_level damage_cancel_resist_level = action_level::level_1;
		for (auto& data : damage_cancel_resist_state_data)
		{
			if (check_same_state(data.state_id))
			{
				bool find = true;
				if (data.attack_id != boss_seraphim_object::attack_id::none)
				{
					find = false;
					if (data.attack_id == current_attack_id)
					{
						find = true;
					}
				}
				if (find)
				{
					float animation_time = get_model_object()->get_animation_time();
					if (data.range.x <= animation_time && animation_time < data.range.y)
					{
						damage_cancel_resist_level = data.action_level;
						break;
					}
				}
			}
		}
		set_damage_cancel_resist_level(damage_cancel_resist_level);
	}

	//	アクション情報クリア
	action_data = {};
}

void boss_seraphim_object::post_update(float elapsed_time)
{
	//	基底の処理を呼び出す
	base::post_update(elapsed_time);
}

#if defined(USE_IMGUI)
void boss_seraphim_object::imgui_render_core()
{
	character_object::imgui_render_core();

	ImGui::DragFloat("walk_accelaration", &walk_accelaration, 0.01f);
	ImGui::DragFloat("dash_accelaration", &dash_accelaration, 0.01f);
	ImGui::DragFloat("hi_dash_accelaration", &hi_dash_accelaration, 0.01f);
	ImGui::DragFloat("walk_max_speed", &walk_max_speed, 0.01f);
	ImGui::DragFloat("dash_max_speed", &dash_max_speed, 0.01f);
	ImGui::DragFloat("hi_dash_max_speed", &hi_dash_max_speed, 0.01f);
}
#endif	//	defined(USE_IMGUI)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void boss_seraphim_object::input_action_data(action_datas action_data)
{
	this->action_data = action_data;
	float mx = this->action_data.move.x;
	float my = this->action_data.move.y;
	float mz = this->action_data.move.z;
	float ml = (mx * mx + my * my + mz * mz);
	if (ml >= 0.1f * 0.1f)
	{
		ml = sqrtf(ml);
		this->action_data.move.x /= ml;
		this->action_data.move.y /= ml;
		this->action_data.move.z /= ml;
	}
}

DirectX::XMFLOAT3 boss_seraphim_object::get_focus_point() const
{
	DirectX::XMFLOAT3 position = get_position();
	position.x += focus_offset.x;
	position.y += focus_offset.y;
	position.z += focus_offset.z;
	return position;
}

boss_seraphim_object::attack_id boss_seraphim_object::get_current_attack_id() const
{
	return current_attack_id;
}

bool boss_seraphim_object::register_parry_attck_start(std::string_view identifier, call_back_parry_attck call_back)
{
	std::string id = std::string(identifier);
	if (on_parry_attck_start.find(id) != on_parry_attck_start.end())
		return false;

	on_parry_attck_start.insert_or_assign(id, call_back);
	return true;
}

bool boss_seraphim_object::unregister_parry_attck_start(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_parry_attck_start.find(id);
	if (it != on_parry_attck_start.end())
		return false;

	on_parry_attck_start.erase(it);
	return true;
}

void boss_seraphim_object::on_damaged(const damage_datas& data)
{
	base::on_damaged(data);

	//	スタッガー中は無視
	if (is_flag(flags::stagger))
		return;

	//	中断レベル未満の攻撃だとダメージモーションに移動しない
	if (get_damage_cancel_resist_level() > data.desc.attack_level)
		return;

	transition_damage(data);
}

void boss_seraphim_object::on_guarded(const damage_datas& data)
{
	base::on_guarded(data);

	transition_parry(data);
}

void boss_seraphim_object::on_attack_box_enter(collision_object::data data)
{
	base::on_attack_box_enter(data);

	attack_hit_flag = true;
}

void boss_seraphim_object::on_deead()
{
	//	既に死亡済みなら呼び出さない
	if (is_flag(flags::dead))
		return;

	base::on_deead();

	//	死亡モーションとかどうしようか？
	change_state(character_object::state_names[state_dead]);
}

void boss_seraphim_object::on_stagger(const stagger_datas& data)
{
	//	既によろけ済みなら呼び出さない
	if (is_flag(flags::stagger))
		return;

	base::on_stagger(data);

	change_state(character_object::state_names[state_stagger]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool boss_seraphim_object::input_action(float elapsed_time)
{
#if 0
	//	回避処理
	if (action_data.dodge && transition_dodge_wait_time <= FLT_EPSILON)
	{
		return change_state(state_names[state_dodge]);
	}
#endif 

	//	ガード処理
	if (action_data.guard)
	{
		return change_state(state_names[state_guard]);
	}

	//	必殺
	if (action_data.deluxe)
	{
		return false;
	}

	//	特殊
	if (action_data.special)
	{
		return false;
	}

	//	攻撃
	if (action_data.is_attacked())
	{
		attack_id id = attack_id::invalid;
		for (auto& onwards_state : onwards_attack_state)
		{
			if (check_same_state(onwards_state.state_id) && onwards_state.attack_id == current_attack_id)
			{
				//	入力の状態に応じて変える
				float animation_time = get_model_object()->get_animation_time();
				int index = action_data.get_attack_index();
				if (onwards_state.next_data[index].start <= animation_time &&
					animation_time <= onwards_state.next_data[index].end)
				{
					id = onwards_state.next_data[index].id;
				}
				break;
			}
		}

		switch (id)
		{
			case attack_id::invalid:
			{
				//	特に何もしない
				break;
			}
			case attack_id::attack_parry_1:
			case attack_id::attack_parry_2:
			{
				//	パリィ反撃へ移行
				return transition_parry_attack(id);
			}
			default:
			{
				//	攻撃へ移行
				return transition_attack(id);
			}
		}
	}

	return false;
}

void boss_seraphim_object::clear_datas()
{
	//	当たり判定破棄
	clear_attack_box();

	//	ロックオンターゲット破棄
	set_lockon_taget_object(std::weak_ptr<character_object>());

	//	攻撃Idクリア
	current_attack_id = attack_id::none;

	//	ヒットボックス生成管理フラグ
	attack_data_flags = false;
	play_omen_flags = false;

	//	サブステートクリア
	sub_state = 0;
	sub_attack_id = 0;

	//	ブレーキング減速力
	set_breaking_deceleration(walk_breaking_deceleration);

	//	押し合い判定戻す
	get_push_box()->set_flag(collision_object::flags::active, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void boss_seraphim_object::enter_entry()
{
	size_t index = get_model_object()->get_animation_index_from_name("Entry");
	if (index == model_resource::invalid_id)
	{
	//	assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}
	clear_datas();
	work_float[1] = -1;
	get_model_object()->play_animation(index, false, 0.0f);
}

void boss_seraphim_object::update_entry(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	float end_time = get_model_object()->get_animation_end_time();
	if (animation_time >= end_time - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		return;
	}

	//	エフェクト再生
	if (work_float[1] < -0.5f && entry_roar_animation_time <= animation_time)
	{
		work_float[1] = 0;
		play_effect_descs desc;
		desc.id = static_cast<size_t>(custom_effect_id::monster_roar);
		auto effect_position = get_position();
		auto effect_rotation = get_rotation();
		size_t node_index = get_model_object()->get_model_instance()->get_node_index("頭");
		if (node_index != model_instance::invalid_id)
		{
			effect_position.x = get_model_object()->get_model_instance()->get_nodes()[node_index].world_transform._41;
			effect_position.y = get_model_object()->get_model_instance()->get_nodes()[node_index].world_transform._42;
			effect_position.z = get_model_object()->get_model_instance()->get_nodes()[node_index].world_transform._43;
		}
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		desc.scale = { 2.0f, 2.0f, 2.0f };
		play_effect(desc);
	}
}

void boss_seraphim_object::enter_idle()
{
	size_t index = get_model_object()->get_animation_index_from_name("Idle");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}
	clear_datas();
	get_model_object()->play_animation(index, true);
}

void boss_seraphim_object::update_idle(float elapsed_time)
{
	//	入力
	if (input_action(elapsed_time))
	{
		return;
	}

	//	移動処理
	move_mode mode = move_mode::walk;
	if (get_basic_movement_dash() || action_data.dash)
	{
		mode = move_mode::dash;
	}

	if (input_move(elapsed_time, mode))
	{
		if (mode == move_mode::walk)
		{
			change_state(character_object::state_names[state_walk]);
		}
		else if (mode == move_mode::dash)
		{
			change_state(character_object::state_names[state_dash]);
		}
	}
}

void boss_seraphim_object::enter_walk()
{
}

void boss_seraphim_object::update_walk(float elapsed_time)
{
	size_t animation_index = get_model_object()->get_animation_index();
	DirectX::XMFLOAT2 move = DirectX::XMFLOAT2(action_data.move.x, action_data.move.z);

	//	入力と方向ベクトルに合わせてモーションを切り替える
	float mx = move.x, my = move.y;
	float ml = sqrtf(mx * mx + my * my);
	if (ml > 0.0f)
	{
		mx /= ml;
		my /= ml;
		auto front = get_model_object()->get_rotation().get_front();
		float tx = front.x, ty = front.z;
		float tl = sqrtf(tx * tx + ty * ty);
		tx /= tl;
		ty /= tl;
		float cross = mx * ty - my * tx;
		float dot = mx * tx + my * ty;

		if (cross < -0.2f)
			animation_index = get_model_object()->get_animation_index_from_name("LeftMove");
		else if (cross > 0.2f)
			animation_index = get_model_object()->get_animation_index_from_name("RightMove");
		else if (dot < -0.2f)
			animation_index = get_model_object()->get_animation_index_from_name("BackMove");
		else
			animation_index = get_model_object()->get_animation_index_from_name("FrontMove");
	}

	//	アニメーション再生
	if (animation_index != get_model_object()->get_animation_index())
	{
		get_model_object()->play_animation(animation_index, true);
	}

	//	入力
	if (input_action(elapsed_time))
	{
		return;
	}

	//	移動処理
	move_mode mode = move_mode::walk;
	if (get_basic_movement_dash() || action_data.dash)
		mode = move_mode::dash;
	if (input_move(elapsed_time, mode))
	{
		if (mode == move_mode::dash)
			change_state(character_object::state_names[state_dash]);
	}
}

void boss_seraphim_object::enter_dash()
{
	size_t index = get_model_object()->get_animation_index_from_name("RunningNormal");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	get_model_object()->play_animation(index, true, 0.1f);
}

void boss_seraphim_object::update_dash(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (input_action(elapsed_time))
	{
		return;
	}

	//	移動
	if (!input_move(elapsed_time, move_mode::dash))
	{
		change_state(character_object::state_names[state_move_stop]);
		return;
	}

	//	ダッシュ入力
	if (action_data.dash && animation_time >= change_dash_state_wait_time)
	{
		change_state(character_object::state_names[state_hi_dash]);
		return;
	}
}

void boss_seraphim_object::enter_hi_dash()
{
	size_t index = get_model_object()->get_animation_index_from_name("RunningFast");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}
	get_model_object()->play_animation(index, true);
}

void boss_seraphim_object::update_hi_dash(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (input_action(elapsed_time))
	{
		return;
	}

	//	移動入力中
	if (!input_move(elapsed_time, move_mode::hi_dash))
	{
		change_state(character_object::state_names[state_move_stop]);
		return;
	}

	//	ダッシュ入力
	if (action_data.dash && animation_time >= change_dash_state_wait_time)
	{
		change_state(character_object::state_names[state_dash]);
		return;
	}
}

void boss_seraphim_object::enter_move_stop()
{
	size_t index = get_model_object()->get_animation_index_from_name("RunStop");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	//	現在のベクトル
	float speed = sqrtf(get_velocity().x * get_velocity().x + get_velocity().z * get_velocity().z);
	move_stop_vector = DirectX::XMFLOAT2(get_velocity().x / speed, get_velocity().z / speed);

	get_model_object()->play_animation(index, false, 0.2f);
}

void boss_seraphim_object::update_move_stop(float elapsed_time)
{
	float end_time = get_model_object()->get_animation_end_time();
	if (get_model_object()->get_animation_time() >= end_time - animation_epsilon_time)
	{
		change_state(character_object::state_names[state_idle]);
		return;
	}

	//	待機処理も利用
	update_idle(elapsed_time);

	//	ブレーキング減速力
	set_breaking_deceleration(setup_data::move_stop_breaking_deceleration);
}

void boss_seraphim_object::transition_damage(const damage_datas& data)
{
	int animation_index = static_cast<int>(data.desc.attack_level);
	size_t index = get_model_object()->get_animation_index_from_name(damage_animation_names[animation_index]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	//	ステートクリア
	clear_datas();
	get_model_object()->play_animation(index, false, 0.0f);
	change_state(character_object::state_names[state_damage]);
}

void boss_seraphim_object::enter_damage()
{
	work_float[0] = 0;
}

void boss_seraphim_object::update_damage(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	float end_time = get_model_object()->get_animation_end_time();
	if ((animation_time >= end_time  - animation_epsilon_time) || work_float[0] > 0.5f)
	{
		if (is_flag(flags::on_ground))
		{
			change_state(character_object::state_names[state_idle]);
		}
		else
		{
			if (work_float[0] < 0.5f)
			{
				work_float[0] = 1.0f;
				size_t index = get_model_object()->get_animation_index_from_name("Idle");
				get_model_object()->play_animation(index, false, system_units::convert_fps_to_sec(2.0f));
			}
		}
		return;
	}
}

void boss_seraphim_object::enter_dead()
{
	size_t index = get_model_object()->get_animation_index_from_name(damage_animation_names[2]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	//	ステートクリア
	clear_datas();

	get_model_object()->play_animation(index, true, system_units::convert_fps_to_sec(30.0f));
}

void boss_seraphim_object::update_dead(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (get_model_object()->get_animation_end_time() <= animation_time)
	{
		change_state(character_object::state_names[state_dead_complete]);
		return;
	}

	//	死亡演出用の処理
}

void boss_seraphim_object::enter_dead_complete()
{
}

void boss_seraphim_object::update_dead_complete(float elapsed_time)
{
}

void boss_seraphim_object::enter_stagger()
{
	size_t index = get_model_object()->get_animation_index_from_name(stagger_animation_names[0]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	//	ステートクリア
	clear_datas();

	get_model_object()->play_animation(index, false, system_units::convert_fps_to_sec(5.0f));
}

void boss_seraphim_object::update_stagger(float elapsed_time)
{
	if (!is_flag(flags::stagger))
	{
		change_state(character_object::state_names[state_stagger_complete]);
		return;
	}

	size_t index[2] =
	{
		get_model_object()->get_animation_index_from_name(stagger_animation_names[0]),
		get_model_object()->get_animation_index_from_name(stagger_animation_names[1]),
	};

	size_t animation_index = get_model_object()->get_animation_index();
	float animation_time = get_model_object()->get_animation_time();
	if (animation_index == index[0])
	{
		if (get_model_object()->get_animation_end_time() - animation_epsilon_time <= animation_time)
		{
			get_model_object()->play_animation(index[1], true, 0.0f);
			return;
		}
	}
}

void boss_seraphim_object::enter_stagger_complete()
{
	size_t index = get_model_object()->get_animation_index_from_name(stagger_animation_names[2]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	//	ステートクリア
	clear_datas();

	set_flag(character_object::flags::invisible, true);
	get_model_object()->play_animation(index, false, 0.0f);
}

void boss_seraphim_object::update_stagger_complete(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (get_model_object()->get_animation_end_time() - animation_epsilon_time <= animation_time)
	{
		change_state(character_object::state_names[state_idle]);
		return;
	}
}

bool boss_seraphim_object::transition_attack(attack_id id)
{
	float animation_time = get_model_object()->get_animation_time();
	size_t index = get_model_object()->get_animation_index_from_name(attack_animation_names[static_cast<int>(id)]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return false;
	}

	//	ヒットボックスを削除しておく
	clear_attack_box();

	//	ターゲットの方向に修正
	auto taget_object = get_lockon_taget_object().lock();
	if (taget_object)
	{
		auto focus = taget_object->get_focus_point();
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		set_turn_vector({ focus.x / length, focus.z / length });
	}

	current_attack_id = id;
	attack_data_flags = false;
	play_omen_flags = false;
	apprpoaching_effect_flag = false;
	set_velocity({ 0.0f, 0.0f, 0.0f });
	get_model_object()->play_animation(index, false);
	change_state(character_object::state_names[state_attack]);
	sub_attack_id = 0;
	return true;
}

void boss_seraphim_object::update_attack(float elapsed_time)
{
	//	待機状態へ移行
	float animation_time = get_model_object()->get_animation_time();
	if (animation_time >= get_model_object()->get_animation_end_time() - animation_epsilon_time)
	{
		current_attack_id = attack_id::none;
		change_state(character_object::state_names[state_idle]);
		return;
	}

	//	エフェクト再生
	if (!play_omen_flags &&
		 normal_attack_omen_play_flames[static_cast<int>(current_attack_id)] <= animation_time)
	{
		play_omen_flags = true;
		play_effect_descs desc;
		desc.id = static_cast<size_t>(custom_effect_id::omen);
		auto effect_position = get_position();
		auto effect_rotation = get_rotation();
		auto effect_up = effect_rotation.get_up();
		effect_position.x += system_units::convert_m_to_unit(effect_up.x);
		effect_position.y += system_units::convert_m_to_unit(effect_up.y);
		effect_position.z += system_units::convert_m_to_unit(effect_up.z);
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		desc.scale = {2.0f, 2.0f, 2.0f};
		play_effect(desc);
	}

	//	攻撃情報設定
	auto data_range = attack_data_range[static_cast<int>(current_attack_id)];
	auto& attack_box = attack_data_list[data_range.first + sub_attack_id];
	if (sub_attack_id <= data_range.second - data_range.first)
	{
		//	当たり判定生成
		if (!attack_data_flags && 
			(attack_box.time_range.x <= animation_time && animation_time < attack_box.time_range.y))
		{
			bounding_datas box_data;
			box_data.desc = attack_box.bounds;
			box_data.position = get_position();
			box_data.rotator = get_rotation();
			damage_datas damage_data;
			damage_data.desc = attack_box.attack;
			damage_data.stagger_direction = get_rotation().get_front();
			add_attack_box(box_data, damage_data);
			attack_data_flags = true;
		}

		//	当たり判定破棄
		if (attack_data_flags && (attack_box.time_range.y <= animation_time))
		{
			clear_attack_box();
			attack_data_flags = false;
			++sub_attack_id;
		}
	}

	//	攻撃毎の特殊対応
	bool hide_model = false;
	get_model_object()->get_model_instance()->set_flag(model_instance::flags::color_render, true);
	get_model_object()->get_model_instance()->set_flag(model_instance::flags::shadow_caster, true);
	switch (current_attack_id)
	{
		case attack_id::attack_1:
		{
			set_velocity({});
			break;
		}
	}

	//	次の攻撃
	if (input_action(elapsed_time))
		return;

	//	透過処理
	get_model_object()->get_model_instance()->set_flag(model_instance::flags::color_render, !hide_model);
	get_model_object()->get_model_instance()->set_flag(model_instance::flags::shadow_caster, !hide_model);

	//	エフェクト再生
	if (hide_model && !apprpoaching_effect_flag)
	{
		apprpoaching_effect_flag = true;
		play_effect_descs desc;
		desc.id = static_cast<size_t>(effect_ids::shock_wave);
		auto effect_position = get_focus_point();
		auto effect_rotation = get_rotation();
		//	カメラをプレイヤーの後ろに送る
		DirectX::XMFLOAT2 direction;
		if (calculate_lockon_target_vector(direction))
		{
			DirectX::XMVECTOR Vector = DirectX::XMVector3Normalize(DirectX::XMVectorSet(direction.x, 0.0f, direction.y, 0.0f));
			effect_rotation.set_parameter_from_direction(system_utility::convert_xmfloat3_from_vector(Vector));
		}

		auto effect_front = effect_rotation.get_front();
		effect_position.x += system_units::convert_m_to_unit(effect_front.x);
		effect_position.y += system_units::convert_m_to_unit(effect_front.y);
		effect_position.z += system_units::convert_m_to_unit(effect_front.z);
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		desc.scale = {2.0f, 2.0f, 2.0f};
		play_effect(desc);
	}
}

void boss_seraphim_object::enter_guard()
{
	size_t index = get_model_object()->get_animation_index_from_name("Guard");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	set_flag(character_object::flags::guard, true);
	get_model_object()->play_animation(index, false, 0.0f);
}

void boss_seraphim_object::update_guard(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	float end_time = get_model_object()->get_animation_end_time();
	if (animation_time >= end_time - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		return;
	}
	set_flag(character_object::flags::guard, true);
}

bool boss_seraphim_object::transition_parry(const damage_datas& data)
{
	size_t index = get_model_object()->get_animation_index_from_name("Parrying");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return false;
	}

	//	よろけ方向に強制的に向く
	math::rotator rotation;
	DirectX::XMFLOAT3 sd;
	sd.x = -data.stagger_direction.x;
	sd.y = -data.stagger_direction.y;
	sd.z = -data.stagger_direction.z;
	rotation.set_parameter_from_direction(sd);

	//	水平には制限しておく
	rotation.x = 0.0f;
	rotation.z = 0.0f;
	set_rotation(rotation);

	//	極限回避演出呼び出し
	{
		//	コールバック呼び出し
		time_controll_datas data;
		data.scale = parry_slow_time_scale;
		data.time = parry_slow_time;
		data.exclude_data.object = weak_from_this();
		data.exclude_data.time = parry_self_slow_time;
		for (auto& func : get_cb_time_controlls())
		{
			func.second(data);
		}
	}

	//	エフェクト再生
	{
		play_effect_descs desc;
		desc.id = static_cast<size_t>(effect_ids::parry);
		auto effect_position = get_focus_point();
		auto effect_rotation = get_rotation();
		{
			size_t parent_node_index = get_model_object()->get_model_instance()->get_node_index(parry_node_name);
			if (parent_node_index != model_resource::invalid_id)
			{
				auto& parent_node = get_model_object()->get_model_instance()->get_nodes()[parent_node_index];
				DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&parent_node.world_transform);
				DirectX::XMVECTOR Scale, Rotation, Translation;
				DirectX::XMMatrixDecompose(&Scale, &Rotation, &Translation, WorldTransform);
				DirectX::XMStoreFloat3(&effect_position, Translation);
			}
		}
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		desc.scale = {2.0f, 2.0f, 2.0f};
		play_effect(desc);
	}

	set_flag(character_object::flags::guard, true);
	bool is_same_animation = index == get_model_object()->get_animation_index();
	get_model_object()->play_animation(index, false, system_units::convert_fps_to_sec(is_same_animation ? 0.0f : 5.0f));
	change_state(state_names[state_parry]);
	return true;
}

void boss_seraphim_object::update_parry(float elapsed_time)
{
	set_breaking_deceleration(setup_data::parrying_breaking_deceleration);

	float animation_time = get_model_object()->get_animation_time();
	float end_time = get_model_object()->get_animation_end_time();
	if (animation_time >= end_time - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		return;
	}
	//	ガード判定なのでパリィ中に再度パリィが発生するようにしておく
	set_flag(character_object::flags::guard, true);

	//	一定期間後に攻撃ボタンを押すとパリィ反撃に移行
	if (input_action(elapsed_time))
		return;
}

bool boss_seraphim_object::transition_parry_attack(attack_id id)
{
	current_attack_id = id;
	return change_state(state_names[state_parry_attack]);
}

void boss_seraphim_object::enter_parry_attack()
{
	//	ヒットボックスを削除しておく
	clear_attack_box();

	attack_data_flags = false;
	play_omen_flags = false;
	apprpoaching_effect_flag = false;
	set_velocity({ 0.0f, 0.0f, 0.0f });
	sub_state = 0;
	sub_attack_id = 0;
}

void boss_seraphim_object::update_parry_attack(float elapsed_time)
{
	//	パリィ攻撃中は無敵
	set_breaking_deceleration(walk_breaking_deceleration);
	set_flag(flags::invisible, true);
	switch (current_attack_id)
	{
		case	attack_id::attack_parry_1:
		{
			update_parry_attack_1(elapsed_time);
			break;
		}
		case	attack_id::attack_parry_2:
		{
			update_parry_attack_2(elapsed_time);
			break;
		}
	}
}

void boss_seraphim_object::update_parry_attack_1(float elapsed_time)
{
	switch (sub_state)
	{
		case	0:
		{
			int attack_id = static_cast<int>(current_attack_id);
			size_t index = get_model_object()->get_animation_index_from_name(attack_animation_names[attack_id]);
			get_model_object()->play_animation(index, false);
			//	予兆演出
			{
				//	エフェクト再生
				{
					play_effect_descs desc;
					desc.id = static_cast<size_t>(custom_effect_id::omen);
					auto effect_position = get_position();
					auto effect_rotation = get_rotation();
					auto effect_up = effect_rotation.get_up();
					effect_position.x += system_units::convert_m_to_unit(effect_up.x);
					effect_position.y += system_units::convert_m_to_unit(effect_up.y);
					effect_position.z += system_units::convert_m_to_unit(effect_up.z);
					desc.rotation = effect_rotation;
					desc.position = effect_position;
					desc.scale = {2.0f, 2.0f, 2.0f};
					play_effect(desc);
				}
				//	極限回避演出破棄
				time_controll_datas data;
				for (auto& func : get_cb_time_controlls())
				{
					func.second(data);
				}
				for (auto& callback : on_parry_attck_start)
				{
					callback.second();
				}
			}
			++sub_state;
		}
	}
	if (auto taget_object = get_lockon_taget_object().lock())
	{
		auto focus = taget_object->get_focus_point();
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		set_turn_vector({ focus.x / length, focus.z / length });
	}
	update_attack(elapsed_time);
}

void boss_seraphim_object::update_parry_attack_2(float elapsed_time)
{
	enum substate
	{
		convergence_power_start = 0,
		convergence_power_wait,
		calculate_move_point,
		calculate_move,
		execute_attack,
		execute_attack_end,
	};
	switch (sub_state)
	{
		case	convergence_power_start:
		{
			size_t index = get_model_object()->get_animation_index_from_name("Idle");

			//	エフェクト再生
			{
				play_effect_descs desc;
				desc.id = static_cast<size_t>(custom_effect_id::special_attack_convergence);
				auto effect_position = get_position();
				auto effect_rotation = get_rotation();
				desc.rotation = effect_rotation;
				desc.position = effect_position;
				desc.scale = { 1.0f, 0.25f, 1.0f };
				auto instance = play_effect(desc);
				instance->set_animation_speed(1.0f / parry_slow_time_scale);
			}

			get_model_object()->play_animation(index, true);
			work_float[0] = 1.0f;
			sub_state = convergence_power_wait;
			break;
		}
		case	convergence_power_wait:
		{
			work_float[0] -= elapsed_time;
			//	予兆演出
			if (!play_omen_flags && work_float[0] <= system_units::convert_fps_to_sec(10.0f))
			{
				play_omen_flags = true;
				//	エフェクト再生
				{
					play_effect_descs desc;
					desc.id = static_cast<size_t>(custom_effect_id::omen);
					auto effect_position = get_position();
					auto effect_rotation = get_rotation();
					auto effect_up = effect_rotation.get_up();
					effect_position.x += system_units::convert_m_to_unit(effect_up.x);
					effect_position.y += system_units::convert_m_to_unit(effect_up.y);
					effect_position.z += system_units::convert_m_to_unit(effect_up.z);
					desc.rotation = effect_rotation;
					desc.position = effect_position;
					desc.scale = {2.0f, 2.0f, 2.0f};
					play_effect(desc);
				}
				for (auto& callback : on_parry_attck_start)
				{
					callback.second();
				}
			}
			if (work_float[0] > 0.0f)
				break;

			sub_state = calculate_move_point;
			break;
		}
		case	calculate_move_point:
		{
			//	極限回避演出破棄
			time_controll_datas data;
			for (auto& func : get_cb_time_controlls())
			{
				func.second(data);
			}

			int attack_id = static_cast<int>(current_attack_id);
			size_t index = get_model_object()->get_animation_index_from_name(attack_animation_names[attack_id]);
			get_model_object()->play_animation(index, true);

			//	移動処理中は当たり判定を無視する
			work_float[0] = parry_attack_2_move_time;
			work_float[1] = get_gravity_scale();
			set_gravity_scale(0.0f);
			get_push_box()->set_flag(collision_object::flags::active, false);

			//	移動ポイント算出
			auto taget_object = get_lockon_taget_object().lock();
			if (taget_object)
			{
				auto focus = taget_object->get_position();
				focus.x -= get_position().x;
				focus.y = 0.0f;
				focus.z -= get_position().z;
				float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
				work_float3[0] = get_position();
				work_float3[2] = taget_object->get_position();
				{
					DirectX::XMVECTOR Front, Right;
					Front = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&focus));
					Right = DirectX::XMVector3Cross(DirectX::XMVectorSet(0, 1, 0, 0), Front);
					Front = DirectX::XMVectorScale(Front, 0.5f);
					Right = DirectX::XMVectorScale(Right, length * 0.5f);
					Right = DirectX::XMVectorAdd(Right, DirectX::XMLoadFloat3(&get_position()));
					DirectX::XMStoreFloat3(&work_float3[1], Right);
				}
				work_float3[2].x += get_rotation().get_front().x * parry_attack_2_takedown_point_front;
				work_float3[2].z += get_rotation().get_front().z * parry_attack_2_takedown_point_front;
				work_float3[2].x += get_rotation().get_right().x * parry_attack_2_takedown_point_right;
				work_float3[2].z += get_rotation().get_right().z * parry_attack_2_takedown_point_right;
			}
			sub_state = calculate_move;
			break;
		}
		case	calculate_move:
		{
			//	座標を直接指定
			float previous_rate = work_float[0] / parry_attack_2_move_time;
			work_float[0] = std::max(work_float[0] - elapsed_time, 0.0f);
			float current_rate = std::max(work_float[0] / parry_attack_2_move_time, 0.0f);
			auto p0 = math::bezier(work_float3[0], work_float3[1], work_float3[2], 1 - previous_rate);
			auto p1 = math::bezier(work_float3[0], work_float3[1], work_float3[2], 1 - current_rate);
			set_position(p1);

			//	向きを変える
			DirectX::XMFLOAT3 v;
			v.x = p1.x - p0.x;
			v.y = p1.y - p0.y;
			v.z = p1.z - p0.z;
			float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
			if (length > FLT_EPSILON)
			{
				v.x /= length;
				v.y /= length;
				v.z /= length;
			}
			set_turn_vector({ v.x, v.z });
			if (work_float[0] <= 0.0f)
			{
				add_impulse({v.x * parry_attack_2_remain_velocity_power, 0.0, v.z * parry_attack_2_remain_velocity_power});
				sub_state = execute_attack;
			}
			break;
		}
		case	execute_attack:
		{
			set_gravity_scale(work_float[1]);
			get_push_box()->set_flag(collision_object::flags::active, true);

			size_t index = get_model_object()->get_animation_index_from_name("SpecialAttack2-2");
			get_model_object()->play_animation(index, true);
			sub_state = execute_attack_end;
		}
		case	execute_attack_end:
		{
			float animation_time = get_model_object()->get_animation_time();
			if (parry_attack2_breaking_time <= animation_time)
			{
				set_breaking_deceleration(hi_dash_breaking_deceleration);
				set_flag(flags::invisible, false);
			}
			update_attack(elapsed_time);
			break;
		}
	}
}

bool boss_seraphim_object::input_move(float elapsed_time, move_mode mode, bool* flip_flag)
{
	float accelaration = walk_accelaration;
	float max_speed = walk_max_speed;
	float deceleration = walk_breaking_deceleration;
	float adjust_move_velocity_rate = 1.0f;
	switch (mode)
	{
		case move_mode::dash:
		{
			accelaration = dash_accelaration;
			max_speed = dash_max_speed;
			deceleration = dash_breaking_deceleration;
			break;
		}
		case move_mode::hi_dash:
		{
			accelaration = hi_dash_accelaration;
			max_speed = hi_dash_max_speed;
			deceleration = hi_dash_breaking_deceleration;
			adjust_move_velocity_rate = 0.1f;
			break;
		}
	}

	DirectX::XMFLOAT2 move = DirectX::XMFLOAT2(action_data.move.x, action_data.move.z);
	adjustment_velocity(move, adjust_move_velocity_rate, flip_flag);

	add_move_vector(move, accelaration);
	set_turn_vector(action_data.use_turn_vector ? action_data.turn_vector : move);
	set_max_move_speed(max_speed);
	set_breaking_deceleration(deceleration);

	return move.x != 0 || move.y != 0;
}

bool boss_seraphim_object::fast_apprpoaching_lockon_target(float elapsed_time, const DirectX::XMFLOAT2& range, float power)
{
	auto taget_object = get_lockon_taget_object().lock();
	if (!taget_object)
		return false;

	//	ターゲットの方向に瞬間移動
	auto focus = taget_object->get_focus_point();
	focus.x -= get_position().x;
	focus.z -= get_position().z;
	float length_sq = (focus.x * focus.x + focus.z * focus.z);

	//	距離制限
	float limit_range = get_push_box()->get_local_scale().x;
	limit_range += taget_object->get_push_box()->get_local_scale().x;	//	カプセルコリジョンと判断して処理
	float start_length = limit_range + range.x;
	float end_length = limit_range + range.y;
	if (start_length * start_length <= length_sq && length_sq <= end_length * end_length)
	{
		set_velocity({ focus.x * power, 0.0f, focus.z * power });
		return true;
	}
	return false;
}

bool boss_seraphim_object::calculate_lockon_target_vector(DirectX::XMFLOAT2& direction, float* distance)
{
	auto taget_object = get_lockon_taget_object().lock();
	if (!taget_object)
		return false;

	//	ターゲットの方向に瞬間移動
	auto focus = taget_object->get_focus_point();
	focus.x -= get_position().x;
	focus.z -= get_position().z;
	float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
	direction.x = focus.x / length;
	direction.y = focus.z / length;
	if (distance)
		*distance = length;
	return true;
}

void boss_seraphim_object::register_states()
{
#if defined(USE_IMGUI)
#define SET_NAME(STATE)	state->set_name(#STATE);
#else // defined(USE_IMGUI)
#define SET_NAME(STATE)
#endif	//	defined(USE_IMGUI)
#define REGISTER_MACRO(STATE) \
	{ \
		auto state = std::make_shared<function_character_state>(); \
		state->set_enter([this]() {transi_##STATE(elapsed_time);}); \
		state->set_update([this](float elapsed_time) {update_##STATE(elapsed_time);}); \
		state->set_exit([this]() {attack_hit_flag = false;}); \
		SET_NAME(STATE) \
		register_state(state_##STATE, state); \
	}

//	REGISTER_MACRO(attack);

#undef REGISTER_MACRO
#undef SET_NAME
}
