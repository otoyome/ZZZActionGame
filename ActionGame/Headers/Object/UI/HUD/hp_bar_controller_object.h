#pragma once

#include "object/ui/ui_object.h"
#include "object/ui/ui_image.h"
#include "object/character/character_object.h"

//	ゲージ用オブジェクト
class hp_bar_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(hp_bar_controller_object, game_object);

private:
	enum class animation_state
	{
		main,
		sub,
		idle,
	};

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
		std::shared_ptr<ui_image> bar_sub;
	};

public:
	directions get_animation_direction() const;
	void set_animation_direction(directions animation_direction);

public:
	hp_bar_controller_object(const object_descs& object_desc);
	~hp_bar_controller_object() override;

	void fixed_update(float elapsed_time) override;

	void post_update(float elapsed_time) override;

	void set_character(std::weak_ptr<character_object> character);
	void call_shake(const shake_descs& shake_desc);

	void set_alpha(float alpha);

private:
	std::weak_ptr<character_object> character_ptr;
	int value = 1;
	int max_value = 1;
	int main_value_inner = 1;
	int sub_value_inner = 1;
	float animation_speed = 0.025f;
	animation_state animation_state = animation_state::idle;
	directions animation_direction = directions::left;
	shake_descs shake_desc;

	std::shared_ptr<ui_image> ui[4];
	DirectX::XMFLOAT4 origin_rects[4];
	DirectX::XMFLOAT4 origin_colors[4];
	DirectX::XMFLOAT2 shake_offset = {0, 0};
};
