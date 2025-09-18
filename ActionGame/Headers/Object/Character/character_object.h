#pragma once

#include "object/collision_object.h"

#include "object/model_object.h"
#include "object/camera_object.h"

#include "graphics/effect_resource.h"
#include "graphics/effect_instance.h"
#include "graphics/effect_manager.h"

#include "object/game_constant.h"

//	キャラクター制御用基底オブジェクト
class character_object : public game_object
{
	GAME_OBJECT_DECRLEAR(character_object, game_object)

public:
	// 共通ステートId
	enum state_ids
	{
		state_entry,

		state_idle,
		state_walk,
		state_dash,
		state_hi_dash,
		state_move_stop,

		state_dodge,
		state_dodge_stop,
		state_dodge_attack,

		state_damage,
		state_dead,
		state_dead_complete,

		state_regenerate,

		state_guard,
		state_parry,
		state_parry_attack,

		state_stagger,
		state_stagger_complete,

		state_break_attack,

		state_attack,
		state_special,
		state_deluxe,

		state_common_max,
	};

	static constexpr std::string_view state_names[] =
	{
		"state_entry",

		"state_idle",
		"state_walk",
		"state_dash",
		"state_hi_dash",
		"state_move_stop",

		"state_dodge",
		"state_dodge_stop",
		"state_dodge_attack",

		"state_damage",
		"state_dead",
		"state_dead_complete",

		"state_regenerate",

		"state_guard",
		"state_parry",
		"state_parry_attack",

		"state_stagger",
		"state_stagger_complete",

		"state_break_attack",

		"state_attack",
		"state_special",
		"state_deluxe",
	};

	// エフェクトId
	enum class effect_ids
	{
		shock_wave,
		parry,

		max,
	};

	//	移動段階
	enum class move_mode
	{
		walk,
		dash,
		hi_dash,
	};

	//	各種様々な行動で利用するレベル
	//	今の所使用しているのは攻撃レベルと中断耐性レベル
	enum class action_level
	{
		level_1,
		level_2,
		level_3,
		level_4,
		level_5,

		max,
	};

	//	状態定義フラグ
	enum class flags
	{
		movable,			//	移動可能フラグ
		on_ground,			//	接地フラグ
		invisible,			//	無敵フラグ
		trunk_invisible,	//	体幹無敵フラグ
		guard,				//	防御フラグ

		dead,				//	死亡フラグ
		stagger,			//	よろけフラグ

		camera_distance_translucent,	//	カメラからの距離による透明処理仕様フラグ
	};

	//	判定形状情報
	struct bounding_descs
	{
		collision::bounding_object::types type = collision::bounding_object::types::box;
		bool is_follow_self = true;
		DirectX::XMFLOAT3 local_scale;
		math::rotator local_rotator;
		DirectX::XMFLOAT3 local_offset;
	};
	struct bounding_datas
	{
		bounding_descs desc;

		math::rotator rotator;
		DirectX::XMFLOAT3 position;
	};

	//	体力変動情報
	struct health_varing_descs
	{
		int value = 0; // 変動量
		bool defat_attack = true; // 撃破誘発技
	};

	//	体幹変動情報
	struct trunk_varing_descs
	{
		int value = 0; // 変動量
		bool stagger_attack = false; // よろけ状態誘発技
		float slow_time_delay_time = 1.0f; // スロー開始時間遅延時間
	};

	//	特殊値変動情報
	struct special_varing_descs
	{
		int value = 0; // 変動量
	};

	//	特殊値変動情報
	struct deluxe_varing_descs
	{
		int value = 0; // 変動量
	};

	//	攻撃情報
	static constexpr std::string_view attack_data_identifier = "Attack Data";
	struct attack_descs
	{
		health_varing_descs health_desc;
		trunk_varing_descs trunk_desc;
		special_varing_descs special_desc;
		deluxe_varing_descs deluxe_descs;

		action_level attack_level = action_level::level_1; // 攻撃レベル
		float hit_stop_time = 0; // ヒットストップ時間
		float blow_power = 0; // 吹き飛ばし量(m/s)
		bool update_turn_vector = true; // ダメージ側に向きを向ける

		//	カメラ関係の情報
		float camera_shake_time = 0; // カメラ揺れ時間
		float camera_shake_power = 0; // カメラ揺れ強度
	};
	struct damage_datas
	{
		attack_descs desc;

		DirectX::XMFLOAT3 stagger_direction = {0, 0, 0}; // よろけ方向
	};

	//	カメラ側への姿勢情報変更要求情報
	struct camera_transform_descs
	{
		enum class flags
		{
			set_focus_direction,
			set_focus_distance_factor,
		};

		bool is_flag(flags flag) const { return system_utility::is_bitflag(flag_data, 1 << static_cast<UINT>(flag)); }
		void set_flag(flags flag, bool value) { system_utility::set_bitflag(flag_data, 1 << static_cast<UINT>(flag), value); }
		int flag_data = 0;
		DirectX::XMFLOAT3 focus_direction; // 向きたい方向
		float focus_distance_factor; // 距離係数
		float focus_distance_rate; // 距離調整割合
	};

	//	ヒットボックス情報
	static constexpr std::string_view hit_box_data_identifier = "HitBox Data";
	struct hit_box_datas
	{
		character_object* self_character;	// ヒットした自身の情報
	};

	//	ステート変更時呼び出し
	using call_back_change_state = std::function<void(character_object*)>;
	using call_back_change_state_inner_map = std::unordered_map<std::string, call_back_change_state>;
	using call_back_change_state_map = std::unordered_map<std::string, call_back_change_state_inner_map>;

	//	ダメージ受け取り関数定義
	using call_back_damage = std::function<void(const damage_datas&)>;
	using call_back_damage_map = std::unordered_map<std::string, call_back_damage>;

	//	カメラ側への姿勢情報変更関数定義
	using call_back_camera_transform = std::function<void(const camera_transform_descs&)>;
	using call_back_camera_transform_map = std::unordered_map<std::string, call_back_camera_transform>;

	//	時間操作関数定義
	struct time_controll_datas
	{
		//	全体情報
		float time = 0; // スロー時間
		float scale = 1; // 時間変更スケール

		//	除外オブジェクト
		struct exclude_object_data
		{
			float time = 0; // スロー時間
			std::weak_ptr<game_object> object;
		};
		exclude_object_data exclude_data;
	};
	using call_back_time_controll = std::function<void(const time_controll_datas&)>;
	using call_back_time_controll_map = std::unordered_map<std::string, call_back_time_controll>;

	//	死亡時受け取り関数定義
	struct dead_descs
	{
		character_object* self_character;	// キャラクター
	};
	using call_back_dead = std::function<void(const dead_descs&)>;
	using call_back_dead_map = std::unordered_map<std::string, call_back_dead>;

	//	よろけ時受け取り関数定義
	struct stagger_datas
	{
		trunk_varing_descs desc; // 体幹ダメージ情報
		character_object* character;	// hit関数は自身、受け手側は相手
	};
	using call_back_stagger = std::function<void(const stagger_datas&)>;
	using call_back_stagger_map = std::unordered_map<std::string, call_back_stagger>;

private:
	struct bounding_inner_datas
	{
		bool is_follow_self = false;
		collision_object_ptr object;
	};

public:
	character_object(std::string_view name, std::shared_ptr<model_object> model, std::string_view collision_filter_name);
	~character_object() override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

	std::shared_ptr<model_object> get_model_object() const;
	void set_model_object(std::shared_ptr<model_object> model);

	texture_ptr get_face_texture() const;
	void set_face_texture(texture_ptr face);

	DirectX::XMFLOAT4 get_face_uv() const;
	void set_face_uv(DirectX::XMFLOAT4 face_uv);

	collision_object_ptr get_push_box() const;

	const DirectX::XMFLOAT3& get_scale() const;
	void set_scale(const DirectX::XMFLOAT3& scale);

	const math::rotator& get_rotation() const;
	void set_rotation(const math::rotator& rotator);

	float get_rotation_speed() const;
	void set_rotation_speed(float rotation_speed);

	const DirectX::XMFLOAT3& get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);

	const DirectX::XMFLOAT3& get_footstep_position() const;

	bool is_flag(flags flag) const { return system_utility::is_bitflag(flag_data, 1 << static_cast<UINT>(flag)); }
	void set_flag(flags flag, bool value) { system_utility::set_bitflag(flag_data, 1 << static_cast<UINT>(flag), value); }

	DirectX::XMFLOAT3 get_velocity() const;
	void set_velocity(DirectX::XMFLOAT3 velocity);

	float get_gravity_scale() const;
	void set_gravity_scale(float gravity_scale);

	float get_step_offset() const;
	void set_step_offset(float step_offset);

	int get_health() const;
	void set_health(int health);
	int get_max_health() const;
	void set_max_health(int max_health);

	int get_trunk() const;
	void set_trunk(int trunk);
	int get_max_trunk() const;
	void set_max_trunk(int trunk);

	int get_special_power() const;
	void set_special_power(int power);
	int get_max_special_power() const;
	void set_max_special_power(int power);

	virtual int get_use_special_power() const;

	int get_deluxe_power() const;
	void set_deluxe_power(int power);
	int get_max_deluxe_power() const;
	void set_max_deluxe_power(int power);

	float get_elapsed_time_scale() const;
	void set_elapsed_time_scale(float elapsed_time_scale);

	float get_stagger_recovery_time() const;
	void set_stagger_recovery_time(float stagger_recovery_time);

	float get_stagger_recovery_time_max() const;
	void set_stagger_recovery_time_max(float stagger_recovery_time_max);

	bool get_basic_movement_dash() const;
	void set_basic_movement_dash(bool basic_movement_dash_flag);

	action_level get_damage_cancel_resist_level() const;
	void set_damage_cancel_resist_level(action_level damage_cancel_resist_level);

	std::weak_ptr<character_object> get_lockon_taget_object() const;
	void set_lockon_taget_object(std::weak_ptr<character_object> lockon_taget_object);

	virtual DirectX::XMFLOAT3 get_focus_point() const;

	void add_move_vector(DirectX::XMFLOAT2 direction, float accelaration);
	void set_turn_vector(DirectX::XMFLOAT2 direction);
	DirectX::XMFLOAT2 get_turn_vector() const;
	void add_impulse(DirectX::XMFLOAT3 delta_velocity);
	float get_max_move_speed() const;
	void set_max_move_speed(float max_move_speed);
	float get_breaking_deceleration() const;
	void set_breaking_deceleration(float breaking_deceleration);

	float get_invisile_timer() const;
	void set_invisile_timer(float invisile_timer);

	void add_attack_box(const bounding_datas& bounding_data, const damage_datas& damage_data);
	void clear_attack_box();

	void add_hit_box(const bounding_datas& bounding_data);
	void clear_hit_box();

	bool register_attack_hit_callback(std::string_view identifier, call_back_damage call_back);
	bool unregister_attack_hit_callback(std::string_view identifier);
	bool register_damage_callback(std::string_view identifier, call_back_damage call_back);
	bool unregister_damage_callback(std::string_view identifier);
	bool register_dead_callback(std::string_view identifier, call_back_dead call_back);
	bool unregister_dead_callback(std::string_view identifier);

	bool register_guard_hit_callback(std::string_view identifier, call_back_damage call_back);
	bool unregister_guard_hit_callback(std::string_view identifier);
	bool register_guarded_callback(std::string_view identifier, call_back_damage call_back);
	bool unregister_guarded_callback(std::string_view identifier);

	bool register_camera_transform(std::string_view identifier, call_back_camera_transform call_back);
	bool unregister_camera_transform(std::string_view identifier);

	bool register_time_controll(std::string_view identifier, call_back_time_controll call_back);
	bool unregister_time_controll(std::string_view identifier);

	bool register_stagger_hit_callback(std::string_view identifier, call_back_stagger call_back);
	bool unregister_stagger_hit_callback(std::string_view identifier);
	bool register_stagger_callback(std::string_view identifier, call_back_stagger call_back);
	bool unregister_stagger_callback(std::string_view identifier);

private:
	void generate_box_data(std::vector<bounding_inner_datas>& datas, const bounding_datas& bounding_data);

protected:
	virtual void on_attack_hit(const damage_datas& data);
	virtual void on_damaged(const damage_datas& data);
	virtual void on_guard_hit(const damage_datas& data);
	virtual void on_guarded(const damage_datas& data);
	virtual void on_hit_while_dodging(const damage_datas& data);
	virtual void on_deead();

	virtual void on_stagger_hit(const stagger_datas& data);
	virtual void on_stagger(const stagger_datas& data);

	virtual void on_attack_box_enter(collision_object::data data);

	virtual void on_push_box_enter(collision_object::data data);
	virtual void on_push_box_stay(collision_object::data data);

	virtual void on_root_animation_translation(const DirectX::XMFLOAT3& translation);
	virtual void on_root_animation_rotation(const DirectX::XMFLOAT4& rotation);

	virtual void on_change_camera_translucent(bool flag) {}

	void call_camera_transforms(const camera_transform_descs& desc);

public: ///////////////////////////////////////////////////////////////////////////////////////
	void fixed_update(float elapsed_time) override;
	void post_update(float elapsed_time) override;

	std::string_view get_current_state() const;
	bool check_same_state(std::string_view state_name) const;
	virtual bool change_state(std::string_view state_id);

	bool register_change_state_callback(std::string_view state_id, std::string_view identifier, call_back_change_state callback);
	bool unregister_change_state_callback(std::string_view state_id, std::string_view identifier);

	struct action_datas
	{
		DirectX::XMFLOAT3 move = { 0, 0, 0 }; // 移動方向
		bool dash = false; // ダッシュ
		input::state attack = input::state::none; // 攻撃
		bool special = false; // 特殊技
		bool deluxe = false; // 必殺技
		bool dodge = false; // 回避
		bool guard = false; // ガード

		bool use_turn_vector = false; // 向きベクトルを別で指定する
		DirectX::XMFLOAT2 turn_vector = { 0, 0 }; // 向き方向

		bool is_attacked() const
		{
			return is_attacked(attack);
		}

		int get_attack_index() const
		{
			return attack == input::state::hold ? 1 : 0;
		}

		static inline bool is_attacked(input::state state)
		{
			return state == input::state::hold || state == input::state::down;
		}
	};
	virtual void input_action_data(action_datas action_data) = 0;

	struct play_effect_descs
	{
		size_t id;

		DirectX::XMFLOAT3 scale = {1, 1, 1};
		math::rotator rotation = {0, 0, 0};
		DirectX::XMFLOAT3 position = { 0, 0, 0 };
	};
	virtual effect_instance_ptr play_effect(const play_effect_descs& desc);

protected:
	template<typename Type, typename ... Arguments>
	bool register_state(const Arguments ... arguments)
	{
		return state_machine.register_state<Type, Arguments...>(arguments...);
	}

private:
	void register_common_states();
	void register_common_effects();

protected:
	call_back_time_controll_map& get_cb_time_controlls() { return on_time_controlls; }

	virtual bool register_effect(size_t effect_id, std::string_view file_path);
	virtual void adjustment_velocity(DirectX::XMFLOAT2 move, float adjust_move_velocity_rate, bool* flip_flag);
	virtual bool calculate_position(DirectX::XMFLOAT3 velocity, bool vertical, DirectX::XMFLOAT3& out_position, DirectX::XMFLOAT3& out_normal, float* out_friction = nullptr);
#define DECLEAR_STATE(STATE) \
	virtual void enter_##STATE(){}; \
	virtual void update_##STATE(float elapsed_time) = 0; \
	virtual void exit_##STATE(){};

	DECLEAR_STATE(entry)

	DECLEAR_STATE(idle)
	DECLEAR_STATE(walk)
	DECLEAR_STATE(dash)
	DECLEAR_STATE(hi_dash)
	DECLEAR_STATE(move_stop)
	DECLEAR_STATE(dodge)
	DECLEAR_STATE(dodge_stop)
	DECLEAR_STATE(dodge_attack)

	DECLEAR_STATE(damage)
	DECLEAR_STATE(dead)
	DECLEAR_STATE(dead_complete)
	DECLEAR_STATE(regenerate)

	DECLEAR_STATE(stagger)
	DECLEAR_STATE(stagger_complete)

	DECLEAR_STATE(break_attack);

	DECLEAR_STATE(attack)
	DECLEAR_STATE(special)
	DECLEAR_STATE(deluxe)

	DECLEAR_STATE(guard)
	DECLEAR_STATE(parry)
	DECLEAR_STATE(parry_attack)
#undef DECLEAR_STATE

private:
	void update_rotation(float elapsed_time);
	void update_vertical_velocity(float elapsed_time);
	void update_vertical_move(float elapsed_time);
	void update_horizontal_velocity(float elapsed_time);
	void update_horizontal_move(float elapsed_time);

private:
	std::shared_ptr<model_object> model;
	texture_ptr face_texture;
	DirectX::XMFLOAT4 face_uv;
	int flag_data; // ビットフラグ情報
	int camera_distance_translucent_inner_flag; // カメラからの距離による半透明処理内部フラグ
	float elapsed_time_scale; // 経過時間スケール
	std::string collision_filter_name; // コリジョンフィルター名

	finite_state_machine state_machine;

	int health; // 体力
	int max_health; // 最大体力
	int trunk; // 体幹
	int max_trunk; // 最大体幹(これいるかな？)
	int special_power; // 特殊値
	int max_special_power; // 特殊値
	int deluxe_power; // 覚醒値
	int max_deluxe_power; // 覚醒値
	float hit_stop_time; // ヒットストップ時間
	float just_dodge_judgement_time; // ジャスト回避判定時間
	float stagger_recovery_time; // よろけ回復時間
	float stagger_recovery_time_max; // よろけ回復最大時間
	float invisile_timer; // 直前回避などによる無敵時間タイマー
	bool basic_movement_dash_flag; // 基本的な動きをダッシュに変更
	action_level damage_cancel_resist_level; // 中断耐性レベル（いわゆるスーパーアーマー）

	collision_object_ptr push_box; // 押し合い判定
	std::vector<bounding_inner_datas> attack_box; // 攻撃判定
	std::vector<bounding_inner_datas> hit_box; // やられ判定
	call_back_damage_map on_attack_hits; // 攻撃ヒット呼び出し関数群
	call_back_stagger_map on_stagger_hits; // スタッガーヒット呼び出し関数群
	call_back_damage_map on_damageds; // ダメージ時呼び出し関数群
	std::vector<character_object*> hit_objects; // 当たり
	call_back_damage_map on_guard_hits; // 防御ヒット呼び出し関数群
	call_back_damage_map on_guardeds; // 防御ヒット呼び出し関数群
	call_back_dead_map on_deads; // 死亡時呼び出し関数群
	call_back_stagger_map on_staggers; // よろけ時呼び出し関数群
	call_back_camera_transform_map on_camera_transforms; // カメラ変更命令
	call_back_time_controll_map on_time_controlls; // 時間操作時呼び出し関数群
	call_back_change_state_map on_change_states; // ステート変更時呼び出し関数群


	DirectX::XMFLOAT2 move_vector; // 移動方向
	DirectX::XMFLOAT2 turn_front_vector; // 回転方向
	DirectX::XMFLOAT3 velocity; // 速度(m/s)
	float accelaration; // 加速量(m/s2)
	float breaking_deceleration; // 減速量(m/s2)
	float max_move_speed; // 最大速度(m/s)
	float rotation_speed; // 回転速度
	float roll_value; // 傾き量

	float gravity_scale; // 重力スケール
	float slope_rate; // 坂道補正係数
	float step_offset; // 段差補正
	float slope_limit; // 傾斜角度が一定以上なら進めないようにする
	float ground_friction; // 地面摩擦係数
	DirectX::XMFLOAT3 ground_normal; // 地面法線
	DirectX::XMFLOAT3 footstep_position; // 足元座標

	DirectX::XMFLOAT3 frame_gravity; // 1Fの重力(deltaの方が良いか？)
	std::weak_ptr<character_object> lockon_taget_object; // ロックオンターゲット

	std::unordered_map<size_t, std::shared_ptr<effect_resource>> effect_resources; // エフェクトリソース
};
