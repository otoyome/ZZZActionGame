#pragma once

namespace common_animation
{
	struct float_animation
	{
		float time;
		math::ease::type ease_type;
		float value;
	};

	struct float2_animation
	{
		float time;
		math::ease::type ease_type;
		DirectX::XMFLOAT2 value;
	};

	struct float3_animation
	{
		float time;
		math::ease::type ease_type;
		DirectX::XMFLOAT3 value;
	};

	struct float4_animation
	{
		float time;
		math::ease::type ease_type;
		DirectX::XMFLOAT4 value;
	};

	void compute_animation(float time, const float_animation* animations, size_t animation_size, std::function<void(float)> func);
	void compute_animation(float time, const float2_animation* animations, size_t animation_size, std::function<void(DirectX::XMFLOAT2)> func);
	void compute_animation(float time, const float3_animation* animations, size_t animation_size, std::function<void(DirectX::XMFLOAT3)> func);
	void compute_animation(float time, const float4_animation* animations, size_t animation_size, std::function<void(DirectX::XMFLOAT4)> func);
}	//	namespace common_animation
