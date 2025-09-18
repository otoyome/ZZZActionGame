#pragma once

#include "object/ui/ui_object.h"
#include "object/ui/ui_image.h"

#include "object/ui/common/fade_controller_object.h"

#include "object/ui/hud/key_icon_controller_object.h"

//	BREAK HUD制御用オブジェクト
class break_hud_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(break_hud_controller_object, game_object)

public:
	enum ui_ids
	{
		plate_base,
		cover,
		main,
		key_text,
	};

	struct object_descs
	{
		std::string name;
		std::shared_ptr<ui_image> break_plate_base;
		std::shared_ptr<ui_image> break_cover;
		std::shared_ptr<ui_image> break_main;
		std::shared_ptr<ui_text> break_key_text;
		std::shared_ptr<key_icon_object> break_key_icon;
	};

public:
	break_hud_controller_object(object_descs desc);
	~break_hud_controller_object() override;

	void post_update(float elapsed_time) override;

	void play_fade(fade::directions direction, float time = 0.2f);

	void play(float time);

private:
	std::shared_ptr<ui_image> break_plate_base;
	std::shared_ptr<ui_image> break_cover;
	std::shared_ptr<ui_image> break_main;
	std::shared_ptr<ui_text> break_key_text;
	std::shared_ptr<key_icon_object> break_key_icon;
	float animation_time;
	float animation_max_time;

	DirectX::XMFLOAT4 original_color[magic_enum::enum_count<ui_ids>()];
	DirectX::XMFLOAT4 original_main_rect;
	fade::directions direction;
	fade::states fade_state;
	float fade_animation_time;
	float fade_animation_max_time;
};
