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

//	リトライHUD制御用オブジェクト
class retry_hud_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(retry_hud_controller_object, game_object)

public:
	enum ui_ids
	{
		text_retry,
		text_regenerate,
		text_dead,
	};

	struct object_descs
	{
		std::string name;
		std::shared_ptr<ui_text> retry_text;

		std::shared_ptr<key_icon_object> regenerate_icon;
		std::shared_ptr<ui_text> regenerate_text;
		std::shared_ptr<key_icon_object> dead_icon;
		std::shared_ptr<ui_text> dead_text;
	};

public:
	retry_hud_controller_object(object_descs desc);
	~retry_hud_controller_object() override;

	void post_update(float elapsed_time) override;

	void play_fade(fade::directions direction, float time = 0.2f);

private:
	void on_finalize() override;

private:
	std::shared_ptr<ui_text> retry_text;

	std::shared_ptr<key_icon_object> regenerate_icon;
	std::shared_ptr<ui_text> regenerate_text;
	std::shared_ptr<key_icon_object> dead_icon;
	std::shared_ptr<ui_text> dead_text;

	DirectX::XMFLOAT4 original_color[magic_enum::enum_count<ui_ids>()];

	fade::directions direction;
	fade::states fade_state;
	float fade_animation_time;
	float fade_animation_max_time;
};
