#include "Scenes/scene_base.h"

scene_base::scene_base()
{
}

scene_base::~scene_base()
{
}

bool scene_base::initialize()
{
	return initialize_core();
}

bool scene_base::uninitialize()
{
	for (auto& object : game_objects)
	{
		if (auto obj = object.lock())
			object_manager::instance()->unregister_object(obj);
	}
	for (auto& object : collision_objects)
	{
		if (auto obj = object.lock())
			object_manager::instance()->unregister_object(obj);
	}
	return uninitialize_core();
}

void scene_base::update(float elapsed_time)
{
	update_core(elapsed_time);
}

#ifdef USE_IMGUI
void scene_base::imgui_render()
{
	imgui_render_core();
}
#endif

void scene_base::register_object(game_object_ptr object)
{
	game_objects.emplace_back(object);
	object_manager::instance()->register_object(object);
}

void scene_base::register_object(collision_object_ptr object)
{
	collision_objects.emplace_back(object);
	object_manager::instance()->register_object(object);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
scene_manager::scene_manager()
{
}

scene_manager::~scene_manager()
{
	remove_all_scene();
}

void scene_manager::updates(float elapsed_time)
{
	for (auto& remove : remove_scenes)
	{
		auto scene = scenes.find(remove);
		scene->second->uninitialize();
		scenes.erase(scene);
	}

	if (!add_scenes.empty())
	{
		std::sort(add_scenes.begin(), add_scenes.end(), [&](add_scene_data& l, add_scene_data& r){return l.order < r.order;});
		for (auto& add_scene : add_scenes)
		{
			add_scene.func();
		}
	}
	remove_scenes.clear();
	add_scenes.clear();

	for (auto& scene : scenes)
	{
		scene.second->update(elapsed_time);
	}
}

void scene_manager::remove_all_scene()
{
	//	‘S‚Ä‚ÌƒV[ƒ“‚ð”jŠü
	for (auto& scene : scenes)
	{
		scene.second->uninitialize();
	}
	scenes.clear();
}

#if defined(USE_IMGUI)
void scene_manager::imgui_render()
{
	if (ImGui::Begin("scenes"))
	{
		for (auto& scene : scenes)
		{
			scene.second->imgui_render();
		}
	}
	ImGui::End();
}
#endif	//	defined(USE_IMGUI)