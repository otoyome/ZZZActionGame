#include "animations/common_animation.h"

#include "math.h"
#include "math_utility.h"

void common_animation::compute_animation(float time, const float_animation* animations, size_t animation_size, std::function<void(float)> func)
{
	UINT index = 0;
	for(; index < animation_size - 1; ++index)
	{
		auto& first = animations[index + 0];
		auto& second = animations[index + 1];
		float factor = (time - first.time) / (second.time - first.time);
		if (0.0f <= factor && factor < 1.0f)
		{
			factor = math::ease::calculate(factor, first.ease_type);
			func(math::lerp(first.value, second.value, factor));
			break;
		}
	}
	if (time >= animations[animation_size - 1].time)
	{
		func(animations[animation_size - 1].value);
	}
}

void common_animation::compute_animation(float time, const float2_animation* animations, size_t animation_size, std::function<void(DirectX::XMFLOAT2)> func)
{
	UINT index = 0;
	for(; index < animation_size - 1; ++index)
	{
		auto& first = animations[index + 0];
		auto& second = animations[index + 1];
		float factor = (time - first.time) / (second.time - first.time);
		if (0.0f <= factor && factor < 1.0f)
		{
			factor = math::ease::calculate(factor, first.ease_type);
			DirectX::XMFLOAT2 value;
			value.x = math::lerp(first.value.x, second.value.x, factor);
			value.y = math::lerp(first.value.y, second.value.y, factor);
			func(value);
			break;
		}
	}
	if (time >= animations[animation_size - 1].time)
	{
		func(animations[animation_size - 1].value);
	}
}

void common_animation::compute_animation(float time, const float3_animation* animations, size_t animation_size, std::function<void(DirectX::XMFLOAT3)> func)
{
	UINT index = 0;
	for(; index < animation_size - 1; ++index)
	{
		auto& first = animations[index + 0];
		auto& second = animations[index + 1];
		float factor = (time - first.time) / (second.time - first.time);
		if (0.0f <= factor && factor < 1.0f)
		{
			factor = math::ease::calculate(factor, first.ease_type);
			DirectX::XMFLOAT3 value;
			value.x = math::lerp(first.value.x, second.value.x, factor);
			value.y = math::lerp(first.value.y, second.value.y, factor);
			value.z = math::lerp(first.value.z, second.value.z, factor);
			func(value);
			break;
		}
	}
	if (time >= animations[animation_size - 1].time)
	{
		func(animations[animation_size - 1].value);
	}
}

void common_animation::compute_animation(float time, const float4_animation* animations, size_t animation_size, std::function<void(DirectX::XMFLOAT4)> func)
{
	UINT index = 0;
	for(; index < animation_size - 1; ++index)
	{
		auto& first = animations[index + 0];
		auto& second = animations[index + 1];
		float factor = (time - first.time) / (second.time - first.time);
		if (0.0f <= factor && factor < 1.0f)
		{
			factor = math::ease::calculate(factor, first.ease_type);
			DirectX::XMFLOAT4 value;
			value.x = math::lerp(first.value.x, second.value.x, factor);
			value.y = math::lerp(first.value.y, second.value.y, factor);
			value.z = math::lerp(first.value.z, second.value.z, factor);
			value.w = math::lerp(first.value.w, second.value.w, factor);
			func(value);
			break;
		}
	}
	if (time >= animations[animation_size - 1].time)
	{
		func(animations[animation_size - 1].value);
	}
}



