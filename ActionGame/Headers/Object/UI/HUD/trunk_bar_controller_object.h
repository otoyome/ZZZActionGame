#pragma once

#include "object/ui/ui_object.h"
#include "object/ui/ui_image.h"
#include "object/ui/ui_text.h"
#include "object/character/character_object.h"

//	ゲージ用オブジェクト
class trunk_bar_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(trunk_bar_controller_object, game_object);

public:
	enum class directions
	{
		left,
		right,
	};

public:
	struct shake_descs
	{
		float time = 0.0f;
		float power = 0.0f;
	};

	struct change_descs
	{
		int value = 1;
		int max_value = 1;
	};

	struct object_descs
	{
		std::string name;
		std::shared_ptr<ui_image> bar_cover;
		std::shared_ptr<ui_image> bar_base;
		std::shared_ptr<ui_image> bar_main;

		std::shared_ptr<ui_text> value;
	};

public:
	directions get_animation_direction() const;
	void set_animation_direction(directions animation_direction);

public:
	trunk_bar_controller_object(const object_descs& object_desc);
	~trunk_bar_controller_object() override;

	void fixed_update(float elapsed_time) override;

	void post_update(float elapsed_time) override;

	void set_character(std::weak_ptr<character_object> character);
	void call_shake(const shake_descs& shake_desc);

	void set_alpha(float alpha);

private:
	float main_value_inner = 0;
	std::weak_ptr<character_object> character_ptr;
	float animation_speed = 0.05f;
	float color_animation_time = 0.0f;
	directions animation_direction = directions::left;
	shake_descs shake_desc;

	std::shared_ptr<ui_object_base> uis[4];
	DirectX::XMFLOAT4 origin_rects[4];
	DirectX::XMFLOAT4 origin_colors[4];
	DirectX::XMFLOAT2 shake_offset = {0, 0};
};
