#pragma once

#include "scene_base.h"

#include "model_instance.h"
#include "animation_resource.h"

#include "scenes/stage/stage_00_scene.h"
#include "scenes/game/stationed_scene.h"

#include "object/cinematics/cinematics_object.h"

#include "object/ui/Common/transition_fade_controller.h"

class title_scene : public scene_base
{
	SCENE_ADD_CHILDREN_DECRLEAR(title_scene, scene_base, stationed_scene, stage_00_scene);

private:
	enum class states
	{
		start_wait,
		fade_in_wait,
		input_wait,
		fade_out_wait,
	};

public:
	title_scene();
	title_scene(const title_scene&) = delete;
	title_scene(title_scene&&) = delete;
	~title_scene() override;

protected:
	bool initialize_core() override;
	bool uninitialize_core() override;
	void update_core(float elapsed_time) override;
#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

private:
	states state;
	float work_timer;
	std::shared_ptr<camera_object> main_camera;
	std::shared_ptr<cinematics_object> main_cinematics;
	std::shared_ptr<fade_controller_object> fade_controller;

	std::shared_ptr<animation_resource> camera_animation;
	float camera_animation_timer = 0;
};
