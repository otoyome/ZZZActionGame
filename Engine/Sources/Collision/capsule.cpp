#include	"collision/capsule.h"

#include	"collision/sphere.h"
#include	"collision/box.h"
#include	"collision/mesh.h"

using namespace collision;

float bounding_capsule::get_radius() const
{
	return get_local_scale().x;
}

void bounding_capsule::set_radius(float radius)
{
	set_local_scale({fabsf(radius), get_length(), 0.0f});
}

float bounding_capsule::get_length() const
{
	return get_local_scale().y;
}

void bounding_capsule::set_length(float length)
{
	set_local_scale({get_radius(), fabsf(length), 0.0f});
}

DirectX::XMVECTOR bounding_capsule::get_axis_vector() const
{
	return DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&get_rotation())).r[1];
}

DirectX::XMFLOAT3 bounding_capsule::get_axis_float3() const
{
	DirectX::XMFLOAT3 vector;
	DirectX::XMStoreFloat3(&vector, get_axis_vector());
	return vector;
}

bounding_capsule::bounding_capsule()
: bounding_object()
{
}

bounding_capsule::bounding_capsule(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& rotation, float radius, float length)
: bounding_object({radius, length, 0.0f}, rotation, position)
{
}

bounding_capsule::~bounding_capsule()
{
}

bool bounding_capsule::intersect_point(const DirectX::XMFLOAT3& point) const
{
	DirectX::XMFLOAT3 horizon;
	float delta;
	return line.calculate_distance(horizon, delta, point) - get_radius() <= 0.0f;
}

bool bounding_capsule::intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect) const
{
	DirectX::XMFLOAT3 horizon1, horizon2;
	float delta1, delta2;
	auto segment = collision::line(collision::line::types::segment, start, end);
	float length = line.calculate_distance(horizon1, delta1, horizon2, delta2, segment) - get_radius();

	if (intersect && length <= 0.0f)
	{
		DirectX::XMVECTOR Horizon1 = DirectX::XMLoadFloat3(&horizon1);
		DirectX::XMVECTOR Horizon2 = DirectX::XMLoadFloat3(&horizon2);
		DirectX::XMVECTOR Line = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(Horizon2, Horizon1));
		DirectX::XMVECTOR Length = DirectX::XMVectorSet(length, length, length, length);

		DirectX::XMFLOAT3 insert_point;
		DirectX::XMVECTOR InsertPoint = DirectX::XMVectorMultiplyAdd(Line, Length, Horizon2);
		DirectX::XMStoreFloat3(&insert_point, InsertPoint);

		Line = DirectX::XMVectorSubtract(InsertPoint, DirectX::XMLoadFloat3(&start));
		DirectX::XMStoreFloat3(&horizon1, DirectX::XMVector3Normalize(Line));

		intersect->add(insert_point, horizon1, DirectX::XMVectorGetX(DirectX::XMVector3Length(Line)));
	}

	return length <= 0.0f;
}

bool bounding_capsule::intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect) const
{
	DirectX::XMFLOAT3 horizon;
	float delta;
	float length = line.calculate_distance(horizon, delta, point) - get_radius() - radius;

	if( intersect && length <= 0.0f )
	{
		DirectX::XMVECTOR Point = DirectX::XMLoadFloat3(&point);
		DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
		DirectX::XMVECTOR Line = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(Position, Point));
		DirectX::XMVECTOR Length = DirectX::XMVectorSet(length + radius, length + radius, length + radius, length + radius);

		DirectX::XMFLOAT3 line;
		DirectX::XMStoreFloat3(&line, Line);
		DirectX::XMStoreFloat3(&horizon, DirectX::XMVectorMultiplyAdd(Line, Length, Point));
		intersect->add(horizon, line, length);
	}

	return length <= 0.0f;
}

bool bounding_capsule::intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect) const
{
	return	intersect_sphere(sphere.get_position(), sphere.get_radius(), intersect);
}

bool bounding_capsule::intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect) const
{
	//	æ‚É‹…“¯Žm‚Å”»’è
	bounding_sphere self_bs = bounding_sphere(get_position(), get_radius() + get_length());
	bounding_sphere capsule_bs = bounding_sphere(capsule.get_position(), capsule.get_radius() + capsule.get_length());
	if (!self_bs.intersect_sphere(capsule_bs))
		return false;

	//	ƒJƒvƒZƒ‹“¯Žm‚Ì“–‚½‚è”»’è
	DirectX::XMFLOAT3 horizon1, horizon2;
	float delta1, delta2;
	float length = line.calculate_distance(horizon1, delta1, horizon2, delta2, capsule.line) - (get_radius() + capsule.get_radius());

	if( intersect && length <= 0.0f )
	{
		DirectX::XMVECTOR Horizon1 = DirectX::XMLoadFloat3(&horizon1);
		DirectX::XMVECTOR Horizon2 = DirectX::XMLoadFloat3(&horizon2);
		DirectX::XMVECTOR Line = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(Horizon2, Horizon1));
		DirectX::XMVECTOR Length = DirectX::XMVectorSet(length + capsule.get_radius(), length + capsule.get_radius(), length + capsule.get_radius(), length + capsule.get_radius());

		DirectX::XMFLOAT3 insert_point;
		DirectX::XMStoreFloat3(&insert_point, DirectX::XMVectorMultiplyAdd(Line, Length, Horizon2));
		DirectX::XMStoreFloat3(&horizon1, DirectX::XMVector3Normalize(Line));
		intersect->add(insert_point, horizon1, length);
	}

	return length <= 0.0f;
}

bool bounding_capsule::intersect_box(const bounding_box& box, intersect_data* intersect) const
{
	if (box.intersect_capsule(*this, intersect))
	{
		if (intersect)
		{
			for (size_t index = 0; index < intersect->size(); ++index)
			{
				(*intersect)[index].normal.x = -(*intersect)[index].normal.x;
				(*intersect)[index].normal.y = -(*intersect)[index].normal.y;
				(*intersect)[index].normal.z = -(*intersect)[index].normal.z;
			}
		}
		return true;
	}
	return	false;
}

bool bounding_capsule::intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect) const
{
	if (mesh.intersect_capsule(*this, intersect))
	{
		if (intersect)
		{
			for (size_t index = 0; index < intersect->size(); ++index)
			{
				(*intersect)[index].normal.x = -(*intersect)[index].normal.x;
				(*intersect)[index].normal.y = -(*intersect)[index].normal.y;
				(*intersect)[index].normal.z = -(*intersect)[index].normal.z;
			}
		}
		return true;
	}
	return	false;
}

void bounding_capsule::include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const
{
	DirectX::XMVECTOR LinePositionA = DirectX::XMLoadFloat3(&line[0]);
	DirectX::XMVECTOR LinePositionB = DirectX::XMLoadFloat3(&line[1]);
	DirectX::XMVECTOR Radius = DirectX::XMVectorSet(get_radius(), get_radius(), get_radius(), get_radius());
	DirectX::XMVECTOR Vertex[4] =
	{
		DirectX::XMVectorSubtract(LinePositionA, Radius),
		DirectX::XMVectorSubtract(LinePositionB, Radius),
		DirectX::XMVectorAdd(LinePositionA, Radius),
		DirectX::XMVectorAdd(LinePositionB, Radius),
	};
	DirectX::XMVECTOR Min = Vertex[0];
	DirectX::XMVECTOR Max = Vertex[0];
	for (size_t index = 1; index < 4; ++index)
	{
		Min = DirectX::XMVectorMin(Min, Vertex[index]);
		Max = DirectX::XMVectorMax(Max, Vertex[index]);
	}
	DirectX::XMStoreFloat3(&min, Min);
	DirectX::XMStoreFloat3(&max, Max);
}

void bounding_capsule::on_change_parameter()
{
	DirectX::XMVECTOR Rotation = DirectX::XMLoadFloat4(&get_rotation());
	DirectX::XMMATRIX Matrix = DirectX::XMMatrixRotationQuaternion(Rotation);

	float length = get_length() * 0.5f;
	DirectX::XMVECTOR Line = DirectX::XMVectorMultiply(Matrix.r[1], DirectX::XMVectorSet(length, length, length, length));
	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR Start = DirectX::XMVectorSubtract(Position, Line);
	DirectX::XMVECTOR End = DirectX::XMVectorAdd(Position, Line);
	DirectX::XMFLOAT3 start, end;
	DirectX::XMStoreFloat3(&start, Start);
	DirectX::XMStoreFloat3(&end, End);
	line = decltype(line)(line::types::segment, start, end);

}
