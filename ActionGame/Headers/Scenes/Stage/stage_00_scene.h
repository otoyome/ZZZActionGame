#pragma once

#include "Scenes/scene_base.h"

#include "model_instance.h"

//	ステージ00シーン
class stage_00_scene : public scene_base
{
	SCENE_DECRLEAR(stage_00_scene, scene_base);

public:
	stage_00_scene();
	stage_00_scene(const stage_00_scene&) = delete;
	stage_00_scene(stage_00_scene&&) = delete;
	~stage_00_scene() override;

	bool initialize_core() override;
	bool uninitialize_core() override;
	void update_core(float elapsed_time) override;
#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

public:
	void generate_model_collision(model_resource_ptr resource, DirectX::XMMATRIX& transform, UINT separate_level);
};
