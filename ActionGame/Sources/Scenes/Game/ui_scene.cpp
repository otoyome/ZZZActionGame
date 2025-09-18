#include "scenes/game/ui_scene.h"

#include "editor/editor_camera.h"

#include "model_instance.h"
#include "camera.h"

#include "object/light_object.h"

#include "object/ui/ui_text.h"
#include "object/ui/hud/hud_controller_object.h"

#include "object/ui/common/result_hud_controller.h"

namespace
{
}

ui_scene::ui_scene()
: scene_base()
{
}

ui_scene::~ui_scene()
{
}

bool ui_scene::initialize_core()
{
	font_data_ptr font;
	font_data_manager::instance()->load_from_file(font, "Assets/Resources/fonts/MS Gothic/MS Gothic.fnt");

	hud_controller_object::object_descs hud_object_desc;
	{
		battle_hud_controller_object::object_descs battle_hud_object_desc;
		//	プレイヤー顔ウィンドウ
		{
			face_image_controller_object::object_descs object_desc;
			object_desc.name = "Player : 顔ウィンドウ コントローラー";

			//	顔画像
			object_desc.base = std::make_shared<ui_image>("Player : 顔ベース");
			object_desc.base->set_screen_rect({ 40, 40, 180, 50 });
			object_desc.base->set_texture("Assets/Resources/texture/hud_base.png");
			object_desc.base->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.base->set_sort_order(100);
			register_object(object_desc.base);

			object_desc.main = std::make_shared<ui_image>("Player : 顔イメージ");
			object_desc.main->set_screen_rect({ 40, 40, 180, 50 });
			object_desc.main->set_texture("Assets/Resources/texture/hud_image.png");
			object_desc.main->set_sort_order(110);
			register_object(object_desc.main);

			object_desc.cover = std::make_shared<ui_image>("Player : 顔カバー");
			object_desc.cover->set_screen_rect({ 40, 40, 180, 50 });
			object_desc.cover->set_texture("Assets/Resources/texture/hud_cover.png");
			object_desc.cover->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.cover->set_sort_order(120);
			register_object(object_desc.cover);

			battle_hud_object_desc.player_face_image = std::make_shared<face_image_controller_object>(object_desc);
			register_object(battle_hud_object_desc.player_face_image);
		}
		//	プレイヤーHPバー
		{
			hp_bar_controller_object::object_descs object_desc;
			object_desc.name = "Player : HPバー コントローラー";

			object_desc.bar_cover = std::make_shared<ui_image>("Player : HPバーカバー");
			object_desc.bar_cover->set_screen_rect({ 190, 40, 520, 50 });
			object_desc.bar_cover->set_texture("Assets/Resources/texture/hud_cover.png");
			object_desc.bar_cover->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_cover->set_sort_order(15);
			register_object(object_desc.bar_cover);

			object_desc.bar_base = std::make_shared<ui_image>("Player : HPバーベース");
			object_desc.bar_base->set_screen_rect({ 200, 42, 500, 46 });
			object_desc.bar_base->set_texture("Assets/Resources/texture/hud_base.png");
			object_desc.bar_base->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_base->set_sort_order(0);
			register_object(object_desc.bar_base);

			object_desc.bar_main = std::make_shared<ui_image>("Player : HPバーメイン");
			object_desc.bar_main->set_screen_rect({ 218, 42, 466, 46 });
			object_desc.bar_main->set_texture("Assets/Resources/texture/hud_image.png");
			object_desc.bar_main->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_main->set_color({0, 1, 0, 1});
			object_desc.bar_main->set_sort_order(10);
			register_object(object_desc.bar_main);

			object_desc.bar_sub = std::make_shared<ui_image>("Player : HPバーサブ");
			object_desc.bar_sub->set_screen_rect({ 218, 42, 466, 46 });
			object_desc.bar_sub->set_color({1, 0, 0, 1});
			object_desc.bar_sub->set_texture("Assets/Resources/texture/hud_image.png");
			object_desc.bar_sub->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_sub->set_sort_order(5);
			register_object(object_desc.bar_sub);

			battle_hud_object_desc.player_hp_bar = std::make_shared<hp_bar_controller_object>(object_desc);
			register_object(battle_hud_object_desc.player_hp_bar);
		}
		//	プレイヤーSPバー
		{
			sp_bar_controller_object::object_descs object_desc;
			object_desc.name = "Player : SPバー コントローラー";

			object_desc.bar_cover = std::make_shared<ui_image>("Player : SPバーカバー");
			object_desc.bar_cover->set_screen_rect({ 340, 64, 369, 26 });
			object_desc.bar_cover->set_texture("Assets/Resources/texture/hud_cover.png");
			object_desc.bar_cover->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_cover->set_sort_order(40);
			register_object(object_desc.bar_cover);

			object_desc.bar_base = std::make_shared<ui_image>("Player : SPバーベース");
			object_desc.bar_base->set_screen_rect({ 356, 65, 330, 24 });
			object_desc.bar_base->set_color({ 0.2f, 0.2f, 0.2f, 1 });
			object_desc.bar_base->set_texture("Assets/Resources/texture/hud_image.png");
			object_desc.bar_base->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_base->set_sort_order(30);
			register_object(object_desc.bar_base);

			object_desc.bar_main = std::make_shared<ui_image>("Player : SPバーメイン");
			object_desc.bar_main->set_screen_rect({ 356, 65, 330, 24 });
			object_desc.bar_main->set_texture("Assets/Resources/texture/hud_image.png");
			object_desc.bar_main->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_main->set_color({ 0, 0.5f, 1, 1 });
			object_desc.bar_main->set_sort_order(35);
			register_object(object_desc.bar_main);

			battle_hud_object_desc.player_sp_bar = std::make_shared<sp_bar_controller_object>(object_desc);
			register_object(battle_hud_object_desc.player_sp_bar);
		}

		//	攻撃アイコン
		{
			key_icon_object::object_descs key_icon_object_desc;
			key_icon_object_desc.name = "攻撃キーアイコン";
			key_icon_object_desc.icon_type = key_icon_object::key_icon_type::mouse_left_click;
			key_icon_object_desc.icon = std::make_shared<ui_image>("攻撃キーアイコン");
			key_icon_object_desc.icon->set_screen_rect({ 1110, 840, 36, 36 });
			key_icon_object_desc.icon->set_draw_mode(ui_image::draw_modes::single);
			key_icon_object_desc.icon->set_sort_order(0);
			register_object(key_icon_object_desc.icon);
			auto& key_object = std::make_shared<key_icon_object>(key_icon_object_desc);
			register_object(key_object);

			auto main_icon = std::make_shared<ui_image>("攻撃メインアイコン");
			main_icon->set_texture("Assets/Resources/texture/attack_icon.png");
			main_icon->set_screen_rect({ 1080, 740, 96, 96 });
			main_icon->set_draw_mode(ui_image::draw_modes::single);
			main_icon->set_sort_order(0);
			register_object(main_icon);

			auto button = std::make_shared<ui_button>("攻撃アイコンボタン", main_icon);
			button->set_color(ui_button::color_type::normal, {1, 1, 1, 1});
			button->set_color(ui_button::color_type::pressed, {1, 1, 1, 1});
			button->set_color(ui_button::color_type::disabled, {1, 1, 1, 1});
			register_object(button);

			key_indicator_object::object_descs object_desc;
			object_desc.name = "攻撃キーインジケータ";
			object_desc.button = button;
			object_desc.key = key_object;
			battle_hud_object_desc.attack_icon = std::make_shared<key_indicator_object>(object_desc);
			register_object(battle_hud_object_desc.attack_icon);
		}

		//	回避アイコン
		{
			key_icon_object::object_descs key_icon_object_desc;
			key_icon_object_desc.name = "回避キーアイコン";
			key_icon_object_desc.icon_type = key_icon_object::key_icon_type::mouse_right_click;
			key_icon_object_desc.icon = std::make_shared<ui_image>("回避キーアイコン");
			key_icon_object_desc.icon->set_screen_rect({ 1230, 840, 36, 36 });
			key_icon_object_desc.icon->set_draw_mode(ui_image::draw_modes::single);
			key_icon_object_desc.icon->set_sort_order(0);
			register_object(key_icon_object_desc.icon);
			auto& key_object = std::make_shared<key_icon_object>(key_icon_object_desc);
			register_object(key_object);

			auto main_icon = std::make_shared<ui_image>("回避メインアイコン");
			main_icon->set_texture("Assets/Resources/texture/dodge_icon.png");
			main_icon->set_screen_rect({ 1200, 740, 96, 96 });
			main_icon->set_draw_mode(ui_image::draw_modes::single);
			main_icon->set_sort_order(0);
			register_object(main_icon);

			auto button = std::make_shared<ui_button>("回避アイコンボタン", main_icon);
			button->set_color(ui_button::color_type::normal, {1, 1, 1, 1});
			button->set_color(ui_button::color_type::pressed, {1, 1, 1, 1});
			button->set_color(ui_button::color_type::disabled, {1, 1, 1, 1});
			register_object(button);

			key_indicator_object::object_descs object_desc;
			object_desc.name = "回避キーインジケータ";
			object_desc.button = button;
			object_desc.key = key_object;
			battle_hud_object_desc.dodge_icon = std::make_shared<key_indicator_object>(object_desc);
			register_object(battle_hud_object_desc.attack_icon);
		}

		//	特殊アイコン
		{
			key_icon_object::object_descs key_icon_object_desc;
			key_icon_object_desc.name = "特殊キーアイコン";
			key_icon_object_desc.icon_type = key_icon_object::key_icon_type::e;
			key_icon_object_desc.icon = std::make_shared<ui_image>("特殊キーアイコン");
			key_icon_object_desc.icon->set_screen_rect({ 1350, 840, 36, 36 });
			key_icon_object_desc.icon->set_draw_mode(ui_image::draw_modes::single);
			key_icon_object_desc.icon->set_sort_order(0);
			register_object(key_icon_object_desc.icon);
			auto& key_object = std::make_shared<key_icon_object>(key_icon_object_desc);
			register_object(key_object);

			auto main_icon = std::make_shared<ui_image>("特殊メインアイコン");
			main_icon->set_texture("Assets/Resources/texture/special_icon.png");
			main_icon->set_screen_rect({ 1320, 740, 96, 96 });
			main_icon->set_draw_mode(ui_image::draw_modes::single);
			main_icon->set_sort_order(0);
			register_object(main_icon);

			auto button = std::make_shared<ui_button>("特殊アイコンボタン", main_icon);
			button->set_color(ui_button::color_type::normal, {1, 0, 1, 1});
			button->set_color(ui_button::color_type::pressed, {1, 1, 0, 1});
			register_object(button);

			key_indicator_object::object_descs object_desc;
			object_desc.name = "特殊キーインジケータ";
			object_desc.button = button;
			object_desc.key = key_object;
			battle_hud_object_desc.special_icon = std::make_shared<key_indicator_object>(object_desc);
			register_object(battle_hud_object_desc.attack_icon);
		}

		//	パリィアイコン
		{
			key_icon_object::object_descs key_icon_object_desc;
			key_icon_object_desc.name = "パリィキーアイコン";
			key_icon_object_desc.icon_type = key_icon_object::key_icon_type::space;
			key_icon_object_desc.icon = std::make_shared<ui_image>("パリィキーアイコン");
			key_icon_object_desc.icon->set_screen_rect({ 1470 - 18, 840, 72, 36 });
			key_icon_object_desc.icon->set_draw_mode(ui_image::draw_modes::single);
			key_icon_object_desc.icon->set_sort_order(0);
			register_object(key_icon_object_desc.icon);
			auto& key_object = std::make_shared<key_icon_object>(key_icon_object_desc);
			register_object(key_object);

			auto main_icon = std::make_shared<ui_image>("パリィメインアイコン");
			main_icon->set_texture("Assets/Resources/texture/parry_icon.png");
			main_icon->set_screen_rect({ 1440, 740, 96, 96 });
			main_icon->set_draw_mode(ui_image::draw_modes::single);
			main_icon->set_sort_order(0);
			register_object(main_icon);

			auto button = std::make_shared<ui_button>("パリィアイコンボタン", main_icon);
			button->set_color(ui_button::color_type::normal, {1, 1, 1, 1});
			button->set_color(ui_button::color_type::pressed, {1, 1, 1, 1});
			button->set_color(ui_button::color_type::disabled, {1, 1, 1, 1});
			register_object(button);

			key_indicator_object::object_descs object_desc;
			object_desc.name = "パリィキーインジケータ";
			object_desc.button = button;
			object_desc.key = key_object;
			battle_hud_object_desc.parry_icon = std::make_shared<key_indicator_object>(object_desc);
			register_object(battle_hud_object_desc.attack_icon);
		}

		//	ボス用HPバー
		{
			hp_bar_controller_object::object_descs object_desc;
			object_desc.name = "ボス : HPバー コントローラー";

			object_desc.bar_cover = std::make_shared<ui_image>("ボス : HPバーカバー");
			object_desc.bar_cover->set_screen_rect({ 780, 40, 650, 50 });
			object_desc.bar_cover->set_texture("Assets/Resources/texture/hud_cover.png");
			object_desc.bar_cover->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_cover->set_sort_order(20);
			register_object(object_desc.bar_cover);

			object_desc.bar_base = std::make_shared<ui_image>("ボス : HPバーベース");
			object_desc.bar_base->set_screen_rect({ 800, 40, 650, 50 });
			object_desc.bar_base->set_texture("Assets/Resources/texture/hud_base.png");
			object_desc.bar_base->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_base->set_sort_order(0);
			register_object(object_desc.bar_base);

			object_desc.bar_main = std::make_shared<ui_image>("ボス : HPバーメイン");
			object_desc.bar_main->set_screen_rect({ 810, 44, 600, 46 });
			object_desc.bar_main->set_texture("Assets/Resources/texture/hud_image.png");
			object_desc.bar_main->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_main->set_color({ 0, 1, 0, 1 });
			object_desc.bar_main->set_sort_order(10);
			register_object(object_desc.bar_main);

			object_desc.bar_sub = std::make_shared<ui_image>("ボス : HPバーサブ");
			object_desc.bar_sub->set_screen_rect({ 810, 44, 600, 46 });
			object_desc.bar_sub->set_texture("Assets/Resources/texture/hud_image.png");
			object_desc.bar_sub->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_sub->set_color({ 1, 0, 0, 1 });
			object_desc.bar_sub->set_sort_order(5);
			register_object(object_desc.bar_sub);

			battle_hud_object_desc.boss_hp_bar = std::make_shared<hp_bar_controller_object>(object_desc);
			battle_hud_object_desc.boss_hp_bar->set_animation_direction(hp_bar_controller_object::directions::right);
			register_object(battle_hud_object_desc.boss_hp_bar);
		}

		//	ボス体幹バー
		{
			trunk_bar_controller_object::object_descs object_desc;
			object_desc.name = "ボス : 体幹バー コントローラー";

			object_desc.bar_cover = std::make_shared<ui_image>("ボス : 体幹バーカバー");
			object_desc.bar_cover->set_screen_rect({ 780, 60, 500, 28 });
			object_desc.bar_cover->set_texture("Assets/Resources/texture/hud_cover.png");
			object_desc.bar_cover->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_cover->set_sort_order(40);
			register_object(object_desc.bar_cover);

			object_desc.bar_base = std::make_shared<ui_image>("ボス : 体幹バーベース");
			object_desc.bar_base->set_screen_rect({ 780, 60, 500, 28 });
			object_desc.bar_base->set_texture("Assets/Resources/texture/hud_base.png");
			object_desc.bar_base->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_base->set_sort_order(25);
			register_object(object_desc.bar_base);

			object_desc.bar_main = std::make_shared<ui_image>("ボス : 体幹バーメイン");
			object_desc.bar_main->set_screen_rect({ 810, 63, 440, 25 });
			object_desc.bar_main->set_texture("Assets/Resources/texture/hud_image.png");
			object_desc.bar_main->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.bar_main->set_color({ 1, 1, 0, 1 });
			object_desc.bar_main->set_sort_order(35);
			register_object(object_desc.bar_main);

			object_desc.value = std::make_shared<ui_text>("ボス：体幹数値", font);
			object_desc.value->set_texts("00");
			object_desc.value->set_screen_rect({ 780, 50, 32, 32 });
			object_desc.value->set_color({ 1, 0, 0, 1 });
			object_desc.value->set_sort_order(140);
			register_object(object_desc.value);

			battle_hud_object_desc.boss_trunk_bar = std::make_shared<trunk_bar_controller_object>(object_desc);
			register_object(battle_hud_object_desc.boss_trunk_bar);
		}

		//	ボス顔ウィンドウ
		{
			face_image_controller_object::object_descs object_desc;
			object_desc.name = "ボス : 顔ウィンドウ コントローラー";

			//	顔画像
			object_desc.base = std::make_shared<ui_image>("ボス : 顔ベース");
			object_desc.base->set_screen_rect({ 1400, 40, 180, 50 });
			object_desc.base->set_texture("Assets/Resources/texture/hud_base.png");
			object_desc.base->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.base->set_sort_order(100);
			register_object(object_desc.base);

			object_desc.main = std::make_shared<ui_image>("ボス : 顔イメージ");
			object_desc.main->set_screen_rect({ 1400, 40, 180, 50 });
			object_desc.main->set_texture("Assets/Resources/texture/hud_image.png");
			object_desc.main->set_sort_order(110);
			register_object(object_desc.main);

			object_desc.cover = std::make_shared<ui_image>("ボス : 顔カバー");
			object_desc.cover->set_screen_rect({ 1400, 40, 180, 50 });
			object_desc.cover->set_texture("Assets/Resources/texture/hud_cover.png");
			object_desc.cover->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.cover->set_sort_order(120);
			register_object(object_desc.cover);

			battle_hud_object_desc.boss_face_image = std::make_shared<face_image_controller_object>(object_desc);
			register_object(battle_hud_object_desc.boss_face_image);
		}

		//	タイマーウィンドウ
		{
			timer_controller_object::object_descs object_desc;
			object_desc.name = "経過時間表示コントローラー";

			//	顔画像
			object_desc.image = std::make_shared<ui_image>("タイマー：ベース");
			object_desc.image->set_screen_rect({ 1338, 124, 243, 50 });
			object_desc.image->set_texture("Assets/Resources/texture/NotificationWindow.png");
			object_desc.image->set_draw_mode(ui_image::draw_modes::sliced);
			object_desc.image->set_sort_order(100);
			register_object(object_desc.image);

			object_desc.text = std::make_shared<ui_text>("タイマー：：数値", font);
			object_desc.text->set_texts("00:00:00");
			object_desc.text->set_screen_rect({ 1397, 135, 32, 32 });
			object_desc.text->set_sort_order(110);
			register_object(object_desc.text);

			battle_hud_object_desc.timer = std::make_shared<timer_controller_object>(object_desc);
			register_object(battle_hud_object_desc.timer);
		}

		battle_hud_object_desc.name = "戦闘メインHUD";
		hud_object_desc.battle_hud = std::make_shared<battle_hud_controller_object>(battle_hud_object_desc);
		register_object(hud_object_desc.battle_hud);
	}

	//	ブレイク通知
	{
		break_hud_controller_object::object_descs break_hud_controller_desc;
		{
			break_hud_controller_desc.break_plate_base = std::make_shared<ui_image>("ブレイク:ベース");
			break_hud_controller_desc.break_plate_base->set_texture("Assets/Resources/texture/hud_base.png");
			break_hud_controller_desc.break_plate_base->set_screen_rect({ 500, 700, 600, 40 });
			break_hud_controller_desc.break_plate_base->set_color({0.3f, 0.3f, 0.3f, 0.3f});
			break_hud_controller_desc.break_plate_base->set_draw_mode(ui_image::draw_modes::sliced);
			break_hud_controller_desc.break_plate_base->set_sort_order(0);
			register_object(break_hud_controller_desc.break_plate_base);

			break_hud_controller_desc.break_cover = std::make_shared<ui_image>("ブレイク:カバー");
			break_hud_controller_desc.break_cover->set_texture("Assets/Resources/texture/hud_cover.png");
			break_hud_controller_desc.break_cover->set_screen_rect({ 500, 700, 600, 40 });
			break_hud_controller_desc.break_cover->set_draw_mode(ui_image::draw_modes::sliced);
			break_hud_controller_desc.break_cover->set_sort_order(10);
			register_object(break_hud_controller_desc.break_cover);

			break_hud_controller_desc.break_main = std::make_shared<ui_image>("ブレイク:メイン");
			break_hud_controller_desc.break_main->set_texture("Assets/Resources/texture/hud_base_2.png");
			break_hud_controller_desc.break_main->set_screen_rect({ 500, 700, 600, 40 });
			break_hud_controller_desc.break_main->set_draw_mode(ui_image::draw_modes::sliced);
			break_hud_controller_desc.break_main->set_color({1, 0.2f, 0.2f, 1.0f});
			break_hud_controller_desc.break_main->set_sort_order(4);
			register_object(break_hud_controller_desc.break_main);

			key_icon_object::object_descs key_icon_object_desc;
			key_icon_object_desc.name = "ブレイクキーアイコン";
			key_icon_object_desc.icon_type = key_icon_object::key_icon_type::mouse_left_click;
			key_icon_object_desc.icon = std::make_shared<ui_image>("ブレイクキーアイコン");
			key_icon_object_desc.icon->set_screen_rect({ 600, 705, 36, 36 });
			key_icon_object_desc.icon->set_draw_mode(ui_image::draw_modes::single);
			key_icon_object_desc.icon->set_sort_order(5);
			register_object(key_icon_object_desc.icon);
			break_hud_controller_desc.break_key_icon = std::make_shared<key_icon_object>(key_icon_object_desc);
			register_object(break_hud_controller_desc.break_key_icon);

			break_hud_controller_desc.break_key_text = std::make_shared<ui_text>("ブレイク:文字", font);
			break_hud_controller_desc.break_key_text->set_screen_rect({ 630, 705, 480, 96 });
			break_hud_controller_desc.break_key_text->set_texts("B R E A K  A T T A C K");
			break_hud_controller_desc.break_key_text->set_sort_order(5);
			register_object(break_hud_controller_desc.break_key_text);
		}

		break_hud_controller_desc.name = "ブレイクメインHUD";
		hud_object_desc.break_hud = std::make_shared<break_hud_controller_object>(break_hud_controller_desc);
		register_object(hud_object_desc.break_hud);
	}

	//	リトライ用HUDコントローラー生成
	{
		retry_hud_controller_object::object_descs retry_object_descs;

		retry_object_descs.retry_text = std::make_shared<ui_text>("リトライ:文字", font);
		retry_object_descs.retry_text->set_screen_rect({ 723, 433, 480, 96 });
		retry_object_descs.retry_text->set_color({ 1, 1, 0, 1 });
		retry_object_descs.retry_text->set_texts("復活しますか？");
		retry_object_descs.retry_text->set_sort_order(1);
		register_object(retry_object_descs.retry_text);

		{
			key_icon_object::object_descs key_icon_object_desc;
			key_icon_object_desc.name = "リトライキーアイコン";
			key_icon_object_desc.icon_type = key_icon_object::key_icon_type::mouse_left_click;
			key_icon_object_desc.icon = std::make_shared<ui_image>("リトライ：キーアイコン");
			key_icon_object_desc.icon->set_screen_rect({ 660, 586, 36, 36 });
			key_icon_object_desc.icon->set_draw_mode(ui_image::draw_modes::single);
			key_icon_object_desc.icon->set_sort_order(5);
			register_object(key_icon_object_desc.icon);
			retry_object_descs.regenerate_icon = std::make_shared<key_icon_object>(key_icon_object_desc);
			register_object(retry_object_descs.regenerate_icon);

			retry_object_descs.regenerate_text = std::make_shared<ui_text>("リトライ:キー文字", font);
			retry_object_descs.regenerate_text->set_screen_rect({ 710, 586, 480, 96 });
			retry_object_descs.regenerate_text->set_color({ 1, 0, 0, 1 });
			retry_object_descs.regenerate_text->set_texts("復活");
			retry_object_descs.regenerate_text->set_sort_order(5);
			register_object(retry_object_descs.regenerate_text);
		}

		{
			key_icon_object::object_descs key_icon_object_desc;
			key_icon_object_desc.name = "リトライキーアイコン";
			key_icon_object_desc.icon_type = key_icon_object::key_icon_type::mouse_right_click;
			key_icon_object_desc.icon = std::make_shared<ui_image>("リトライ：キーアイコン");
			key_icon_object_desc.icon->set_screen_rect({ 900, 586, 36, 36 });
			key_icon_object_desc.icon->set_draw_mode(ui_image::draw_modes::single);
			key_icon_object_desc.icon->set_sort_order(5);
			register_object(key_icon_object_desc.icon);

			retry_object_descs.dead_icon = std::make_shared<key_icon_object>(key_icon_object_desc);
			register_object(retry_object_descs.dead_icon);

			retry_object_descs.dead_text = std::make_shared<ui_text>("リトライ:キー文字", font);
			retry_object_descs.dead_text->set_screen_rect({ 950, 586, 480, 96 });
			retry_object_descs.dead_text->set_color({ 0, 0, 1, 1 });
			retry_object_descs.dead_text->set_texts("終了");
			retry_object_descs.dead_text->set_sort_order(5);
			register_object(retry_object_descs.dead_text);
		}

		retry_object_descs.name = "リトライHUD";
		hud_object_desc.retry_hud = std::make_shared<retry_hud_controller_object>(retry_object_descs);
		register_object(hud_object_desc.retry_hud);
	}

	hud_object_desc.name = "HUD コントローラー";
	auto hud_controller = std::make_shared<hud_controller_object>(hud_object_desc);
	register_object(hud_controller);

	//	リザルト用フェードコントローラー生成
	//	そのまま背景としても利用する
	{
		result_controller_object::object_descs result_object_descs;

		result_object_descs.back_image = std::make_shared<ui_image>("リザルト 背景用オブジェクト");
		result_object_descs.back_image->set_texture("Assets/Resources/texture/result_back.png");
		result_object_descs.back_image->set_screen_rect({ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT });
		result_object_descs.back_image->set_sort_order(100000);
		register_object(result_object_descs.back_image);

		result_object_descs.front_image = std::make_shared<ui_image>("リザルト 文字用オブジェクト");
		result_object_descs.front_image->set_texture("Assets/Resources/texture/thunk_you_for_playing.png");
		result_object_descs.front_image->set_screen_rect({ 213, 154, 1225, 396 });
		result_object_descs.front_image->set_sort_order(100100);
		register_object(result_object_descs.front_image);

		result_object_descs.result_text = std::make_shared<ui_text>("リザルト:テキスト", font);
		result_object_descs.result_text->set_screen_rect({ 568, 600, 64, 64 });
		result_object_descs.result_text->set_sort_order(100200);
		result_object_descs.result_text->set_texts("RESULT TIME");
		register_object(result_object_descs.result_text);

		result_object_descs.time_text = std::make_shared<ui_text>("リザルト:時間テキスト", font);
		result_object_descs.time_text->set_screen_rect({ 920, 600, 64, 64 });
		result_object_descs.time_text->set_sort_order(100200);
		result_object_descs.time_text->set_texts("00:00:00");
		register_object(result_object_descs.time_text);

		result_object_descs.pless_click_text = std::make_shared<ui_text>("リザルト:クリックテキスト", font);
		result_object_descs.pless_click_text->set_screen_rect({ 690, 680, 64, 64 });
		result_object_descs.pless_click_text->set_sort_order(100200);
		result_object_descs.pless_click_text->set_color({ 1, 1, 0, 1 });
		result_object_descs.pless_click_text->set_texts("PLEASE MOUSE CLICK");
		register_object(result_object_descs.pless_click_text);

		std::shared_ptr<result_controller_object> fade_controller;
		fade_controller = std::make_shared<result_controller_object>("リザルト フェードイン/アウト管理オブジェクト", result_object_descs);

		register_object(fade_controller);

		result_hud_controller::create_instance();
		result_hud_controller::instance()->set_fade_object(fade_controller);
		fade_controller->play(fade::directions::in, 0.0f);
	}

	return true;
}

bool ui_scene::uninitialize_core()
{
	result_hud_controller::delete_instance();
	return true;
}

void ui_scene::update_core(float elapsed_time)
{
}

#if defined(USE_IMGUI)
void ui_scene::imgui_render_core()
{
	if (ImGui::TreeNode("ui_scene"))
	{
		ImGui::TreePop();
	}
}
#endif	//	defined(USE_IMGUI)
