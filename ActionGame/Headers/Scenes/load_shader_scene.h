#pragma once

#include "scene_base.h"

//	シェーダーコンパイル/ロードシーン
//	何かしら表示すればよいかな？
class load_shader_scene : public scene_base
{
	SCENE_DECRLEAR(load_shader_scene, scene_base);

public:
	load_shader_scene();
	load_shader_scene(const load_shader_scene&) = delete;
	load_shader_scene(load_shader_scene&&) = delete;
	~load_shader_scene() override;

protected:
	bool initialize_core() override;
	bool uninitialize_core() override;
	void update_core(float elapsed_time) override;
#ifdef USE_IMGUI
	virtual void imgui_render_core() override;
#endif
};
