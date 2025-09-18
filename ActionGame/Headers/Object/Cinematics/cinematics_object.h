#pragma once

#include "object/character/character_object.h"
#include "object/character/player/player_controller_object.h"
#include "object/camera_object.h"

//	演出管理クラス
class cinematics_object : public game_object
{
	GAME_OBJECT_DECRLEAR(cinematics_object, game_object);

	struct time_control_datas
	{
		float time = 0; // スロー時間
		float scale = 1; // 時間変更スケール
	};

	struct parry_datas
	{
		float time = 0.0f;
		float color_separate_time = 0.0f;
		bool high_power = true;
		DirectX::XMFLOAT2 center{ 0.5f, 0.5f };
	};

	struct break_datas
	{
		float time = 0.0f;
		float scale = 1.0f;
		float slow_time_delay_time = 1.0f;
	};

	struct exclude_object_datas
	{
		std::weak_ptr<game_object> object;
		float time = 0.0f;
		float time_scale;
	};

	enum class states
	{
		scene,
		external,
	};

public:
	cinematics_object(std::string_view name, std::shared_ptr<camera_object> camera);
	~cinematics_object() override;

	void fixed_update(float elapsed_time) override;
	void pre_render() override;

#if defined(USE_IMGUI)
	void imgui_render_core();
#endif	//	defined(USE_IMGUI)

public:
	void register_effects_function(std::shared_ptr<character_object> character);
	void register_effects_function(std::shared_ptr<player_controller_object> controller);

	//	入場とかクリア演出用にもう外側から設定する事とする
	void external_setting_mode(bool flag);
	bool is_external_setting_mode() const;
	const post_process_datas& get_external_post_process_data() const;
	void set_external_post_process_data(const post_process_datas& data);
	void set_external_time_scale(float scale);

private:
	void update_scene(float elapsed_time);
	void update_external(float elapsed_time);

private:
	void on_slow(const character_object::time_controll_datas& data);
	void on_parry(const character_object::damage_datas& data);
	void on_break_enter(const character_object::stagger_datas& data);
	void on_break_exit();
	void on_active() override;
	void on_deactive() override;

private:
	states state;
	int sub_state;
	post_process_volume_ptr post_process_volume; // ポストプロセスボリューム
	time_control_datas time_control_data; // 時間系演出情報
	std::vector<exclude_object_datas> exclude_objects;
	std::shared_ptr<camera_object> camera; // カメラオブジェクト
	parry_datas parry_data; // パリィ演出情報
	break_datas break_data; // ブレイク演出情報
	float previous_time_scale;

	post_process_datas time_slow_post_process_origin_data;
	post_process_datas parry_post_process_origin_data;
	post_process_datas break_post_process_origin_data;

	post_process_datas external_post_process_data;
	float external_time_scale;

	texture_ptr breaked_texture;
};


