#pragma once

#include "object/ui/ui_object.h"
#include "object/ui/ui_image.h"

#include "object/ui/common/fade_controller_object.h"

#include "object/ui/hud/hp_bar_controller_object.h"
#include "object/ui/hud/sp_bar_controller_object.h"
#include "object/ui/hud/trunk_bar_controller_object.h"
#include "object/ui/hud/face_image_controller_object.h"
#include "object/ui/hud/key_icon_controller_object.h"
#include "object/ui/hud/timer_controller_object.h"

#include "object/character/enemy/enemy_controller_object.h"
#include "object/character/player/player_controller_object.h"

//	HUD制御用オブジェクト
class battle_hud_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(battle_hud_controller_object, game_object)

public:
	struct shake_descs
	{
		float time = 0.0f;
		float power = 0.0f;
	};

	struct object_descs
	{
		std::string name;
		std::shared_ptr<hp_bar_controller_object> player_hp_bar;
		std::shared_ptr<sp_bar_controller_object> player_sp_bar;
		std::shared_ptr<face_image_controller_object> player_face_image;

		std::shared_ptr<key_indicator_object> attack_icon;
		std::shared_ptr<key_indicator_object> special_icon;
		std::shared_ptr<key_indicator_object> dodge_icon;
		std::shared_ptr<key_indicator_object> parry_icon;

		std::shared_ptr<hp_bar_controller_object> boss_hp_bar;
		std::shared_ptr<trunk_bar_controller_object> boss_trunk_bar;
		std::shared_ptr<face_image_controller_object> boss_face_image;

		std::shared_ptr<timer_controller_object> timer;
	};

public:
	battle_hud_controller_object(object_descs desc);
	~battle_hud_controller_object() override;

	void post_update(float elapsed_time) override;

	void play_fade(fade::directions direction, float time = 0.2f);

private:
	void on_finalize() override;

	void on_damage_player(const character_object::damage_datas& data);
	void on_damage_boss(const character_object::damage_datas& data);
	void on_damage_boss_stagger_end();

private:
	bool setup_player_datas;
	bool setup_enemy_datas;
	std::shared_ptr<hp_bar_controller_object> player_hp_bar;
	std::shared_ptr<sp_bar_controller_object> player_sp_bar;
	std::shared_ptr<face_image_controller_object> player_face_image;
	std::weak_ptr<player_controller_object> player_object;

	std::shared_ptr<key_indicator_object> attack_icon;
	std::shared_ptr<key_indicator_object> special_icon;
	std::shared_ptr<key_indicator_object> dodge_icon;
	std::shared_ptr<key_indicator_object> parry_icon;

	std::shared_ptr<hp_bar_controller_object> boss_hp_bar;
	std::shared_ptr<trunk_bar_controller_object> boss_trunk_bar;
	std::shared_ptr<face_image_controller_object> boss_face_image;
	std::weak_ptr<enemy_controller_object> boss_object;

	std::shared_ptr<timer_controller_object> timer;

	fade::directions direction;
	fade::states fade_state;
	float fade_animation_time;
	float fade_animation_max_time;
};
