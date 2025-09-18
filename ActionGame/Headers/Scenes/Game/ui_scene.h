#pragma once

#include "scenes/scene_base.h"

class ui_scene : public scene_base
{
	SCENE_DECRLEAR(ui_scene, scene_base);

public:
	ui_scene();
	ui_scene(const ui_scene&) = delete;
	ui_scene(ui_scene&&) = delete;
	~ui_scene() override;

protected:
	bool initialize_core() override;
	bool uninitialize_core() override;
	void update_core(float elapsed_time) override;
#ifdef USE_IMGUI
	void imgui_render_core() override;
#endif
};
