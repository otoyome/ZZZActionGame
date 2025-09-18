#include "object/character/enemy/BossSeraphim/boss_seraphim_controller_object.h"

#include "object/character/player/player_controller_object.h"

#include "behavior_tree.h"
#include "BehaviorTree/node_base.h"
#include "BehaviorTree/judgment_interface.h"
#include "BehaviorTree/action_interface.h"
#include "BehaviorTree/behavior_data.h"

namespace
{
	constexpr float persuit_hi_dash_judge_range = system_units::convert_m_to_unit(5.0f);
	constexpr float attack_judge_range = system_units::convert_m_to_unit(1.5f);
	constexpr float attack_judge_range_margin = system_units::convert_m_to_unit(0.5f);
	constexpr DirectX::XMFLOAT2 attack_judge_time = {0.8f, 1.2f};

	constexpr DirectX::XMFLOAT2 parry_wait_timer = { 2.0f, 4.0f };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//	登場判定
class entry_judgment : public judgment_interface
{
public:
	entry_judgment(hfsn_boss_seraphim_base* owner)
	: judgment_interface()
	, owner(owner)
	{
	}
	~entry_judgment() override {}
	bool judgment() override
	{
		//	0番が入場フラグ
		return !owner->entry_flag;
	}

private:
	hfsn_boss_seraphim_base* owner;
};

// 登場行動
class entry_action : public action_interface
{
public:
	entry_action(hfsn_boss_seraphim_base* owner)
	: action_interface()
	, owner(owner)
	, step(0)
	{
	}

	~entry_action() override
	{
	}

	action_interface::state run(float elapsed_time) override
	{
		switch (step)
		{
			case 0:
			{
				owner->work_float[0] = 1.0f;
				++step;
				break;
			}
			case 1:
			{
				owner->work_float[0] -= elapsed_time;
				if (owner->work_float[0] <= 0.0f)
				{
					++step;
				}
				break;
			}
			case 2:
			{
				owner->work_float[0] = math::rand_range(1.0f, 2.0f);
				owner->get_controll_object()->set_position(system_units::convert_m_to_unit({0, 0, 0}));
				owner->get_controll_object()->set_active(true);
				owner->get_controll_object()->change_state(character_object::state_names[character_object::state_entry]);
				++step;
				break;
			}
			case 3:
			{
				owner->work_float[0] -= elapsed_time;

				// 待機時間が過ぎた時に通常モードに移行させる
				if (!owner->get_controll_object()->check_same_state(character_object::state_names[character_object::state_entry]) &&
					owner->work_float[0] <= 0.0f)
				{
					step = 0;
					owner->entry_flag = true;
					return action_interface::state::complete;
				}

				break;
			}
		}

		return action_interface::state::run;
	}

private:
	hfsn_boss_seraphim_base* owner;
	int step;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//	第二
////////////////////////////////////////////////////////////////////////////////////////////////////
//	第二移行判定
class second_phase_judgment : public judgment_interface
{
public:
	second_phase_judgment(hfsn_boss_seraphim_base* owner)
	: judgment_interface()
	, owner(owner)
	{
	}
	~second_phase_judgment() override {}
	bool judgment() override
	{
		auto controll_object = owner->get_controll_object();
		float health = static_cast<float>(controll_object->get_health()) / controll_object->get_max_health();
		if (health <= 0.5f)
		{
			owner->is_second_mode = true;
			return true;
		}
		return false;
	}

private:
	hfsn_boss_seraphim_base* owner;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//	第一
////////////////////////////////////////////////////////////////////////////////////////////////////
//	攻撃判定
class attack_phase_judgment : public judgment_interface
{
public:
	attack_phase_judgment(hfsn_boss_seraphim_base* owner)
	: judgment_interface()
	, owner(owner)
	{
	}
	~attack_phase_judgment() override {}
	bool judgment() override
	{
		if (owner->wait_timer > 0.0f)
			return false;

		//	一定範囲内であれば攻撃
		auto target_object = owner->get_controller()->search_lockon_taget_object().lock();
		DirectX::XMFLOAT3 position = owner->get_controll_object()->get_position();
		DirectX::XMFLOAT3 target_position;
		if (target_object)
		{
			target_position = target_object->get_position();
		}
		else
		{
			target_position = owner->get_controll_object()->get_rotation().get_front();
			target_position.x += position.x;
			target_position.y += position.y;
			target_position.z += position.z;
		}

		float vx = target_position.x - position.x;
		float vy = target_position.y - position.y;
		float vz = target_position.z - position.z;
		float length = sqrtf(vx * vx + vy * vy + vz * vz);
		return length < attack_judge_range;
	}

private:
	hfsn_boss_seraphim_base* owner;
};

// 攻撃行動
class attack_action : public action_interface
{
public:
	attack_action(hfsn_boss_seraphim_base* owner)
	: action_interface()
	, owner(owner)
	, step(0)
	{
	}

	~attack_action() override
	{
	}

	action_interface::state run(float elapsed_time) override
	{
		//	第二段階はパリィ判定が入るようになる
		if (owner->is_second_mode)
		{
			owner->parry_wait_timer = std::max(owner->parry_wait_timer - elapsed_time, 0.0f);
			if ((owner->parry_wait_timer <= 0.0f) &&
				(owner->get_controll_object()->get_trunk() > 0) && // スタッガーゲージが最大ならパリィには移行させない
				owner->get_controll_object()->check_same_state(character_object::state_names[character_object::state_damage]))
			{
				owner->work_flag[0] = true;
				owner->get_controll_object()->change_state(character_object::state_names[character_object::state_guard]);
				return action_interface::state::complete;
			}
		}

		switch(step)
		{
			case 0:
			{
				//	距離判定
				{
					auto target_object = owner->get_controller()->search_lockon_taget_object().lock();
					DirectX::XMFLOAT3 position = owner->get_controll_object()->get_position();
					DirectX::XMFLOAT3 target_position;
					if (target_object)
					{
						target_position = target_object->get_position();
					}
					else
					{
						target_position = owner->get_controll_object()->get_rotation().get_front();
						target_position.x += position.x;
						target_position.y += position.y;
						target_position.z += position.z;
					}

					float vx = target_position.x - position.x;
					float vy = target_position.y - position.y;
					float vz = target_position.z - position.z;
					float length = sqrtf(vx * vx + vy * vy + vz * vz);
					//	一定範囲から外に出た
					if (length >= attack_judge_range + attack_judge_range_margin)
					{
						step = 0;
						return action_interface::state::complete;
					}
				}
				character_object::action_datas action_data = {};
				action_data.attack = rand() % 2 == 0 ? input::state::down : input::state::hold;
				owner->get_controll_object()->input_action_data(action_data);
				++step;
				break;
			}
			case 1:
			{
				//	攻撃
				if (owner->get_controll_object()->check_same_state(character_object::state_names[character_object::state_attack]) &&
					owner->get_controll_object()->get_current_attack_id() != boss_seraphim_object::attack_id::attack_3)
				{
					character_object::action_datas action_data = {};
					action_data.attack = input::state::down;
					owner->get_controll_object()->input_action_data(action_data);
				}
				else
				{
					//	攻撃が終わったので終了
					step = 0;
					owner->wait_timer = math::rand_range(attack_judge_time.x, attack_judge_time.y);
					return action_interface::state::complete;
				}
				break;
			}
		}

		return action_interface::state::run;
	}

private:
	hfsn_boss_seraphim_base* owner;
	int step;
};

// 追跡判定
class pursuit_phase_judgment : public judgment_interface
{
public:
	pursuit_phase_judgment(hfsn_boss_seraphim_base* owner)
	: judgment_interface()
	, owner(owner)
	{
	}
	~pursuit_phase_judgment() override {}
	bool judgment() override
	{
		if (owner->wait_timer > 0.0f)
			return false;

		//	一定範囲内であれば攻撃
		auto target_object = owner->get_controller()->search_lockon_taget_object().lock();
		DirectX::XMFLOAT3 position = owner->get_controll_object()->get_position();
		DirectX::XMFLOAT3 target_position;
		if (target_object)
		{
			target_position = target_object->get_position();
		}
		else
		{
			target_position = owner->get_controll_object()->get_rotation().get_front();
			target_position.x += position.x;
			target_position.y += position.y;
			target_position.z += position.z;
		}

		float vx = target_position.x - position.x;
		float vy = target_position.y - position.y;
		float vz = target_position.z - position.z;
		float length = sqrtf(vx * vx + vy * vy + vz * vz);
		return length >= attack_judge_range;
	}

private:
	hfsn_boss_seraphim_base* owner;
};

// 追跡行動
class pursuit_action : public action_interface
{
public:
	pursuit_action(hfsn_boss_seraphim_base* owner)
	: action_interface()
	, owner(owner)
	{
	}

	~pursuit_action() override
	{
	}

	action_interface::state run(float elapsed_time) override
	{
		//	第二段階は喰らい中にパリィ判定が入るようになる
		if (owner->is_second_mode &&
			owner->get_controll_object()->check_same_state(character_object::state_names[character_object::state_damage]))
		{
			owner->work_flag[0] = true;
			owner->get_controll_object()->change_state(character_object::state_names[character_object::state_guard]);
			return action_interface::state::complete;
		}
		auto target_object = owner->get_controller()->search_lockon_taget_object().lock();
		DirectX::XMFLOAT3 position = owner->get_controll_object()->get_position();
		DirectX::XMFLOAT3 target_position;
		if (target_object)
		{
			target_position = target_object->get_position();
		}
		else
		{
			target_position = owner->get_controll_object()->get_rotation().get_front();
			target_position.x += position.x;
			target_position.y += position.y;
			target_position.z += position.z;
		}

		float vx = target_position.x - position.x;
		float vy = target_position.y - position.y;
		float vz = target_position.z - position.z;
		float length = sqrtf(vx * vx + vy * vy + vz * vz);
		//	一定範囲に近づいた
		if (length < attack_judge_range)
		{
			return action_interface::state::complete;
		}

		//	入力
		character_object::action_datas action_data = {};
		action_data.move.x = vx / length;
		action_data.move.z = vz / length;
		auto current_state = owner->get_controll_object()->get_current_state();
		if ((length >= persuit_hi_dash_judge_range && current_state != character_object::state_names[character_object::state_hi_dash])
			||
			(length < persuit_hi_dash_judge_range && current_state != character_object::state_names[character_object::state_dash]))
		{
			action_data.dash = true;
		}
		owner->get_controll_object()->input_action_data(action_data);
		return action_interface::state::run;
	}

private:
	hfsn_boss_seraphim_base* owner;
};

// パリィ攻撃判定
class parry_attack_phase_judgment : public judgment_interface
{
public:
	parry_attack_phase_judgment(hfsn_boss_seraphim_base* owner)
	: judgment_interface()
	, owner(owner)
	{
	}
	~parry_attack_phase_judgment() override {}
	bool judgment() override
	{
		if (!owner->work_flag[0])
			return false;

		if (owner->get_controll_object()->check_same_state(character_object::state_names[character_object::state_parry]))
			return true;
		if (owner->get_controll_object()->check_same_state(character_object::state_names[character_object::state_guard]))
			return true;
		return false;
	}

private:
	hfsn_boss_seraphim_base* owner;
};

// パリィ攻撃行動
class parry_attack_action : public action_interface
{
public:
	parry_attack_action(hfsn_boss_seraphim_base* owner)
	: action_interface()
	, owner(owner)
	, step(0)
	, timer(0)
	{
	}

	~parry_attack_action() override
	{
	}

	action_interface::state run(float elapsed_time) override
	{
		switch(step)
		{
			case 0:
			{
				//attack = rand() % 2 == 0 ? input::state::down : input::state::hold;
				if (owner->parry_level == 0)
				{
					owner->parry_level++;
					attack = input::state::down;
				}
				else if (owner->parry_level == 1)
				{
					owner->parry_level = 0;
					owner->parry_wait_timer = math::rand_range(parry_wait_timer.x, parry_wait_timer.y);
					attack = input::state::hold;
				}
				++step;
				break;
			}
			case 1:
			{
				//	パリィ攻撃が成立するまで待機
				character_object::action_datas action_data = {};
				action_data.attack = attack;
				owner->get_controll_object()->input_action_data(action_data);
				if (owner->get_controll_object()->check_same_state(character_object::state_names[character_object::state_parry_attack]))
				{
					owner->work_flag[0] = 0;
					timer = 0;
					++step;
					break;
				}

				//	一定時間たっても処理されない場合は完了扱いで進める
				timer += elapsed_time;
				if (timer >= 7.0f)
				{
					//	プレイヤー側の入力を可能にしておく
					if (auto& player_controller = object_manager::instance()->search_object<player_controller_object>())
					{
						player_controller->set_input_purmit_flag(true);
					}
					owner->work_flag[0] = 0;
					timer = 0;
					return action_interface::state::complete;
				}
				break;
			}
			case 2:
			{
				if (!owner->get_controll_object()->check_same_state(character_object::state_names[character_object::state_parry_attack]))
				{
					owner->work_flag[0] = 0;
					step = 0;
					timer = 0;
					owner->wait_timer = math::rand_range(attack_judge_time.x, attack_judge_time.y);
					return action_interface::state::complete;
				}
				break;
			}
		}

		return action_interface::state::run;
	}

private:
	hfsn_boss_seraphim_base* owner;
	int step;
	float timer = 0;
	input::state attack = input::state::none;
};

// 待機行動
class idle_action : public action_interface
{
	enum class states
	{
		judge,
		walk_start,
		walking,
	};

public:
	idle_action(hfsn_boss_seraphim_base* owner)
	: action_interface()
	, owner(owner)
	, state(states::judge)
	{
	}

	~idle_action() override
	{
	}

	action_interface::state run(float elapsed_time) override
	{
		//	一定範囲内であれば攻撃
		auto target_object = owner->get_controller()->search_lockon_taget_object().lock();
		DirectX::XMFLOAT3 position = owner->get_controll_object()->get_position();
		DirectX::XMFLOAT3 target_position;
		if (target_object)
		{
			target_position = target_object->get_position();
		}
		else
		{
			target_position = owner->get_controll_object()->get_rotation().get_front();
			target_position.x += position.x;
			target_position.y += position.y;
			target_position.z += position.z;
		}

		float vx = target_position.x - position.x;
		float vz = target_position.z - position.z;
		float length = sqrtf(vx * vx + vz * vz);

		//	向きベクトルのみ変える
		character_object::action_datas action_data = {};
		action_data.use_turn_vector = true;
		action_data.turn_vector.x = vx / length;
		action_data.turn_vector.y = vz / length;

		if (state == states::judge)
		{
			state = states::walk_start;
		}
		switch (state)
		{
		//	case states::idle:
		//	{
		//		break;
		//	}
			case states::walk_start:
			{
				//	左側か右側か判定
				left = rand() % 2 == 0;
				state = states::walking;
			}
			case states::walking:
			{
				DirectX::XMVECTOR Vector = DirectX::XMVectorSet(action_data.turn_vector.x, 0, action_data.turn_vector.y, 0);
				DirectX::XMVECTOR Right = DirectX::XMVector3Cross(Vector, DirectX::XMVectorSet(0.0f, left ? -1.0f : 1.0f, 0.0f, 0.0f));
				DirectX::XMStoreFloat3(&direction, Right);
				action_data.move = direction;
				break;
			}
		}
		owner->get_controll_object()->input_action_data(action_data);

		//	第二段階はパリィ判定が入るようになる
		if (owner->is_second_mode)
		{
			owner->parry_wait_timer = std::max(owner->parry_wait_timer - elapsed_time, 0.0f);

			if ((owner->parry_wait_timer <= 0.0f) &&
				(owner->get_controll_object()->get_trunk() > 0) && // スタッガーゲージが最大ならパリィには移行させない
				owner->get_controll_object()->check_same_state(character_object::state_names[character_object::state_damage]))
			{
				owner->work_float[1] = 0;
				owner->work_flag[0] = true;
				state = states::judge;
				owner->get_controll_object()->change_state(character_object::state_names[character_object::state_guard]);
				return action_interface::state::complete;
			}
		}

		// 待機時間が過ぎた時
		if (owner->wait_timer <= 0.0f)
		{
			state = states::judge;
			return action_interface::state::complete;
		}

		return action_interface::state::run;
	}

private:
	states state;
	bool left;
	DirectX::XMFLOAT3 direction;
	hfsn_boss_seraphim_base* owner;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//	第一段階
class hfsn_first_phase : public hfsn_boss_seraphim_base
{
	HFSN_DECRLEAR(hfsn_first_phase, hfsn_boss_seraphim_base)

public:
	hfsn_first_phase(std::weak_ptr<boss_seraphim_controller_object> controller)
	: hfsn_boss_seraphim_base(type_name(), controller)
	{
	}

	void on_initialize_core() override
	{
		//	ビヘイビアツリー設定
		ai_data = std::make_shared<behavior_data>();
		ai_tree = std::make_shared<behavior_tree>();

		//	登場演出
		ai_tree->add_node("", "Root", 0, behavior_tree_select_rule::priority, nullptr, nullptr);
		ai_tree->add_node("Root", "Entry", 1, behavior_tree_select_rule::priority, std::make_shared<entry_judgment>(this), std::make_shared<entry_action>(this));

		//	第一フェイズ
		ai_tree->add_node("Root", "First", 10, behavior_tree_select_rule::priority, nullptr, nullptr);
		ai_tree->add_node("First", "Attack", 1, behavior_tree_select_rule::none, std::make_shared<attack_phase_judgment>(this), std::make_shared<attack_action>(this));
		ai_tree->add_node("First", "Persuit", 2, behavior_tree_select_rule::none, std::make_shared<pursuit_phase_judgment>(this), std::make_shared<pursuit_action>(this));
		ai_tree->add_node("First", "Idle", 3, behavior_tree_select_rule::priority, nullptr, nullptr);
		ai_tree->add_node("Idle", "Wait", 10, behavior_tree_select_rule::priority, nullptr, std::make_shared<idle_action>(this));

		//	第二フェイズ
		ai_tree->add_node("Root", "Second", 9, behavior_tree_select_rule::priority, std::make_shared<second_phase_judgment>(this), nullptr);
		ai_tree->add_node("Second", "Second Attack", 1, behavior_tree_select_rule::none, std::make_shared<attack_phase_judgment>(this), std::make_shared<attack_action>(this));
		ai_tree->add_node("Second", "Second Persuit", 2, behavior_tree_select_rule::none, std::make_shared<pursuit_phase_judgment>(this), std::make_shared<pursuit_action>(this));
		ai_tree->add_node("Second", "Second Idle", 3, behavior_tree_select_rule::priority, nullptr, nullptr);
		ai_tree->add_node("Second Idle", "Second Parry Attack", 1, behavior_tree_select_rule::priority, std::make_shared<parry_attack_phase_judgment>(this), std::make_shared<parry_attack_action>(this));
		ai_tree->add_node("Second Idle", "Second Wait", 10, behavior_tree_select_rule::priority, nullptr, std::make_shared<idle_action>(this));
	}

	void update(float elapsed_time) override
	{
		//	タイマー処理
		wait_timer -= elapsed_time;

		// 次に実行するノードを推論する。
		if (!active_node)
			active_node = ai_tree->active_node_inference(ai_data);
	
		// ビヘイビアツリーからノードを実行。
		if (active_node)
			active_node = ai_tree->run(active_node, ai_data, elapsed_time);
	}

private:
	std::shared_ptr<behavior_tree> ai_tree;
	std::shared_ptr<behavior_data> ai_data;
	std::shared_ptr<node_base> active_node;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
boss_seraphim_controller_object::boss_seraphim_controller_object(std::string_view name, std::shared_ptr<character_object> controll_object)
: enemy_controller_object(name, controll_object)
{
	//	モデルのカリングフラグを落としておく
	controll_object->get_model_object()->get_model_instance()->set_flag(model_instance::flags::culling, false);
	controll_object->get_model_object()->get_model_instance()->set_flag(model_instance::flags::culling_target_mesh, false);

	get_boss_seraphim_object()->register_change_state_callback(character_object::state_names[character_object::state_parry_attack], TRANSITION_PARRY_ATTACK, [](character_object*)
		{
			//	プレイヤー側の入力を止めておく
			if (auto& player_controller = object_manager::instance()->search_object<player_controller_object>())
			{
				player_controller->set_input_purmit_flag(false);
			}
		});
	get_boss_seraphim_object()->register_parry_attck_start(PARRY_ATTACK_START, []()
		{
			//	プレイヤー側の入力を止めておく
			if (auto& player_controller = object_manager::instance()->search_object<player_controller_object>())
			{
				player_controller->set_input_purmit_flag(true);
			}
		});
}

boss_seraphim_controller_object::~boss_seraphim_controller_object()
{
}

void boss_seraphim_controller_object::on_initialize()
{
	auto controller = std::static_pointer_cast<boss_seraphim_controller_object>(shared_from_this());
	register_state<hfsn_first_phase>(controller);

	change_state(hfsn_first_phase::type_name());
}

void boss_seraphim_controller_object::on_character_dead(character_object* character)
{
	base::on_character_dead(character);
}

#if defined(USE_IMGUI)
void boss_seraphim_controller_object::imgui_render_core()
{
	base::imgui_render_core();
}
#endif	//	defined(USE_IMGUI)
