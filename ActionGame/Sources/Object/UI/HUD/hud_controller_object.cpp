#include "object/ui/hud/hud_controller_object.h"

#include "object/game_constant.h"

namespace
{
	constexpr std::string_view ENTER_BREAK_HUD = "ENTER_BREAK_HUD";
	constexpr std::string_view EXIT_BREAK_HUD = "EXIT_BREAK_HUD";

	constexpr std::string_view ENTER_RESULT_HUD = "ENTER_RESULT_HUD";
	constexpr std::string_view EXIT_RESULT_HUD = "EXIT_RESULT_HUD";
}

hud_controller_object::hud_controller_object(object_descs desc)
: game_object(desc.name)
, display(static_cast<display_hud>(-1))
, display_inner(static_cast<display_hud>(-1))
, battle_hud(desc.battle_hud)
, break_hud(desc.break_hud)
, retry_hud(desc.retry_hud)
, fade_rate(0)
, setup_player_datas(false)
{
}

hud_controller_object::~hud_controller_object()
{
}

void hud_controller_object::post_update(float elapsed_time)
{
	if (!setup_player_datas)
	{
		if (auto player = object_manager::instance()->search_object<player_controller_object>())
		{
			//	ブレイク演出
			player->register_stagger_enter_callback(ENTER_BREAK_HUD, [this](const character_object::stagger_datas& data)
				{
					set_display(hud_controller_object::display_hud::break_hud);
					break_hud->play(game_constants::stagger_wait_time);
				});
			player->register_stagger_exit_callback(EXIT_BREAK_HUD, [this]()
				{
					set_display(hud_controller_object::display_hud::battle_hud);
				});

			//	リトライ画面
			player->register_dead_start_callback(ENTER_RESULT_HUD, [this]()
				{
					set_display(hud_controller_object::display_hud::retry_hud);
				});
			player->register_select_dead_callback(EXIT_RESULT_HUD, [this]()
				{
					set_display(hud_controller_object::display_hud::none);
				});
			player->register_select_regenerate_callback(EXIT_RESULT_HUD, [this]()
				{
					set_display(hud_controller_object::display_hud::battle_hud);
				});
			setup_player_datas = true;
		}
	}
}

void hud_controller_object::set_display(display_hud display, float fade_rate)
{
	if (this->display == display)
		return;

	this->display = display;
	this->fade_rate = fade_rate;
	switch (display)
	{
		case display_hud::none:
		{
			battle_hud->play_fade(fade::directions::in, fade_rate);
			break_hud->play_fade(fade::directions::in, fade_rate);
			retry_hud->play_fade(fade::directions::in, fade_rate);
			break;
		}
		case display_hud::battle_hud:
		{
			battle_hud->play_fade(fade::directions::out, fade_rate);
			break_hud->play_fade(fade::directions::in, fade_rate);
			retry_hud->play_fade(fade::directions::in, fade_rate);
			break;
		}
		case display_hud::break_hud:
		{
			battle_hud->play_fade(fade::directions::in, fade_rate);
			break_hud->play_fade(fade::directions::out, fade_rate);
			retry_hud->play_fade(fade::directions::in, fade_rate);
			break;
		}
		case display_hud::retry_hud:
		{
			battle_hud->play_fade(fade::directions::in, fade_rate);
			break_hud->play_fade(fade::directions::in, fade_rate);
			retry_hud->play_fade(fade::directions::out, fade_rate);
			break;
		}
	}
}

void hud_controller_object::on_finalize()
{
	battle_hud.reset();
	break_hud.reset();
	retry_hud.reset();
}

#if defined(USE_IMGUI)
void hud_controller_object::imgui_render_core()
{
	const char* texts[] =
	{
		magic_enum::enum_name(display_hud::none).data(),
		magic_enum::enum_name(display_hud::battle_hud).data(),
		magic_enum::enum_name(display_hud::break_hud).data(),
		magic_enum::enum_name(display_hud::retry_hud).data(),
	};
	int id = static_cast<int>(display);
	if (ImGui::Combo("Display", &id, texts, ARRAYSIZE(texts)))
	{
		set_display(static_cast<display_hud>(id));
	}
}
#endif	//	defined(USE_IMGUI)
