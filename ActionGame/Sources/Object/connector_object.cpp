#include "Object/connector_object.h"

#include "Object/light_object.h"

shadow_light_to_model_connector_object::shadow_light_to_model_connector_object(std::string_view name, setup_datas setup_data)
: game_object(name)
, setup_data(setup_data)
{
}

shadow_light_to_model_connector_object::~shadow_light_to_model_connector_object()
{
}


void shadow_light_to_model_connector_object::pre_render()
{
	//	平行光源の情報を取得してくる
	math::rotator light_rotation;
	std::shared_ptr<directional_light_object> directional_light;
	for (auto light : object_manager::instance()->search_objects<directional_light_object>())
	{
		if (!light->is_active())
			continue;

		light_rotation = light->get_rotation();
		break;
	}

	//	メインモデルを注視点とする
	DirectX::XMFLOAT3 light_position = {};
	if (setup_data.main_model)
	{
		light_position = setup_data.main_model->get_position();
	}

	//	影ライトの姿勢更新
	if (setup_data.shadow_light)
	{
		DirectX::XMFLOAT3 light_direction = light_rotation.get_front();
		setup_data.shadow_light->set_range(system_units::convert_m_to_unit(20.0f));
		setup_data.shadow_light->get_shadow_light()->set_shadow_bias(0.00000001f);
		light_position.x = light_position.x - light_direction.x * setup_data.shadow_light->get_range() * 0.5f;
		light_position.y = light_position.y - light_direction.y * setup_data.shadow_light->get_range() * 0.5f;
		light_position.z = light_position.z - light_direction.z * setup_data.shadow_light->get_range() * 0.5f;
		setup_data.shadow_light->set_position(light_position);
		setup_data.shadow_light->set_rotation(light_rotation);
		setup_data.shadow_light->get_shadow_light()->set_shadow_flag(setup_data.shadow_category, true);

		//	マテリアルにコネクターによるマスク値を設定
		if (setup_data.main_model)
		{
			for (auto& material : setup_data.main_model->get_model_instance()->get_materials())
			{
				material.set_uint("shadow_category", static_cast<UINT>(setup_data.shadow_category));
			}
		}
		//	サブモデル側に設定
		for (auto& model : setup_data.sub_models)
		{
			for (auto& material : model->get_model_instance()->get_materials())
			{
				material.set_uint("shadow_category", static_cast<UINT>(setup_data.shadow_category));
			}
		}
	}
}
