#pragma once

#include "object/ui/hud/battle_hud_controller_object.h"
#include "object/ui/hud/break_hud_controller_object.h"
#include "object/ui/hud/retry_hud_controller_object.h"

//	HUD制御用オブジェクト
class hud_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(hud_controller_object, game_object)

public:
	enum class display_hud
	{
		none,
		battle_hud,
		break_hud,
		retry_hud,
	};

	struct object_descs
	{
		std::string name;
		std::shared_ptr<battle_hud_controller_object> battle_hud;
		std::shared_ptr<break_hud_controller_object> break_hud;
		std::shared_ptr<retry_hud_controller_object> retry_hud;
	};

public:
	hud_controller_object(object_descs desc);
	~hud_controller_object() override;

	void post_update(float elapsed_time) override;

	void set_display(display_hud display, float fade_rate = 0.2f);

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

private:
	void on_finalize() override;

private:
	display_hud display;
	display_hud display_inner;
	std::shared_ptr<battle_hud_controller_object> battle_hud;
	std::shared_ptr<break_hud_controller_object> break_hud;
	std::shared_ptr<retry_hud_controller_object> retry_hud;
	float fade_rate;
	bool setup_player_datas;
};
