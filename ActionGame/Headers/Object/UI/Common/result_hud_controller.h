#pragma once

#include "fade_controller_object.h"
#include "object/ui/ui_text.h"

//	リザルト用フェード制御用オブジェクト
class result_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(result_controller_object, game_object)

public:
	struct object_descs
	{
		std::string name;
		std::shared_ptr<ui_image> back_image;
		std::shared_ptr<ui_image> front_image;
		std::shared_ptr<ui_text> result_text;
		std::shared_ptr<ui_text> time_text;
		std::shared_ptr<ui_text> pless_click_text;
	};

public:
	bool is_processing();

public:
	result_controller_object(std::string_view name, object_descs desc);
	~result_controller_object() override;

	void post_update(float elapsed_time) override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

public:
	bool play(fade::directions direction, float fade_time);

	void set_display_time(float time);

private:
	std::shared_ptr<ui_image> back_image;
	std::shared_ptr<ui_image> front_image;
	std::shared_ptr<ui_text> result_text;
	std::shared_ptr<ui_text> time_text;
	std::shared_ptr<ui_text> pless_click_text;
	fade::states state;
	fade::directions direction;
	float total_time;
	float fade_time;
};

//	リザルト遷移用のフェードコントローラー
class result_hud_controller
{
	SINGLETON(result_hud_controller)

public:
	void set_fade_object(std::weak_ptr<result_controller_object> object);

	bool is_processing();
	bool play(fade::directions direction, float fade_time);
	void set_display_time(float time);

public:
	std::weak_ptr<result_controller_object> object;
};
