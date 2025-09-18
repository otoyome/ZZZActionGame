#include "scenes/title_scene.h"

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

#include "object/connector_object.h"

#include "scenes/game/game_scene.h"

#include "object/ui/ui_image.h"
#include "object/ui/ui_text.h"

namespace
{
	constexpr std::string_view camera_animation_file_path = "Assets/Resources/camera_animations/title_camera.ema";
	constexpr std::string_view event_function_fade_in = "Fade In";
	constexpr std::string_view event_function_fade_out = "Fade Out";
}

title_scene::title_scene()
: scene_base()
{
}

title_scene::~title_scene()
{
}

bool title_scene::initialize_core()
{
	main_camera = object_manager::instance()->search_object<camera_object>("メインカメラ");
	main_cinematics = object_manager::instance()->search_object<cinematics_object>("演出管理オブジェクト");

	//	フェードイン・アウト用オブジェクト生成
	{
		std::shared_ptr<ui_image> fade_in_out;
		fade_in_out = std::make_shared<ui_image>("フェードイン/アウト用オブジェクト");
		fade_in_out->set_sort_order(INT_MIN);
		register_object(fade_in_out);

		fade_controller = std::make_shared<fade_controller_object>("フェードイン/アウト管理オブジェクト", fade_in_out);
		register_object(fade_controller);

		//	真っ黒から始まる
		transition_fade_controller::instance()->play(fade::directions::out, 0.0f, fade::modes::black);
	}

	//	カメラアニメーション準備
	{
		//	変換済みファイルがあるか？
		std::filesystem::path path(camera_animation_file_path);
#if defined(_DEBUG)
		if (!std::filesystem::exists(path))
		{
			//	無いので変換
			path.replace_extension("vmd");
			{
				std::string directory = path.parent_path().string();

				std::ifstream	read_file;
				read_file.open(path, std::ios::in | std::ios::binary);
				_ASSERT_EXPR_A(read_file, L"Failed to load file");

				std::string	file_buffer;
				file_buffer = std::string(std::istreambuf_iterator<char>(read_file), std::istreambuf_iterator<char>());

				vmd_loader loader;
				bool succeeded = loader.load(file_buffer.data(), static_cast<unsigned int>(file_buffer.size()));
				_ASSERT_EXPR_A(succeeded, L"Failed to load file");
				if (succeeded)
				{
					std::shared_ptr<animation_resource> resource = std::make_shared<animation_resource>();
					if (resource->convert_from_vmd("TitleCamera", loader))
					{
						animation::event event;
						event.time = 0.0f / 30.0f;
						event.type = animation::event::variable_type::Float;
						event.function_name = std::string(event_function_fade_in);
						event.variable = std::to_string(system_units::convert_fps_to_sec(120.0f));
						resource->get_animations_for_debug()[0].events.push_back(event);

						event.time = 460.0f / 30.0f;
						event.type = animation::event::variable_type::Float;
						event.function_name = "Fade Out";
						event.function_name = event_function_fade_out;
						event.variable = std::to_string(system_units::convert_fps_to_sec(120.0f));
						resource->get_animations_for_debug()[0].events.push_back(event);

						resource->serialize(camera_animation_file_path);
					}
				}
			}
			path.replace_extension("ema");
		}
#endif	//	defined(_DEBUG)

		camera_animation = std::make_shared<animation_resource>();
		camera_animation->deserialize(path.string());
	}

	//	リザルト用フェードコントローラー生成
	//	そのまま背景としても利用する
	{
		font_data_ptr font;
		font_data_manager::instance()->load_from_file(font, "Assets/Resources/fonts/MS Gothic/MS Gothic.fnt");
		auto title_back = std::make_shared<ui_image>("タイトル背景");
		title_back->set_screen_rect({ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT });
		title_back->set_color({ 0, 0, 0, 0.75f });
		title_back->set_sort_order(0);
		register_object(title_back);

		auto title_image = std::make_shared<ui_image>("タイトルロゴ");
		title_image->set_texture("Assets/Resources/texture/title.png");
		title_image->set_screen_rect({ 200, 50, 1200.000, 700 });
		title_image->set_sort_order(10);
		register_object(title_image);

		auto title_click_text = std::make_shared<ui_text>("タイトル:クリックテキスト", font);
		title_click_text->set_screen_rect({ 690, 768.000, 64, 64 });
		title_click_text->set_sort_order(20);
		title_click_text->set_color({ 1, 1, 0, 1 });
		title_click_text->set_texts("PLEASE MOUSE CLICK");
		register_object(title_click_text);
	}

	state = states::start_wait;
	work_timer = 0;
	return true;
}

bool title_scene::uninitialize_core()
{
	fade_controller.reset();
	return true;
}

void title_scene::update_core(float elapsed_time)
{
	auto keyboard_state = keyboard::instance()->get_state();
	auto mouse_state = mouse::instance()->get_state();
	switch (state)
	{
		case states::start_wait:
		{
			++work_timer;
			if (work_timer >= 10.0f)
			{
				//	カーソル関係の非表示
				mouse::instance()->set_center_lock(false);
				mouse::instance()->set_cursol_hide(false);

				transition_fade_controller::instance()->play(fade::directions::in, 1.0f, fade::modes::black);
				work_timer = 0.0f;
				state = states::fade_in_wait;
			}
			break;
		}
		case states::fade_in_wait:
		{
			if (!transition_fade_controller::instance()->is_processing())
			{
				work_timer = 0.0f;
				state = states::input_wait;
			}
			break;
		}
		case states::input_wait:
		{
			if (mouse_state.is_key_state(mouse::key_states::left) == input::state::down)
			{
				transition_fade_controller::instance()->play(fade::directions::out, 1.0f, fade::modes::black);
				state = states::fade_out_wait;
			}
			break;
		}
		case states::fade_out_wait:
		{
			if (!transition_fade_controller::instance()->is_processing())
			{
				scene_manager::instance()->remove_scene<stage_00_scene>();
				scene_manager::instance()->remove_scene<title_scene>();
				scene_manager::instance()->add_scene<game_scene>();
			}
			break;
		}
	}

	//	カメラの処理
	if (camera_animation)
	{
		auto animations = camera_animation->get_animation();
		if (!animations.empty())
		{
			animation::camera_spherical animation;
			float previous_camera_animation_timer = camera_animation_timer;
			camera_animation_timer = fmodf(camera_animation_timer + elapsed_time, animations[0].end_time);

			if (animations[0].compute_animation(animation, camera_animation_timer))
			{
				math::rotator rotation = animation.rotation;
				DirectX::XMFLOAT3 vector = rotation.get_front();
				DirectX::XMFLOAT3 focus = animation.focus;
				focus.x -= vector.x * animation.distance;
				focus.y -= vector.y * animation.distance;
				focus.z -= vector.z * animation.distance;
				main_camera->set_position(focus);
				main_camera->set_rotation(rotation);
			}

			auto func = [&](const animation::event& event)
			{
				if (fade_controller)
				{
					if (event.function_name == event_function_fade_in)
					{
						float variable;
						event.get_variable(variable);
						fade_controller->play(fade::directions::in, variable, fade::modes::black);
					}
					if (event.function_name == event_function_fade_out)
					{
						float variable;
						event.get_variable(variable);
						fade_controller->play(fade::directions::out, variable, fade::modes::black);
					}
				}
			};
			animations[0].call_events({ previous_camera_animation_timer, camera_animation_timer }, func);
		}
	}
}

#if defined(USE_IMGUI)
void title_scene::imgui_render_core()
{
	if (ImGui::TreeNode("title_scene"))
	{
		ImGui::TreePop();
	}
}
#endif	//	defined(USE_IMGUI)
