#include "model_instance.h"
#include "texture.h"

#if defined(_DEBUG)
#include "Graphics/debug_primitive_collector.h"
#endif	//	defined(_DEBUG)

namespace
{
}

model_instance::model_instance()
: system_flags(0)
, resource()
, materials()
, nodes()
{
	set_flag(flags::active, true);
	set_flag(flags::color_render, true);
	set_flag(flags::shadow_caster, true);
	set_flag(flags::culling, true);
#if	defined(_DEBUG)
	set_flag(flags::debug_draw_culling_object, true);
#endif	//	defined(_DEBUG)
}

model_instance::model_instance(std::shared_ptr<model_resource> resource)
: model_instance()
{
	this->resource = resource;
	build_model();
}

model_instance::~model_instance()
{
}

bool model_instance::compute_animation_nodes(std::vector<node_animation>& nodes, size_t animation_index, float time) const
{
	if (!resource)
		return false;

	return resource->compute_animation_nodes(nodes, animation_index, time);
}

bool model_instance::compute_animation_node(node_animation& node, size_t animation_index, size_t node_index, float time) const
{
	if (!resource)
		return false;

	return resource->compute_animation_node(node, animation_index, node_index, time);
}

void model_instance::set_node_poses(const std::vector<node_animation>& node_animations)
{
	for (size_t index = 0; index < node_animations.size(); ++index)
	{
		auto& instance_node = nodes[index];
		auto& animation_node = node_animations[index];
		DirectX::XMMATRIX Scale = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&animation_node.scale));
		DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&animation_node.rotation));
		DirectX::XMMATRIX Translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&animation_node.translation));
		DirectX::XMStoreFloat4x4(&instance_node.local_transform, Scale * Rotation * Translation);
	}
}

void model_instance::set_node_pose(size_t node_index, const node_animation& node)
{
	if (node_index >= nodes.size())
	{
		assert(node_index < nodes.size());
		return;
	}

	auto& instance_node = nodes[node_index];
	DirectX::XMMATRIX Scale = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&node.scale));
	DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation));
	DirectX::XMMATRIX Translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&node.translation));
	DirectX::XMStoreFloat4x4(&instance_node.local_transform, Scale * Rotation * Translation);
}

void model_instance::update_transform(const DirectX::XMFLOAT4X4& world_transform, float random_offset_power)
{
	DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&world_transform);
	for (auto& node : nodes)
	{
		DirectX::XMMATRIX GlobalTransform = DirectX::XMLoadFloat4x4(&node.local_transform);
		if (node.parent_index != invalid_id)
		{
			auto& parent_node = nodes[node.parent_index];
			GlobalTransform *= DirectX::XMLoadFloat4x4(&parent_node.global_transform);
		}
		if (random_offset_power != 0.0f)
		{
			DirectX::XMVECTOR RandomOffset;
			RandomOffset = DirectX::XMVectorSet((rand() % 3 - 1) * random_offset_power,
												(rand() % 3 - 1) * random_offset_power,
												(rand() % 3 - 1) * random_offset_power,
												0.0f);
			GlobalTransform.r[3] = DirectX::XMVectorAdd(GlobalTransform.r[3], RandomOffset);
		}
		DirectX::XMStoreFloat4x4(&node.global_transform, GlobalTransform);
		DirectX::XMStoreFloat4x4(&node.world_transform, GlobalTransform * WorldTransform);
	}
}

void model_instance::update_transform(const DirectX::XMMATRIX& world_transform, float random_offset_power)
{
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, world_transform);
	update_transform(matrix, random_offset_power);
}

void model_instance::build_model()
{
	if(!resource)
		return;

	//	マテリアルをコピーしておく
	materials.reserve(resource->get_materials().size());
	for (auto& resource_material : resource->get_materials())
	{
		material instance_material;
		resource_material.duplicate(instance_material);
		materials.emplace_back(instance_material);
	}

	//	ノード構築
	nodes.reserve(resource->get_nodes().size());
	for (auto& resource_node : resource->get_nodes())
	{
		node instance_node;
		instance_node.name = resource_node.name;
		instance_node.parent_index = resource_node.parent_index;
		DirectX::XMStoreFloat4x4(&instance_node.local_transform,
								DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&resource_node.scale)) *
								DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&resource_node.rotation)) *
								DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&resource_node.translation))
		);
		instance_node.global_transform = instance_node.local_transform;
		instance_node.world_transform = instance_node.local_transform;
		nodes.emplace_back(instance_node);
	}
}

size_t model_instance::get_node_index(std::string_view node_name) const
{
	for (size_t index = 0; index < nodes.size(); ++index)
	{
		auto& node = nodes[index];
		if (node.name == node_name)
			return index;
	}

	return model_resource::invalid_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
model_instance_maanger::model_instance_maanger()
{
}

model_instance_maanger::~model_instance_maanger()
{
}

void model_instance_maanger::registered(std::weak_ptr<model_instance> instance)
{
	//	登録済みであれば登録しない
	auto it  = std::find_if(instances.begin(),
							instances.end(),
							[instance](std::weak_ptr<model_instance> value)
							{
								return system_utility::equals(value, instance);
							});
	if (it != instances.end())
		return;

	instances.emplace_back(instance);
}

void model_instance_maanger::get_container(mesh_container& container, const collision::frustum_culling& frustum_culling)
{
	container.clear();

#if defined(_DEBUG)
	max_draw_mesh_count = draw_mesh_count = 0;
	draw_shadow_mesh_count = 0;
#endif	//	defined(_DEBUG)

	auto it = instances.begin();
	for (; it != instances.end();)
	{
		if (it->expired())
		{
			it = instances.erase(it);
			continue;
		}

		auto instance = it->lock();
		++it;	//	次のインスタンスに移動させておく

		if (!instance->is_flag(model_instance::flags::active))
			continue;

		if (!instance->is_flag(model_instance::flags::color_render) && !instance->is_flag(model_instance::flags::shadow_caster))
			continue;

		//	見えているか確認
		auto check_insight = [&](const model_resource::mesh& mesh)
		{
			DirectX::XMFLOAT3 scale((mesh.bounds_max.x - mesh.bounds_min.x) * 0.5f, (mesh.bounds_max.y - mesh.bounds_min.y) * 0.5f, (mesh.bounds_max.z - mesh.bounds_min.z) * 0.5f);
			DirectX::XMFLOAT3 center(mesh.bounds_min.x + scale.x, mesh.bounds_min.y + scale.y, mesh.bounds_min.z + scale.z);
			DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
			DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(center.x, center.y, center.z);

			DirectX::XMFLOAT4X4 node_world_transform = instance->get_nodes()[mesh.node_index].world_transform;
			DirectX::XMVECTOR Scale, Rotation, Translation;
			DirectX::XMMatrixDecompose(&Scale, &Rotation, &Translation, S * T * DirectX::XMLoadFloat4x4(&node_world_transform));
			DirectX::XMStoreFloat3(&scale, Scale);
			DirectX::XMFLOAT4 rotation;
			DirectX::XMStoreFloat4(&rotation, Rotation);
			DirectX::XMStoreFloat3(&center, Translation);
			return frustum_culling.to_box(collision::bounding_box(scale, rotation, center));
		};

#if defined(_DEBUG)
		//	描画メッシュ数
		max_draw_mesh_count += instance->get_resource()->get_meshes().size();
#endif	//	defined(_DEBUG)

		//	インスタンス単位でカリング
		bool in_sight = true;
		if (instance->is_flag(model_instance::flags::culling) && !instance->is_flag(model_instance::flags::culling_target_mesh))
		{
			for (auto& mesh : instance->get_resource()->get_meshes())
			{
				if (mesh.material_index == model_resource::invalid_id)
					continue;

				//	カリングしてどれか一つでも見えていたらその時点で終了
				in_sight = check_insight(mesh);
				if (in_sight)
					break;
			}

			//	見えていない、かつ影を生成しないモデルの場合はこの時点で除外
			if (!in_sight && !instance->is_flag(model_instance::flags::shadow_caster))
				continue;
		}

		//	描画情報構築
		for (auto& mesh : instance->get_resource()->get_meshes())
		{
			if (mesh.material_index == model_resource::invalid_id)
				continue;

			//	メッシュ単位でカリング
			if (instance->is_flag(model_instance::flags::culling) && instance->is_flag(model_instance::flags::culling_target_mesh))
			{
				in_sight = check_insight(mesh);

				//	見えていない、かつ影を生成しないモデルの場合はこの時点で除外
				if(!in_sight && !instance->is_flag(model_instance::flags::shadow_caster))
					continue;
			}

			DirectX::XMFLOAT3 bounds_min, bounds_max;
			{
				DirectX::XMFLOAT3 scale((mesh.bounds_max.x - mesh.bounds_min.x) * 0.5f, (mesh.bounds_max.y - mesh.bounds_min.y) * 0.5f, (mesh.bounds_max.z - mesh.bounds_min.z) * 0.5f);
				DirectX::XMFLOAT3 center(mesh.bounds_min.x + scale.x, mesh.bounds_min.y + scale.y, mesh.bounds_min.z + scale.z);
				DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
				DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(center.x, center.y, center.z);

				DirectX::XMFLOAT4X4 node_world_transform = instance->get_nodes()[mesh.node_index].world_transform;
				DirectX::XMVECTOR Scale, Rotation, Translation;
				DirectX::XMMatrixDecompose(&Scale, &Rotation, &Translation, S * T * DirectX::XMLoadFloat4x4(&node_world_transform));
				DirectX::XMStoreFloat3(&scale, Scale);
				DirectX::XMFLOAT4 rotation;
				DirectX::XMStoreFloat4(&rotation, Rotation);
				DirectX::XMStoreFloat3(&center, Translation);
				collision::bounding_box box = collision::bounding_box(scale, rotation, center);
				box.include_corner(bounds_min, bounds_max);
			}

#if defined(_DEBUG)
			//	カリングオブジェクトの表示フラグ
			if (draw_culling_mesh_aabb && instance->is_flag(model_instance::flags::debug_draw_culling_object))
			{
				DirectX::XMFLOAT3 scale((mesh.bounds_max.x - mesh.bounds_min.x) * 0.5f, (mesh.bounds_max.y - mesh.bounds_min.y) * 0.5f, (mesh.bounds_max.z - mesh.bounds_min.z) * 0.5f);
				DirectX::XMFLOAT3 center(mesh.bounds_min.x + scale.x, mesh.bounds_min.y + scale.y, mesh.bounds_min.z + scale.z);
				DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
				DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(center.x, center.y, center.z);

				DirectX::XMFLOAT4X4 node_world_transform = instance->get_nodes()[mesh.node_index].world_transform;
				DirectX::XMVECTOR Scale, Rotation, Translation;
				DirectX::XMMatrixDecompose(&Scale, &Rotation, &Translation, S * T * DirectX::XMLoadFloat4x4(&node_world_transform));
				DirectX::XMStoreFloat3(&scale, Scale);
				DirectX::XMFLOAT4 rotation;
				DirectX::XMStoreFloat4(&rotation, Rotation);
				DirectX::XMStoreFloat3(&center, Translation);
				DirectX::XMFLOAT4 color = {1.0f, 0.0f, 1.0f, 1.0f};
				PUSH_DEBUG_PRIMITIVE_BOX(scale, rotation, center, color);
			}
#endif	//	defined(_DEBUG)

			//	バッファ情報構築
			container.buffer_datas.emplace_back();
			auto& buffer_data = container.buffer_datas.back();
			buffer_data.vertex_buffer_views = &mesh.vertex_buffer_views;
			buffer_data.index_buffer_view = &mesh.index_buffer_view;
			buffer_data.transform_matrices.reserve(instance->get_nodes().size());
			buffer_data.bounds_min = bounds_min;	//	mesh.bounds_min;
			buffer_data.bounds_max = bounds_max;	//	mesh.bounds_max;
			for (auto& bone : mesh.bones)
			{
				DirectX::XMMATRIX node_world_transform = DirectX::XMLoadFloat4x4(&instance->get_nodes()[bone.node_index].world_transform);
				DirectX::XMMATRIX node_offset_transform = DirectX::XMLoadFloat4x4(&bone.offset_matrix);
				buffer_data.transform_matrices.emplace_back();
				DirectX::XMStoreFloat4x4(&buffer_data.transform_matrices.back(), node_offset_transform * node_world_transform);
			}

			//	マテリアル情報構築
			container.material_datas.emplace_back();
			auto& material_data = container.material_datas.back();
			material_data.material = instance->get_materials().data() + mesh.material_index;

			//	描画情報構築
			container.draw_datas.emplace_back();
			auto& draw_data = container.draw_datas.back();
			draw_data.buffer_index = container.buffer_datas.size() - 1;
			draw_data.material_index = container.material_datas.size() - 1;

			//	見えている場合でかつ色描画するなら登録
			if(in_sight && instance->is_flag(model_instance::flags::color_render))
			{
				container.color_datas.emplace_back(container.draw_datas.size() - 1);

#if defined(_DEBUG)
				++draw_mesh_count;
#endif	//	defined(_DEBUG)
			}

			//	影描画するなら登録
			if (instance->is_flag(model_instance::flags::shadow_caster))
			{
				container.shadow_datas.emplace_back(container.draw_datas.size() - 1);

#if defined(_DEBUG)
				++draw_shadow_mesh_count;
#endif	//	defined(_DEBUG)
				//if (instance->get_resource()->get_capsule_shadows().empty())
				//{
				//	//	カプセルシャドウオブジェクトがなければ通常シャドウ
				//	container.shadow_datas.emplace_back(container.shadow_datas.size() - 1);
				//}
				//else if (!in_sight)	//	カプセルシャドウは、メッシュが見えている場合のみ登録
				//{
				//	for (auto& capsule : instance->get_resource()->get_capsule_shadows())
				//	{
				//		DirectX::XMFLOAT4X4 node_world_transform = instance->get_nodes()[capsule.node_index].world_transform;
				//		capsule.capsule.
				//		container.capsule_shadow_datas.emplace_back();
				//	}
				//}
			}
		}
	}
}
