#pragma once

#include "collision/collision.h"
#include "Object/light_object.h"
#include "Object/Character/character_object.h"

//	影ライトとモデルのコネクターオブジェクト
class shadow_light_to_model_connector_object : public game_object
{
	GAME_OBJECT_DECRLEAR(shadow_light_to_model_connector_object, game_object)

public:
	struct setup_datas
	{
		std::shared_ptr<shadow_light_object> shadow_light;
		std::shared_ptr<model_object> main_model;
		std::vector<std::shared_ptr<model_object>> sub_models;
		common_pipeline_states::shadow_category shadow_category;
	};
	shadow_light_to_model_connector_object(std::string_view name, setup_datas setup_data);
	~shadow_light_to_model_connector_object() override;

	void pre_render() override;

protected:
	setup_datas setup_data;
};
