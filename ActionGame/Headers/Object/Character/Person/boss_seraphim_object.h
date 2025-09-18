#pragma once

#include "object/character/character_object.h"

//	ボス用セラフィム用制御オブジェクト
class boss_seraphim_object : public character_object
{
	GAME_OBJECT_DECRLEAR(boss_seraphim_object, character_object)

	//	パリィ攻撃開始/終了関数
	using call_back_parry_attck = std::function<void()>;
	using call_back_parry_attck_start_map = std::unordered_map<std::string, call_back_parry_attck>;

public:
	enum boss_seraphim_states
	{
		state_entry,
	};

	static constexpr std::string_view boss_seraphim_state_names[] =
	{
		"state_entry",
	};

	enum class attack_id
	{
		none,
		attack_1,
		attack_2,
		attack_3,

		attack_parry_1,
		attack_parry_2,
		invalid,
	};

	enum class custom_effect_id
	{
		start = static_cast<size_t>(effect_ids::max) - 1,
		omen,
		monster_roar,

		special_attack_convergence,
	};

public:
	static std::shared_ptr<model_object> generate_character_model_object();

public:
	boss_seraphim_object(std::shared_ptr<model_object> character_model, std::string_view collision_filter_name);
	~boss_seraphim_object() override;

	void fixed_update(float elapsed_time) override;
	void post_update(float elapsed_time) override;

#if defined(USE_IMGUI)
	void imgui_render_core();
#endif // defined(USE_IMGUI)

public:
	void input_action_data(action_datas action_data) override;

	DirectX::XMFLOAT3 get_focus_point() const override;

	attack_id get_current_attack_id() const;

	bool register_parry_attck_start(std::string_view identifier, call_back_parry_attck call_back);
	bool unregister_parry_attck_start(std::string_view identifier);

protected:
	void on_damaged(const damage_datas& data) override;
	void on_guarded(const damage_datas& data) override;
	void on_attack_box_enter(collision_object::data data) override;
	void on_deead() override;
	void on_stagger(const stagger_datas& data) override;

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

	void enter_hi_dash() override;
	void update_hi_dash(float elapsed_time) override;

	void enter_move_stop() override;
	void update_move_stop(float elapsed_time) override;

void enter_dodge() override {}
void update_dodge(float elapsed_time) override {}

void enter_dodge_stop() override {}
void update_dodge_stop(float elapsed_time) override {}

	void transition_damage(const damage_datas& data);
	void enter_damage() override;
	void update_damage(float elapsed_time) override;

	void enter_dead() override;
	void update_dead(float elapsed_time) override;

	void enter_dead_complete() override;
	void update_dead_complete(float elapsed_time) override;

void enter_regenerate() override {}
void update_regenerate(float elapsed_time) override {}

	void enter_stagger() override;
	void update_stagger(float elapsed_time) override;
	void enter_stagger_complete() override;
	void update_stagger_complete(float elapsed_time) override;

void enter_break_attack(){}
void update_break_attack(float elapsed_time){}

	bool transition_attack(attack_id id);
	void update_attack(float elapsed_time) override;

void enter_dodge_attack() override {}
void update_dodge_attack(float elapsed_time) override {}

void update_special(float elapsed_time) override {}
void update_deluxe(float elapsed_time) override {}

	void enter_guard() override;
	void update_guard(float elapsed_time) override;

	bool transition_parry(const damage_datas& data);
	void update_parry(float elapsed_time) override;

	bool transition_parry_attack(attack_id id);
	void enter_parry_attack() override;
	void update_parry_attack(float elapsed_time) override;
	void update_parry_attack_1(float elapsed_time);
	void update_parry_attack_2(float elapsed_time);

	bool input_move(float elapsed_time, move_mode mode, bool* flip_flag = nullptr);

	bool fast_apprpoaching_lockon_target(float elapsed_time, const DirectX::XMFLOAT2& range, float power = 3.5f);
	bool calculate_lockon_target_vector(DirectX::XMFLOAT2& direction, float* distance = nullptr);

private:
	void register_states();
	void register_effects();

private:
	size_t sub_state = 0;
	action_datas action_data; // 入力情報

	call_back_parry_attck_start_map on_parry_attck_start; // 攻撃を開始する時に呼び出される関数

	//	ワーキングメモリ
	DirectX::XMFLOAT3 work_float3[3];
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

	//	攻撃情報
	attack_id current_attack_id;
	size_t sub_attack_id;
	bool attack_data_flags;
	bool play_omen_flags;
	bool attack_hit_flag;
	bool apprpoaching_effect_flag;

	//	回避情報
	float transition_dodge_wait_time;
};
