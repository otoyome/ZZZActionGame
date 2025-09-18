#pragma once

#include "scenes/game/stationed_scene.h"
#include "scenes/scene_base.h"

class setup_scene : public scene_base
{
	SCENE_ADD_CHILDREN_DECRLEAR(setup_scene, scene_base, stationed_scene);

public:
	setup_scene();
	setup_scene(const setup_scene&) = delete;
	setup_scene(setup_scene&&) = delete;
	~setup_scene() override;

protected:
	bool initialize_core() override;
	bool uninitialize_core() override;
	void update_core(float elapsed_time) override;
#ifdef USE_IMGUI
	virtual void imgui_render_core() override;
#endif
};
