#include	"collision/sphere.h"

#include	"collision/capsule.h"
#include	"collision/box.h"
#include	"collision/mesh.h"

#include	<DirectXCollision.h>

using namespace collision;


float bounding_sphere::get_radius() const
{
	return get_local_scale().x;
}

void bounding_sphere::set_radius(float radius)
{
	set_local_scale({radius, 0.0f, 0.0f});
}

bounding_sphere::bounding_sphere()
: bounding_object()
{
}

bounding_sphere::bounding_sphere(const DirectX::XMFLOAT3& position, float radius)
: bounding_object({radius, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, position)
{
}

bounding_sphere::~bounding_sphere()
{
}

bool bounding_sphere::intersect_point(const DirectX::XMFLOAT3& point) const
{
	DirectX::XMVECTOR Point = DirectX::XMLoadFloat3(&point);
	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR Vector = DirectX::XMVectorSubtract(Position, Point);
	DirectX::XMVECTOR Distance = DirectX::XMVector3Length(Vector);
	return	DirectX::XMVectorGetX(Distance) - get_radius() <= 0.0f;
}

bool bounding_sphere::intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect) const
{
	DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
	DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);

	return intersect_segment(Start, End, intersect);
}

bool bounding_sphere::intersect_segment(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& End, intersect_data* intersect) const
{
	DirectX::XMVECTOR Direction = DirectX::XMVectorSubtract(End, Start);
	float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(Direction));
	Direction = DirectX::XMVector3Normalize(Direction);

	DirectX::BoundingSphere sphere = DirectX::BoundingSphere(get_position(), get_radius());
	if (sphere.Intersects(Start, Direction, distance))
	{
		if (intersect)
		{
			DirectX::XMVECTOR Distance = DirectX::XMVectorSet(distance, distance, distance, distance);
			DirectX::XMFLOAT3 position, direction;
			DirectX::XMStoreFloat3(&position, DirectX::XMVectorMultiplyAdd(Direction, Distance, Start));
			DirectX::XMStoreFloat3(&direction, DirectX::XMVectorNegate(Direction));
			intersect->add(position, direction, distance);
		}
		return true;
	}
	return false;
}

bool bounding_sphere::intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect) const
{
	DirectX::XMVECTOR Point = DirectX::XMLoadFloat3(&point);
	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR Vector = DirectX::XMVectorSubtract(Position, Point);
	DirectX::XMVECTOR Distance = DirectX::XMVector3Length(Vector);
	float distance = DirectX::XMVectorGetX(Distance) - get_radius() - radius;
	bool insert = distance <= 0.0f;
	if (intersect && insert)
	{
		Vector = DirectX::XMVector3Normalize(Vector);
		DirectX::XMFLOAT3 point, normal;
		DirectX::XMStoreFloat3(&normal, Vector);
		radius += distance;
		Distance = DirectX::XMVectorSet(radius, radius, radius, radius);
		DirectX::XMStoreFloat3(&point, DirectX::XMVectorMultiplyAdd(Vector, Distance, Point));
		intersect->add(point, normal, distance);
	}
	return insert;
}

bool bounding_sphere::intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect) const
{
	return intersect_sphere(sphere.get_position(), sphere.get_radius(), intersect);
}

bool bounding_sphere::intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect) const
{
	return	capsule.intersect_sphere(*this, intersect);
}

bool bounding_sphere::intersect_box(const bounding_box& box, intersect_data* intersect) const
{
	return	box.intersect_sphere(*this, intersect);
}

bool bounding_sphere::intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect) const
{
	return	mesh.intersect_sphere(*this, intersect);
}

void bounding_sphere::include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const
{
	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR Radius = DirectX::XMVectorSet(get_radius(), get_radius(), get_radius(), 0);
	DirectX::XMStoreFloat3(&min, DirectX::XMVectorSubtract(Position, Radius));
	DirectX::XMStoreFloat3(&max, DirectX::XMVectorAdd(Position, Radius));
}
