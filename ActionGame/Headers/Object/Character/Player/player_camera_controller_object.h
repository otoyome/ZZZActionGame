#pragma once

#include "camera.h"
#include "object/game_object.h"
#include "object/character/character_object.h"
#include "object/character/player/player_controller_object.h"

#include "animation_resource.h"

//	カメラ制御用オブジェクト
class player_camera_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(player_camera_controller_object, game_object)

private:
	enum class dead_states
	{
		none,
		call,
		end,
	};

public:
	// ステート
	enum class states
	{
		entry,

		free,
		parry,
		stagger,

		clear_performance,
	};

public:
	std::shared_ptr<camera_object> get_camera_object() const {return camera;}

	const math::rotator& get_rotation() const;
	void set_rotation(const math::rotator& rotator);

	const DirectX::XMFLOAT3& get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);

	void set_input_purmit_flag(bool flag);

	//	カメラ側のアニメーションが進行可能判定フラグ
	bool is_camera_transition_wait();

public:
	player_camera_controller_object(std::string_view name, std::shared_ptr<player_controller_object> player_controller, std::shared_ptr<camera_object> camera);
	~player_camera_controller_object() override;

	void fixed_update(float elapsed_time) override;
	void update(float elapsed_time) override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

	void change_state(states state);

	void register_character_object(std::shared_ptr<character_object> character);

private:
	void fixed_update_entry(float elapsed_time);
	void fixed_update_clear_performance(float elapsed_time);
	void fixed_update_default(float elapsed_time);

	void on_shake(const character_object::damage_datas& damage_data);
	void on_camera_transform(const character_object::camera_transform_descs& desc);

	void on_character_dead(character_object* character);
	void on_parry_camera(character_object* character);
	void on_stagger_camera_enter(const character_object::stagger_datas& data);
	void on_stagger_camera_exit();

	void dead_process();

	void register_animations();

	bool play_animation(std::string_view animation_name, bool loop);
	void update_animation(float time);
	bool is_playing_animation();
	float calculate_animation_rate();

private:
	std::shared_ptr<player_controller_object> player_controller;
	std::shared_ptr<camera_object> camera;

	dead_states dead_state; // 死亡ステート
	states state; // モード
	int sub_state; // サブステート
	float work_float; // ワーク変数
	bool input_purmit_flag; // 入力許可フラグ
	bool clear_performance_transition_flag;

	float minimum_distance; // 最低距離
	float current_distance; // 現在距離
	float maximum_distance; // 最大距離
	float physique_distance_scale; // 体格差による距離補正係数
	float attack_hit_distance_scale; // 攻撃ヒットによる距離補正係数
	float attack_hit_distance_scale_time; // ヒットによる距離補正適応時間
	float game_distance_scale; // ゲーム側からの距離補正係数
	float game_distance_scale_inner;
	float game_distance_scale_rate;

	math::rotator disengage_direction; // カメラを離す方向(目標方向)
	float update_disengage_direction_inner_rate; // 方向適応割合
	float update_disengage_direction_rate; // 方向適応割合

	DirectX::XMFLOAT3 focus; // 注視点座標
	float focus_distance; // 注視点距離
	float focus_limit_range; // 注視点移動制限範囲
	float update_focus_rate_inner; // 注視点更新係数

	std::vector<std::shared_ptr<animation_resource>> animations;
	int character_animation_index = 0;
	size_t current_animation_index;
	bool animation_loop = false;
	float animation_time;
};
