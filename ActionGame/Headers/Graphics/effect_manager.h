#pragma once

#include "Effekseer.h"

#include "effect_resource.h"

namespace effect_utility
{
	inline DirectX::XMFLOAT2 to_float2(const ::Effekseer::Vector2D& vector3)
	{
		return DirectX::XMFLOAT2(vector3.X, vector3.Y);
	}
	inline DirectX::XMFLOAT3 to_float3(const ::Effekseer::Vector3D& vector3)
	{
		return DirectX::XMFLOAT3(vector3.X, vector3.Y, vector3.Z);
	}
	inline ::Effekseer::Vector2D to_vector2(const DirectX::XMFLOAT2& vector3)
	{
		return ::Effekseer::Vector2D(vector3.x, vector3.y);
	}
	inline ::Effekseer::Vector3D to_vector3(const DirectX::XMFLOAT3& vector3)
	{
		return ::Effekseer::Vector3D(vector3.x, vector3.y, vector3.z);
	}
	inline ::Effekseer::Vector3D to_vector3(const DirectX::XMFLOAT4& vector4)
	{
		return ::Effekseer::Vector3D(vector4.x, vector4.y, vector4.z);
	}
	inline ::Effekseer::Color to_color(const DirectX::XMFLOAT4& vector4)
	{
		return ::Effekseer::Color(static_cast<int>(vector4.x * 255),
								  static_cast<int>(vector4.y * 255),
								  static_cast<int>(vector4.z * 255),
								  static_cast<int>(vector4.w * 255));
	}
	inline ::Effekseer::Matrix44 to_matrix44(const DirectX::XMFLOAT4X4& matrix)
	{
		::Effekseer::Matrix44 m;
		m.Values[0][0] = matrix._11;
		m.Values[0][1] = matrix._12;
		m.Values[0][2] = matrix._13;
		m.Values[0][3] = matrix._14;
		m.Values[1][0] = matrix._21;
		m.Values[1][1] = matrix._22;
		m.Values[1][2] = matrix._23;
		m.Values[1][3] = matrix._24;
		m.Values[2][0] = matrix._31;
		m.Values[2][1] = matrix._32;
		m.Values[2][2] = matrix._33;
		m.Values[2][3] = matrix._34;
		m.Values[3][0] = matrix._41;
		m.Values[3][1] = matrix._42;
		m.Values[3][2] = matrix._43;
		m.Values[3][3] = matrix._44;
		return m;
	}
	inline DirectX::XMFLOAT4X4 to_float4x4(const ::Effekseer::Matrix44& matrix)
	{
		DirectX::XMFLOAT4X4 m;
		m._11 = matrix.Values[0][0];
		m._12 = matrix.Values[0][1];
		m._13 = matrix.Values[0][2];
		m._14 = matrix.Values[0][3];
		m._21 = matrix.Values[2][0];
		m._22 = matrix.Values[2][1];
		m._23 = matrix.Values[2][2];
		m._24 = matrix.Values[2][3];
		m._31 = matrix.Values[3][0];
		m._32 = matrix.Values[3][1];
		m._33 = matrix.Values[3][2];
		m._34 = matrix.Values[3][3];
		m._41 = matrix.Values[4][0];
		m._42 = matrix.Values[4][1];
		m._43 = matrix.Values[4][2];
		m._44 = matrix.Values[4][3];
		return m;
	}
}

class effect_manager
{
	SINGLETON(effect_manager)

public:
	::Effekseer::ManagerRef get_effekseer_manager() { return effekseer_manager; }
	static constexpr int get_effekseer_instance_max(){ return 0x00008FFF; }

	float get_elapsed_time_scale() const;
	void set_elapsed_time_scale(float elapsed_time_scale);

private:
	effect_manager();
	~effect_manager();

public:
	void updates(float elapsed_time);

	bool load_resource(std::shared_ptr<effect_resource>& resource, std::string_view file_path);

private:
	float elapsed_time_scale;
	::Effekseer::ManagerRef effekseer_manager;
};
