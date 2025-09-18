#pragma once

#include "object/model_object.h"

#include "collision/mesh.h"

//	ステージオブジェクト
class stage_object : public game_object
{
	GAME_OBJECT_DECRLEAR(stage_object, game_object)

public:
	std::shared_ptr<model_object> get_model_object() const;
	void set_model_object(std::shared_ptr<model_object> model);

	const DirectX::XMFLOAT3& get_scale() const;
	void set_scale(const DirectX::XMFLOAT3& scale);

	const math::rotator& get_rotation() const;
	void set_rotation(const math::rotator& rotator);

	const DirectX::XMFLOAT3& get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);

public:
	stage_object(std::string_view name, std::shared_ptr<model_object> model);
	~stage_object() override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

public:
#if 0
	static constexpr size_t invalid_material_index = -1;
	size_t ray_cast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, collision::intersect_data& intersect_data);
	size_t ray_cast(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& End, collision::intersect_data& intersect_data);
#endif

private:
	void update_transform();

private:
	std::shared_ptr<model_object> model;
};
