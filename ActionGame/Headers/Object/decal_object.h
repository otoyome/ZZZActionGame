#pragma once

#include "camera.h"
#include "object/game_object.h"

#include "PipelineState/deferred_decal_pipeline_state.h"

//	デカール用オブジェクト
class decal_object : public game_object
{
	GAME_OBJECT_DECRLEAR(decal_object, game_object)

public:
	static constexpr std::string_view material_variable_name_base_color = decal_material_parameter_name::base_color;
	static constexpr std::string_view material_variable_name_base_texture = decal_material_parameter_name::base_texture;

public:
	material& get_material(){return this->material;}

	const DirectX::XMFLOAT3& get_scale() const;
	void set_scale(const DirectX::XMFLOAT3& scale);

	const math::rotator& get_rotation() const;
	void set_rotation(const math::rotator& rotator);

	const DirectX::XMFLOAT3& get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);

	decal_datas convert_decal_data() const;

public:
	decal_object(std::string_view name, math::compute_fade::calculator fade_time = math::compute_fade::calculator({ 0.0f, 0.2f }, { 0.8f, 1.0f }));
	~decal_object() override;

	void post_update(float elapsed_time) override;
	void pre_render() override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

private:
	DirectX::XMFLOAT3 scale;
	math::rotator rotation;
	DirectX::XMFLOAT3 position;
	material material;
	float time;
	math::compute_fade::calculator fade_time;
};
