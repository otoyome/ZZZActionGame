#pragma once

#include "object/character/character_object.h"

//	クロコ用キャラクター制御オブジェクト
class kuroko_object : public character_object
{
	GAME_OBJECT_DECRLEAR(kuroko_object, character_object)

public:
	enum kuroko_state_ids
	{
		state_attack6_1,
		state_attack6_2,
		state_attack6_3,
		state_hi_dash_flip,

		state_special_2,
		state_special_3,
		state_special_4,
		state_special_5,

		state_break_attack_2,
		state_break_attack_3,

		state_dead_start,
	};

	static constexpr std::string_view kuroko_state_names[] =
	{
		"state_attack6_1",
		"state_attack6_2",
		"state_attack6_3",
		"state_hi_dash_flip",

		"state_special_2",
		"state_special_3",
		"state_special_4",
		"state_special_5",

		"state_break_attack_2",
		"state_break_attack_3",

		"state_dead_start",
	};

	enum class attack_id
	{
		none,
		attack_1,
		attack_2,
		attack_3,
		attack_4,
		attack_5,
		attack_6_1,
		attack_6_3,
		attack_dodge,
		attack_parry,

		special_2,
		special_3,
		special_4,
		special_5,

		break_attack,

		invalid,
	};

	enum class custom_effect_id
	{
		start = static_cast<size_t>(effect_ids::max) - 1,
		attack6_3,
		grand_viper_frame,
		ground_break_effect,
		front_wind_line,

		break_attack_impact,
	};

public:
	static std::shared_ptr<model_object> generate_character_model_object();
	static std::shared_ptr<model_object> generate_weapon_model_object();

public:
	kuroko_object(std::shared_ptr<model_object> character_model, std::shared_ptr<model_object> weapon_model, std::string_view collision_filter_name);
	~kuroko_object() override;

	void fixed_update(float elapsed_time) override;
	void post_update(float elapsed_time) override;

#if defined(USE_IMGUI)
	void imgui_render_core();
#endif // defined(USE_IMGUI)

public:
	void input_action_data(action_datas action_data) override;

	int get_use_special_power() const override;
	DirectX::XMFLOAT3 get_focus_point() const override;

	attack_id get_current_attack_id() const;

protected:
	void on_damaged(const damage_datas& data) override;
	void on_guarded(const damage_datas& data) override;
	void on_attack_box_enter(collision_object::data data) override;
	void on_hit_while_dodging(const damage_datas& data) override;
	void on_deead() override;
	void on_stagger(const stagger_datas& data) override;

	void on_change_camera_translucent(bool flag) override;

private:
	bool input_action(float elapsed_time);
	void clear_datas();

	void enter_entry() override;
	void update_entry(float elapsed_time) override;

	void enter_idle() override;
	void update_idle(float elapsed_time) override;

	void enter_walk() override;
	void update_walk(float elapsed_time) override;

	void enter_dash() override;
	void update_dash(float elapsed_time) override;

	void transition_hi_dash(float blend_animation_time = 0.2f);
	void update_hi_dash(float elapsed_time) override;

	void enter_hi_dash_flip();
	void update_hi_dash_flip(float elapsed_time);

	void enter_move_stop() override;
	void update_move_stop(float elapsed_time) override;

	void enter_dodge() override;
	void update_dodge(float elapsed_time) override;

	void transition_dodge_stop();
	void update_dodge_stop(float elapsed_time) override;

	void transition_damage(const damage_datas& data);
	void enter_damage() override;
	void update_damage(float elapsed_time) override;


	void transition_dead();
	void enter_dead_start();
	void update_dead_start(float elapsed_time);
	void enter_dead();
	void update_dead(float elapsed_time) override;

	void enter_dead_complete() override;
	void update_dead_complete(float elapsed_time) override;

	void enter_regenerate() override;
	void update_regenerate(float elapsed_time) override;

	void transition_stagger();
	void update_stagger(float elapsed_time) override;
	void transition_stagger_complete();
	void update_stagger_complete(float elapsed_time) override;

	void enter_break_attack() override;
	void update_break_attack(float elapsed_time) override;
	void enter_break_attack_2();
	void update_break_attack_2(float elapsed_time);
	void enter_break_attack_3();
	void update_break_attack_3(float elapsed_time);

	bool transition_attack(attack_id id);
	void update_attack(float elapsed_time) override;
	void enter_attack6_1();
	void update_attack6_1(float elapsed_time);
	void enter_attack6_2();
	void update_attack6_2(float elapsed_time);
	void enter_attack6_3();
	void update_attack6_3(float elapsed_time);
	void enter_dodge_attack() override;
	void update_dodge_attack(float elapsed_time) override;

	void enter_special() override;
	void update_special(float elapsed_time) override;

	void enter_special_2();
	void update_special_2(float elapsed_time);
	void enter_special_3();
	void update_special_3(float elapsed_time);
	void enter_special_4();
	void update_special_4(float elapsed_time);
	void enter_special_5();
	void update_special_5(float elapsed_time);

	void update_deluxe(float elapsed_time) override;

	void enter_guard() override;
	void update_guard(float elapsed_time) override;
	bool transition_parry(const damage_datas& data);
	void update_parry(float elapsed_time) override;
	void enter_parry_attack() override;
	void update_parry_attack(float elapsed_time) override;

	bool input_move(float elapsed_time, move_mode mode, bool* flip_flag = nullptr);

	void update_attack_box(float animation_time, DirectX::XMFLOAT3 stagger_direction);

	bool fast_apprpoaching_lockon_target(float elapsed_time, const DirectX::XMFLOAT2& range, float power = 3.5f);
	bool calculate_lockon_target_vector(DirectX::XMFLOAT2& direction, float* distance = nullptr);

private:
	void register_states();
	void register_effects();

private:
	size_t sub_state = 0;
	std::shared_ptr<model_object> weapon_model; // 武器モデル
	action_datas action_data; // 入力情報
	float weapon_alpha = 1;

	//	ワーキングメモリ
	DirectX::XMFLOAT3 work_float3[2];
	float work_float[4];

	//	移動情報
	float walk_accelaration;
	float dash_accelaration;
	float hi_dash_accelaration;
	float current_accelaration;

	float walk_breaking_deceleration;
	float dash_breaking_deceleration;
	float hi_dash_breaking_deceleration;

	float walk_max_speed;
	float dash_max_speed;
	float hi_dash_max_speed;

	DirectX::XMFLOAT2 move_stop_vector;
	float move_stop_max_speed;
	float roll_value;

	//	攻撃情報
	attack_id current_attack_id;
	size_t sub_attack_id;
	bool attack_data_flags;
	bool attack_hit_flag;
	bool apprpoaching_effect_flag;

	//	回避情報
	float transition_dodge_wait_time;

	//	特殊エフェクト関係情報(@remainder これは後々、エフェクト再生機構に纏める事)
	texture_ptr attack_6_decal;
};
