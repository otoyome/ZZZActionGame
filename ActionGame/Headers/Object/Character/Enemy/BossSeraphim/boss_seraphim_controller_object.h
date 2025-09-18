#pragma once

#include "object/character/enemy/enemy_controller_object.h"
#include "object/character/person/boss_seraphim_object.h"

//	敵コントローラーオブジェクト
class boss_seraphim_controller_object : public enemy_controller_object
{
	GAME_OBJECT_DECRLEAR(boss_seraphim_controller_object, enemy_controller_object)

	static constexpr std::string_view TRANSITION_PARRY_ATTACK = "TRANSITION_PARRY_ATTACK";
	static constexpr std::string_view PARRY_ATTACK_START = "PARRY_ATTACK_START";

public:
	enemy_type_data get_enemy_type() const override { return std::make_pair(enemy_type::boss, UINT_MAX); }
	inline std::shared_ptr<boss_seraphim_object> get_boss_seraphim_object() { return std::static_pointer_cast<boss_seraphim_object>(get_controll_object()); }

public:
	boss_seraphim_controller_object(std::string_view name, std::shared_ptr<character_object> controll_object);
	~boss_seraphim_controller_object() override;

	void on_initialize() override;
	void on_character_dead(character_object* character) override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class hfsn_boss_seraphim_base : public hierarchy_finite_state_node
{
public:
	static constexpr std::string_view type_name() {return NAMEOF_TYPE(hfsn_boss_seraphim_base);}
	virtual std::string_view get_type_name() const {return type_name();}
	virtual bool check_type(std::string_view text) const {return type_name() == text;}

public:
	inline std::shared_ptr<boss_seraphim_controller_object> get_controller() {return controller.lock();}
	inline std::shared_ptr<character_object> get_row_controll_object() {return get_controller()->get_controll_object();}
	inline std::shared_ptr<boss_seraphim_object> get_controll_object() { return std::static_pointer_cast<boss_seraphim_object>(get_row_controll_object()); }

protected:
	hfsn_boss_seraphim_base(std::string_view name, std::weak_ptr<boss_seraphim_controller_object> controller)
	: hierarchy_finite_state_node(name)
	, controller(controller)
	{
	}

private:
	std::weak_ptr<boss_seraphim_controller_object> controller;

public:
	bool entry_flag = false;
	float wait_timer = 0.0f;
	bool is_second_mode = false;
	int parry_level = 0;
	float parry_wait_timer = 0;
	bool work_flag[3] = {false, false, false};
	float work_float[3] = {0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 work_float3[3] = {};
};

#define	HFSN_DECRLEAR(ABSTRACT_TYPE, BASE_TYPE)	\
public:	\
using base = BASE_TYPE;	\
static constexpr std::string_view type_name() {return NAMEOF_TYPE(ABSTRACT_TYPE);}	\
std::string_view get_type_name() const override {return type_name();}	\
bool check_type(std::string_view text) const override {return type_name() == text ? true : BASE_TYPE::check_type(text);}	\
private:
