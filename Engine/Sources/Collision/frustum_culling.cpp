#include	"collision/frustum_culling.h"

#include	"collision/sphere.h"
#include	"collision/capsule.h"
#include	"collision/box.h"

using namespace collision;

frustum_culling::frustum_culling()
{
}

frustum_culling::frustum_culling(const DirectX::XMFLOAT4X4& projection_matrix)
{
	create_planes(projection_matrix);
}

frustum_culling::~frustum_culling()
{
}

bool frustum_culling::intersection(const bounding_object* object) const
{
	switch (object->get_type())
	{
		case bounding_object::types::sphere: return to_sphere(*static_cast<const bounding_sphere*>(object));
		case bounding_object::types::capsule: return to_capsule(*static_cast<const bounding_capsule*>(object));
		case bounding_object::types::box: return to_box(*static_cast<const bounding_box*>(object));
	}
	assert(!"Not Comaptible Collilsion.");
	return false;
}

bool frustum_culling::to_point(const DirectX::XMFLOAT3& point) const
{
	for (size_t index = 0; index < surface_max; ++index)
	{
		if (planes[index].calculate_distance(point) < 0.0f)
		{
			return false;
		}
	}
	return true;
}

bool frustum_culling::to_sphere(const DirectX::XMFLOAT3& point, float radius) const
{
	for (size_t index = 0; index < surface_max; ++index)
	{
		if (planes[index].calculate_distance(point) < -radius)
		{
			return false;
		}
	}
	return true;
}

bool frustum_culling::to_sphere(const bounding_sphere& sphere) const
{
    return	to_sphere(sphere.get_position(), sphere.get_radius());
}

bool frustum_culling::to_capsule(const bounding_capsule& capsule) const
{
	//	‚½‚¾‚ÌAABB‚Å‚Ì”»’è
	DirectX::XMFLOAT3 min, max;
	capsule.include_corner(min, max);
	return to_aabb(min, max);
}

bool frustum_culling::to_box(const bounding_box& box) const
{
	//	‚½‚¾‚ÌAABB‚Å‚Ì”»’è
	DirectX::XMFLOAT3 min, max;
	box.include_corner(min, max);
	return to_aabb(min, max);
}

bool frustum_culling::to_aabb(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max) const
{
	int r = 0;
	for (int index = 0; index < surface_max; ++index)
	{
		r = 0;
		r += planes[index].calculate_distance({min.x, min.y, min.z}) <= 0 ? 1 : 0;
		r += planes[index].calculate_distance({max.x, min.y, min.z}) <= 0 ? 1 : 0;
		r += planes[index].calculate_distance({min.x, max.y, min.z}) <= 0 ? 1 : 0;
		r += planes[index].calculate_distance({max.x, max.y, min.z}) <= 0 ? 1 : 0;
		r += planes[index].calculate_distance({min.x, min.y, max.z}) <= 0 ? 1 : 0;
		r += planes[index].calculate_distance({max.x, min.y, max.z}) <= 0 ? 1 : 0;
		r += planes[index].calculate_distance({min.x, max.y, max.z}) <= 0 ? 1 : 0;
		r += planes[index].calculate_distance({max.x, max.y, max.z}) <= 0 ? 1 : 0;
		if (r == 8)
			return false;
	}

	r = 0;
	for (int index = 0; index < 8; ++index)
		r += ((corners[index].x <= min.x) ? 1 : 0);
	if (r == 8)
		return false;
	r = 0;
	for (int index = 0; index < 8; ++index)
		r += ((corners[index].x >= max.x) ? 1 : 0);
	if (r == 8)
		return false;

	r = 0;
	for (int index = 0; index < 8; ++index)
		r += ((corners[index].y <= min.y) ? 1 : 0);
	if (r == 8)
		return false;
	r = 0;
	for (int index = 0; index < 8; ++index)
		r += ((corners[index].y >= max.y) ? 1 : 0);
	if (r == 8)
		return false;

	r = 0;
	for (int index = 0; index < 8; ++index)
		r += ((corners[index].z <= min.z) ? 1 : 0);
	if (r == 8)
		return false;
	r = 0;
	for (int index = 0; index < 8; ++index)
		r += ((corners[index].z >= max.z) ? 1 : 0);
	if (r == 8)
		return false;

	return true;
}

void frustum_culling::create_planes(const DirectX::XMFLOAT4X4& view_projection_matrix)
{
	DirectX::XMFLOAT4 col0(view_projection_matrix.m[0][0], view_projection_matrix.m[1][0], view_projection_matrix.m[2][0], view_projection_matrix.m[3][0] );
	DirectX::XMFLOAT4 col1(view_projection_matrix.m[0][1], view_projection_matrix.m[1][1], view_projection_matrix.m[2][1], view_projection_matrix.m[3][1] );
	DirectX::XMFLOAT4 col2(view_projection_matrix.m[0][2], view_projection_matrix.m[1][2], view_projection_matrix.m[2][2], view_projection_matrix.m[3][2] );
	DirectX::XMFLOAT4 col3(view_projection_matrix.m[0][3], view_projection_matrix.m[1][3], view_projection_matrix.m[2][3], view_projection_matrix.m[3][3] );
	DirectX::XMVECTOR Col0 = DirectX::XMLoadFloat4(&col0);
	DirectX::XMVECTOR Col1 = DirectX::XMLoadFloat4(&col1);
	DirectX::XMVECTOR Col2 = DirectX::XMLoadFloat4(&col2);
	DirectX::XMVECTOR Col3 = DirectX::XMLoadFloat4(&col3);
	DirectX::XMFLOAT4 work;

	DirectX::XMStoreFloat4(&work, DirectX::XMVectorAdd(Col3, Col0));
	planes[surface_left].set_parameter(work);
	DirectX::XMStoreFloat4(&work, DirectX::XMVectorSubtract(Col3, Col0));
	planes[surface_right].set_parameter(work);

	DirectX::XMStoreFloat4(&work, DirectX::XMVectorAdd(Col3, Col1));
	planes[surface_bottom].set_parameter(work);
	DirectX::XMStoreFloat4(&work, DirectX::XMVectorSubtract(Col3, Col1));
	planes[surface_top].set_parameter(work);

	DirectX::XMStoreFloat4(&work, Col2);
	planes[surface_near].set_parameter(work);
	DirectX::XMStoreFloat4(&work, DirectX::XMVectorSubtract(Col3, Col2));
	planes[surface_far].set_parameter(work);

	for (size_t index = 0; index < surface_max; ++index)
	{
		planes[index].normlize();
	}
}

void frustum_culling::create_frustum_corners(const DirectX::XMFLOAT4X4& inverse_view_projection_matrix)
{
	create_frustum_corners(DirectX::XMLoadFloat4x4(&inverse_view_projection_matrix));
}

void frustum_culling::create_frustum_corners(const DirectX::XMMATRIX & inverse_view_projection_matrix)
{
	const DirectX::XMVECTOR base_corners[] =
	{
		DirectX::XMVectorSet(-1, -1, -1, 1),
		DirectX::XMVectorSet( 1, -1, -1, 1),
		DirectX::XMVectorSet( 1,  1, -1, 1),
		DirectX::XMVectorSet(-1,  1, -1, 1),
		DirectX::XMVectorSet(-1, -1,  1, 1),
		DirectX::XMVectorSet( 1, -1,  1, 1),
		DirectX::XMVectorSet( 1,  1,  1, 1),
		DirectX::XMVectorSet(-1,  1,  1, 1)
	};
	
	for (int index = 0; index < 8; ++index)
	{
		DirectX::XMVECTOR Point;
		Point = DirectX::XMVector4Transform(base_corners[index], inverse_view_projection_matrix);
		float w = DirectX::XMVectorGetW(Point);
		DirectX::XMStoreFloat4(&corners[index], DirectX::XMVectorScale(Point, 1.0f / w));
	}
}

