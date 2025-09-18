#include "object/character/person/kuroko_object.h"

#include "object/decal_object.h"

#include "collision/capsule.h"

#include "collision/sphere.h"

namespace setup_data
{
	constexpr std::string_view model_control_object_name = "モノ・クロコ 制御";
	constexpr int health = 15000; // 体力
	constexpr int trunk = 10000; // 体幹
	constexpr float mass = 48.0f; // 重さ
	constexpr float capsule_push_radius = system_units::convert_m_to_unit(0.4f); // 押し合い判定半径
	constexpr float capsule_hit_radius = system_units::convert_m_to_unit(0.2f); // 当たり判定半径
	constexpr float capsule_length = system_units::convert_m_to_unit(1.4f); // 当たり判定長さ
	constexpr DirectX::XMFLOAT3 capsule_offset = system_units::convert_m_to_unit({0.0f, 1.1f, 0.0f}); // 当たり判定長さオフセット

	constexpr float friction = 0.995f;	// 摩擦係数(ゴム靴？)

	constexpr float walk_accelaration = setup_data::mass * system_units::convert_m_to_unit(5.0f) * 2.0f; // 歩きの加速度(m/s2)
	constexpr float dash_accelaration = setup_data::mass * system_units::convert_m_to_unit(15.0f) * 2.0f; // ダッシュ加速度(m/s2)
	constexpr float hi_dash_accelaration = setup_data::mass * system_units::convert_m_to_unit(20.0f) * 2.0f; // ハイダッシュ加速度(m/s2)

	constexpr float walk_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(3.0f); // 歩きの減速度(m/s2)
	constexpr float dash_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(15.0f) * 2.0f; // ダッシュ減速度(m/s2)
	constexpr float hi_dash_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(20.0f) * 2.0f; // ハイダッシュ減速度(m/s2)
	constexpr float move_stop_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(24.0f) * 3.0f; // ブレーキング減速力(m/s2)
	constexpr float parrying_breaking_deceleration = setup_data::mass * system_units::convert_m_to_unit(24.0f) * 3.0f; 

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
		kuroko_object::attack_id attack_id; // 対応する攻撃Id
		struct next_attack_data
		{
			float start;
			float end;
			kuroko_object::attack_id id;
		};
		next_attack_data next_data[2]; // 移行する攻撃Id
	};
	static constexpr attack_state_datas onwards_attack_state[] =
	{
		{
			character_object::state_names[character_object::state_idle],
			kuroko_object::attack_id::none,
			{
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
			}
		},
		{
			character_object::state_names[character_object::state_walk],
			kuroko_object::attack_id::none,
			{
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
			}
		},
		{
			character_object::state_names[character_object::state_dash],
			kuroko_object::attack_id::none,
			{
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
			}
		},
		{
			character_object::state_names[character_object::state_hi_dash],
			kuroko_object::attack_id::none,
			{
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
			}
		},
		{
			character_object::state_names[character_object::state_move_stop],
			kuroko_object::attack_id::none,
			{
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_1},
			}
		},
		{
			character_object::state_names[character_object::state_dodge],
			kuroko_object::attack_id::none,
			{
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_dodge},
				{0.0f, FLT_MAX, kuroko_object::attack_id::attack_dodge},
			}
		},
		{
			character_object::state_names[character_object::state_dodge_stop],
			kuroko_object::attack_id::none,
			{
				{7.0f / 30.0f, FLT_MAX, kuroko_object::attack_id::attack_dodge},
				{7.0f / 30.0f, FLT_MAX, kuroko_object::attack_id::attack_dodge},
			}
		},
		{
			character_object::state_names[character_object::state_parry],
			kuroko_object::attack_id::none,
			{
				{15.0f / 30.0f, FLT_MAX, kuroko_object::attack_id::attack_parry},
				{15.0f / 30.0f, FLT_MAX, kuroko_object::attack_id::attack_parry},
			}
		},
		{
			character_object::state_names[character_object::state_parry_attack],
			kuroko_object::attack_id::attack_parry,
			{
				{26.0f / 30.0f, 37.0f / 30.0f, kuroko_object::attack_id::attack_6_1},
				{26.0f / 30.0f, 37.0f / 30.0f, kuroko_object::attack_id::attack_6_1},
			}
		},
		/////////////////////////////////////////////////////////////////////////////////////////
		{
			character_object::state_names[character_object::state_attack],
			kuroko_object::attack_id::attack_1,
			{
				{10.0f / 30.0f, 15.0f / 30.0f, kuroko_object::attack_id::attack_2},
				{10.0f / 30.0f, 15.0f / 30.0f, kuroko_object::attack_id::attack_2},
			}
		},
		{
			character_object::state_names[character_object::state_attack],
			kuroko_object::attack_id::attack_2,
			{
				{7.0f / 30.0f, 15.0f / 30.0f, kuroko_object::attack_id::attack_3},
				{7.0f / 30.0f, 15.0f / 30.0f, kuroko_object::attack_id::attack_3},
			}
		},
		{
			character_object::state_names[character_object::state_attack],
			kuroko_object::attack_id::attack_3,
			{
				{15.0f / 30.0f, 25.0f / 30.0f, kuroko_object::attack_id::attack_4},
				{15.0f / 30.0f, 25.0f / 30.0f, kuroko_object::attack_id::attack_4},
			}
		},
		{
			character_object::state_names[character_object::state_attack],
			kuroko_object::attack_id::attack_4,
			{
				{20.0f / 30.0f, 25.0f / 30.0f, kuroko_object::attack_id::attack_5},
				{20.0f / 30.0f, 25.0f / 30.0f, kuroko_object::attack_id::attack_5},
			}
		},
		{
			character_object::state_names[character_object::state_attack],
			kuroko_object::attack_id::attack_5,
			{
				{26.0f / 30.0f, 37.0f / 30.0f, kuroko_object::attack_id::attack_6_1},
				{26.0f / 30.0f, 37.0f / 30.0f, kuroko_object::attack_id::attack_6_1},
			}
		},
		{
			kuroko_object::kuroko_state_names[kuroko_object::state_attack6_1],
			kuroko_object::attack_id::attack_6_1,
			{
				{10000.0f, 10000.0f, kuroko_object::attack_id::none},
				{10000.0f, 10000.0f, kuroko_object::attack_id::none},
			}
		},
		{
			kuroko_object::kuroko_state_names[kuroko_object::state_attack6_3],
			kuroko_object::attack_id::attack_6_3,
			{
				{10.0f / 30.0f, 10000.0f, kuroko_object::attack_id::none},
				{10.0f / 30.0f, 10000.0f, kuroko_object::attack_id::none},
			}
		},
		{
			character_object::state_names[character_object::state_dodge_attack],
			kuroko_object::attack_id::attack_dodge,
			{
				{15.0f / 30.0f, 10000.0f, kuroko_object::attack_id::attack_2},
				{15.0f / 30.0f, 10000.0f, kuroko_object::attack_id::attack_2},
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
		"Attack4",
		"Attack5",
		"Attack6_1",
		"Attack6_3",

		"AttackDodge",
		"AttackParry",

		"AttackSpecial",
	};

	//	ステップアニメーション名
	static constexpr std::string_view step_animation_names[] =
	{
		"Step-Front-Split1",
		"Step-Front-Split2",
		"Step-Back-Split1",
		"Step-Back-Split2",
	};

	//	ダメージアニメーション名
	static constexpr std::string_view damage_animation_names[] =
	{
		"DamageNormal",
		"DamageNormal",
		"DamageNormal",
		"DamageBlown",
		"DamageBlown",
		"DamageBlown",
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
				{0, false},
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
			{3.0f / 30.0f, 15.0f / 30.0f},
			{
				{100, true},
				{100, false, 0.0f},
				{30},
				{30},
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
			{3.0f / 30.0f, 15.0f / 30.0f},
			{
				{100, true},
				{100, false, 0.0f},
				{30},
				{30},
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
			{3.0f / 30.0f, 15.0f / 30.0f},
			{
				{100, true},
				{100, false, 0.0f},
				{30},
				{30},
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
		//	Attack4
		{
			{15.0f / 30.0f, 17.0f / 30.0f},
			{
				{800, true},
				{800, true, 0.25f},
				{150},
				{150},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(6.0f),
				system_units::convert_m_to_unit(0.7f),
				true,
				system_units::convert_fps_to_sec(6.0f),
				system_units::convert_m_to_unit(0.1f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		//	Attack5
		{
			{15.0f / 30.0f, 20.0f / 30.0f},
			{
				{200, false},
				{200, false, 0.0f},
				{50},
				{50},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		{
			{21.0f / 30.0f, 26.0f / 30.0f},
			{
				{200, true},
				{200, false, 0.0f},
				{50},
				{50},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		//	Attack6-1
		{
			{2.0f / 30.0f, 3.0f / 30.0f},
			{
				{100, false},
				{100, false, 0.0f},
				{30},
				{30},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(4.0f),
				system_units::convert_m_to_unit(0.5f),
				true,
				system_units::convert_fps_to_sec(4.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 5.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		{
			{4.0f / 30.0f, 5.0f / 30.0f},
			{
				{100, false},
				{100, false, 0.0f},
				{30},
				{30},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(4.0f),
				system_units::convert_m_to_unit(0.5f),
				true,
				system_units::convert_fps_to_sec(4.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 5.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		{
			{9.0f / 30.0f, 10.0f / 30.0f},
			{
				{100, false},
				{100, false, 0.0f},
				{30},
				{30},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(4.0f),
				system_units::convert_m_to_unit(0.5f),
				true,
				system_units::convert_fps_to_sec(4.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 5.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		{
			{11.0f / 30.0f, 12.0f / 30.0f},
			{
				{100, false},
				{100, false, 0.0f},
				{30},
				{30},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(4.0f),
				system_units::convert_m_to_unit(0.5f),
				true,
				system_units::convert_fps_to_sec(4.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 5.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		//	Attack6-3
		{
			{0.0f / 30.0f, 10.0f / 30.0f},
			{
				{1000, true},
				{1000, true, 0.25f},
				{50},
				{50},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(0.5f),
				true,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(0.4f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({3.0f, 3.0f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 1.5f, 0.0f})}
		},
		//	AttackDodge
		{
			{5.0f / 30.0f, 10.0f / 30.0f},
			{
				{200, false},
				{200, false, 0.0f},
				{30},
				{30},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		{
			{11.0f / 30.0f, 16.0f / 30.0f},
			{
				{200, true},
				{200, true, 0.0f},
				{30},
				{30},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		//	AttackParry
		{
			{15.0f / 30.0f, 20.0f / 30.0f},
			{
				{200, false},
				{200, false, 0.0f},
				{30},
				{30},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		{
			{21.0f / 30.0f, 26.0f / 30.0f},
			{
				{200, true},
				{200, true, 0.0f},
				{30},
				{30},
				character_object::action_level::level_1,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		//	special_2
		{
			{0.0f / 60.0f, 1.0f / 60.0f},
			{
				{50, false},
				{50, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_3,
				system_units::convert_fps_to_sec(0.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		//	special_3
		{
			{6.0f / 30.0f, 7.0f / 30.0f},
			{
				{750, false},
				{250, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_4,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(1.0f),
				true,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(0.4f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 7.5f, 4.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{7.0f / 30.0f, 10.0f / 30.0f},
			{
				{250, false},
				{250, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_4,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(1.0f),
				true,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(0.4f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 7.5f, 4.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		//	special_4
		{
			{0.0f / 60.0f, 1.0f / 60.0f},
			{
				{50, false},
				{50, false, 0.0f},
				{0},
				{0},
				character_object::action_level::level_3,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.1f),
				true,
				system_units::convert_fps_to_sec(2.0f),
				system_units::convert_m_to_unit(0.05f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({2.0f, 1.5f, 2.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 0.0f})}
		},
		//	special_5
		{
			{0.0f / 30.0f, 10.0f / 30.0f},
			{
				{2000, true},
				{1000, true, 0.25f},
				{0},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(0.5f),
				true,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(0.4f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({3.0f, 3.0f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 1.5f, 0.0f})}
		},
		//	BreakAttack
		{
			{5.0f / 30.0f, 6.0f / 30.0f},
			{
				{1500, false},
				{500, false, 0.0f},
				{100},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(1.0f),
				true,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(0.4f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({3.0f, 5.0f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
		{
			{25.0f / 30.0f, 30.0f / 30.0f},
			{
				{3500, true},
				{500, true, 0.0f},
				{400},
				{0},
				character_object::action_level::level_5,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(1.0f),
				true,
				system_units::convert_fps_to_sec(14.0f),
				system_units::convert_m_to_unit(0.4f)
			},
			{collision::bounding_object::types::box, true,
			system_units::convert_m_to_unit({3.0f, 5.0f, 3.0f}), {0, 0, 0}, system_units::convert_m_to_unit({0.0f, 0.5f, 1.0f})}
		},
	};

	//	攻撃ごとの攻撃情報の範囲
	constexpr std::pair<size_t, size_t> attack_data_range[] =
	{
		{00, 00}, // None
		{01, 01}, // Attack1
		{02, 02}, // Attack2
		{03, 03}, // Attack3
		{04, 04}, // Attack4
		{05, 06}, // Attack5
		{07, 10}, // Attack6-1
		{11, 11}, // Attack6-3
		{12, 13}, // AttackDodge
		{14, 15}, // AttackParry
		{16, 16}, // special_2
		{17, 18}, // special_3
		{19, 19}, // special_4
		{20, 20}, // special_5
		{21, 22}, // attack_special
	};

	//	ステートごとの中断耐性
	struct damage_cancel_resist_state_datas
	{
		std::string_view state_id; // 対応するステートId
		kuroko_object::attack_id attack_id; // 対応する攻撃Id
		DirectX::XMFLOAT2 range; // 範囲
		character_object::action_level action_level; // 対応する攻撃Id
	};
	constexpr damage_cancel_resist_state_datas damage_cancel_resist_state_data[] =
	{
		{
			character_object::state_names[character_object::state_attack],
			kuroko_object::attack_id::attack_5,
			{0.0f / 30.0f, 25.0f / 30.0f},
			character_object::action_level::level_5,
		},
		{
			character_object::state_names[kuroko_object::state_attack6_1],
			kuroko_object::attack_id::none,
			{0.0f / 30.0f, 1000.0f / 30.0f},
			character_object::action_level::level_5,
		},
		{
			character_object::state_names[kuroko_object::state_attack6_2],
			kuroko_object::attack_id::none,
			{0.0f / 30.0f, 1000.0f / 30.0f},
			character_object::action_level::level_5,
		},
		{
			character_object::state_names[kuroko_object::state_attack6_3],
			kuroko_object::attack_id::none,
			{0.0f / 30.0f, 1000.0f / 30.0f},
			character_object::action_level::level_5,
		}
	};

	//	
	constexpr float animation_epsilon_time = system_units::convert_fps_to_sec(1.0f);

	//	
	constexpr std::string_view face_texture_path = "Assets/Resources/Kuroko/face_small.png";
	constexpr DirectX::XMFLOAT4 face_texture_uv = {0.0f, 0.0f, 1.0f, 1.0f};

	//	武器関係
	constexpr std::string_view weapon_attach_parent_node_name = "右手首";
	constexpr std::string_view weapon_attack_node_name = "root node";
	constexpr std::string_view weapon_material_color_name = "base_color";

	//	ダッシュ速度変更待機時間
	constexpr float change_dash_state_wait_time = system_units::convert_fps_to_sec(30.0f);

	//	回避待機時間
	constexpr float transition_dodge_wait_lock_time = 0.5f;

	//	カメラフォーカス位置補正
	constexpr DirectX::XMFLOAT3 focus_offset = system_units::convert_m_to_unit({0.0f, 1.4f, 0.0f});

	//	攻撃時にロックオンターゲットに向かって急接近させるための反応範囲
	constexpr DirectX::XMFLOAT2 fast_apprpoaching_range = {system_units::convert_m_to_unit(2.5f), system_units::convert_m_to_unit(7.0f)};

	//	パリィ関係
	constexpr std::string_view parry_node_name = "右手首";
	constexpr DirectX::XMFLOAT2 parry_attack_fast_apprpoaching_range = { system_units::convert_m_to_unit(2.5f), system_units::convert_m_to_unit(100.0f) };

	//	ブレイク攻撃
	constexpr float special_2_fast_apprpoaching_power = system_units::convert_m_to_unit(30.0f);
	constexpr DirectX::XMFLOAT2 special_2_extension_max_time = { 0.5f, 1.0f };

	//	特殊攻撃
	constexpr float attack_special_2_gravity_apply_time = 15.0f / 30.0f;

	//	攻撃の際に各種処理を行う係数
	constexpr DirectX::XMFLOAT2 attack_1_approaching_time = {0.0f / 30.0f, 5.0f / 30.0f};
	constexpr DirectX::XMFLOAT2 attack_5_approaching_time = {3.0f / 30.0f, 15.0f / 30.0f};
	constexpr DirectX::XMFLOAT2 attack_parry_approaching_time = { 3.0f / 30.0f, 15.0f / 30.0f };
	constexpr float attack_6_camera_distance_factor = 0.75f;
	constexpr float attack_6_camera_distance_factor_rate = 0.5f;
	constexpr DirectX::XMFLOAT2 attack_dodge_hide_time = {0.0f / 30.0f, 9.0f / 30.0f};
	constexpr float attack_dodge_normal_speed = 15.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
kuroko_object::kuroko_object(std::shared_ptr<model_object> character_model, std::shared_ptr<model_object> weapon_model, std::string_view collision_filter_name)
: character_object(setup_data::model_control_object_name, character_model, collision_filter_name)
, sub_state(0)
, weapon_model(weapon_model)
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
, roll_value(0.0f)

, current_attack_id(attack_id::none)
, sub_attack_id(0)
, attack_data_flags(false)
, attack_hit_flag(false)

, transition_dodge_wait_time(0)
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

	//	特殊技ゲージ設定
	set_max_special_power(1000);
	set_special_power(500);

	//	覚醒技ゲージ設定
	set_max_deluxe_power(3000);
	set_deluxe_power(1000);

	//	押し合い判定の登録
	get_push_box()->set_filter(collision_filter_name);
	get_push_box()->set_bounding_object(std::make_shared<collision::bounding_capsule>());
	get_push_box()->set_local_scale({setup_data::capsule_push_radius, setup_data::capsule_length, 0.0f});
	get_push_box()->set_local_offset(setup_data::capsule_offset);
	get_push_box()->set_physical_data({setup_data::friction});
	get_push_box()->set_mass(setup_data::mass);
#if defined(USE_IMGUI)
	get_push_box()->set_name(std::string(get_name()) + std::string("push"));
#endif	//	defined(USE_IMGUI)

	//	喰らい判定
	bounding_datas bounding_data;
	bounding_data.desc.is_follow_self = true;
	bounding_data.desc.type = collision::bounding_object::types::capsule;
	bounding_data.desc.local_scale = {setup_data::capsule_hit_radius, setup_data::capsule_length, 0.0f};
	bounding_data.desc.local_offset = setup_data::capsule_offset;
	add_hit_box(bounding_data);

	//	ステップオフセットを腰当たりにしておく
	set_step_offset((setup_data::capsule_push_radius + setup_data::capsule_length) * 0.5f);

	//	ステート情報登録
	register_states();

	//	エフェクト準備
	register_effects();

	//	待機状態へ以降
	change_state(state_names[state_idle]);
}

kuroko_object::~kuroko_object()
{
	object_manager::instance()->unregister_object(weapon_model);
	weapon_model.reset();
}

void kuroko_object::fixed_update(float elapsed_time)
{
	//	傾き破棄
	set_rotation({get_rotation().x, get_rotation().y, 0});

	//	減速力を落とす
	set_breaking_deceleration(walk_breaking_deceleration);

	//	基底の処理を呼び出す
	base::fixed_update(elapsed_time);

	//	中断耐性レベルを変更する
	//	※本来はアニメーション側等で設定できるようにしておきたい所
	{
		action_level damage_cancel_resist_level = action_level::level_1;
		for (auto& data : damage_cancel_resist_state_data)
		{
			if (get_current_state() == data.state_id)
			{
				bool find = true;
				if (data.attack_id != kuroko_object::attack_id::none)
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

	//	傾き適応
	set_rotation({get_rotation().x, get_rotation().y, roll_value});
	roll_value *= 0.1f;

	//	アクション情報クリア
	action_data = {};
}

void kuroko_object::post_update(float elapsed_time)
{
	//	基底の処理を呼び出す
	base::post_update(elapsed_time);

	//	回避時間待機時間
	transition_dodge_wait_time = std::max(transition_dodge_wait_time - elapsed_time, 0.0f);

	//	武器のノードの位置を反映
	if (weapon_model)
	{
		size_t parent_node_index = get_model_object()->get_model_instance()->get_node_index(weapon_attach_parent_node_name);
		size_t attach_node_index = get_model_object()->get_model_instance()->get_node_index(weapon_attack_node_name);
		if (parent_node_index != model_resource::invalid_id && attach_node_index != model_resource::invalid_id)
		{
			//	指定のノードの姿勢を取得
			auto& parent_node = get_model_object()->get_model_instance()->get_nodes()[parent_node_index];
			DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&parent_node.world_transform);
			DirectX::XMVECTOR Scale, Rotation, Translation;
			DirectX::XMMatrixDecompose(&Scale, &Rotation, &Translation, WorldTransform);
			Rotation = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, DirectX::XM_PI), Rotation);

			//	ノードに指定して更新
			model_instance::node_animation node_animation;
			DirectX::XMStoreFloat3(&node_animation.scale, Scale);
			DirectX::XMStoreFloat4(&node_animation.rotation, Rotation);
			DirectX::XMStoreFloat3(&node_animation.translation, Translation);
			math::rotator rotation = node_animation.rotation;
			node_animation.translation.x += rotation.get_right().x;
			node_animation.translation.y += rotation.get_right().y;
			node_animation.translation.z += rotation.get_right().z;
			weapon_model->set_node_pose(attach_node_index, node_animation);
			weapon_model->update_transform();
		}

		for (auto& material : weapon_model->get_model_instance()->get_materials())
		{
			DirectX::XMFLOAT4 color;
			material.get_float4(weapon_material_color_name, color);
			color.w = weapon_alpha;
			material.set_float4(weapon_material_color_name, color);
		}
	}
}

#if defined(USE_IMGUI)
void kuroko_object::imgui_render_core()
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
void kuroko_object::input_action_data(action_datas action_data)
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

int kuroko_object::get_use_special_power() const
{
	//	固定で500使用
	return 500;
}

DirectX::XMFLOAT3 kuroko_object::get_focus_point() const
{
	DirectX::XMFLOAT3 position = get_position();
	if (check_same_state(state_names[state_break_attack]) ||
		check_same_state(kuroko_state_names[state_break_attack_2]) ||
		check_same_state(kuroko_state_names[state_break_attack_3]) ||
		check_same_state(kuroko_state_names[state_special_5]))
	{
		//	6番攻撃のみ浮遊前の高さを使用
		size_t node_index = get_model_object()->get_model_instance()->get_node_index("頭");
		if (node_index != model_instance::invalid_id)
		{
			position.x = get_model_object()->get_model_instance()->get_nodes()[node_index].world_transform._41;
			position.y = get_model_object()->get_model_instance()->get_nodes()[node_index].world_transform._42;
			position.z = get_model_object()->get_model_instance()->get_nodes()[node_index].world_transform._43;
		}
	}
	else
	{
		//	6番攻撃のみ浮遊前の高さを使用
		if (current_attack_id == attack_id::attack_6_1 || current_attack_id == attack_id::attack_6_3)
		{
			position.y = get_footstep_position().y;
		}
		position.x += focus_offset.x;
		if (current_attack_id == attack_id::attack_6_3)
		{
			position.y += focus_offset.y * 0.5f;
		}
		else
		{
			position.y += focus_offset.y;
		}
		position.z += focus_offset.z;
	}
	return position;
}

kuroko_object::attack_id kuroko_object::get_current_attack_id() const
{
	return current_attack_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void kuroko_object::on_damaged(const damage_datas& data)
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

void kuroko_object::on_guarded(const damage_datas& data)
{
	base::on_guarded(data);

	transition_parry(data);
}

void kuroko_object::on_attack_box_enter(collision_object::data data)
{
	base::on_attack_box_enter(data);

	attack_hit_flag = true;
}

void kuroko_object::on_deead()
{
	//	既に死亡済みなら呼び出さない
	if (is_flag(flags::dead))
		return;

	base::on_deead();

	transition_dead();
}

void kuroko_object::on_stagger(const stagger_datas& data)
{
	//	既によろけ済みなら呼び出さない
	if (is_flag(flags::stagger))
		return;

	base::on_stagger(data);

	transition_stagger();
}

void kuroko_object::on_change_camera_translucent(bool flag)
{
	for (auto& material : weapon_model->get_model_instance()->get_materials())
	{
		material.set_flag(material::flags::camera_distance_translucent, true);
	}
}

void kuroko_object::on_hit_while_dodging(const damage_datas& data)
{
	base::on_hit_while_dodging(data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool kuroko_object::input_action(float elapsed_time)
{
	//	回避処理
	if (action_data.dodge && transition_dodge_wait_time <= FLT_EPSILON)
	{
		return change_state(state_names[state_dodge]);
	}

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
	if (action_data.special && get_special_power() >= get_use_special_power())
	{
		return change_state(state_names[state_special]);
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

		switch(id)
		{
			case attack_id::invalid:
			{
				//	特に何もしない
				break;
			}
			case attack_id::attack_dodge:
			{
				//	回避攻撃へ移行
				return change_state(state_names[state_dodge_attack]);
			}
			case attack_id::attack_parry:
			{
				//	パリィ反撃へ移行
				return change_state(state_names[state_parry_attack]);
			}
			case attack_id::attack_6_1:
			{
				//	6番の攻撃は特殊なため処理を変更する
				return change_state(kuroko_state_names[state_attack6_1]);
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

void kuroko_object::clear_datas()
{
	//	当たり判定破棄
	clear_attack_box();

	//	ロックオンターゲット破棄
	set_lockon_taget_object(std::weak_ptr<character_object>());

	//	攻撃Idクリア
	current_attack_id = attack_id::none;

	//	ヒットボックス生成管理フラグ
	attack_data_flags = false;

	//	サブステートクリア
	sub_state = 0;
	sub_attack_id = 0;

	//	ブレーキング減速力
	set_breaking_deceleration(walk_breaking_deceleration);

	//	押し合い判定戻す
	get_push_box()->set_flag(collision_object::flags::active, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void kuroko_object::enter_entry()
{
	size_t index = get_model_object()->get_animation_index_from_name("Entry");
	if (index == model_resource::invalid_id)
	{
		change_state(state_names[state_idle]);
		return;
	}
	clear_datas();
	work_float[0] = 0;
	get_model_object()->play_animation(index, false, 0.0f);
}

void kuroko_object::update_entry(float elapsed_time)
{
	float end_time = get_model_object()->get_animation_end_time();
	if (get_model_object()->get_animation_time() >= end_time - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		return;
	}
	work_float[0] += elapsed_time;
	static constexpr DirectX::XMFLOAT2 range = {85.0f / 30.0f, 115.0f / 30.0f};
	weapon_alpha = math::smoothstep(range.x, range.y, work_float[0]);
}

void kuroko_object::enter_idle()
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

void kuroko_object::update_idle(float elapsed_time)
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

void kuroko_object::enter_walk()
{
	//	この時点では特に何もしない
}

void kuroko_object::update_walk(float elapsed_time)
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
#if 0
		if (cross < -0.2f)
			animation_index = get_model_object()->get_animation_index_from_name("LeftMove");
		else if (cross > 0.2f)
			animation_index = get_model_object()->get_animation_index_from_name("RightMove");
		else
#endif
		if (dot < -0.2f)
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

void kuroko_object::enter_dash()
{
	size_t index = get_model_object()->get_animation_index_from_name("RunningNormal");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	get_model_object()->play_animation(index, true, 0.1f);
}

void kuroko_object::update_dash(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (input_action(elapsed_time))
	{
		return;
	}

	//	移動
	if (!input_move(elapsed_time, move_mode::dash))
	{
		change_state(state_names[state_move_stop]);
		return;
	}

	//	ダッシュ入力
	if (action_data.dash && animation_time >= change_dash_state_wait_time)
	{
		transition_hi_dash();
		return;
	}
}

void kuroko_object::transition_hi_dash(float blend_animation_time)
{
	size_t index = get_model_object()->get_animation_index_from_name("RunningFast");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}
	get_model_object()->play_animation(index, true, blend_animation_time);
	change_state(state_names[state_hi_dash]);
}

void kuroko_object::update_hi_dash(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (input_action(elapsed_time))
	{
		return;
	}

	//	移動入力中
	bool input_flip = false;
	if (!input_move(elapsed_time, move_mode::hi_dash, &input_flip))
	{
		change_state(state_names[state_move_stop]);
		return;
	}

	//	反転
	if (input_flip)
	{
		change_state(kuroko_state_names[state_hi_dash_flip]);
		return;
	}

	//	ダッシュ入力
	if (action_data.dash && animation_time >= change_dash_state_wait_time)
	{
		change_state(state_names[state_dash]);
		return;
	}
}

void kuroko_object::enter_hi_dash_flip()
{
	size_t index = get_model_object()->get_animation_index_from_name("RunningFastFlip");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}
	set_turn_vector({0, 0});
	set_breaking_deceleration(hi_dash_breaking_deceleration);
	get_model_object()->play_animation(index, false);
}

void kuroko_object::update_hi_dash_flip(float elapsed_time)
{
	set_turn_vector({0, 0});
	set_breaking_deceleration(hi_dash_breaking_deceleration);
	if (input_action(elapsed_time))
	{
		return;
	}

	float animation_time = get_model_object()->get_animation_time();
	float end_time = get_model_object()->get_animation_end_time();
	if (animation_time < end_time - animation_epsilon_time)
		return;

	if (animation_time >= 10.0f / 30.0f)
	{
		//	初速を与えておく
		DirectX::XMFLOAT2 move = DirectX::XMFLOAT2(action_data.move.x, action_data.move.z);
		add_impulse({move.x * hi_dash_max_speed, 0.0f, move.y * hi_dash_max_speed});
	}

	bool input_flip = false;
	if (input_move(elapsed_time, move_mode::hi_dash))
	{
		transition_hi_dash();
		return;
	}
	change_state(state_names[state_move_stop]);
}

void kuroko_object::enter_move_stop()
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

	get_model_object()->play_animation(index, false, 0.1f);
}

void kuroko_object::update_move_stop(float elapsed_time)
{
	float end_time = get_model_object()->get_animation_end_time();
	if (get_model_object()->get_animation_time() >= end_time - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		return;
	}

	//	待機処理も利用
	update_idle(elapsed_time);

	//	ブレーキング減速力
	set_breaking_deceleration(setup_data::move_stop_breaking_deceleration);
}

void kuroko_object::enter_dodge()
{
	//	入力があれば
	bool axis_input = action_data.move.x * action_data.move.x + action_data.move.z * action_data.move.z > 0.01f * 0.01f;
	size_t index = get_model_object()->get_animation_index_from_name(step_animation_names[(!axis_input) * 2]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	clear_datas();

	//	エフェクト再生
	{
		play_effect_descs desc;
		desc.id = static_cast<size_t>(effect_ids::shock_wave);
		auto effect_position = get_focus_point();
		auto effect_rotation = get_rotation();
		auto effect_front = effect_rotation.get_front();
		if (axis_input)
		{
			effect_position.x += system_units::convert_m_to_unit(effect_front.x);
			effect_position.y += system_units::convert_m_to_unit(effect_front.y);
			effect_position.z += system_units::convert_m_to_unit(effect_front.z);
		}
		else
		{
			effect_rotation.y = math::normalize_angle(effect_rotation.y + DirectX::XM_PI);
			effect_position.x -= system_units::convert_m_to_unit(effect_front.x);
			effect_position.y -= system_units::convert_m_to_unit(effect_front.y);
			effect_position.z -= system_units::convert_m_to_unit(effect_front.z);
		}
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		desc.scale = {2.0f, 2.0f, 2.0f};
		play_effect(desc);
	}

	//	回避中は無敵
	set_flag(character_object::flags::invisible, true);

	//	速度は無理やり切る
	work_float3[0] = get_velocity();
	set_velocity({0.0f, 0.0f, 0.0f});
	transition_dodge_wait_time = transition_dodge_wait_lock_time;
	set_turn_vector({action_data.move.x * axis_input, action_data.move.z * axis_input});
	get_model_object()->play_animation(index, false, system_units::convert_fps_to_sec(3.0f));
}

void kuroko_object::update_dodge(float elapsed_time)
{
	if (get_model_object()->get_animation_time() >= get_model_object()->get_animation_end_time() - animation_epsilon_time)
	{
		transition_dodge_stop();
		return;
	}

	//	回避中は無敵
	set_flag(character_object::flags::invisible, true);

	//	回避中に攻撃ボタンが押されれば反撃
	if (input_action(elapsed_time))
	{
		return;
	}
}

void kuroko_object::transition_dodge_stop()
{
	bool axis_input = action_data.move.x * action_data.move.x + action_data.move.z * action_data.move.z > 0.01f * 0.01f;
	bool axis_input_motion = get_model_object()->get_playing_animation_name() != step_animation_names[0];
	size_t index = get_model_object()->get_animation_index_from_name(step_animation_names[1 + 2 * axis_input_motion]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	前ステップ時に入力していたらハイダッシュへ移行
	if (!axis_input_motion && axis_input)
	{

		set_velocity({action_data.move.x * hi_dash_max_speed, 0, action_data.move.z * hi_dash_max_speed});
		transition_hi_dash();
		return;
	}

	get_model_object()->play_animation(index, false, 0.0f);
	change_state(state_names[state_dodge_stop]);
}

void kuroko_object::update_dodge_stop(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (animation_time >= get_model_object()->get_animation_end_time() - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		return;
	}

	update_idle(elapsed_time);
}

void kuroko_object::transition_damage(const damage_datas& data)
{
	int animation_index = static_cast<int>(data.desc.attack_level);
	size_t index = get_model_object()->get_animation_index_from_name(damage_animation_names[animation_index]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	ステートクリア
	clear_datas();

	//get_model_object()->play_animation(index, false, system_units::convert_fps_to_sec(2.0f));
	get_model_object()->play_animation(index, false, 0.0f);
	change_state(state_names[state_damage]);
}

void kuroko_object::enter_damage()
{
	work_float[0] = 0;
}

void kuroko_object::update_damage(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (animation_time >= get_model_object()->get_animation_end_time() - animation_epsilon_time)
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

void kuroko_object::transition_dead()
{
	change_state(kuroko_state_names[state_dead_start]);
}

void kuroko_object::enter_dead_start()
{
	size_t index = get_model_object()->get_animation_index_from_name("DeadStart");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	ステートクリア
	clear_datas();

	get_model_object()->play_animation(index, false);
}

void kuroko_object::update_dead_start(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (get_model_object()->get_animation_end_time() <= animation_time)
	{
		change_state(state_names[state_dead]);
		return;
	}
}

void kuroko_object::enter_dead()
{
	size_t index = get_model_object()->get_animation_index_from_name("Dead");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	ステートクリア
	clear_datas();

	get_model_object()->play_animation(index, true, 0.0f);
}

void kuroko_object::update_dead(float elapsed_time)
{
	//	ゲーム的には待機状態に以降
}

void kuroko_object::enter_dead_complete()
{
}

void kuroko_object::update_dead_complete(float elapsed_time)
{
	//	特に何もしない
	//	コントローラー側で処理する
}

void kuroko_object::enter_regenerate()
{
	size_t index = get_model_object()->get_animation_index_from_name("Regenerate");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	ステートクリア
	clear_datas();

	//	復活時は少しの間無敵にする
	set_flag(character_object::flags::dead, false);
	set_health(get_max_health());
	set_invisile_timer(game_constants::regenerate_invisible_time);

	get_model_object()->play_animation(index, false);
}

void kuroko_object::update_regenerate(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (get_model_object()->get_animation_end_time() <= animation_time)
	{
		change_state(state_names[state_idle]);
		return;
	}
}

void kuroko_object::transition_stagger()
{
	size_t index = get_model_object()->get_animation_index_from_name(stagger_animation_names[0]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	ステートクリア
	clear_datas();

	get_model_object()->play_animation(index, false, system_units::convert_fps_to_sec(5.0f));
	change_state(state_names[state_stagger]);
}

void kuroko_object::update_stagger(float elapsed_time)
{
	if (!is_flag(flags::stagger))
	{
		transition_stagger_complete();
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

void kuroko_object::transition_stagger_complete()
{
	size_t index = get_model_object()->get_animation_index_from_name(stagger_animation_names[2]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	ステートクリア
	clear_datas();

	set_flag( character_object::flags::invisible, true);
	get_model_object()->play_animation(index, false, 0.0f);
	change_state(state_names[state_stagger_complete]);
}

void kuroko_object::update_stagger_complete(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (get_model_object()->get_animation_end_time() - animation_epsilon_time <= animation_time)
	{
		change_state(state_names[state_idle]);
		return;
	}
}

void kuroko_object::enter_break_attack()
{
	//	ロックオンターゲットの検索
	auto taget_object = get_lockon_taget_object().lock();
	if (taget_object)
	{
		//	ターゲットの方向に修正
		auto focus = taget_object->get_focus_point();
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		set_turn_vector({focus.x / length, focus.z / length});
	}
	
	size_t index = get_model_object()->get_animation_index_from_name("AttackSpecial-1");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	ヒットボックスを削除しておく
	clear_attack_box();

	sub_state = 0;
	sub_attack_id = 0;
	attack_data_flags = false;
	current_attack_id = attack_id::break_attack;
	set_flag(character_object::flags::invisible, work_float[3] > 0.5f);
	set_position({get_position().x, get_position().y + 0.01f, get_position().z});
	set_gravity_scale(system_units::convert_fps_to_sec(0.0f));
	set_flag(flags::on_ground, false);
	get_model_object()->play_animation(index, false);
	DirectX::XMFLOAT3 front = get_rotation().get_front();
	move_stop_vector = {front.x, front.z};
}

void kuroko_object::update_break_attack(float elapsed_time)
{
	//	ロックオンターゲットの検索
	auto taget_object = get_lockon_taget_object().lock();
	if (taget_object)
	{
		//	ターゲットの方向に修正
		auto focus = taget_object->get_focus_point();
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		set_turn_vector({ focus.x / length, focus.z / length });
	}

	//	カメラをプレイヤーの後ろに送る
	{
		camera_transform_descs desc;
		desc.set_flag(camera_transform_descs::flags::set_focus_direction, true);
		auto front = get_rotation().get_front();
		auto right = get_rotation().get_right();
		desc.focus_direction.x = +front.x + right.x * 0.5f;
		desc.focus_direction.y = +front.y + right.y * 0.5f;
		desc.focus_direction.z = +front.z + right.z * 0.5f;
		desc.set_flag(camera_transform_descs::flags::set_focus_distance_factor, true);
		desc.focus_distance_factor = 0.5f;
		desc.focus_distance_rate = 0.5f;
		call_camera_transforms(desc);
	}

	float animation_time = get_model_object()->get_animation_time();
	if (animation_time >= get_model_object()->get_animation_end_time() - animation_epsilon_time)
	{
		change_state(kuroko_state_names[state_break_attack_2]);
		return;
	}

	//	動作中は無敵付与
	set_flag(character_object::flags::invisible, true);
	DirectX::XMFLOAT2 move = {get_rotation().get_front().x, get_rotation().get_front().z};
	add_move_vector(move, walk_accelaration);
	set_max_move_speed(walk_max_speed);
	set_breaking_deceleration(hi_dash_breaking_deceleration);
}

void kuroko_object::enter_break_attack_2()
{
	size_t index = get_model_object()->get_animation_index_from_name("AttackSpecial-2");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	風エフェクト再生
	{
		play_effect_descs desc;
		desc.id = static_cast<size_t>(custom_effect_id::front_wind_line);
		auto effect_position = get_position();
		auto effect_rotation = get_rotation();
		auto effect_front = effect_rotation.get_front();
		effect_position.x += system_units::convert_m_to_unit(effect_front.x * 0.5f);
		effect_position.y += system_units::convert_m_to_unit(effect_front.y * 0.5f);
		effect_position.z += system_units::convert_m_to_unit(effect_front.z * 0.5f);
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		play_effect(desc);
	}

	clear_attack_box();

	attack_data_flags = false;
	sub_state = 0;
	sub_attack_id = 0;
	work_float[0] = 0.0f;
	set_flag(character_object::flags::invisible, true);
	get_model_object()->play_animation(index, true, 0.1f);
	DirectX::XMFLOAT3 front = get_rotation().get_front();
	move_stop_vector = {front.x, front.z};
}

void kuroko_object::update_break_attack_2(float elapsed_time)
{
	//	動作中は無敵付与
	set_flag(character_object::flags::invisible, true);

	//	カメラを引く
	{
		camera_transform_descs desc;
		desc.set_flag(camera_transform_descs::flags::set_focus_direction, true);
		auto front = get_rotation().get_front();
		auto right = get_rotation().get_right();
		desc.focus_direction.x = +front.x + right.x * 0.5f;
		desc.focus_direction.y = +front.y + right.y * 0.5f;
		desc.focus_direction.z = +front.z + right.z * 0.5f;
		desc.set_flag(camera_transform_descs::flags::set_focus_distance_factor, true);
		desc.focus_distance_factor = 0.5f;
		desc.focus_distance_rate = 0.5f;
		call_camera_transforms(desc);
	}

	//	移動処理
	{
		DirectX::XMFLOAT3 target_position;
		auto taget_object = get_lockon_taget_object().lock();
		if (taget_object)
		{
			target_position = taget_object->get_position();
		}
		else
		{
			target_position = get_rotation().get_front();
		}
	
		//	ターゲットの方向に瞬間移動
		auto focus = target_position;
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		focus.x /= length;
		focus.z /= length;
		set_velocity({ focus.x * special_2_fast_apprpoaching_power, 0.0f, focus.z * special_2_fast_apprpoaching_power });

		if (length <= system_units::convert_m_to_unit(2.5f))
		{
			change_state(kuroko_state_names[state_break_attack_3]);
			return;
		}
	}
}

void kuroko_object::enter_break_attack_3()
{
	size_t index = get_model_object()->get_animation_index_from_name("AttackSpecial-3");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	爆破エフェクト再生
	{
		play_effect_descs desc;
		desc.id = static_cast<size_t>(custom_effect_id::ground_break_effect);
		auto effect_position = get_position();
		auto effect_rotation = get_rotation();
		auto effect_front = effect_rotation.get_front();
		effect_position.x += system_units::convert_m_to_unit(effect_front.x * 0.5f);
		effect_position.y += system_units::convert_m_to_unit(effect_front.y * 0.5f);
		effect_position.z += system_units::convert_m_to_unit(effect_front.z * 0.5f);
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		play_effect(desc);
	}

	//	ヒットボックスを削除しておく
	clear_attack_box();

	//	現在のベクトル
	float speed = sqrtf(get_velocity().x * get_velocity().x + get_velocity().z * get_velocity().z);
	move_stop_vector = DirectX::XMFLOAT2(get_velocity().x / speed, get_velocity().z / speed);
	sub_state = 0;
	sub_attack_id = 0;
	attack_data_flags = false;
	current_attack_id = attack_id::break_attack;
	set_gravity_scale(1.0f);
	set_velocity({0.0f, 0.0f, 0.0f});
	get_model_object()->play_animation(index, false, 0.0f);
}

void kuroko_object::update_break_attack_3(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	float end_time = get_model_object()->get_animation_end_time();
	if (animation_time >= end_time - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		return;
	}

	//	攻撃エフェクト再生
	{
		auto& data_range = attack_data_range[static_cast<int>(attack_id::break_attack)];
		auto& data_list = attack_data_list[data_range.second];
		if ((data_list.time_range.x <= animation_time && animation_time < data_list.time_range.y) &&
			!attack_data_flags)
		{
			play_effect_descs desc;
			desc.id = static_cast<size_t>(custom_effect_id::break_attack_impact);
			auto effect_position = get_position();
			auto effect_rotation = get_rotation();
			auto effect_front = effect_rotation.get_front();
			auto effect_up = effect_rotation.get_up();
			effect_position.x += system_units::convert_m_to_unit(effect_front.x * 0.5f) + system_units::convert_m_to_unit(effect_up.x * 1.5f);
			effect_position.y += system_units::convert_m_to_unit(effect_front.y * 0.5f) + system_units::convert_m_to_unit(effect_up.y * 1.5f);
			effect_position.z += system_units::convert_m_to_unit(effect_front.z * 0.5f) + system_units::convert_m_to_unit(effect_up.z * 1.5f);
			desc.rotation = effect_rotation;
			desc.position = effect_position;
			play_effect(desc);
		}
	}

	//	攻撃判定関係
	update_attack_box(animation_time, get_rotation().get_front());

	//	動作中は無敵付与
	set_flag(character_object::flags::invisible, true);
}

bool kuroko_object::transition_attack(attack_id id)
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

	current_attack_id = id;
	attack_data_flags = false;
	apprpoaching_effect_flag = false;
	set_velocity({ 0.0f, 0.0f, 0.0f });
	get_model_object()->play_animation(index, false);
	change_state(state_names[state_attack]);
	sub_state = 0;
	sub_attack_id = 0;
	return true;
}

void kuroko_object::update_attack(float elapsed_time)
{
	//	ロックオンターゲットの検索
	auto taget_object = get_lockon_taget_object().lock();
	if (taget_object)
	{
		//	ターゲットの方向に修正
		auto focus = taget_object->get_focus_point();
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		set_turn_vector({focus.x / length, focus.z / length});
	}

	//	待機状態へ移行
	float animation_time = get_model_object()->get_animation_time();
	if (animation_time >= get_model_object()->get_animation_end_time() - animation_epsilon_time)
	{
		current_attack_id = attack_id::none;
		change_state(state_names[state_idle]);
		return;
	}

	//	攻撃判定関係
	update_attack_box(animation_time, get_rotation().get_front());

	//	攻撃毎の特殊対応
	bool hide_model = false;
	get_model_object()->get_model_instance()->set_flag(model_instance::flags::color_render, true);
	get_model_object()->get_model_instance()->set_flag(model_instance::flags::shadow_caster, true);
	weapon_model->get_model_instance()->set_flag(model_instance::flags::color_render, true);
	weapon_model->get_model_instance()->set_flag(model_instance::flags::shadow_caster, true);
	switch (current_attack_id)
	{
		case attack_id::attack_1:
		{
			if (attack_1_approaching_time.x <= animation_time && animation_time < attack_1_approaching_time.y)
			{
				//	相手の方向に瞬間移動
				if (fast_apprpoaching_lockon_target(elapsed_time, fast_apprpoaching_range, 7.0f))
				{
					set_flag( character_object::flags::invisible, true);
					hide_model = true;
				}
			}
			else
			{
				set_velocity({});
			}
			break;
		}
		case attack_id::attack_5:
		{
			//	特定フレーム間は消す
			if (attack_5_approaching_time.x <= animation_time && animation_time < attack_5_approaching_time.y)
			{
				//	カメラをプレイヤーの後ろに送る
				{
					camera_transform_descs desc;
					desc.set_flag(camera_transform_descs::flags::set_focus_direction, true);
					auto front = get_rotation().get_front();
					auto right = get_rotation().get_right();
					desc.focus_direction.x = +front.x + right.x;
					desc.focus_direction.y = +front.y + right.y;
					desc.focus_direction.z = +front.z + right.z;
					call_camera_transforms(desc);
				}

				//	相手の方向に瞬間移動
				fast_apprpoaching_lockon_target(elapsed_time, fast_apprpoaching_range);
				set_flag( character_object::flags::invisible, true);
				hide_model = true;
			}
			else
			{
				set_velocity({});
			}
			break;
		}
		case attack_id::attack_parry:
		{
			//	特定フレーム間は消す
			if (attack_parry_approaching_time.x <= animation_time && animation_time < attack_parry_approaching_time.y)
			{
				//	カメラをプレイヤーの後ろに送る
				{
					camera_transform_descs desc;
					desc.set_flag(camera_transform_descs::flags::set_focus_direction, true);
					auto front = get_rotation().get_front();
					auto right = get_rotation().get_right();
					desc.focus_direction.x = +front.x + right.x;
					desc.focus_direction.y = +front.y + right.y;
					desc.focus_direction.z = +front.z + right.z;
					call_camera_transforms(desc);
				}

				//	相手の方向に瞬間移動
				fast_apprpoaching_lockon_target(elapsed_time, parry_attack_fast_apprpoaching_range);
				set_flag(character_object::flags::invisible, true);
				hide_model = true;
			}
			else
			{
				set_velocity({});
		}
			break;
	}
		case attack_id::attack_dodge:
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
	weapon_model->get_model_instance()->set_flag(model_instance::flags::color_render, !hide_model);
	weapon_model->get_model_instance()->set_flag(model_instance::flags::shadow_caster, !hide_model);

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

void kuroko_object::enter_attack6_1()
{
	//	ロックオンターゲットの検索
	auto taget_object = get_lockon_taget_object().lock();
	if (taget_object)
	{
		//	ターゲットの方向に修正
		auto focus = taget_object->get_focus_point();
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		set_turn_vector({focus.x / length, focus.z / length});
	}

	size_t index = get_model_object()->get_animation_index_from_name("Attack6-1");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	current_attack_id = attack_id::attack_6_1;
	sub_state = 0;
	sub_attack_id = 0;
	attack_data_flags = false;
	set_flag(character_object::flags::invisible, true);
	set_velocity({ 0.0f, 0.0f, 0.0f });
	set_position({get_position().x, get_position().y + 0.01f, get_position().z});
	set_gravity_scale(system_units::convert_fps_to_sec(-0.0125f));	//	ちょっと浮かせる
	set_flag(flags::on_ground, false);
	get_model_object()->play_animation(index, false);
}

void kuroko_object::update_attack6_1(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (animation_time >= get_model_object()->get_animation_end_time() - animation_epsilon_time)
	{
		change_state(kuroko_state_names[state_attack6_2]);
		return;
	}

	//	動作中は無敵付与
	set_flag(character_object::flags::invisible, true);

	//	重力をスケーリングして浮かす
	set_gravity_scale(-0.025f * elapsed_time);

	//	攻撃判定関係
	update_attack_box(animation_time, get_rotation().get_front());

	//	カメラを引く
	{
		camera_transform_descs desc;
		desc.set_flag(camera_transform_descs::flags::set_focus_distance_factor, true);
		desc.focus_distance_factor = attack_6_camera_distance_factor;
		desc.focus_distance_rate = attack_6_camera_distance_factor_rate;
		call_camera_transforms(desc);
	}
}

void kuroko_object::enter_attack6_2()
{
	size_t index = get_model_object()->get_animation_index_from_name("Attack6-2");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	set_flag(character_object::flags::invisible, true);
	DirectX::XMFLOAT3 front = get_rotation().get_front();
	move_stop_vector = {front.x, front.z};
	attack_data_flags = false;
	sub_state = 0;
	sub_attack_id = 0;
	add_move_vector(move_stop_vector, hi_dash_accelaration);
	set_breaking_deceleration(hi_dash_accelaration);
	set_max_move_speed(hi_dash_max_speed);
	get_model_object()->play_animation(index, true, 0.0f);
}

void kuroko_object::update_attack6_2(float elapsed_time)
{
	if (is_flag(flags::on_ground))
	{
		change_state(kuroko_state_names[state_attack6_3]);
		return;
	}

	//	カメラを引く
	{
		camera_transform_descs desc;
		desc.set_flag(camera_transform_descs::flags::set_focus_distance_factor, true);
		desc.focus_distance_factor = attack_6_camera_distance_factor;
		desc.focus_distance_rate = attack_6_camera_distance_factor_rate;
		call_camera_transforms(desc);
	}

	//	動作中は無敵付与
	set_flag(character_object::flags::invisible, true);
	set_gravity_scale(15.0f);

	add_move_vector(move_stop_vector, hi_dash_accelaration * 2);
	set_max_move_speed(hi_dash_max_speed);
	set_breaking_deceleration(hi_dash_accelaration);
}

void kuroko_object::enter_attack6_3()
{
	set_gravity_scale(1.0f);

	size_t index = get_model_object()->get_animation_index_from_name("Attack6-3");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	爆破エフェクト再生
	{
		play_effect_descs desc;
		desc.id = static_cast<size_t>(custom_effect_id::ground_break_effect);
		auto effect_position = get_position();
		auto effect_rotation = get_rotation();
		auto effect_front = effect_rotation.get_front();
		effect_position.x += system_units::convert_m_to_unit(effect_front.x * 0.5f);
		effect_position.y += system_units::convert_m_to_unit(effect_front.y * 0.5f);
		effect_position.z += system_units::convert_m_to_unit(effect_front.z * 0.5f);
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		play_effect(desc);
	}

	//	ステージに対してデカール(エフェクト側で設定する方が望ましい)
	{
		math::compute_fade::calculator time_data = math::compute_fade::calculator({0.0f, 0.01f}, {5.0f, 10.0f});
		auto decal = std::make_shared<decal_object>("攻撃6-3 : デカール", time_data);
		decal->set_scale(system_units::convert_m_to_unit({ 5.0f, 5.0f, 5.0f }));
		decal->set_rotation({ DirectX::XM_PI/2, 0, 0 });
		decal->set_position(get_position());
		decal->get_material().set_texture(decal_object::material_variable_name_base_texture, attack_6_decal);
		object_manager::instance()->register_object(decal);
	}

	//	現在のベクトル
	float speed = sqrtf(get_velocity().x * get_velocity().x + get_velocity().z * get_velocity().z);
	move_stop_vector = DirectX::XMFLOAT2(get_velocity().x / speed, get_velocity().z / speed);
	current_attack_id = attack_id::attack_6_3;
	sub_state = 0;
	sub_attack_id = 0;
	attack_data_flags = false;
	set_gravity_scale(1.0f);
	set_velocity({0.0f, 0.0f, 0.0f});
	get_model_object()->play_animation(index, false, 0.0f);
}

void kuroko_object::update_attack6_3(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	float end_time = get_model_object()->get_animation_end_time();
	if (animation_time >= end_time - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		set_gravity_scale(1.0f);
		return;
	}

	//	攻撃判定関係
	update_attack_box(animation_time, get_rotation().get_front());

	//	動作中は無敵付与
	set_flag( character_object::flags::invisible, true);

	//	次の行動
	if (input_action(elapsed_time))
	{
		set_gravity_scale(1.0f);
		return;
	}
}

void kuroko_object::enter_special()
{
	size_t index = get_model_object()->get_animation_index_from_name("AttackBreak-1");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	特殊技使用
	work_float[3] = 0;
	if (get_use_special_power() <= get_special_power())
	{
		set_special_power(get_special_power() - get_use_special_power());
		work_float[3] = 1;
	}

	sub_state = 0;
	sub_attack_id = 0;
	attack_data_flags = false;
	set_flag(character_object::flags::invisible, true);
	set_velocity({ 0.0f, 0.0f, 0.0f });
	get_model_object()->play_animation(index, false);
}

void kuroko_object::update_special(float elapsed_time)
{
	//	ロックオンターゲットの検索
	auto taget_object = get_lockon_taget_object().lock();
	if (taget_object)
	{
		//	ターゲットの方向に修正
		auto focus = taget_object->get_focus_point();
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		set_turn_vector({focus.x / length, focus.z / length});
	}

	float animation_time = get_model_object()->get_animation_time();
	if (animation_time >= get_model_object()->get_animation_end_time() - animation_epsilon_time)
	{
		change_state(kuroko_state_names[state_special_2]);
		return;
	}

	//	動作中は無敵付与
	set_flag(character_object::flags::invisible, true);

	//	カメラを引く
	{
		camera_transform_descs desc;
		desc.set_flag(camera_transform_descs::flags::set_focus_distance_factor, true);
		desc.focus_distance_factor = attack_6_camera_distance_factor;
		desc.focus_distance_rate = attack_6_camera_distance_factor_rate;
		call_camera_transforms(desc);
	}
}

void kuroko_object::enter_special_2()
{
	size_t index = get_model_object()->get_animation_index_from_name("AttackBreak-2");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	エフェクト再生
	{
		play_effect_descs desc;
		desc.id = static_cast<size_t>(custom_effect_id::grand_viper_frame);
		auto effect_position = get_position();
		auto effect_rotation = get_rotation();
		auto effect_front = effect_rotation.get_front();
		effect_position.x += system_units::convert_m_to_unit(effect_front.x);
		effect_position.y += system_units::convert_m_to_unit(effect_front.y);
		effect_position.z += system_units::convert_m_to_unit(effect_front.z);
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		desc.scale = {2.0f, 2.0f, 2.0f};
		play_effect(desc);
	}

	//	ヒットボックスを削除しておく
	clear_attack_box();

	current_attack_id = attack_id::special_2;
	set_flag(character_object::flags::invisible, true);
	DirectX::XMFLOAT3 front = get_rotation().get_front();
	move_stop_vector = {front.x, front.z};
	attack_data_flags = false;
	sub_state = 0;
	sub_attack_id = 0;
	work_float[0] = 0.0f;
	get_model_object()->play_animation(index, true, 0.0f);
}

void kuroko_object::update_special_2(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (animation_time >= special_2_extension_max_time.x)
	{
		//	入力があり、かつ制限時間までは続ける
		if (!(action_data.is_attacked() && (animation_time < special_2_extension_max_time.y)))
		{
			change_state(kuroko_state_names[state_special_3]);
			return;
		}
	}

	//	移動処理
	{
		DirectX::XMFLOAT3 target_position;
		auto taget_object = get_lockon_taget_object().lock();
		if (taget_object)
		{
			target_position = taget_object->get_position();
		}
		else
		{
			target_position = get_rotation().get_front();
		}
	
		//	ターゲットの方向に瞬間移動
		auto focus = target_position;
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		focus.x /= length;
		focus.z /= length;
		set_velocity({ focus.x * special_2_fast_apprpoaching_power, 0.0f, focus.z * special_2_fast_apprpoaching_power });
	}

	//	攻撃判定関係
	{
		update_attack_box(animation_time - work_float[0], get_rotation().get_front());
		if (sub_attack_id != 0)
		{
			work_float[0] = animation_time;
			sub_attack_id = 0;

			//	エフェクト再生
			{
				play_effect_descs desc;
				desc.id = static_cast<size_t>(custom_effect_id::grand_viper_frame);
				auto effect_position = get_position();
				auto effect_rotation = get_rotation();
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
	}

	//	動作中は無敵付与
	set_flag(character_object::flags::invisible, true);
	set_max_move_speed(hi_dash_max_speed);
	set_breaking_deceleration(walk_breaking_deceleration);
}

void kuroko_object::enter_special_3()
{
	//	ロックオンターゲットの検索
	auto taget_object = get_lockon_taget_object().lock();
	if (taget_object)
	{
		//	ターゲットの方向に修正
		auto focus = taget_object->get_focus_point();
		focus.x -= get_position().x;
		focus.z -= get_position().z;
		float length = sqrtf(focus.x * focus.x + focus.z * focus.z);
		set_turn_vector({focus.x / length, focus.z / length});
	}

	size_t index = get_model_object()->get_animation_index_from_name("AttackBreak-3");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	ヒットボックスを削除しておく
	clear_attack_box();

	sub_state = 0;
	sub_attack_id = 0;
	attack_data_flags = false;
	current_attack_id = attack_id::special_3;
	set_flag(character_object::flags::invisible, true);
	set_position({get_position().x, get_position().y + 0.01f, get_position().z});
	set_gravity_scale(system_units::convert_fps_to_sec(0.0f));
	set_flag(flags::on_ground, false);
	get_model_object()->play_animation(index, false);
	DirectX::XMFLOAT3 front = get_rotation().get_front();
	move_stop_vector = {front.x, front.z};
}

void kuroko_object::update_special_3(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (animation_time >= get_model_object()->get_animation_end_time() - animation_epsilon_time)
	{
		change_state(kuroko_state_names[state_special_4]);
		return;
	}

	//	重力をスケーリングして浮かす
	set_gravity_scale(0.0f);

	//	攻撃判定関係
	update_attack_box(animation_time, get_rotation().get_front());

	//	カメラを引く
	{
		camera_transform_descs desc;
		desc.set_flag(camera_transform_descs::flags::set_focus_distance_factor, true);
		desc.focus_distance_factor = attack_6_camera_distance_factor;
		desc.focus_distance_rate = attack_6_camera_distance_factor_rate;
		call_camera_transforms(desc);
	}

	//	動作中は無敵付与
	set_flag(character_object::flags::invisible, true);
	set_max_move_speed(walk_max_speed);
	set_breaking_deceleration(hi_dash_breaking_deceleration * 3);
}

void kuroko_object::enter_special_4()
{
	size_t index = get_model_object()->get_animation_index_from_name("AttackBreak-4");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_special_4]);
		return;
	}

	attack_data_flags = false;
	sub_state = 0;
	sub_attack_id = 0;
	work_float[0] = 0.0f;
	current_attack_id = attack_id::special_4;
	set_flag(character_object::flags::invisible, true);
	get_model_object()->play_animation(index, true, 0.0f);
	DirectX::XMFLOAT3 front = get_rotation().get_front();
	move_stop_vector = {front.x, front.z};
}

void kuroko_object::update_special_4(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	if (is_flag(flags::on_ground))
	{
		change_state(kuroko_state_names[state_special_5]);
		return;
	}

	//	攻撃判定関係
	{
		update_attack_box(animation_time - work_float[0], get_rotation().get_front());
		if (sub_attack_id != 0)
		{
			work_float[0] = animation_time;
			sub_attack_id = 0;
		}
	}

	//	カメラを引く
	{
		camera_transform_descs desc;
		desc.set_flag(camera_transform_descs::flags::set_focus_distance_factor, true);
		desc.focus_distance_factor = attack_6_camera_distance_factor;
		desc.focus_distance_rate = attack_6_camera_distance_factor_rate;
		call_camera_transforms(desc);
	}

	//	動作中は無敵付与
	set_flag(character_object::flags::invisible, true);
	set_gravity_scale(25.0f);

	add_move_vector(move_stop_vector, hi_dash_accelaration * 4.0f);
	set_max_move_speed(hi_dash_max_speed);
	set_breaking_deceleration(walk_breaking_deceleration);
}

void kuroko_object::enter_special_5()
{
	set_gravity_scale(1.0f);

	size_t index = get_model_object()->get_animation_index_from_name("AttackBreak-5");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		change_state(state_names[state_idle]);
		return;
	}

	//	爆破エフェクト再生
	{
		play_effect_descs desc;
		desc.id = static_cast<size_t>(custom_effect_id::ground_break_effect);
		auto effect_position = get_position();
		auto effect_rotation = get_rotation();
		auto effect_front = effect_rotation.get_front();
		effect_position.x += system_units::convert_m_to_unit(effect_front.x * 0.5f);
		effect_position.y += system_units::convert_m_to_unit(effect_front.y * 0.5f);
		effect_position.z += system_units::convert_m_to_unit(effect_front.z * 0.5f);
		desc.rotation = effect_rotation;
		desc.position = effect_position;
		play_effect(desc);
	}

	//	ステージに対してデカール(エフェクト側で設定する方が望ましい)
	{
		math::compute_fade::calculator time_data = math::compute_fade::calculator({0.0f, 0.01f}, {5.0f, 10.0f});
		auto decal = std::make_shared<decal_object>("ブレイク攻撃-5 : デカール", time_data);
		decal->set_scale(system_units::convert_m_to_unit({ 10.0f, 10.0f, 10.0f }));
		decal->set_rotation({ DirectX::XM_PI/2, 0, 0 });
		decal->set_position(get_position());
		decal->get_material().set_texture(decal_object::material_variable_name_base_texture, attack_6_decal);
		object_manager::instance()->register_object(decal);
	}

	//	現在のベクトル
	sub_state = 0;
	sub_attack_id = 0;
	attack_data_flags = false;
	current_attack_id = attack_id::special_5;
	set_gravity_scale(1.0f);
	set_velocity({0.0f, 0.0f, 0.0f});
	get_model_object()->play_animation(index, false, 0.0f);

	float speed = sqrtf(get_velocity().x * get_velocity().x + get_velocity().z * get_velocity().z);
	move_stop_vector = DirectX::XMFLOAT2(get_velocity().x / speed, get_velocity().z / speed);
}

void kuroko_object::update_special_5(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	float end_time = get_model_object()->get_animation_end_time();
	if (animation_time >= end_time - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		set_gravity_scale(1.0f);
		return;
	}

	//	攻撃判定関係
	update_attack_box(animation_time, get_rotation().get_front());

	//	動作中は無敵付与
	set_flag( character_object::flags::invisible, true);

	//	次の行動
	if (input_action(elapsed_time))
	{
		set_gravity_scale(1.0f);
		return;
	}
}

void kuroko_object::enter_dodge_attack()
{
	float animation_time = get_model_object()->get_animation_time();
	size_t index = get_model_object()->get_animation_index_from_name(attack_animation_names[static_cast<int>(attack_id::attack_dodge)]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	//	ヒットボックスを削除しておく
	clear_attack_box();

	current_attack_id = attack_id::attack_dodge;
	attack_data_flags = false;
	apprpoaching_effect_flag = false;
	set_velocity({ 0.0f, 0.0f, 0.0f });
	get_model_object()->play_animation(index, false);
	sub_state = 0;
	sub_attack_id = 0;
}

void kuroko_object::update_dodge_attack(float elapsed_time)
{
	update_attack(elapsed_time);
}

void kuroko_object::update_deluxe(float elapsed_time)
{
}

void kuroko_object::enter_guard()
{
	size_t index = get_model_object()->get_animation_index_from_name("Guard");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	set_flag(character_object::flags::guard, true);
	get_model_object()->play_animation(index, false, system_units::convert_fps_to_sec(3.0f));
}

void kuroko_object::update_guard(float elapsed_time)
{
	float animation_time = get_model_object()->get_animation_time();
	float end_time = get_model_object()->get_animation_end_time();
	if (animation_time >= end_time - animation_epsilon_time)
	{
		change_state(state_names[state_idle]);
		return;
	}
	set_breaking_deceleration(hi_dash_breaking_deceleration);
	set_flag(character_object::flags::guard, true);
}

bool kuroko_object::transition_parry(const damage_datas& data)
{
	size_t index = get_model_object()->get_animation_index_from_name("Parrying");
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return false;
	}
	clear_datas();

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

void kuroko_object::update_parry(float elapsed_time)
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

void kuroko_object::enter_parry_attack()
{
	int attack_id = static_cast<int>(kuroko_object::attack_id::attack_parry);
	float animation_time = get_model_object()->get_animation_time();
	size_t index = get_model_object()->get_animation_index_from_name(attack_animation_names[attack_id]);
	if (index == model_resource::invalid_id)
	{
		assert(index == model_resource::invalid_id);
		return;
	}

	//	ヒットボックスを削除しておく
	clear_attack_box();

	current_attack_id = kuroko_object::attack_id::attack_parry;
	attack_data_flags = false;
	apprpoaching_effect_flag = false;
	set_velocity({ 0.0f, 0.0f, 0.0f });
	get_model_object()->play_animation(index, false);
	sub_state = 0;
	sub_attack_id = 0;
}

void kuroko_object::update_parry_attack(float elapsed_time)
{
	update_attack(elapsed_time);
}

bool kuroko_object::input_move(float elapsed_time, move_mode mode, bool* flip_flag)
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

	//	入力ベクトル方向に速度ベクトルを修正する
	DirectX::XMFLOAT2 move = DirectX::XMFLOAT2(action_data.move.x, action_data.move.z);
	adjustment_velocity(move, adjust_move_velocity_rate, flip_flag);

	add_move_vector(move, accelaration);
	set_turn_vector(move);
	set_max_move_speed(max_speed);
	set_breaking_deceleration(deceleration);

	return move.x != 0 || move.y != 0;
}

void kuroko_object::update_attack_box(float animation_time, DirectX::XMFLOAT3 stagger_direction)
{
	auto& data_range = attack_data_range[static_cast<int>(current_attack_id)];
	auto& attack_box = attack_data_list[data_range.first + sub_attack_id];
	if (sub_attack_id <= data_range.second - data_range.first)
	{
		//	当たり判定生成
		if (!attack_data_flags && (attack_box.time_range.x <= animation_time && animation_time < attack_box.time_range.y))
		{
			bounding_datas box_data;
			box_data.desc = attack_box.bounds;
			box_data.position = get_position();
			box_data.rotator = get_rotation();
			damage_datas damage_data;
			damage_data.desc = attack_box.attack;
			damage_data.stagger_direction = stagger_direction;
			add_attack_box(box_data, damage_data);
			attack_data_flags = true;
		}

		//	当たり判定破棄
		if (attack_data_flags && attack_box.time_range.y <= animation_time)
		{
			clear_attack_box();
			attack_data_flags = false;
			++sub_attack_id;
		}
	}
}

bool kuroko_object::fast_apprpoaching_lockon_target(float elapsed_time, const DirectX::XMFLOAT2& range, float power)
{
	auto taget_object = get_lockon_taget_object().lock();
	if (!taget_object)
		return false;
	
	//	ターゲットの方向に瞬間移動
	auto focus = taget_object->get_position();
	focus.x -= get_position().x;
	focus.z -= get_position().z;
	float length_sq = (focus.x * focus.x + focus.z * focus.z);

	//	距離制限
	float limit_range = get_push_box()->get_local_scale().x
					  + taget_object->get_push_box()->get_local_scale().x;	//	カプセルコリジョンと判断して処理
	float start_length = limit_range + range.x;
	float end_length = limit_range + range.y;
	if (start_length * start_length <= length_sq && length_sq <= end_length * end_length)
	{
		set_velocity({ focus.x * power, 0.0f, focus.z * power });
		return true;
	}
	return false;
}

bool kuroko_object::calculate_lockon_target_vector(DirectX::XMFLOAT2& direction, float* distance)
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

void kuroko_object::register_states()
{
#define REGISTER_MACRO(STATE) \
	{ \
		register_state<function_finite_state_node>("state_"###STATE, \
													[this](){enter_##STATE();}, \
													[this](float elapsed_time) {update_##STATE(elapsed_time);}, \
													[this](){}); \
	}

	REGISTER_MACRO(attack6_1);
	REGISTER_MACRO(attack6_2);
	REGISTER_MACRO(attack6_3);
	REGISTER_MACRO(hi_dash_flip);

	REGISTER_MACRO(special_2);
	REGISTER_MACRO(special_3);
	REGISTER_MACRO(special_4);
	REGISTER_MACRO(special_5);

	REGISTER_MACRO(break_attack_2);
	REGISTER_MACRO(break_attack_3);

	REGISTER_MACRO(dead_start);
#undef REGISTER_MACRO
}
