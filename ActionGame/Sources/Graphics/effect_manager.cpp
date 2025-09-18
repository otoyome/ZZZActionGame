#include "graphics/effect_manager.h"

#include "render_data.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float effect_manager::get_elapsed_time_scale() const
{
	return elapsed_time_scale;
}

void effect_manager::set_elapsed_time_scale(float elapsed_time_scale)
{
	this->elapsed_time_scale = elapsed_time_scale;
}

effect_manager::effect_manager()
: elapsed_time_scale(1.0f)
, effekseer_manager()
{
	//	エフェクトのマネージャーの作成
	effekseer_manager = ::Effekseer::Manager::Create(get_effekseer_instance_max());
}

effect_manager::~effect_manager()
{
}

void effect_manager::updates(float elapsed_time)
{
	float elapsed_frame = system_units::convert_sec_to_fps(elapsed_time);

	Effekseer::Manager::UpdateParameter update_parameter;
	update_parameter.DeltaFrame = elapsed_frame * elapsed_time_scale;
	update_parameter.UpdateInterval = elapsed_time_scale;

	effekseer_manager->Update(update_parameter);
}

bool effect_manager::load_resource(std::shared_ptr<effect_resource>& resource, std::string_view file_path)
{
	resource = std::make_shared<effect_resource>();
	return resource->deserialize(effekseer_manager, file_path);
}