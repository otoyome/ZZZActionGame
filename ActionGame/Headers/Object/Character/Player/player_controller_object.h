#pragma once

#include "object/character/character_object.h"

//	プレイヤーコントローラーオブジェクト
class player_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(player_controller_object, game_object)

private:
	enum class modes
	{
		default,
		stagger,
		dead_select,
	};
	enum class dead_states
	{
		none,
		call,
		end,
	};

	using call_back_stagger_enter = character_object::call_back_stagger;
	using call_back_stagger_enter_map = std::unordered_map<std::string, call_back_stagger_enter>;
	using call_back_stagger_exit = std::function<void()>;
	using call_back_stagger_exit_map = std::unordered_map<std::string, call_back_stagger_exit>;

	using call_back_dead_start = std::function<void()>;
	using call_back_dead_start_map = std::unordered_map<std::string, call_back_dead_start>;

	using call_back_select_dead = std::function<void()>;
	using call_back_select_dead_map = std::unordered_map<std::string, call_back_select_dead>;

	using call_back_select_regenerate = std::function<void()>;
	using call_back_select_regenerate_map = std::unordered_map<std::string, call_back_select_regenerate>;

public:
	std::shared_ptr<character_object> get_controll_object() const;
	DirectX::XMFLOAT3 get_focus_point() const;

	bool get_input_purmit_flag() const;
	void set_input_purmit_flag(bool input_stop);

public:
	player_controller_object(std::string_view name, std::shared_ptr<character_object> controll_object, std::shared_ptr<camera_object> camera);
	~player_controller_object() override;

	void fixed_update(float elapsed_time) override;

	bool register_stagger_enter_callback(std::string_view identifier, call_back_stagger_enter call_back);
	bool unregister_stagger_enter_callback(std::string_view identifier);

	bool register_stagger_exit_callback(std::string_view identifier, call_back_stagger_exit call_back);
	bool unregister_stagger_exit_callback(std::string_view identifier);

	bool register_dead_start_callback(std::string_view identifier, call_back_dead_start call_back);
	bool unregister_dead_start_callback(std::string_view identifier);

	bool register_select_dead_callback(std::string_view identifier, call_back_select_dead call_back);
	bool unregister_select_dead_callback(std::string_view identifier);

	bool register_select_regenerate_callback(std::string_view identifier, call_back_select_regenerate call_back);
	bool unregister_select_regenerate_callback(std::string_view identifier);

private:
	DirectX::XMFLOAT3 get_move_vector();
	std::weak_ptr<character_object> search_lockon_taget_object();

	void on_search_target(character_object* character);
	void on_character_dead(character_object* character);
	void on_character_dead_complete(character_object* character);
	void on_character_regenerate(character_object* character);
	void on_stagger_enter(const character_object::stagger_datas& data);

	void on_finalize() override;
	void dead_process();

private:
	float work_time;
	bool input_purmit_flag;
	std::shared_ptr<camera_object> camera;
	std::shared_ptr<character_object> controll_object;

	call_back_stagger_enter_map on_stagger_enters;
	call_back_stagger_exit_map on_stagger_exits;

	call_back_dead_start_map on_dead_start;
	call_back_select_dead_map on_select_dead;
	call_back_select_regenerate_map on_select_regenerate;

	modes mode;
	character_object::action_datas action_datas[system_units::frame_per_sec];

	dead_states dead_state;
};
