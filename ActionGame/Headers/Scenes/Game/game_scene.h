#pragma once

#include "scenes/scene_base.h"

#include "model_instance.h"

#include "scenes/stage/stage_01_scene.h"
#include "scenes/game/ui_scene.h"
#include "scenes/game/stationed_scene.h"

class game_scene : public scene_base
{
	SCENE_ADD_CHILDREN_DECRLEAR(game_scene, scene_base, stationed_scene, stage_01_scene, ui_scene);

private:
	enum class states
	{
		start_oneshot,
		start_wait,
		fade_in_wait,
		entry_camera_wait,
		state_wait,

		dead_fade_start,
		dead_fade_complete,

		clear_performance_start,
		clear_performance_wait,

		result_fade_out_wait,
		result_wait,
		game_end,
	};

public:
	game_scene();
	game_scene(const game_scene&) = delete;
	game_scene(game_scene&&) = delete;
	~game_scene() override;

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
	float game_timer;
	texture_ptr slashed_texture;
};
