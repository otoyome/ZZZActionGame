#include "object/stage_object.h"

std::shared_ptr<model_object> stage_object::get_model_object() const
{
	return model;
}

void stage_object::set_model_object(std::shared_ptr<model_object> model)
{
	this->model = model;
}

const DirectX::XMFLOAT3& stage_object::get_scale() const
{
	return model->get_scale();
}

void stage_object::set_scale(const DirectX::XMFLOAT3& scale)
{
	model->set_scale(scale);
}

const math::rotator& stage_object::get_rotation() const
{
	return model->get_rotation();
}

void stage_object::set_rotation(const math::rotator& rotator)
{
	model->set_rotation(rotator);
}

const DirectX::XMFLOAT3& stage_object::get_position() const
{
	return model->get_position();
}

void stage_object::set_position(const DirectX::XMFLOAT3& position)
{
	model->set_position(position);
}

stage_object::stage_object(std::string_view name, std::shared_ptr<model_object> model)
: game_object(name)
, model(model)
{
}

stage_object::~stage_object()
{
}

#if defined(USE_IMGUI)
void stage_object::imgui_render_core()
{
}
#endif	//	defined(USE_IMGUI)

#if 0
size_t stage_object::ray_cast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, collision::intersect_data& intersect_data)
{
	return ray_cast(DirectX::XMLoadFloat3(&start), DirectX::XMLoadFloat3(&end), intersect_data);
}

size_t stage_object::ray_cast(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& End, collision::intersect_data& intersect_data)
{
	if (!model || !model->get_model_instance() || !model->get_model_instance()->get_resource())
		return stage_object::invalid_material_index;

	DirectX::XMVECTOR WorldStart = Start;
	DirectX::XMVECTOR WorldEnd = End;
	//	レイキャスト前に姿勢を確定
	update_transform();

	auto& instance = model->get_model_instance();
	auto& resource = instance->get_resource();

	size_t material_index = invalid_material_index;
	for(auto& mesh : resource->get_meshes())
	{
		if (mesh.node_index == model_resource::invalid_id)
			continue;

		auto it = mesh.vertex_buffer_views.find(semantic("POSITION", 0));
		if (it == mesh.vertex_buffer_views.end())
			continue;

		auto& node = instance->get_nodes()[mesh.node_index];
		DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&node.world_transform);
		//	AABBと直線とで交差判定してレイキャストを飛ばせるようにする
		{
			mesh.bounds_min, mesh.bounds_max;
		}

		DirectX::XMMATRIX InverseWorldTransform = DirectX::XMMatrixInverse(nullptr, WorldTransform);
		DirectX::XMVECTOR LocalStart = DirectX::XMVector3TransformCoord(WorldStart, InverseWorldTransform);
		DirectX::XMVECTOR LocalEnd = DirectX::XMVector3TransformCoord(WorldEnd, InverseWorldTransform);
		collision::triangles triangles;
		triangles.positions = reinterpret_cast<const DirectX::XMFLOAT3*>(it->second.row_data.data());
		triangles.indices = reinterpret_cast<const UINT*>(mesh.index_buffer_view.row_data.data());
		triangles.index_count = mesh.index_buffer_view.count();
		triangles.reverse_index = true;
		collision::intersect_data temp_intersect_data;
		if (collision::ray_cast(LocalStart, LocalEnd, triangles, &temp_intersect_data))
		{
			DirectX::XMVECTOR LocalPoint = DirectX::XMLoadFloat3(&temp_intersect_data[0].point);
			DirectX::XMVECTOR WorldPoint = DirectX::XMVector3TransformCoord(LocalPoint, WorldTransform);
			DirectX::XMVECTOR WorldLength = DirectX::XMVector3Length(DirectX::XMVectorSubtract(WorldPoint, WorldStart));

			float distance = DirectX::XMVectorGetX(WorldLength);
			if (intersect_data.size() == 0)
			{
				intersect_data = temp_intersect_data;
				DirectX::XMVECTOR LocalNormal = DirectX::XMLoadFloat3(&temp_intersect_data[0].normal);
				DirectX::XMVECTOR WorldNormal = DirectX::XMVector3TransformNormal(LocalNormal, WorldTransform);
				DirectX::XMStoreFloat3(&intersect_data[0].point, WorldPoint);
				DirectX::XMStoreFloat3(&intersect_data[0].normal, WorldNormal);
				intersect_data[0].length = distance;
				material_index = mesh.material_index;
			}
			else if (distance < intersect_data[0].length)
			{
				DirectX::XMVECTOR LocalNormal = DirectX::XMLoadFloat3(&temp_intersect_data[0].normal);
				DirectX::XMVECTOR WorldNormal = DirectX::XMVector3TransformNormal(LocalNormal, WorldTransform);
				DirectX::XMStoreFloat3(&intersect_data[0].point, WorldPoint);
				DirectX::XMStoreFloat3(&intersect_data[0].normal, WorldNormal);
				intersect_data[0].length = distance;
				material_index = mesh.material_index;
			}
		}
	}

	return material_index;
}
#endif

void stage_object::update_transform()
{
	if (!model || !model->get_model_instance() || !model->get_model_instance()->get_resource())
		return;

	model->update_transform();
}
