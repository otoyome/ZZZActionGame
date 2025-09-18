#include "Object/model_object.h"

model_instance_ptr model_object::get_model_instance()
{
	update_transform();
	return instance;
}

const DirectX::XMFLOAT3& model_object::get_scale() const
{
	return scale;
}

void model_object::set_scale(const DirectX::XMFLOAT3& scale)
{
	dirty = true;
	this->scale = scale;
}

const math::rotator& model_object::get_rotation() const
{
	return rotator;
}

void model_object::set_rotation(const math::rotator& rotator)
{
	dirty = true;
	this->rotator = rotator;
}

const DirectX::XMFLOAT3& model_object::get_position() const
{
	return position;
}

void model_object::set_position(const DirectX::XMFLOAT3& position)
{
	dirty = true;
	this->position = position;
}

bool model_object::get_apply_root_animation_transform() const
{
	return apply_root_animation_transform;
}

void model_object::set_apply_root_animation_transform(bool flag)
{
	apply_root_animation_transform = flag;
}

model_object::model_object(std::string_view name, std::string_view file_path)
: game_object(name)
{
	//	モデルファイルが存在するかチェック
	std::filesystem::path path(file_path);
	if (std::filesystem::exists(path))
	{
		model_resource_ptr resource;
		if (model_resource_manageer::instance()->load_model(resource, file_path))
		{
			instance = std::make_shared<model_instance>(resource);
			model_instance_maanger::instance()->registered(instance);
		}
	}
}

model_object::~model_object()
{
}

void model_object::fixed_update(float elapsed_time)
{
	auto& animation_data = instance->get_resource()->get_animations()[animation_index];
	//	アニメーションフレーム算出
	float previous_time = animation_time;

	animation_time += elapsed_time * animation_speed;
	float current_time = animation_time;
	if (current_time > animation_data.end_time)
	{
		current_time = animation_loop ? fmodf(current_time, animation_data.end_time) : animation_data.end_time;
	}

	//	アニメーション算出
	instance->compute_animation_nodes(current_node_animations, animation_index, current_time);

	//	ルートモーション
	if (animation_data.root_motion_xz || animation_data.root_motion_y || animation_data.root_rotation)
	{
		size_t node_index = instance->get_node_index(animation_data.root_motion_node_name);
		if (node_index != model_resource::invalid_id)
		{
			//	ワールド行列
			DirectX::XMMATRIX Scale = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&scale));
			DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotator.convert_quaternion()));
			DirectX::XMMATRIX Translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&position));
			DirectX::XMMATRIX WorldTransform = Scale * Rotation * Translation;

			model_instance::node_animation& current_node_animation = current_node_animations[node_index];
			model_instance::node_animation begin_node_animation, previous_node_animation, end_node_animation;
			bool is_end_compite_animation = false;
			instance->compute_animation_node(begin_node_animation, animation_index, node_index, 0.0f);
			instance->compute_animation_node(previous_node_animation, animation_index, node_index, previous_time);

			if (animation_data.root_motion_xz || animation_data.root_motion_y)
			{
				//	ローカル移動値を算出
				DirectX::XMFLOAT3 local_translation;
				if (animation_loop && (previous_time > current_time))
				{
					if (!is_end_compite_animation)
					{
						is_end_compite_animation = true;
						instance->compute_animation_node(end_node_animation, animation_index, node_index, animation_data.end_time);
					}
					local_translation.x = (end_node_animation.translation.x - previous_node_animation.translation.x) + (current_node_animation.translation.x - begin_node_animation.translation.x);
					local_translation.y = (end_node_animation.translation.y - previous_node_animation.translation.y) + (current_node_animation.translation.y - begin_node_animation.translation.y);
					local_translation.z = (end_node_animation.translation.z - previous_node_animation.translation.z) + (current_node_animation.translation.z - begin_node_animation.translation.z);
				}
				else
				{
					local_translation.x = current_node_animation.translation.x - previous_node_animation.translation.x;
					local_translation.y = current_node_animation.translation.y - previous_node_animation.translation.y;
					local_translation.z = current_node_animation.translation.z - previous_node_animation.translation.z;
				}

				//	親のノード空間へ移動
				DirectX::XMMATRIX ParentGlobalTransform;
				if (instance->get_nodes()[node_index].parent_index != model_resource::invalid_id)
				{
					auto& nodes = instance->get_nodes();
					auto& node = nodes[node_index];
					auto& parent_node = nodes[node.parent_index];
					ParentGlobalTransform = DirectX::XMLoadFloat4x4(&parent_node.global_transform);
				}
				else
				{
					ParentGlobalTransform = DirectX::XMMatrixIdentity();
				}
				DirectX::XMVECTOR LocalTranslation = DirectX::XMLoadFloat3(&local_translation);
				DirectX::XMVECTOR GlobalTranslation = DirectX::XMVector3TransformNormal(LocalTranslation, ParentGlobalTransform);

				//	アニメーション側から情報を破棄
				{
					if (!animation_data.root_motion_xz)
					{
						GlobalTranslation = DirectX::XMVectorSetX(GlobalTranslation, 0);
						GlobalTranslation = DirectX::XMVectorSetZ(GlobalTranslation, 0);
					}
					if (!animation_data.root_motion_y)
					{
						GlobalTranslation = DirectX::XMVectorSetY(GlobalTranslation, 0);
					}

					DirectX::XMVECTOR LocalPosition = DirectX::XMLoadFloat3(&current_node_animation.translation);
					DirectX::XMVECTOR GlobalPosition = DirectX::XMVector3TransformNormal(LocalPosition, ParentGlobalTransform);

					if (animation_data.root_motion_xz)
					{
						GlobalPosition = DirectX::XMVectorSetX(GlobalPosition, 0);
						GlobalPosition = DirectX::XMVectorSetZ(GlobalPosition, 0);
					}
					if (animation_data.root_motion_y)
					{
						GlobalPosition = DirectX::XMVectorSetY(GlobalPosition, 0);
					}
					DirectX::XMMATRIX InverseParentGlobalTransform = DirectX::XMMatrixInverse(nullptr, ParentGlobalTransform);
					LocalPosition = DirectX::XMVector3Transform(GlobalPosition, InverseParentGlobalTransform);

					DirectX::XMStoreFloat3(&current_node_animation.translation, LocalPosition);
				}

				//	グローバル空間での移動ベクトルをワールド空間に移動させる
				DirectX::XMVECTOR WorldTranslation = DirectX::XMVector3TransformNormal(GlobalTranslation, WorldTransform);
				DirectX::XMFLOAT3 world_translation;
				DirectX::XMStoreFloat3(&world_translation, WorldTranslation);
				// 位置を更新
				if (apply_root_animation_transform)
				{
					position.x += world_translation.x;
					position.y += world_translation.y;
					position.z += world_translation.z;
					dirty = true;
				}

				//	コールバック呼び出し
				if (on_root_animation_translation)
					on_root_animation_translation(world_translation);
			}

			if (animation_data.root_rotation)
			{
				//	ローカル回転を算出
				DirectX::XMVECTOR LocalRotation, WorkRotation[2];
				if (animation_loop && (previous_time > current_time))
				{
					if (!is_end_compite_animation)
					{
						is_end_compite_animation = true;
						instance->compute_animation_node(end_node_animation, animation_index, node_index, animation_data.end_time);
					}
					WorkRotation[0] = DirectX::XMLoadFloat4(&end_node_animation.rotation);
					WorkRotation[1] = DirectX::XMLoadFloat4(&previous_node_animation.rotation);
					WorkRotation[1] = DirectX::XMQuaternionInverse(WorkRotation[1]);
					LocalRotation = DirectX::XMQuaternionMultiply(WorkRotation[0], WorkRotation[1]);
					WorkRotation[0] = DirectX::XMLoadFloat4(&current_node_animation.rotation);
					WorkRotation[1] = DirectX::XMLoadFloat4(&begin_node_animation.rotation);
					WorkRotation[1] = DirectX::XMQuaternionInverse(WorkRotation[1]);
					WorkRotation[1] = DirectX::XMQuaternionMultiply(WorkRotation[0], WorkRotation[1]);
					LocalRotation = DirectX::XMQuaternionMultiply(LocalRotation, WorkRotation[1]);
				}
				else
				{
					WorkRotation[0] = DirectX::XMLoadFloat4(&current_node_animation.rotation);
					WorkRotation[1] = DirectX::XMLoadFloat4(&previous_node_animation.rotation);
					WorkRotation[1] = DirectX::XMQuaternionInverse(WorkRotation[1]);
					LocalRotation = DirectX::XMQuaternionMultiply(WorkRotation[0], WorkRotation[1]);
				}

				//	ローカル回転量を破棄
				DirectX::XMStoreFloat4(&current_node_animation.rotation, DirectX::XMQuaternionIdentity());

				//	ワールド回転更新
				DirectX::XMFLOAT4 local_rotation;
				DirectX::XMStoreFloat4(&local_rotation, LocalRotation);
				if (apply_root_animation_transform)
				{
					math::rotator local_rotator = local_rotation;
					rotator.x = math::normalize_angle(rotator.x + local_rotator.x);
					rotator.y = math::normalize_angle(rotator.y + local_rotator.y);
					rotator.z = math::normalize_angle(rotator.z + local_rotator.z);
					dirty = true;
				}
				//	コールバック呼び出し
				if (on_root_animation_rotation)
					on_root_animation_rotation(local_rotation);
			}
		}
	}

	//	ブレンド処理
	if (!previous_node_animations.empty())
	{
		if (animation_time <= animation_blend_time)
		{
			float rate = math::clamp(animation_time / animation_blend_time, 0.0f, 1.0f);
			rate *= rate;
			for (size_t index = 0; index < previous_node_animations.size(); ++index)
			{
				auto& node = current_node_animations[index];
				auto& current = current_node_animations[index];
				auto& previous = previous_node_animations[index];
				DirectX::XMStoreFloat3(&node.scale, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&previous.scale), DirectX::XMLoadFloat3(&current.scale), rate));
				DirectX::XMStoreFloat4(&node.rotation, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&previous.rotation), DirectX::XMLoadFloat4(&current.rotation), rate));
				DirectX::XMStoreFloat3(&node.translation, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&previous.translation), DirectX::XMLoadFloat3(&current.translation), rate));
			}
		}
		else
		{
			previous_node_animations.clear();
		}
	}

	//	揺れ処理
	if (shake_time > 0)
		shake_time -= elapsed_time;

	//	ノード姿勢設定
	instance->set_node_poses(current_node_animations);
	dirty = true;
}

void model_object::set_node_pose(size_t node_index, const model_instance::node_animation& node)
{
	instance->set_node_pose(node_index, node);
	dirty = true;
}

bool model_object::register_root_animation_translation_callback(call_back_root_animation_translation callback)
{
	on_root_animation_translation = callback;
	return true;
}

bool model_object::unregister_root_animation_translation_callback()
{
	on_root_animation_translation = {};
	return true;
}

bool model_object::register_root_animation_rotation_callback(call_back_root_animation_rotation callback)
{
	on_root_animation_rotation = callback;
	return true;
}

bool model_object::unregister_root_animation_rotation_callback()
{
	on_root_animation_rotation = {};
	return true;
}

void model_object::pre_render()
{
	update_transform();
}

void model_object::update_transform()
{
	if (!dirty)
		return;

	dirty = false;
	DirectX::XMMATRIX Scale = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&scale));
	DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotator.convert_quaternion()));
	DirectX::XMMATRIX Translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&position));

	//	揺れ処理
	float shake_value = 0.0f;
	if (shake_time > 0 && shake_time_max > 0)
	{
	//	shake_value = math::ease::calculate(shake_time / shake_time_max,  math::ease::type::out_bounce) * shake_power;
		shake_value = shake_power;
		shake_value = std::min(shake_value, system_units::convert_cm_to_unit(10.0f));
	}
	instance->update_transform(Scale * Rotation * Translation, shake_value);
}

void model_object::play_animation(size_t animation_index, bool loop, float blend_time)
{
	this->animation_index = animation_index;
	this->animation_loop = loop;
	this->animation_time = 0;
	this->animation_blend_time = blend_time;
	previous_node_animations.clear();
	if (this->animation_blend_time > 0)
	{
		previous_node_animations = current_node_animations;
	}
}

size_t model_object::get_animation_index() const
{
	return animation_index;
}

std::string_view model_object::get_playing_animation_name() const
{
	if (animation_index == model_resource::invalid_id)
		return "";

	return instance->get_resource()->get_animations()[animation_index].name;
}

size_t model_object::get_animation_index_from_name(std::string_view animation_name) const
{
	return instance->get_resource()->get_animation_index_from_name(animation_name);
}

float model_object::get_animation_end_time() const
{
	if (animation_index == model_resource::invalid_id)
		return 0.0f;

	return instance->get_resource()->get_animations()[animation_index].end_time;
}

float model_object::get_animation_time() const
{
	return animation_time;
}

float model_object::get_animation_speed() const
{
	return animation_speed;
}

void model_object::set_animation_speed(float animation_speed)
{
	this->animation_speed = animation_speed;
}

void model_object::shake(float time, float power)
{
	shake_time = time;
	shake_time_max = time;
	shake_power = power;
	dirty = true;
}

void model_object::on_active()
{
	if (instance)
		instance->set_flag(model_instance::flags::active, true);
}

void model_object::on_deactive()
{
	if (instance)
		instance->set_flag(model_instance::flags::active, false);
}

#if defined(USE_IMGUI)
#define CHECK_BOX(FLAG) \
{\
	bool is_##FLAG = instance->is_flag(model_instance::flags::##FLAG);\
	ImGui::Checkbox(#FLAG, &is_##FLAG);\
	instance->set_flag(model_instance::flags::##FLAG, is_##FLAG);\
}

void model_object::imgui_render_core()
{
	CHECK_BOX(color_render);
	CHECK_BOX(shadow_caster);
	CHECK_BOX(culling);
	CHECK_BOX(culling_target_mesh);
	CHECK_BOX(self_shadow_off);
	CHECK_BOX(use_capsule_shadow);
	CHECK_BOX(debug_draw_culling_object);

	ImGui::DragFloat3("scale", &scale.x, 0.01f);
	ImGui::DragFloat3("rotation", &rotator.x, 0.01f);
	ImGui::DragFloat3("position", &position.x, 0.01f);

	int animation_id = static_cast<int>(animation_index);
	ImGui::DragInt("animation_index", &animation_id, 0.01f, 0, static_cast<int>(instance->get_resource()->get_animations().size()) - 1);
	animation_index = animation_id;
	ImGui::DragFloat("animation_time", &animation_time, 0.01f);
}
#undef CHECK_BOX
#endif	//	defined(USE_IMGUI)

