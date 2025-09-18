#include "framework.h"

#if defined(USE_IMGUI)
#include "Scenes/title_scene.h"
#include "Scenes/game/game_scene.h"
#include "scenes/editscene/edit_model_scene.h"

template<typename Type>
void change_scene()
{
	if (ImGui::Button(Type::type_name().data()))
	{
		scene_manager::instance()->change_scene_for_imgui<Type>();
	}
}

void framework::imgui_scene_change()
{
	if (ImGui::Begin("Chane Change Window"))
	{
		change_scene<title_scene>();
		change_scene<game_scene>();
		change_scene<edit_model_scene>();
	}
	ImGui::End();
}

#endif // defined(USE_IMGUI)
