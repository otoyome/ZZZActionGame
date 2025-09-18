#pragma once

#include "scenes/scene_base.h"

class stationed_scene : public scene_base
{
	SCENE_DECRLEAR_AND_SET_ORDER(stationed_scene, -1000, scene_base);

public:
	stationed_scene();
	stationed_scene(const stationed_scene&) = delete;
	stationed_scene(stationed_scene&&) = delete;
	~stationed_scene() override;

protected:
	bool initialize_core() override;
	bool uninitialize_core() override;
	void update_core(float elapsed_time) override;
#ifdef USE_IMGUI
	virtual void imgui_render_core() override;
#endif
};
