#include "object/character/enemy/enemy_controller_object.h"

#include "object/character/player/player_controller_object.h"

namespace
{
	constexpr std::string_view SearchLockOnTarget = "ATTACK_SEARCH_LOCKON_TARGET";
	constexpr std::string_view ControllObjectDead = "CONTROLL_OBJECT_DEAD";
}

std::shared_ptr<character_object> enemy_controller_object::get_controll_object() const
{
	return controll_object;
}

DirectX::XMFLOAT3 enemy_controller_object::get_focus_point() const
{
	return controll_object->get_position();
}

enemy_controller_object::enemy_controller_object(std::string_view name, std::shared_ptr<character_object> controll_object)
: game_object(name)
, controll_object(controll_object)
, dead_state(dead_states::none)
#if defined(USE_IMGUI)
, debug_control_mode(false)
#endif	//	defined(USE_IMGUI)
{
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_attack], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_dodge_attack], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_guard], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_parry], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_parry_attack], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_dead_complete], ControllObjectDead,
		[this](character_object* character) {on_character_dead(character); });
}

enemy_controller_object::~enemy_controller_object()
{
	on_finalize();
}

void enemy_controller_object::on_finalize()
{
	state_machine.clear();
	dead_process();
}

void enemy_controller_object::fixed_update(float elapsed_time)
{
	//	死亡破棄処理
	if (dead_state != dead_states::none)
	{
		if (dead_state == dead_states::call)
			dead_process();
		return;
	}

	//	AI更新
#if defined(USE_IMGUI)
	if (!debug_control_mode)
#endif	//	defined(USE_IMGUI)
	{
		state_machine.update(elapsed_time);
	}

#if defined(USE_IMGUI)
	//	入力処理
	if (controll_object && debug_control_mode)
	{
		character_object::action_datas action_data = {};
		action_data.attack = keyboard::instance()->get_state().is_key_state(keyboard::key_states::z);
	//	action_data.dash = true;
		action_data.move = {};
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::j) == input::state::hold)
			action_data.move.x = -1;
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::l) == input::state::hold)
			action_data.move.x = +1;
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::i) == input::state::hold)
			action_data.move.z = +1;
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::k) == input::state::hold)
			action_data.move.z = -1;
		controll_object->input_action_data(action_data);
	}
#endif	//	defined(USE_IMGUI)
}

void enemy_controller_object::update(float elapsed_time)
{
	//	死亡破棄処理
	if (dead_state != dead_states::none)
	{
		if (dead_state == dead_states::call)
			dead_process();
		return;
	}
}

#if defined(USE_IMGUI)
void enemy_controller_object::imgui_render_core()
{
	ImGui::Checkbox("debug_control_mode", &debug_control_mode);
	{
		std::string current_state_name = "none";
		if (state_machine.get_current_state())
			current_state_name = state_machine.get_current_state()->get_name();
		ImGui::Text("current_ai_state : %s", current_state_name.data(), current_state_name.size());
	}
}
#endif	//	defined(USE_IMGUI)

std::string_view enemy_controller_object::get_current_state() const
{
	if (state_machine.get_current_state())
		return state_machine.get_current_state()->get_name();
	return "";
}

void enemy_controller_object::change_state(std::string_view state_id)
{
	state_machine.change_state(state_id);
}

std::weak_ptr<character_object> enemy_controller_object::search_lockon_taget_object(float search_range)
{
	//	基準として、近い的に照準を合わせてるっぽい？
	auto enemy_objects = object_manager::instance()->search_objects<player_controller_object>();
	float nearlest_distance = search_range;

	std::weak_ptr<character_object> nearlest_object;
	for (auto& enemy_object : enemy_objects)
	{
		//	自分は除外
		if (controll_object == enemy_object->get_controll_object() || !enemy_object->get_controll_object())
			continue;

		auto& position = enemy_object->get_controll_object()->get_position();
		float square_distance = position.x * position.x + position.y * position.y + position.z * position.z;
		if (square_distance < nearlest_distance)
		{
			nearlest_object = enemy_object->get_controll_object();
			nearlest_distance = square_distance;
		}
	}

	return nearlest_object;
}

void enemy_controller_object::on_character_dead(character_object* character)
{
	if (controll_object && controll_object.get() == character && dead_state == dead_states::none)
	{
		dead_state = dead_states::call;
	}
}

void enemy_controller_object::on_search_target(character_object* character)
{
	//	ロックオンターゲットの設定
	controll_object->set_lockon_taget_object(search_lockon_taget_object());
}

void enemy_controller_object::dead_process()
{
	if (dead_state != dead_states::call)
		return;

	state_machine.clear();
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_attack], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_dodge_attack], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_guard], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_parry], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_parry_attack], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_dead], ControllObjectDead);

	object_manager::instance()->unregister_object(controll_object);
	object_manager::instance()->unregister_object(shared_from_this());
	controll_object.reset();
	dead_state = dead_states::end;
}
