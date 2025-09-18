#include "scenes/game/stationed_scene.h"

#include "editor/editor_camera.h"

#include "model_instance.h"
#include "camera.h"

#include "object/light_object.h"
#include "object/cinematics/cinematics_object.h"

#include "object/ui/hud/hud_controller_object.h"
#include "object/ui/common/transition_fade_controller.h"

#if defined(_DEBUG)
#include "Object/Debug/debug_camera.h"
#include "Object/Debug/debug_menu.h"
#endif // defined(_DEBUG)

stationed_scene::stationed_scene()
: scene_base()
{
}

stationed_scene::~stationed_scene()
{
}

bool stationed_scene::initialize_core()
{
	//	メインカメラ生成
	std::shared_ptr<camera_object> main_camera;
	{
		main_camera = std::make_shared<camera_object>("メインカメラ");
		register_object(main_camera);
	}

	//	演出オブジェクト生成
	std::shared_ptr<cinematics_object> main_cinematics;
	{
		main_cinematics = std::make_shared<cinematics_object>("演出管理オブジェクト", main_camera);
		register_object(main_cinematics);
	}

	//	フェードイン・アウト用オブジェクト生成
	std::shared_ptr<fade_controller_object> fade_controller;
	{
		std::shared_ptr<ui_image> fade_in_out;
		fade_in_out = std::make_shared<ui_image>("フェードイン/アウト用オブジェクト");
		fade_in_out->set_sort_order(0x00FFFFFF);
		register_object(fade_in_out);

		fade_controller = std::make_shared<fade_controller_object>("フェードイン/アウト管理オブジェクト", fade_in_out);
		register_object(fade_controller);

		transition_fade_controller::create_instance();
		transition_fade_controller::instance()->set_fade_object(fade_controller);

		//	真っ黒から始まる
		transition_fade_controller::instance()->play(fade::directions::out, 0.0f, fade::modes::black);
	}

#if defined(_DEBUG)
	//	デバッグカメラ生成
	{
		auto camera = std::make_shared<debug_camera>(main_camera);
		register_object(camera);
		register_object(std::make_shared<debug_menu>(camera));
	}
#endif	//	defined(_DEBUG)

	return true;
}

bool stationed_scene::uninitialize_core()
{
	transition_fade_controller::delete_instance();
	return true;
}

void stationed_scene::update_core(float elapsed_time)
{
}

#if defined(USE_IMGUI)
void stationed_scene::imgui_render_core()
{
	if (ImGui::TreeNode("stationed_scene"))
	{
		ImGui::TreePop();
	}
}
#endif	//	defined(USE_IMGUI)
