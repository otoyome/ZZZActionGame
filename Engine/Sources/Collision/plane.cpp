#include	"collision/plane.h"

DirectX::XMFLOAT3 collision::plane::get_plane_position() const
{
	return DirectX::XMFLOAT3(parameter.x * parameter.w, parameter.y * parameter.w, parameter.z * parameter.w);
}

DirectX::XMFLOAT3 collision::plane::get_normal() const
{
	return DirectX::XMFLOAT3(parameter.x, parameter.y, parameter.z);
}

float collision::plane::get_distance() const
{
	return parameter.w;
}

const DirectX::XMFLOAT4& collision::plane::get_parameter() const
{
	return parameter;
}

void collision::plane::set_parameter(const DirectX::XMFLOAT4& parameter)
{
	this->parameter = parameter;
}

collision::plane::plane(const DirectX::XMFLOAT4& parameter)
{
	initialize(parameter);
}

collision::plane::plane(const DirectX::XMFLOAT3& normal, float distance)
{
	initialize(normal, distance);
}

void collision::plane::initialize(const DirectX::XMFLOAT4& parameter)
{
	this->parameter = parameter;
}

void collision::plane::initialize(const DirectX::XMFLOAT3& normal, float distance)
{
	this->parameter.x = normal.x;
	this->parameter.y = normal.y;
	this->parameter.z = normal.z;
	this->parameter.w = distance;
}

void collision::plane::normlize()
{
	DirectX::XMVECTOR Parameter = DirectX::XMLoadFloat4(&parameter);
	DirectX::XMVECTOR Length = DirectX::XMVector3Length(Parameter);
	DirectX::XMStoreFloat4(&parameter, DirectX::XMVectorDivide(Parameter, Length));
}

float collision::plane::calculate_distance(const DirectX::XMFLOAT3& point) const
{
	return	parameter.x * point.x + parameter.y * point.y + parameter.z * point.z + parameter.w;
}

bool collision::plane::to_line(const line& line, float* distance, DirectX::XMFLOAT3* insert_point) const
{
	DirectX::XMFLOAT3 plane_position = get_plane_position();
	DirectX::XMVECTOR PlanePosition = DirectX::XMLoadFloat3(&plane_position);
	DirectX::XMFLOAT3 line_0_position = line[0];
	DirectX::XMFLOAT3 line_1_position = line[1];
	DirectX::XMVECTOR Line0Position = DirectX::XMLoadFloat3(&line_0_position);
	DirectX::XMVECTOR Line1Position = DirectX::XMLoadFloat3(&line_1_position);
	DirectX::XMFLOAT3 normal = get_normal();
	DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&normal);
	if (line.get_type() == line::types::segment)
	{
		DirectX::XMVECTOR V1 = DirectX::XMVectorSubtract(Line0Position, PlanePosition);
		DirectX::XMVECTOR V2 = DirectX::XMVectorSubtract(Line1Position, PlanePosition);
		float dotV1_Normal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(V1, Normal));
		float dotV2_Normal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(V2, Normal));
		if (dotV1_Normal == dotV2_Normal)
		{
			if (distance)
				*distance = 0.0f;
			return true;
		}
		else if ((dotV1_Normal >= 0.0 && dotV2_Normal <= 0.0) || (dotV1_Normal <= 0.0 && dotV2_Normal >= 0.0))
		{
			float length = abs(dotV1_Normal) / (fabsf(dotV1_Normal) + fabsf(dotV2_Normal));
			if (distance)
				*distance = length * DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(Line1Position, Line0Position)));
			if (insert_point)
				DirectX::XMStoreFloat3(insert_point, DirectX::XMVectorAdd(Line0Position, DirectX::XMVectorScale(Line1Position, length)));
			return true;
		}
		//	åç∑ÇµÇƒÇ¢Ç»Ç¢
		return false;
	}
	else
	{
		DirectX::XMVECTOR V1 = DirectX::XMVectorSubtract(PlanePosition, Line0Position);
		DirectX::XMVECTOR V2 = DirectX::XMVectorSubtract(Line1Position, PlanePosition);
		float dotV1_Normal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(V1, Normal));
		if (fabsf(dotV1_Normal) < FLT_EPSILON)
		{
			if (distance)
				*distance	= 0.0f;
			return true;
		}
		float dotLine2_Normal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(Line1Position, Normal));
		if (dotLine2_Normal != 0.0f)
		{
			float length = dotV1_Normal / dotLine2_Normal;
			if (distance)
				*distance = length;
			if (insert_point)
				DirectX::XMStoreFloat3(insert_point, DirectX::XMVectorAdd(Line0Position, DirectX::XMVectorScale(Line1Position, length)));
			return true;
		}
	}
	return false;
}


