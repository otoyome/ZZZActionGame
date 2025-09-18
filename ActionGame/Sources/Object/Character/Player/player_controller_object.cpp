#include "object/character/player/player_controller_object.h"

#include "object/character/enemy/enemy_controller_object.h"

#include "object/game_constant.h"

namespace
{
	constexpr std::string_view SearchLockOnTarget = "SEARCH_LOCKON_TARGET";
	constexpr std::string_view ControllObjectDead = "CONTROLL_OBJECT_DEAD";
	constexpr std::string_view ControllObjectDeadComplete = "CONTROLL_OBJECT_DEAD_COMPLETE";

	constexpr std::string_view OnStaggerEnterId = "STAGGER_ENTER_CALLBACK";
}

std::shared_ptr<character_object> player_controller_object::get_controll_object() const
{
	return controll_object;
}

DirectX::XMFLOAT3 player_controller_object::get_focus_point() const
{
	return controll_object->get_position();
}

bool player_controller_object::get_input_purmit_flag() const
{
	return input_purmit_flag;
}

void player_controller_object::set_input_purmit_flag(bool input_purmit_flag)
{
	this->input_purmit_flag = input_purmit_flag;
}

player_controller_object::player_controller_object(std::string_view name, std::shared_ptr<character_object> controll_object, std::shared_ptr<camera_object> camera)
: game_object(name)
, work_time(0)
, input_purmit_flag(true)
, camera(camera)
, controll_object(controll_object)
, mode(modes::default)
, action_datas()
, dead_state(dead_states::none)
{
	controll_object->set_basic_movement_dash(true);
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_attack], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_special], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_deluxe], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_dodge_attack], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_guard], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_parry], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });
	controll_object->register_change_state_callback(character_object::state_names[character_object::state_break_attack], SearchLockOnTarget,
		[this](character_object* character) {on_search_target(character); });

	controll_object->register_change_state_callback(character_object::state_names[character_object::state_dead], ControllObjectDead,
		[this](character_object* character) {on_character_dead(character); });

	controll_object->register_change_state_callback(character_object::state_names[character_object::state_dead_complete], ControllObjectDeadComplete,
		[this](character_object* character) {on_character_dead_complete(character); });

	controll_object->register_stagger_hit_callback(OnStaggerEnterId, [this](const character_object::stagger_datas& data) {on_stagger_enter(data); });

	//	プレイヤーなので体幹は無敵にしておく
	controll_object->set_flag(character_object::flags::trunk_invisible, true);
}

player_controller_object::~player_controller_object()
{
	dead_process();
}

void player_controller_object::fixed_update(float elapsed_time)
{
	//	死亡破棄処理
	if (dead_state != dead_states::none)
	{
		if (dead_state == dead_states::call)
			dead_process();
		return;
	}

	//	入力記録
	auto keyboard_state = keyboard::instance()->get_state();
	auto mouse_state = mouse::instance()->get_state();
	switch (mode)
	{
		case	modes::default:
		{
			for (size_t index = ARRAYSIZE(action_datas) - 1; index > 0; --index)
			{
				action_datas[index] = action_datas[index - 1];
			}
			action_datas[0].move = get_move_vector();
			action_datas[0].dash = keyboard_state.is_key_state(keyboard::key_states::shift) == input::state::down;
			action_datas[0].guard = keyboard_state.is_key_state(keyboard::key_states::space) == input::state::down;

			action_datas[0].attack = mouse_state.is_key_state(mouse::key_states::left);
			action_datas[0].dodge = mouse_state.is_key_state(mouse::key_states::right) == input::state::down;
			action_datas[0].special = keyboard_state.is_key_state(keyboard::key_states::e) == input::state::down;
			action_datas[0].deluxe = keyboard_state.is_key_state(keyboard::key_states::q) == input::state::down;

			//	アクション情報引き渡し
			static constexpr size_t check_frame = static_cast<size_t>(system_units::convert_sec_to_fps(0.1f) + 0.1f);
			static constexpr float frame_move_rate = 1.0f / static_cast<float>(check_frame);
			character_object::action_datas action_data = {};
			bool input_attack = false;
			for (size_t index = 0; index < check_frame; ++index)
			{
				action_data.move.x += action_datas[index].move.x * frame_move_rate;
				action_data.move.y += action_datas[index].move.y * frame_move_rate;
				action_data.move.z += action_datas[index].move.z * frame_move_rate;
				action_data.dash = action_datas[index].dash ? true : action_data.dash;
				action_data.guard = action_datas[index].guard ? true : action_data.guard;

				if (action_datas[index].attack == input::state::down || action_datas[index].attack == input::state::hold)
					input_attack = true;
				action_data.dodge = action_datas[index].dodge ? true : action_data.dodge;
				action_data.special = action_datas[index].special ? true : action_data.special;
				action_data.deluxe = action_datas[index].deluxe ? true : action_data.deluxe;
			}

			//	ため判定
			if (input_attack)
			{
				bool flag = true;
				for (size_t index = 0; index < check_frame; ++index)
				{
					if (!(action_datas[index].attack == input::state::down || action_datas[index].attack == input::state::hold))
						flag = false;
				}
				action_data.attack = flag ? input::state::hold : input::state::down;
			}

			//	
			if (input_purmit_flag)
			{
				controll_object->input_action_data(action_data);
			}
			break;
		}
		case	modes::stagger:
		{
			work_time -= elapsed_time;
			if (work_time >= game_constants::stagger_wait_time - game_constants::stagger_input_wait_time)
				break;

			bool is_attack = character_object::action_datas::is_attacked(mouse_state.is_key_state(mouse::key_states::left));
			bool is_cancel = character_object::action_datas::is_attacked(mouse_state.is_key_state(mouse::key_states::right));
			if (is_attack || is_cancel || work_time <= 0.0f)
			{
				if (is_attack)
					controll_object->change_state(character_object::state_names[character_object::state_break_attack]);

				mode = modes::default;

				//	コールバック
				for (auto& func : on_stagger_exits)
				{
					func.second();
				}
				break;
			}
			break;
		}
		case	modes::dead_select:
		{
			work_time += elapsed_time;
			if (work_time < 1.5f)
				break;

			bool is_attack = character_object::action_datas::is_attacked(mouse_state.is_key_state(mouse::key_states::left));
			bool is_cancel = character_object::action_datas::is_attacked(mouse_state.is_key_state(mouse::key_states::right));
			if (is_attack)
			{
				controll_object->change_state(character_object::state_names[character_object::state_regenerate]);
				mode = modes::default;

				//	コールバック
				for (auto& func : on_select_regenerate)
				{
					func.second();
				}
				break;
			}
			if (is_cancel)
			{
				controll_object->change_state(character_object::state_names[character_object::state_dead_complete]);
				mode = modes::default;

				//	コールバック
				for (auto& func : on_select_dead)
				{
					func.second();
				}
				break;
			}
			break;
		}
	}
}

bool player_controller_object::register_stagger_enter_callback(std::string_view identifier, call_back_stagger_enter call_back)
{
	std::string id = std::string(identifier);
	if (on_stagger_enters.find(id) != on_stagger_enters.end())
		return false;

	on_stagger_enters.insert_or_assign(id, call_back);
	return true;
}

bool player_controller_object::unregister_stagger_enter_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_stagger_enters.find(id);
	if (it != on_stagger_enters.end())
		return false;

	on_stagger_enters.erase(it);
	return true;
}

bool player_controller_object::register_stagger_exit_callback(std::string_view identifier, call_back_stagger_exit call_back)
{
	std::string id = std::string(identifier);
	if (on_stagger_exits.find(id) != on_stagger_exits.end())
		return false;

	on_stagger_exits.insert_or_assign(id, call_back);
	return true;
}

bool player_controller_object::unregister_stagger_exit_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_stagger_exits.find(id);
	if (it != on_stagger_exits.end())
		return false;

	on_stagger_exits.erase(it);
	return true;
}

bool player_controller_object::register_dead_start_callback(std::string_view identifier, call_back_dead_start call_back)
{
	std::string id = std::string(identifier);
	if (on_dead_start.find(id) != on_dead_start.end())
		return false;

	on_dead_start.insert_or_assign(id, call_back);
	return true;
}

bool player_controller_object::unregister_dead_start_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_dead_start.find(id);
	if (it != on_dead_start.end())
		return false;

	on_dead_start.erase(it);
	return true;
}

bool player_controller_object::register_select_dead_callback(std::string_view identifier, call_back_select_dead call_back)
{
	std::string id = std::string(identifier);
	if (on_select_dead.find(id) != on_select_dead.end())
		return false;

	on_select_dead.insert_or_assign(id, call_back);
	return true;
}

bool player_controller_object::unregister_select_dead_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_select_dead.find(id);
	if (it != on_select_dead.end())
		return false;

	on_select_dead.erase(it);
	return true;
}

bool player_controller_object::register_select_regenerate_callback(std::string_view identifier, call_back_select_regenerate call_back)
{
	std::string id = std::string(identifier);
	if (on_select_regenerate.find(id) != on_select_regenerate.end())
		return false;

	on_select_regenerate.insert_or_assign(id, call_back);
	return true;
}

bool player_controller_object::unregister_select_regenerate_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_select_regenerate.find(id);
	if (it != on_select_regenerate.end())
		return false;

	on_select_regenerate.erase(it);
	return true;
}

DirectX::XMFLOAT3 player_controller_object::get_move_vector()
{
	auto keyboard_state = keyboard::instance()->get_state();
	int y_axis  = (keyboard_state.is_key_state(keyboard::key_states::w) == input::state::hold)
				- (keyboard_state.is_key_state(keyboard::key_states::s) == input::state::hold);
	int x_axis  = (keyboard_state.is_key_state(keyboard::key_states::d) == input::state::hold)
				- (keyboard_state.is_key_state(keyboard::key_states::a) == input::state::hold);
	if (y_axis == 0 && x_axis == 0)
		return DirectX::XMFLOAT3(0, 0, 0);

	float camera_right_x = camera->get_right().x;
	float camera_right_z = camera->get_right().z;
	float camera_right_length = sqrtf(camera_right_x * camera_right_x + camera_right_z * camera_right_z);
	if (camera_right_length > 0.0f)
	{
		camera_right_x /= camera_right_length;
		camera_right_z /= camera_right_length;
	}

	float camera_front_x = camera->get_front().x;
	float camera_front_z = camera->get_front().z;
	float camera_front_length = sqrtf(camera_front_x * camera_front_x + camera_front_z * camera_front_z);
	if (camera_front_length > 0.0f)
	{
		camera_front_x	/= camera_front_length;
		camera_front_z	/= camera_front_length;
	}

	DirectX::XMFLOAT3 vector;
	vector.x = (camera_right_x * x_axis) + (camera_front_x * y_axis);
	vector.y = 0.0f;
	vector.z = (camera_right_z * x_axis) + (camera_front_z * y_axis);
	float vector_length = sqrtf(vector.x * vector.x + vector.z * vector.z);
	vector.x /= vector_length;
	vector.z /= vector_length;
	return vector;
}

std::weak_ptr<character_object> player_controller_object::search_lockon_taget_object()
{
	//	一番近い敵に焦点を当てる
	auto enemy_objects = object_manager::instance()->search_objects<enemy_controller_object>();
	float nearlest_distance = FLT_MAX;

	std::weak_ptr<character_object> nearlest_object;
	for (auto& enemy_object : enemy_objects)
	{
		//	自分は除外
		if (controll_object == enemy_object->get_controll_object())
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

void player_controller_object::on_search_target(character_object* character)
{
	//	ロックオンターゲットの設定
	controll_object->set_lockon_taget_object(search_lockon_taget_object());
}

void player_controller_object::on_character_dead(character_object* character)
{
	mode = modes::dead_select;
	work_time = 0.0f;

	//	コールバック呼び出し
	for (auto& func : on_dead_start)
	{
		func.second();
	}

	for (auto& action_data : action_datas)
	{
		action_data = {};
	}
}

void player_controller_object::on_character_dead_complete(character_object* character)
{
	if (controll_object && controll_object.get() == character && dead_state == dead_states::none)
	{
		dead_state = dead_states::call;
	}
}

void player_controller_object::on_character_regenerate(character_object* character)
{
}

void player_controller_object::on_stagger_enter(const character_object::stagger_datas& data)
{
	mode = modes::stagger;
	work_time = game_constants::stagger_wait_time;

	//	コールバック呼び出し
	for (auto& func : on_stagger_enters)
	{
		func.second(data);
	}

	for (auto& action_data : action_datas)
	{
		action_data = {};
	}
}

void player_controller_object::on_finalize()
{
	dead_process();
}

void player_controller_object::dead_process()
{
	if (dead_state != dead_states::call)
		return;

	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_attack], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_special], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_deluxe], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_dodge_attack], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_guard], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_parry], SearchLockOnTarget);
	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_break_attack], SearchLockOnTarget);

	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_dead], ControllObjectDead);

	controll_object->unregister_change_state_callback(character_object::state_names[character_object::state_dead_complete], ControllObjectDeadComplete);

	controll_object->unregister_stagger_hit_callback(OnStaggerEnterId);

//	object_manager::instance()->unregister_object(controll_object);
	object_manager::instance()->unregister_object(shared_from_this());
	controll_object.reset();
	dead_state = dead_states::end;
}
