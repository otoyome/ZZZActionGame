#include "scenes/game/game_scene.h"

#include "object/debug/debug_menu.h"

#include "model_instance.h"
#include "camera.h"

#include "object/light_object.h"

#include "object/model_object.h"

#include "object/character/person/boss_seraphim_object.h"
#include "object/character/person/kuroko_object.h"

#include "object/character/player/player_controller_object.h"
#include "object/character/player/player_camera_controller_object.h"

#include "object/character/enemy/BossSeraphim/boss_seraphim_controller_object.h"

#include "object/cinematics/cinematics_object.h"

#include "object/ui/Common/transition_fade_controller.h"
#include "object/ui/Common/result_hud_controller.h"

#include "object/connector_object.h"

#include "object/ui/HUD/hud_controller_object.h"

#include "scenes/title_scene.h"

namespace
{
	constexpr std::string_view TRANSITION_CLEAR = "TRANSITION_CLEAR";
	constexpr std::string_view SlashTexturePath = "Assets/Resources/texture/Slashed.png";
}

game_scene::game_scene()
: scene_base()
{
}

game_scene::~game_scene()
{
}

bool game_scene::initialize_core()
{
	//	カーソル関係の非表示
	mouse::instance()->set_center_lock(true);
	mouse::instance()->set_cursol_hide(true);

	std::shared_ptr<camera_object> main_camera = object_manager::instance()->search_object<camera_object>("メインカメラ");
	std::shared_ptr<cinematics_object> main_cinematics = object_manager::instance()->search_object<cinematics_object>("演出管理オブジェクト");

	//	主人公モデル生成
	std::shared_ptr<player_camera_controller_object> player_camera_controller;
	{
		//	モデルオブジェクト生成
		std::shared_ptr<model_object> hero_model = kuroko_object::generate_character_model_object();
		std::shared_ptr<model_object> weapon_model = kuroko_object::generate_weapon_model_object();
		register_object(hero_model);
		register_object(weapon_model);

		auto player1 = std::make_shared<kuroko_object>(hero_model, weapon_model, "player");
		player1->set_position(system_units::convert_m_to_unit({0, 0, -50}));
		player1->set_rotation({0.0f, 0.0f, 0.0f});
		register_object(player1);

		auto controller_object = std::make_shared<player_controller_object>("プレイヤー入力", player1, main_camera);
		register_object(controller_object);

		player_camera_controller = std::make_shared<player_camera_controller_object>("プレイヤーカメラ", controller_object, main_camera);
		player_camera_controller->set_input_purmit_flag(false);
		register_object(player_camera_controller);

		auto shadow_light = std::make_shared<shadow_light_object>("影ライト：クロコ");
		shadow_light->set_color({0.6f, 0.6f, 0.6f, 1.0f});
		shadow_light_to_model_connector_object::setup_datas setup_data;
		setup_data.shadow_light = shadow_light;
		setup_data.main_model = hero_model;
		setup_data.sub_models.emplace_back(weapon_model);
		setup_data.shadow_category = common_pipeline_states::shadow_category::mask_1;
		register_object(shadow_light);
		register_object(std::make_shared<shadow_light_to_model_connector_object>("影ライトコネクター：クロコ", setup_data));
		
		//	演出オブジェクトに登録
		main_cinematics->register_effects_function(controller_object);
	}

	//	ボスモデル生成
	{
		auto shadow_light = std::make_shared<shadow_light_object>("影ライト：ボス");
		shadow_light->set_color({0.6f, 0.6f, 0.6f, 1.0f});
		shadow_light_to_model_connector_object::setup_datas setup_data;
		setup_data.shadow_light = shadow_light;

		//	モデルオブジェクト生成
		std::shared_ptr<model_object> boss_model = boss_seraphim_object::generate_character_model_object();
		register_object(boss_model);

		auto enemy1 = std::make_shared<boss_seraphim_object>(boss_model, "enemy");
		enemy1->set_active(false);
		enemy1->set_position(system_units::convert_m_to_unit({0, -1000, 0}));
		enemy1->set_rotation({ 0.0f, DirectX::XM_PI, 0.0f });
		register_object(enemy1);

		//	ボス死亡時
		enemy1->register_dead_callback(TRANSITION_CLEAR, [this](const character_object::dead_descs&) {state = states::clear_performance_start;});

		auto controller = std::make_shared<boss_seraphim_controller_object>("ボスセラフィム制御用クラス", enemy1);
		controller->set_active(false);

		register_object(controller);

		setup_data.main_model = boss_model;
		setup_data.shadow_category = common_pipeline_states::shadow_category::mask_2;
		register_object(shadow_light);
		register_object(std::make_shared<shadow_light_to_model_connector_object>("影ライトコネクター：ボス", setup_data));

		main_cinematics->register_effects_function(enemy1);
		player_camera_controller->register_character_object(enemy1);
	}

	//	スラッシュテクスチャ読み込み
	texture_manager::instance()->load_texture_from_file(slashed_texture, SlashTexturePath);

	work_timer = 10;
	game_timer = 0;
	state = states::start_oneshot;
	return true;
}

bool game_scene::uninitialize_core()
{
	return true;
}

void game_scene::update_core(float elapsed_time)
{
	auto keyboard_state = keyboard::instance()->get_state();
	auto mouse_state = mouse::instance()->get_state();
	switch (state)
	{
		case states::start_oneshot:
		{
			if (++work_timer < 10.0f)
				break;

			//	カーソル関係の非表示
			mouse::instance()->set_center_lock(true);
			mouse::instance()->set_cursol_hide(true);

			//	HUDを透過しておく
			auto hud_controller = object_manager::instance()->search_object<hud_controller_object>();
			hud_controller->set_display(hud_controller_object::display_hud::none, 0.2f);

			//	プレイヤーが終了を選択した際にゲームを終わらせる
			{
				auto controller = object_manager::instance()->search_object<player_controller_object>();
				controller->register_select_dead_callback("Game End", [&]() {state = states::dead_fade_start; });
			}

			transition_fade_controller::instance()->play(fade::directions::out, 0.0f, fade::modes::black);
			work_timer = 0;
			state = states::start_wait;
			break;
		}
		case states::start_wait:
		{
			if (++work_timer >= 10.0f)
			{
				//	操作キャラクターの入場演出を呼び出す
				auto controller = object_manager::instance()->search_object<player_controller_object>();
				controller->get_controll_object()->change_state(character_object::state_names[character_object::state_entry]);
				controller->get_controll_object()->set_flag(character_object::flags::camera_distance_translucent, false);

				//	カメラの入場演出を呼び出す
				auto camera_controller = object_manager::instance()->search_object<player_camera_controller_object>();
				camera_controller->change_state(player_camera_controller_object::states::entry);

				//	フェードイン
				transition_fade_controller::instance()->play(fade::directions::in, 0.5f, fade::modes::black);
				work_timer = 0.0f;
				state = states::fade_in_wait;
			}
			break;
		}
		case states::fade_in_wait:
		{
			if (transition_fade_controller::instance()->is_processing())
			{
				work_timer = 0.0f;
				state = states::entry_camera_wait;
			}
			break;
		}
		case states::entry_camera_wait:
		{
			auto camera_controller = object_manager::instance()->search_object<player_camera_controller_object>();
			if (!camera_controller->is_camera_transition_wait())
			{
				//	操作キャラクターの操作をできるようにする
				auto player_controller = object_manager::instance()->search_object<player_controller_object>();
				player_controller->get_controll_object()->set_flag(character_object::flags::camera_distance_translucent, true);

				//	カメラの入場演出を呼び出す
				auto camera_controller = object_manager::instance()->search_object<player_camera_controller_object>();
				camera_controller->set_input_purmit_flag(true);

				//	ボスを動かす
				auto boss_controller = object_manager::instance()->search_object<boss_seraphim_controller_object>("ボスセラフィム制御用クラス");
				boss_controller->set_active(true);

				//	HUDを透過しておく
				auto hud_controller = object_manager::instance()->search_object<hud_controller_object>();
				hud_controller->set_display(hud_controller_object::display_hud::battle_hud, 0.2f);

				work_timer = 0.0f;
				state = states::state_wait;
			}
			break;
		}
		case states::state_wait:
		{
			//	ゲーム時間経過
			game_timer += elapsed_time;
			if (auto timer_object = object_manager::instance()->search_object<timer_controller_object>("経過時間表示コントローラー"))
			{
				timer_object->set_time(game_timer);
			}

#if defined(_DEBUG)
			auto keyboard_state = keyboard::instance()->get_state();
			if (keyboard_state.is_key_state(keyboard::key_states::f1) == input::state::down)
			{
				state = states::clear_performance_start;
			}
#endif	//	defined(_DEBUG)
			break;
		}
		case states::dead_fade_start:
		{
			//	カーソル関係の非表示
			mouse::instance()->set_center_lock(true);
			mouse::instance()->set_cursol_hide(true);

			transition_fade_controller::instance()->play(fade::directions::out, 0.5f, fade::modes::black);
			state = states::dead_fade_complete;
			break;
		}
		case states::dead_fade_complete:
		{
			if (!transition_fade_controller::instance()->is_processing())
			{
				state = states::game_end;
			}
			break;
		}
		case states::clear_performance_start:
		{
			//	各種入力系統を停止
			if (auto player_controller = object_manager::instance()->search_object<player_controller_object>())
			{
				player_controller->set_active(false);
				player_controller->get_controll_object()->set_flag(character_object::flags::camera_distance_translucent, false);
			}

			auto boss_controlleres = object_manager::instance()->search_objects<enemy_controller_object>();
			for (auto& boss_controller : boss_controlleres)
			{
				boss_controller->set_active(false);
				boss_controller->get_controll_object()->set_flag(character_object::flags::camera_distance_translucent, false);
			}

			//	カメラ側はクリア演出処理へ移行
			if (auto player_camera_controller = object_manager::instance()->search_object<player_camera_controller_object>())
			{
				player_camera_controller->change_state(player_camera_controller_object::states::clear_performance);
				player_camera_controller->set_input_purmit_flag(false);
			}

			//	演出オブジェクトに通知
			if (auto cinematics = object_manager::instance()->search_object<cinematics_object>())
			{
				cinematics->external_setting_mode(true);
				//	もういっそここでスラッシュテクスチャを設定しておく
				auto data = cinematics->get_external_post_process_data();
				data.overwrite_text.intensity = 1.0f;
				data.overwrite_text_texture = slashed_texture;
				cinematics->set_external_post_process_data(data);
			}

			//	HUDを透過しておく
			auto hud_controller = object_manager::instance()->search_object<hud_controller_object>();
			hud_controller->set_display(hud_controller_object::display_hud::none);

			state = states::clear_performance_wait;
			break;
		}
		case states::clear_performance_wait:
		{
			auto player_camera_controller = object_manager::instance()->search_object<player_camera_controller_object>();
			if (player_camera_controller->is_camera_transition_wait())
			{
				//	リザルト画面へ移動
				result_hud_controller::instance()->play(fade::directions::out, 0.5f);
				result_hud_controller::instance()->set_display_time(game_timer);

				state = states::result_fade_out_wait;
			}
			break;
		}
		case states::result_fade_out_wait:
		{
			if (!result_hud_controller::instance()->is_processing())
			{
				//	カーソル関係の非表示
				mouse::instance()->set_center_lock(false);
				mouse::instance()->set_cursol_hide(false);
				state = states::result_wait;
			}
			break;
		}
		case states::result_wait:
		{
			//	キー入力があれば終了
			auto mouse_state = mouse::instance()->get_state();
			if (mouse_state.is_key_state(mouse::key_states::left) == input::state::down ||
				mouse_state.is_key_state(mouse::key_states::right) == input::state::down)
			{
				//	カーソル関係の非表示
				mouse::instance()->set_center_lock(true);
				mouse::instance()->set_cursol_hide(true);

				transition_fade_controller::instance()->play(fade::directions::out, 0.5f, fade::modes::black);
				state = states::game_end;
			}
			break;
		}
		case states::game_end:
		{
			if (!transition_fade_controller::instance()->is_processing())
			{
				std::shared_ptr<cinematics_object> main_cinematics = object_manager::instance()->search_object<cinematics_object>("演出管理オブジェクト");
				main_cinematics->external_setting_mode(false);

				scene_manager::instance()->remove_scene<ui_scene>();
				scene_manager::instance()->remove_scene<stage_01_scene>();
				scene_manager::instance()->remove_scene<game_scene>();
				scene_manager::instance()->add_scene<title_scene>();
			}
			break;
		}
	}
}

#if defined(USE_IMGUI)
void game_scene::imgui_render_core()
{
	if (ImGui::TreeNode("game_scene"))
	{
		ImGui::TreePop();
	}
}
#endif	//	defined(USE_IMGUI)
