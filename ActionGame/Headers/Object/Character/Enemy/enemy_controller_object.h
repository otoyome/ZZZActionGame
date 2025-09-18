#pragma once

#include "object/character/character_object.h"

//	敵コントローラーオブジェクト
class enemy_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(enemy_controller_object, game_object)

public:
	enum class enemy_type
	{
		enemy,
		boss,
	};

	using enemy_type_data = std::pair<enemy_type, int>;

private:
	enum class dead_states
	{
		none,
		call,
		end,
	};

public:
	virtual enemy_type_data get_enemy_type() const = 0;

	std::shared_ptr<character_object> get_controll_object() const;
	DirectX::XMFLOAT3 get_focus_point() const;

public:
	enemy_controller_object(std::string_view name, std::shared_ptr<character_object> controll_object);
	~enemy_controller_object() override;

	void on_finalize() override;

	void fixed_update(float elapsed_time) override;
	void update(float elapsed_time) override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

public:
	std::string_view get_current_state() const;
	void change_state(std::string_view state_id);
	template<typename Type, typename ... Arguments>
	bool register_state(const Arguments ... arguments)
	{
		return state_machine.register_state<Type, Arguments...>(arguments...);
	}

public:
	std::weak_ptr<character_object> search_lockon_taget_object(float search_range = FLT_MAX);

protected:
	virtual void on_character_dead(character_object* character);

private:
	void on_search_target(character_object* character);

	void dead_process();

private:
	std::shared_ptr<character_object> controll_object;
	finite_state_machine state_machine;

	dead_states dead_state;

#if defined(USE_IMGUI)
	bool debug_control_mode;
#endif	//	defined(USE_IMGUI)
};
