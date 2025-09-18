#include <cmath>

#include "animation_resource.h"
#include "graphics_utility.h"
#include "render_data.h"
#include "texture.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/unordered_map.hpp"

#include "animation_data_version.h"

namespace
{
	//	正規化フレームVMD
	static constexpr float NormalizedFrameForVMD = 30.0f;

	//	MMDは1.0 == 8cm(目測)、EMOは 1.0 == 10cm なのでスケールしておく
	static constexpr float ScaleFactorForMMDtoEMO = 10.0f / 8.0f;

	//	回転補正
	static constexpr DirectX::XMFLOAT3 RotationFactorForMMDtoEMO = { 0.0f, DirectX::XM_PI, 0.0f };
}

bool animation::data::compute_animation(std::vector<animation::node>& outs, float time) const
{
	float calculate_time = time;

	size_t node_index = 0;
	for (auto& animation_nodes : nodes)
	{
		outs.emplace_back();
		auto& node = outs[node_index++];

		size_t index = 0;
		for (; index < animation_nodes.size() - 1; ++index)
		{
			auto& animation_1 = animation_nodes[index + 0];
			auto& animation_2 = animation_nodes[index + 1];
			float value = (calculate_time - animation_1.time) / (animation_2.time - animation_1.time);
			if (0.0f <= value && value < 1.0f)
			{
				DirectX::XMStoreFloat3(&node.scale, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&animation_1.scale), DirectX::XMLoadFloat3(&animation_2.scale), value));
				DirectX::XMStoreFloat4(&node.rotation, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&animation_1.rotation), DirectX::XMLoadFloat4(&animation_2.rotation), value));
				DirectX::XMStoreFloat3(&node.translation, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&animation_1.translation), DirectX::XMLoadFloat3(&animation_2.translation), value));
				index = animation_nodes.size();
				break;
			}
		}
		if (index != animation_nodes.size() && animation_nodes[index].time <= calculate_time)
		{
			node.scale = animation_nodes[index].scale;
			node.rotation = animation_nodes[index].rotation;
			node.translation = animation_nodes[index].translation;
		}
	}
	return true;
}

bool animation::data::compute_animation(animation::node& out, size_t node_index, float time) const
{
	float calculate_time = time;

	size_t index = 0;
	auto& animation_nodes = nodes[node_index];
	for (; index < nodes.size() - 1; ++index)
	{
		auto& animation_1 = animation_nodes[index + 0];
		auto& animation_2 = animation_nodes[index + 1];
		float value = (calculate_time - animation_1.time) / (animation_2.time - animation_1.time);
		if (0.0f <= value && value < 1.0f)
		{
			DirectX::XMStoreFloat3(&out.scale, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&animation_1.scale), DirectX::XMLoadFloat3(&animation_2.scale), value));
			DirectX::XMStoreFloat4(&out.rotation, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&animation_1.rotation), DirectX::XMLoadFloat4(&animation_2.rotation), value));
			DirectX::XMStoreFloat3(&out.translation, DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&animation_1.translation), DirectX::XMLoadFloat3(&animation_2.translation), value));
			index = animation_nodes.size();
			break;
		}
	}
	if (index != animation_nodes.size() && animation_nodes[index].time <= calculate_time)
	{
		out.scale = animation_nodes[index].scale;
		out.rotation = animation_nodes[index].rotation;
		out.translation = animation_nodes[index].translation;
	}
	return true;
}

bool animation::data::compute_animation(animation::camera_spherical& out, float time) const
{
	float calculate_time = time;

	size_t index = 0;
	auto data = camera_sphericals.data();
	for (; index < camera_sphericals.size() - 1; ++index)
	{
		auto& animation_1 = data[index + 0];
		auto& animation_2 = data[index + 1];
		float value = (calculate_time - animation_1.time) / (animation_2.time - animation_1.time);
		if (0.0f <= value && value < 1.0f)
		{
			out.distance = math::lerp(animation_1.distance, animation_2.distance, value);
			out.focus = math::lerp(animation_1.focus, animation_2.focus, value);
			out.is_parth = math::lerp(animation_1.is_parth, animation_2.is_parth, value);

			DirectX::XMVECTOR Quaternion1, Quaternion2;
			Quaternion1 = DirectX::XMQuaternionRotationRollPitchYaw(animation_1.rotation.x, animation_1.rotation.y, animation_1.rotation.z);
			Quaternion2 = DirectX::XMQuaternionRotationRollPitchYaw(animation_2.rotation.x, animation_2.rotation.y, animation_2.rotation.z);
			DirectX::XMFLOAT4 rotation;
			DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionSlerp(Quaternion1, Quaternion2, value));
			out.rotation = math::convert_quaternion_to_euler(rotation);
			out.view_angle = math::lerp(animation_1.view_angle, animation_2.view_angle, value);
			index = camera_sphericals.size();
			break;
		}
	}
	if (index != camera_sphericals.size() && camera_sphericals[index].time <= calculate_time)
	{
		out.distance = camera_sphericals[index].distance;
		out.focus = camera_sphericals[index].focus;
		out.is_parth = camera_sphericals[index].is_parth;
		out.rotation = camera_sphericals[index].rotation;
		out.view_angle = camera_sphericals[index].view_angle;
	}
	return true;
}

bool animation::data::compute_animation(float time, std::string_view identifier, variable_function function) const
{
	auto it = variables.find(std::string(identifier));
	if (it == variables.end())
		return false;

	float calculate_time = time;
	size_t index = 0;
	auto data = it->second.data();
	for (; index < it->second.size() - 1; ++index)
	{
		auto& animation_1 = data[index + 0];
		auto& animation_2 = data[index + 1];
		float value = (calculate_time - animation_1.time) / (animation_2.time - animation_1.time);
		if (0.0f <= value && value < 1.0f)
		{
			function(value, identifier, animation_1, animation_2);
			index = variables.size();
			break;
		}
	}
	if (index != variables.size() && it->second[index].time <= calculate_time)
	{
		function(0.0f, identifier, it->second[index], it->second[index]);
	}
	return true;
}

bool animation::data::compute_animation(float time, variable_function function) const
{
	float calculate_time = time;

	//	登録されている物全ての処理を行う
	for (auto& it_variable : variables)
	{
		size_t index = 0;
		auto data = it_variable.second.data();
		for (; index < it_variable.second.size() - 1; ++index)
		{
			auto& animation_1 = data[index + 0];
			auto& animation_2 = data[index + 1];
			float value = (calculate_time - animation_1.time) / (animation_2.time - animation_1.time);
			if (0.0f <= value && value < 1.0f)
			{
				function(value, it_variable.first, animation_1, animation_2);
				index = variables.size();
				break;
			}
		}
		if (index != variables.size() && it_variable.second[index].time <= calculate_time)
		{
			function(0.0f, it_variable.first, it_variable.second[index], it_variable.second[index]);
		}
	}
	return true;
}

bool animation::data::call_events(DirectX::XMFLOAT2 time_range, std::function<void(const event& event)> function) const
{
	float previous_time = time_range.x;
	float current_time = time_range.y;
	for (auto& event : events)
	{
		bool skip = !(previous_time <= event.time && event.time < current_time);
		if (current_time < previous_time)
		{
			skip = true;
			if ((previous_time <= event.time && event.time < end_time) ||
				(0.0f <= event.time && event.time < current_time))
			{
				skip = false;
			}
		}
		if (skip)
			continue;
		function(event);
	}
	return true;
}

animation_resource::animation_resource()
: file_path()
, animations()
{
}

animation_resource::~animation_resource()
{
}

size_t animation_resource::get_index_from_name(std::string_view name) const
{
	size_t index = 0;
	for (auto& animation : animations)
	{
		if (animation.name == name)
		{
			return index;
		}
		++index;
	}
	return invalid_id;
}

bool animation_resource::convert_from_vmd(std::string_view animation_name, const vmd_loader& loader)
{
	//	カメラ以外未対応
	animation::data animation;
	animation.name = animation_name;
	animation.end_time = loader.get_end_frame() / NormalizedFrameForVMD;

	DirectX::XMMATRIX rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(RotationFactorForMMDtoEMO.x, RotationFactorForMMDtoEMO.y, RotationFactorForMMDtoEMO.z);

	auto calculate_camera_animation = [&](const vmd_loader& loader, float time)
	{
		animation::camera_spherical camera;
		camera.time = time;

		auto camera_animations = loader.get_camera();
		float frame = time * NormalizedFrameForVMD;
		size_t index = 0;
		for (; index < camera_animations.size() - 1; ++index)
		{
			auto& animation_1 = camera_animations[index + 0];
			auto& animation_2 = camera_animations[index + 1];
			float value = (frame - animation_1.frame) / (animation_2.frame - animation_1.frame);
			if (0.0f <= value && value < 1.0f)
			{
				camera.distance = math::lerp(animation_1.distance, animation_2.distance, value);
				camera.distance *= -ScaleFactorForMMDtoEMO;
				camera.focus = math::lerp(animation_1.position, animation_2.position, value);
				camera.focus.x *= ScaleFactorForMMDtoEMO;
				camera.focus.y *= ScaleFactorForMMDtoEMO;
				camera.focus.z *= ScaleFactorForMMDtoEMO;
				DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&camera.focus);
				position = DirectX::XMVector3TransformCoord(position, rotation_matrix);
				DirectX::XMStoreFloat3(&camera.focus, position);

				camera.is_parth = math::lerp(animation_1.is_parth, animation_2.is_parth, value);
				camera.rotation = math::lerp(animation_1.rotation, animation_2.rotation, value);

				auto quat = math::rotator(camera.rotation).convert_quaternion();
				DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&quat));
				rotation *= rotation_matrix;
				camera.rotation = math::convert_rotation_matrix_to_euler(system_utility::convert_xmfloat4x4_from_matrix(rotation));
				camera.rotation.x = -camera.rotation.x;
				camera.rotation.y = -camera.rotation.y;
				camera.rotation.z = -camera.rotation.z;

				camera.view_angle = DirectX::XMConvertToRadians(static_cast<float>(math::lerp(animation_1.view_angle, animation_2.view_angle, value)));
				return camera;
			}
		}

		camera.distance = camera_animations[index].distance;
		camera.distance *= -ScaleFactorForMMDtoEMO;
		camera.focus = camera_animations[index].position;
		camera.focus.x *= ScaleFactorForMMDtoEMO;
		camera.focus.y *= ScaleFactorForMMDtoEMO;
		camera.focus.z *= ScaleFactorForMMDtoEMO;
		DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&camera.focus);
		position = DirectX::XMVector3TransformCoord(position, rotation_matrix);
		DirectX::XMStoreFloat3(&camera.focus, position);

		camera.is_parth = camera_animations[index].is_parth;
		camera.rotation = camera_animations[index].rotation;

		auto quat = math::rotator(camera.rotation).convert_quaternion();
		DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&quat));
		rotation *= rotation_matrix;
		camera.rotation = math::convert_rotation_matrix_to_euler(system_utility::convert_xmfloat4x4_from_matrix(rotation));
		camera.rotation.x = -camera.rotation.x;
		camera.rotation.y = -camera.rotation.y;
		camera.rotation.z = -camera.rotation.z;

		camera.view_angle = DirectX::XMConvertToRadians(static_cast<float>(camera_animations[index].view_angle));
		return camera;
	};

	//	これデータ量増えるけど……まぁいいや
	float sampling_rate = system_units::convert_fps_to_sec(1.0f);
	float sampling_time = 0;
	for (sampling_time = 0; sampling_time <= animation.end_time; sampling_time += sampling_rate)
	{
		animation.camera_sphericals.emplace_back(calculate_camera_animation(loader, sampling_time));
	}
	animations.push_back(animation);
	return true;
}

bool animation_resource::add_animation_data(const animation::data& data)
{
	animations.push_back(data);
	return true;
}

bool animation_resource::serialize(std::string_view file_path)
{
	std::ofstream ostream(file_path.data(), std::ios::binary);
	if(ostream.is_open())
	{
		cereal::BinaryOutputArchive archive(ostream);

		try
		{
			archive(CEREAL_NVP(animations));
		}
		catch (...)
		{
			OutputDebugStringA(file_path.data());
			assert(!"material serialize failed.\n");
			return false;
		}

		return true;
	}
	return false;
}

bool animation_resource::deserialize(std::string_view file_path)
{
	std::ifstream istream(file_path.data(), std::ios::binary);
	if(istream.is_open())
	{
		cereal::BinaryInputArchive archive(istream);

		try
		{
			archive(CEREAL_NVP(animations));
		}
		catch (...)
		{
			OutputDebugStringA(file_path.data());
			assert(!"material deserialize failed.\n");
			return false;
		}

		this->file_path = file_path;
		return true;
	}
	OutputDebugStringA(file_path.data());
	assert(!"material deserialize failed.\n");
	return false;
}
