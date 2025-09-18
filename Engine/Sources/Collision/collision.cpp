#include	"collision/collision.h"

#include	"collision/sphere.h"
#include	"collision/capsule.h"
#include	"collision/box.h"

#include	"system_utility.h"

using namespace collision;

intersect_data::intersect_data()
{
}

intersect_data::~intersect_data()
{
}

void intersect_data::add(const infomation& infomation)
{
	infomations.emplace_back(infomation);
}

void intersect_data::add(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT3& normal, float length)
{
	infomation	infomation;
	infomation.point = point;
	DirectX::XMStoreFloat3(&infomation.normal, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normal)));
	infomation.length = length;
	add(infomation);
}

void intersect_data::clear()
{
	system_utility::std_vector_swap_clear(infomations);
}

bool intersect_data::multiply_matrix(std::size_t index, DirectX::XMFLOAT4X4 matrix)
{
	return multiply_matrix(index, DirectX::XMLoadFloat4x4(&matrix));
}

bool intersect_data::multiply_matrix(std::size_t index, DirectX::XMMATRIX matrix)
{
	if (index >= infomations.size())
		return false;

	DirectX::XMVECTOR Position, Normal;
	Position = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&infomations[index].point), matrix);
	Normal = DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&infomations[index].normal), matrix);
	DirectX::XMStoreFloat3(&infomations[index].point, Position);
	DirectX::XMStoreFloat3(&infomations[index].normal, DirectX::XMVector3Normalize(Normal));
	return true;
}

std::size_t intersect_data::size()
{
	return infomations.size();
}

const intersect_data::infomation& intersect_data::operator[](std::size_t index) const
{
	return infomations[index];
}

intersect_data::infomation& intersect_data::operator[](std::size_t index)
{
	return infomations[index];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const DirectX::XMFLOAT3& bounding_object::get_local_scale() const
{
	return	local_scale;
}

void bounding_object::set_local_scale(const DirectX::XMFLOAT3& scale)
{
	this->local_scale = scale;
	on_change_parameter();
}

const DirectX::XMFLOAT4& bounding_object::get_rotation() const
{
	return	rotation;
}

void bounding_object::set_rotation(const DirectX::XMFLOAT4& rotation)
{
	this->rotation = rotation;
	on_change_parameter();
}

const DirectX::XMFLOAT3& bounding_object::get_position() const
{
	return position;
}

void bounding_object::set_position(const DirectX::XMFLOAT3& position)
{
	this->position	= position;
	on_change_parameter();
}

DirectX::XMFLOAT4X4 bounding_object::get_transform_float4x4() const
{
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, get_transform_matrix());
	return matrix;
}

DirectX::XMMATRIX bounding_object::get_transform_matrix() const
{
	DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
	DirectX::XMMATRIX Translation = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	return	Rotation * Translation;
}

bounding_object::bounding_object()
: local_scale( 1, 1, 1 )
, rotation( 0, 0, 0, 1 )
, position( 0, 0, 0 )
{
	on_change_parameter();
}

bounding_object::bounding_object(const DirectX::XMFLOAT3& local_scale, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& position)
: local_scale(local_scale)
, rotation(rotation)
, position(position)
{
	on_change_parameter();
}

bounding_object::~bounding_object()
{
}

bool bounding_object::intersection(const bounding_object* object, intersect_data* intersect) const
{
	switch (object->get_type())
	{
		case types::sphere: return intersect_sphere(*static_cast<const bounding_sphere*>(object), intersect);
		case types::capsule: return intersect_capsule(*static_cast<const bounding_capsule*>(object), intersect);
		case types::box: return intersect_box(*static_cast<const bounding_box*>(object), intersect);
		case types::mesh: return intersect_mesh(*reinterpret_cast<const bounding_mesh*>(object), intersect);
	}
	assert(!"Not Comaptible Collilsion.");
	return	false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool collision::ray_cast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const triangles& triangles, intersect_data* result)
{
	DirectX::XMVECTOR Start	= DirectX::XMLoadFloat3(&start);
	DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
	return ray_cast(Start, End, triangles, result);
}

bool collision::ray_cast(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& End, const triangles& triangles, intersect_data* result)
{
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(End, Start);
	DirectX::XMVECTOR Dir = DirectX::XMVector3Normalize(Vec);
	DirectX::XMVECTOR Length = DirectX::XMVector3Length(Vec);

	float neart = DirectX::XMVectorGetX(Length);

	bool hit = false;
	DirectX::XMVECTOR HitPosition, HitNormal;
	for (size_t index = triangles.index_start; index < triangles.index_count; index += 3)
	{
		const DirectX::XMFLOAT3& a = triangles.positions[triangles.indices[index + (triangles.reverse_index ? 2 : 0)]];
		const DirectX::XMFLOAT3& b = triangles.positions[triangles.indices[index + (triangles.reverse_index ? 1 : 1)]];
		const DirectX::XMFLOAT3& c = triangles.positions[triangles.indices[index + (triangles.reverse_index ? 0 : 2)]];

		DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&a);
		DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&b);
		DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&c);

		DirectX::XMVECTOR AB = DirectX::XMVectorSubtract(B, A);
		DirectX::XMVECTOR BC = DirectX::XMVectorSubtract(C, B);
		DirectX::XMVECTOR CA = DirectX::XMVectorSubtract(A, C);

		DirectX::XMVECTOR Normal = DirectX::XMVector3Cross(AB, BC);
		DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(Normal, Dir);
		if (DirectX::XMVectorGetX(Dot) >= 0)
			continue;

		DirectX::XMVECTOR V = DirectX::XMVectorSubtract(A, Start);
		DirectX::XMVECTOR T = DirectX::XMVectorDivide( DirectX::XMVector3Dot(Normal, V), Dot);

		float t = DirectX::XMVectorGetX(T);
		if (t < 0 || t > neart)
			continue;

		DirectX::XMVECTOR Position = DirectX::XMVectorAdd(DirectX::XMVectorMultiply(Dir, T), Start);

		DirectX::XMVECTOR Cross;
		V = DirectX::XMVectorSubtract(A, Position);
		Cross = DirectX::XMVector3Cross(V, AB);
		Dot = DirectX::XMVector3Dot(Cross, Normal);
		if (DirectX::XMVectorGetX(Dot) < 0)
			continue;

		V = DirectX::XMVectorSubtract(B, Position);
		Cross = DirectX::XMVector3Cross(V, BC);
		Dot = DirectX::XMVector3Dot(Cross, Normal);
		if (DirectX::XMVectorGetX(Dot) < 0)
			continue;

		V = DirectX::XMVectorSubtract(C, Position);
		Cross = DirectX::XMVector3Cross(V, CA);
		Dot = DirectX::XMVector3Dot(Cross, Normal);
		if (DirectX::XMVectorGetX(Dot) < 0)
			continue;

		neart = t;
		HitPosition = Position;
		HitNormal = Normal;
		hit = true;
	}
	if (hit && result)
	{
		intersect_data::infomation infomation;
		DirectX::XMStoreFloat3(&infomation.point, HitPosition);
		DirectX::XMStoreFloat3(&infomation.normal, HitNormal);
		infomation.length = neart;
		result->add(infomation);
	}
	return	hit;
}