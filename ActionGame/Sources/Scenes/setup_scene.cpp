#include "scenes/setup_scene.h"

#include "editor/editor_camera.h"

#include "model_instance.h"
#include "camera.h"

#include "object/light_object.h"
#include "object/ui/hud/hud_controller_object.h"
#include "object/cinematics/cinematics_object.h"

#include "Scenes/title_scene.h"
#include "Scenes/game/game_scene.h"

setup_scene::setup_scene()
: scene_base()
{
}

setup_scene::~setup_scene()
{
}

bool setup_scene::initialize_core()
{
	//	他のシーンをセットアップするだけのシーン
	return true;
}

bool setup_scene::uninitialize_core()
{
	return true;
}

void setup_scene::update_core(float elapsed_time)
{
	scene_manager::instance()->remove_scene<setup_scene>();
#if defined(_DEBUG)
//	scene_manager::instance()->add_scene<title_scene>();
	scene_manager::instance()->add_scene<game_scene>();
#else // defined(_DEBUG)
	scene_manager::instance()->add_scene<title_scene>();
#endif // defined(_DEBUG)
}

#if defined(USE_IMGUI)
void setup_scene::imgui_render_core()
{
	if (ImGui::TreeNode("setup_scene"))
	{
		ImGui::TreePop();
	}
}
#endif	//	defined(USE_IMGUI)
