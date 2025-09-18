#include "Scenes/load_shader_scene.h"

#include "PipelineState/toon_rendering_pipeline_states.h"
#include "PipelineState/deferred_decal_pipeline_state.h"
#include "PipelineState/ui_rendering_pipeline_states.h"

#include "scenes/editscene/edit_model_scene.h"
#include "scenes/game/ui_scene.h"

#include "Scenes/setup_scene.h"

load_shader_scene::load_shader_scene()
: scene_base()
{
}

load_shader_scene::~load_shader_scene()
{
}

bool load_shader_scene::initialize_core()
{
	pipeline_state_manager* instance = pipeline_state_manager::instance();

	//	モデルシェーダー準備
	{
		CONST char* shader_names[] =
		{
			"toon_depth_write",
			"toon_csm_write",
			"toon_gbuffer",
		};
		instance->register_pipeline_state<toon_depth_write_pipeline_state>(shader_names[0]);
		instance->register_pipeline_state<toon_csm_write_pipeline_state>(shader_names[1]);
		instance->register_pipeline_state<toon_gbuffer_pipeline_state>(shader_names[2]);
	}

	//	デカールシェーダー準備
	{
		CONST char* shader_names[] =
		{
			"deferred_decal_stencil_write",
			"deferred_decal_primitive",
			"deferred_decal_fullscreen",
		};
		instance->register_pipeline_state<deferred_decal_stencil_write_pipeline_state>(shader_names[0]);
		instance->register_pipeline_state<deferred_decal_primitive_pipeline_state>(shader_names[1]);
		instance->register_pipeline_state<deferred_decal_fullscreen_pipeline_state>(shader_names[2]);
	}

	//	UIシェーダー準備
	{
		CONST char* shader_names[] =
		{
			"ui_shader",
		};
		instance->register_pipeline_state<ui_pipeline_state>(shader_names[0]);
	}

	return true;
}

bool load_shader_scene::uninitialize_core()
{
	return true;
}

void load_shader_scene::update_core(float elapsed_time)
{
#if 0
//	scene_manager::instance()->change_scene<edit_model_scene>();
	scene_manager::instance()->change_scene<ui_scene>();
#else
	scene_manager::instance()->change_scene<setup_scene>();
#endif
}

#if defined(USE_IMGUI)
void load_shader_scene::imgui_render_core()
{
	if (ImGui::TreeNode("load_shader_scene"))
	{
		ImGui::TreePop();
	}
}
#endif	//	defined(USE_IMGUI)
