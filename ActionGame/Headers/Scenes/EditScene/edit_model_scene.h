#pragma once

#include "scenes/scene_base.h"


#include "model_instance.h"

#include "object/model_object.h"

#if defined(USE_IMGUI)
class edit_model_scene : public scene_base
{
	SCENE_DECRLEAR(edit_model_scene, scene_base);

public:
	edit_model_scene();
	edit_model_scene(const edit_model_scene&) = delete;
	edit_model_scene(edit_model_scene&&) = delete;
	~edit_model_scene() override;

protected:
	bool initialize_core() override;
	bool uninitialize_core() override;
	void update_core(float elapsed_time) override;
#if defined(USE_IMGUI)
	virtual void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

private:
	bool imgui_all_item_changer();

private:
	std::vector<std::shared_ptr<model_object>> edit_models;
	std::shared_ptr<model_object> edit_model;

	char all_variable_name_texts[MAX_PATH];
	int all_edit_material_flags = 0;
	material::parameter::data all_edit_item;
	int all_variable_type = static_cast<int>(material::parameter::type::Unknown);

	char variable_name_texts[MAX_PATH];
	int variable_type = static_cast<int>(material::parameter::type::Unknown);
};
#endif // defined(USE_IMGUI)
