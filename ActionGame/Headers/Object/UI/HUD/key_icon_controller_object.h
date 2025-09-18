#pragma once

#include "object/ui/ui_object.h"
#include "object/ui/ui_image.h"
#include "object/ui/ui_text.h"
#include "object/ui/ui_button.h"
#include "object/character/character_object.h"

//	キーアイコンオブジェクト
class key_icon_object : public game_object
{
	GAME_OBJECT_DECRLEAR(key_icon_object, game_object);

public:
	enum class key_icon_type
	{
		e,
		q,
		space,
		mouse_left_click,
		mouse_right_click,
	};

	struct object_descs
	{
		std::string name;
		std::shared_ptr<ui_image> icon;
		key_icon_type icon_type;
	};

public:
	key_icon_object(const object_descs& object_desc);
	~key_icon_object() override;

	void set_alpha(float alpha);
	void set_key_icon_type(key_icon_type type);

private:
	std::shared_ptr<ui_image> icon;
	DirectX::XMFLOAT4 origin_color;
};

//	キーインジケータ―オブジェクト
class key_indicator_object : public game_object
{
	GAME_OBJECT_DECRLEAR(key_indicator_object, game_object);

public:
	struct object_descs
	{
		std::string name;
		std::shared_ptr<ui_button> button;
		std::shared_ptr<key_icon_object> key;
	};

public:
	key_indicator_object(const object_descs& object_desc);
	~key_indicator_object() override;

	void fixed_update(float elapsed_time) override;
	void set_alpha(float alpha);
	void set_enabled(bool enabled);

	void press();

private:
	bool enabled;
	std::shared_ptr<ui_button> button;
	std::shared_ptr<key_icon_object> key;
};
