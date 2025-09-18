#include	"collision/box.h"

#include	"collision/sphere.h"
#include	"collision/capsule.h"
#include	"collision/mesh.h"

#include	<DirectXCollision.h>

using namespace collision;

DirectX::XMMATRIX inner::axis_aligned_bounding_box::get_transeform_matrix() const
{
	DirectX::XMVECTOR Scale = DirectX::XMLoadFloat3(&get_local_scale());
	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	return	DirectX::XMMatrixScalingFromVector(Scale) * DirectX::XMMatrixTranslationFromVector(Position);
}

DirectX::XMFLOAT4X4 inner::axis_aligned_bounding_box::get_transeform_float4x4() const
{
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, get_transeform_matrix());
	return matrix;
}

inner::axis_aligned_bounding_box::axis_aligned_bounding_box()
: bounding_object()
{
}

inner::axis_aligned_bounding_box::axis_aligned_bounding_box(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& position)
: bounding_object(scale, DirectX::XMFLOAT4(0, 0, 0, 1), position)
{
}

inner::axis_aligned_bounding_box::~axis_aligned_bounding_box()
{
}

float inner::axis_aligned_bounding_box::calculate_point_to_length(const DirectX::XMFLOAT3& point) const
{
	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR Scale = DirectX::XMLoadFloat3(&get_local_scale());

	DirectX::XMFLOAT3 min, max;
	DirectX::XMStoreFloat3(&min, DirectX::XMVectorAdd(Position, Scale));
	DirectX::XMStoreFloat3(&max, DirectX::XMVectorSubtract(Position, Scale));
	DirectX::XMFLOAT3 center = point;

	float* center_ptr = &center.x;
	float* min_ptr = &min.x;
	float* max_ptr = &max.x;
	for (size_t index = 0; index < 3; ++index)
	{
		if (center_ptr[index] < min_ptr[index])
		{
			center_ptr[index] = min_ptr[index];
		}
		if (center_ptr[index] > max_ptr[index])
		{
			center_ptr[index] = max_ptr[index];
		}
	}
	DirectX::XMVECTOR Point = DirectX::XMLoadFloat3(&point);
	DirectX::XMVECTOR Center = DirectX::XMLoadFloat3(&center);
	return	DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(Point, Center)));
}

bool inner::axis_aligned_bounding_box::intersect_point(const DirectX::XMFLOAT3& point) const
{
	return calculate_point_to_length(point) <= 0.0f;
}

bool inner::axis_aligned_bounding_box::intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect) const
{
	DirectX::BoundingBox box;
	box.Center = get_position();
	box.Extents = get_local_scale();
	DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
	DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
	DirectX::XMVECTOR Direction = DirectX::XMVectorSubtract(End, Start);
	float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(Direction));
	Direction = DirectX::XMVector3Normalize(Direction);
	if (box.Intersects(Start, Direction, distance))
	{
		if (intersect)
		{
			End = DirectX::XMVectorSet(distance, distance, distance, distance);
			DirectX::XMFLOAT3 position, direction;
			DirectX::XMStoreFloat3(&position, DirectX::XMVectorMultiplyAdd(Direction, End, Start));
			DirectX::XMStoreFloat3(&direction, DirectX::XMVectorNegate(Direction));
			intersect->add(position, direction, distance);
		}
		return true;
	}
	return false;
}

bool inner::axis_aligned_bounding_box::intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect) const
{
	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR Scale = DirectX::XMLoadFloat3(&get_local_scale());

	DirectX::XMFLOAT3 min, max;
	DirectX::XMStoreFloat3(&max, DirectX::XMVectorAdd(Position, Scale));
	DirectX::XMStoreFloat3(&min, DirectX::XMVectorSubtract(Position, Scale));

	DirectX::XMFLOAT3 closest_point;
	closest_point.x = math::clamp(point.x, min.x, max.x);
	closest_point.y = math::clamp(point.y, min.y, max.y);
	closest_point.z = math::clamp(point.z, min.z, max.z);

	DirectX::XMVECTOR Point = DirectX::XMLoadFloat3(&point);
	DirectX::XMVECTOR ClosestPoint = DirectX::XMLoadFloat3(&closest_point);
	DirectX::XMVECTOR VectorC_P = DirectX::XMVectorSubtract(ClosestPoint, Point);
	float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(VectorC_P));
	if (length < radius)
	{
		if (intersect)
		{
			DirectX::XMVECTOR Radius = DirectX::XMVectorSet(radius, radius, radius, radius);
			if (length > 0.0f)
			{
				VectorC_P = DirectX::XMVector3Normalize(VectorC_P);
				DirectX::XMFLOAT3 position;
				Position = DirectX::XMVectorSubtract(Point, DirectX::XMVectorMultiply(VectorC_P, Radius));
				DirectX::XMStoreFloat3(&position, Position);
				DirectX::XMStoreFloat3(&min, Radius);
				intersect->add(position, min, radius - length);
			}
			else
			{
				VectorC_P = DirectX::XMVectorSubtract(Point, Position);
				if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(VectorC_P)) <= FLT_EPSILON)
					return	false;

				//	どの軸が最も短い距離か判定する
				size_t axis_index = 0;
				DirectX::XMFLOAT3 vectorC_P;
				DirectX::XMStoreFloat3(&vectorC_P, VectorC_P);
				float distance = vectorC_P.x / get_local_scale().x;
				float new_distance = vectorC_P.y / get_local_scale().y;
				if (fabsf(new_distance) > fabsf(distance))
				{
					distance = new_distance;
					axis_index = 1;
				}
				new_distance = vectorC_P.z / get_local_scale().z;
				if (fabsf(new_distance) > fabsf(distance))
				{
					distance = new_distance;
					axis_index = 2;
				}

				//	法線算出
				DirectX::XMFLOAT3 normal(0.0f, 0.0f, 0.0f);
				(&normal.x)[axis_index] = distance < 0.0f ? -1.0f : 1.0f;

				//	距離を算出
				float delta = fabsf((&vectorC_P.x)[axis_index]);
				float length = (radius + (&get_local_scale().x)[axis_index]) - delta;
				DirectX::XMVECTOR Normal = DirectX::XMLoadFloat3(&normal);

				DirectX::XMFLOAT3 position;
				Position = DirectX::XMVectorSubtract(Point, DirectX::XMVectorMultiply(Normal, Radius));
				DirectX::XMStoreFloat3(&position, Position);
				intersect->add(position, normal, radius - length);
			}
		}
			
		return true;
	}

	return false;
}

bool inner::axis_aligned_bounding_box::intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect) const
{
	return intersect_sphere(sphere.get_position(), sphere.get_radius(), intersect);
}

bool inner::axis_aligned_bounding_box::intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect) const
{
	DirectX::XMVECTOR Direction = capsule.get_axis_vector();
	DirectX::XMVECTOR d = DirectX::XMVectorScale(Direction, capsule.get_length());

	DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&capsule.get_position());
	DirectX::XMVECTOR t = DirectX::XMVectorSubtract(PositionB, PositionA);
	DirectX::XMVECTOR S1 = DirectX::XMVectorSubtract(t, d);
	DirectX::XMVECTOR S2 = DirectX::XMVectorAdd(t, d);
	DirectX::XMVECTOR e = DirectX::XMLoadFloat3(&get_local_scale());

	float pen_depth = FLT_MAX;
	size_t pen_axis = 0;

	float adx = fabsf(DirectX::XMVectorGetX(d));
	float projR = adx + capsule.get_radius();
	float pd = (projR + DirectX::XMVectorGetX(e)) - fabsf(DirectX::XMVectorGetX(t));

	if (pd <= 0.0f)
		return false;

	{
		pen_depth = pd;
		pen_axis = 0;
	}

	float ady = fabsf(DirectX::XMVectorGetY(d));
	projR = ady + capsule.get_radius();
	pd = (projR + DirectX::XMVectorGetY(e)) - fabsf(DirectX::XMVectorGetY(t));

	if (pd <= 0.0f)
		return false;

	if (pd < pen_depth)
	{
		pen_depth = pd;
		pen_axis = 1;
	}

	float adz = fabsf(DirectX::XMVectorGetZ(d));
	projR = adz + capsule.get_radius();
	pd = (projR + DirectX::XMVectorGetZ(e)) - fabsf(DirectX::XMVectorGetZ(t));

	if( pd <= 0.0f )
		return false;

	if (pd < pen_depth)
	{
		pen_depth = pd;
		pen_axis = 2;
	}

	DirectX::XMVECTOR capep = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	adx = DirectX::XMVectorGetX(Direction);
	capep = DirectX::XMVectorSetX(capep, adx < 0.0f ? adx - FLT_EPSILON : adx + FLT_EPSILON);
	ady = DirectX::XMVectorGetY(Direction);
	capep = DirectX::XMVectorSetY(capep, ady < 0.0f ? ady - FLT_EPSILON : ady + FLT_EPSILON);
	adz = DirectX::XMVectorGetZ(Direction);
	capep = DirectX::XMVectorSetZ(capep, adz < 0.0f ? adz - FLT_EPSILON : adz + FLT_EPSILON);

	DirectX::XMVECTOR Axes[3] =
	{
		DirectX::XMVectorSet(0.0f, -DirectX::XMVectorGetZ(capep), DirectX::XMVectorGetY(capep), 0.0f),
		DirectX::XMVectorSet(DirectX::XMVectorGetZ(capep), 0.0f, -DirectX::XMVectorGetX(capep), 0.0f),
		DirectX::XMVectorSet(-DirectX::XMVectorGetY(capep), DirectX::XMVectorGetX(capep), 0.0f, 0.0f),
	};

	for (size_t index = 0; index < 3; ++index)
	{
		float mag = DirectX::XMVectorGetX(DirectX::XMVector3Length(Axes[index]));
		float invMag = 1.0f / mag;
		Axes[index] = DirectX::XMVectorScale(Axes[index], invMag);

		float rb = DirectX::XMVectorGetX(e) * fabsf(DirectX::XMVectorGetX(Axes[index]))
					+ DirectX::XMVectorGetY(e) * fabsf(DirectX::XMVectorGetY(Axes[index]))
					+ DirectX::XMVectorGetZ(e) * fabsf(DirectX::XMVectorGetZ(Axes[index]));
		float dist = fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(Axes[index], t)));

		pd = (rb + capsule.get_radius()) - dist;
		if (pd <= 0.0f)
			return false;

		if (pd < pen_depth)
		{
			pen_depth = pd;
			pen_axis = 3 + index;
		}
	}

	if (intersect)
	{
		if (pen_axis < 3)
		{
			size_t clip_Axes[3][2] = {{1, 2}, {0, 2}, {0, 1}};

			for (size_t index = 0; index < 2; ++index)
			{
				size_t clip_axis = clip_Axes[pen_axis][index];
				float offset = (&get_local_scale().x)[clip_axis];

				if (DirectX::XMVectorGetByIndex(S1, clip_axis) < offset && DirectX::XMVectorGetByIndex(S2, clip_axis) > offset)
				{
					DirectX::XMVECTOR Delta = DirectX::XMVectorSubtract(S2, S1);
					float w = (offset - DirectX::XMVectorGetByIndex(S1, clip_axis)) / DirectX::XMVectorGetByIndex(Delta, clip_axis);
					S2 = DirectX::XMVectorAdd(S1, DirectX::XMVectorScale(Delta, w));
				}
				else if (DirectX::XMVectorGetByIndex(S1, clip_axis) > offset && DirectX::XMVectorGetByIndex(S2, clip_axis) < offset)
				{
					DirectX::XMVECTOR Delta = DirectX::XMVectorSubtract(S1, S2);
					float w = (offset - DirectX::XMVectorGetByIndex(S2, clip_axis)) / DirectX::XMVectorGetByIndex(Delta, clip_axis);
					S1 = DirectX::XMVectorAdd(S2, DirectX::XMVectorScale(Delta, w));
				}
					
				if (DirectX::XMVectorGetByIndex(S1, clip_axis) > -offset && DirectX::XMVectorGetByIndex(S2, clip_axis) < -offset)
				{
					DirectX::XMVECTOR Delta = DirectX::XMVectorSubtract(S2, S1);
					float w = (-offset - DirectX::XMVectorGetByIndex(S1, clip_axis)) / DirectX::XMVectorGetByIndex(Delta, clip_axis);
					S2 = DirectX::XMVectorAdd(S1, DirectX::XMVectorScale(Delta, w));
				}
				else if (DirectX::XMVectorGetByIndex(S1, clip_axis) < -offset && DirectX::XMVectorGetByIndex(S2, clip_axis) > -offset)
				{
					DirectX::XMVECTOR Delta = DirectX::XMVectorSubtract(S1, S2);
					float w = (-offset - DirectX::XMVectorGetByIndex(S2, clip_axis)) / DirectX::XMVectorGetByIndex(Delta, clip_axis);
					S1 = DirectX::XMVectorAdd(S2, DirectX::XMVectorScale(Delta, w));
				}
			}

			DirectX::XMVECTOR Normal = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			float element = DirectX::XMVectorGetByIndex(t, pen_axis) > 0.0f ? 1.0f : -1.0f;
			Normal = DirectX::XMVectorSetByIndex(Normal, element, pen_axis);

			float diff = DirectX::XMVectorGetByIndex(S1, pen_axis) - DirectX::XMVectorGetByIndex(S2, pen_axis);

			DirectX::XMVECTOR Insert;
			if (fabs(diff) < FLT_EPSILON)
			{
				Insert = DirectX::XMVectorScale(DirectX::XMVectorAdd(S1, S2), 0.5f);
				Insert = DirectX::XMVectorSubtract(Insert, DirectX::XMVectorScale(Normal, capsule.get_radius()));
				Insert = DirectX::XMVectorAdd(Insert, DirectX::XMLoadFloat3(&get_position()));
			}
			else if (diff < 0.0f)
			{
				Insert = DirectX::XMVectorSubtract(S1, DirectX::XMVectorScale(Normal, capsule.get_radius()));
				Insert = DirectX::XMVectorAdd(Insert, DirectX::XMLoadFloat3(&get_position()));
			}
			else
			{
				Insert = DirectX::XMVectorSubtract(S2, DirectX::XMVectorScale(Normal, capsule.get_radius()));
				Insert = DirectX::XMVectorAdd(Insert, DirectX::XMLoadFloat3(&get_position()));
			}
			DirectX::XMFLOAT3 insert, normal;
			DirectX::XMStoreFloat3(&insert, Insert);
			DirectX::XMStoreFloat3(&normal, Normal);
			intersect->add(insert, normal, pen_depth);
		}
		else
		{
			pen_axis -= 3;
			if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(t, Axes[pen_axis])) < 0.0f)
			{
				Axes[pen_axis] = DirectX::XMVectorNegate(Axes[pen_axis]);
			}

			float pen_axis_value = DirectX::XMVectorGetByIndex(e, pen_axis);

			DirectX::XMVECTOR scaled_axis = DirectX::XMVectorDivide(Axes[pen_axis], e);

			size_t max_axis = 0;
			if (fabsf(DirectX::XMVectorGetY(scaled_axis)) > fabsf(DirectX::XMVectorGetX(scaled_axis)))
				max_axis = 1;
			if (fabsf(DirectX::XMVectorGetZ(scaled_axis)) > fabsf(DirectX::XMVectorGetByIndex(scaled_axis, max_axis)))
				max_axis = 2;

			float max_axis_value = DirectX::XMVectorGetByIndex(scaled_axis, max_axis);
			max_axis_value = max_axis_value < 0.0f ? -DirectX::XMVectorGetByIndex(e, max_axis) : DirectX::XMVectorGetByIndex(e, max_axis);

			size_t third_axis = 3 - (pen_axis | max_axis);

			float third_value = DirectX::XMVectorGetByIndex(Axes[pen_axis], third_axis);
			third_value = third_value < 0.0f ? -DirectX::XMVectorGetByIndex(e, third_axis) : DirectX::XMVectorGetByIndex(e, third_axis);

			DirectX::XMVECTOR P1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			P1 = DirectX::XMVectorSetByIndex(P1, max_axis_value, max_axis);
			P1 = DirectX::XMVectorSetByIndex(P1, pen_axis_value, pen_axis);
			P1 = DirectX::XMVectorSetByIndex(P1, third_value, third_axis);

			DirectX::XMVECTOR Q1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			Q1 = DirectX::XMVectorSetByIndex(Q1, max_axis_value, max_axis);
			Q1 = DirectX::XMVectorSetByIndex(Q1, -pen_axis_value, pen_axis);
			Q1 = DirectX::XMVectorSetByIndex(Q1, third_value, third_axis);

			DirectX::XMFLOAT3 s1, s2, p1, q1;
			DirectX::XMStoreFloat3(&s1, S1);
			DirectX::XMStoreFloat3(&s2, S2);
			DirectX::XMStoreFloat3(&p1, P1);
			DirectX::XMStoreFloat3(&q1, Q1);
			line line1(line::types::segment, s1, p1);
			line line2(line::types::segment, s2, q1);

			DirectX::XMFLOAT3 cp_aabb, cp_cap;
			float length_aabb, length_cap;
			line1.calculate_distance(cp_cap, length_cap, cp_aabb, length_aabb, line2);

			DirectX::XMVECTOR CP_AABB, CP_CAP;
			CP_AABB = DirectX::XMLoadFloat3(&cp_aabb);
			CP_CAP = DirectX::XMLoadFloat3(&cp_cap);
			P1 = DirectX::XMVectorSubtract(CP_CAP, DirectX::XMVectorScale(Axes[pen_axis], capsule.get_radius()));
			P1 = DirectX::XMVectorAdd(P1, DirectX::XMLoadFloat3(&get_position()));
			DirectX::XMStoreFloat3(&p1, P1);

			DirectX::XMFLOAT3 axes;
			DirectX::XMStoreFloat3(&axes, Axes[pen_axis]);
			intersect->add(p1, axes, pen_depth);
		}

		for (size_t index = 0; index < intersect->size(); ++index)
		{
			(*intersect)[index].normal.x = -(*intersect)[index].normal.x;
			(*intersect)[index].normal.y = -(*intersect)[index].normal.y;
			(*intersect)[index].normal.z = -(*intersect)[index].normal.z;
		}
	}

	return true;
}

bool inner::axis_aligned_bounding_box::intersect_box(const bounding_box& box, intersect_data* intersect) const
{
	//	未対応
	return false;
}

bool inner::axis_aligned_bounding_box::intersect_box(const axis_aligned_bounding_box& box) const
{
	DirectX::BoundingBox self_aabb = DirectX::BoundingBox(get_position(), get_local_scale());
	DirectX::BoundingBox box_aabb = DirectX::BoundingBox(box.get_position(), box.get_local_scale());
	return self_aabb.Intersects(box_aabb);
}

bool inner::axis_aligned_bounding_box::intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect) const
{
	//	未対応
	return false;
}

void inner::axis_aligned_bounding_box::include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const
{
	DirectX::XMVECTOR Min, Max;
	include_corner(Min, Max);
	DirectX::XMStoreFloat3(&min, Min);
	DirectX::XMStoreFloat3(&max, Max);
}

void inner::axis_aligned_bounding_box::include_corner(DirectX::XMVECTOR& Min, DirectX::XMVECTOR& Max) const
{
	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR Scale = DirectX::XMLoadFloat3(&get_local_scale());
	Min = DirectX::XMVectorSubtract(Position, Scale);
	Max = DirectX::XMVectorAdd(Position, Scale);
}

void inner::axis_aligned_bounding_box::calculate_vertex(DirectX::XMFLOAT3* vertices, size_t vertex_count) const
{
	if (!vertices)
		return;

	DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR X = DirectX::XMVectorSet(get_local_scale().x, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR Y = DirectX::XMVectorSet(0.0f, get_local_scale().y, 0.0f, 0.0f);
	DirectX::XMVECTOR Z = DirectX::XMVectorSet(0.0f, 0.0f, get_local_scale().z, 0.0f);
	DirectX::XMVECTOR AddPpX = DirectX::XMVectorAdd(P, X);
	DirectX::XMVECTOR AddPnX = DirectX::XMVectorSubtract(P, X);
	DirectX::XMVECTOR AddYpZ = DirectX::XMVectorAdd(Y, Z);
	DirectX::XMVECTOR AddYnZ = DirectX::XMVectorSubtract(Y, Z);
	DirectX::XMStoreFloat3(&vertices[0], DirectX::XMVectorAdd(AddPnX, AddYnZ));
	DirectX::XMStoreFloat3(&vertices[1], DirectX::XMVectorAdd(AddPpX, AddYnZ));
	DirectX::XMStoreFloat3(&vertices[2], DirectX::XMVectorAdd(AddPnX, AddYpZ));
	DirectX::XMStoreFloat3(&vertices[3], DirectX::XMVectorAdd(AddPpX, AddYpZ));

	DirectX::XMStoreFloat3(&vertices[4], DirectX::XMVectorSubtract(AddPnX, AddYnZ));
	DirectX::XMStoreFloat3(&vertices[5], DirectX::XMVectorSubtract(AddPpX, AddYnZ));
	DirectX::XMStoreFloat3(&vertices[6], DirectX::XMVectorSubtract(AddPnX, AddYpZ));
	DirectX::XMStoreFloat3(&vertices[7], DirectX::XMVectorSubtract(AddPpX, AddYpZ));
}

namespace
{
	static constexpr size_t TriFace[3 * 2 * 6] =
	{
		0, 1, 2,	1, 3, 2,	//!<	上
		4, 6, 5,	5, 6, 7,	//!<	下
		0, 4, 1,	1, 4, 5,	//!<	左
		1, 5, 3,	3, 5, 7,	//!<	後
		3, 6, 2,	3, 7, 6,	//!<	右
		2, 6, 0,	0, 6, 4		//!<	前
	};

	static constexpr size_t PlaneFace[4 * 6] =
	{
		0, 1, 2, 3,	//!<	上
		4, 6, 5, 7,	//!<	下
		0, 4, 1, 5,	//!<	左
		1, 5, 3, 7,	//!<	後
		3, 6, 2, 7,	//!<	右
		2, 6, 0, 4,	//!<	前
	};

	#define	FLT_EPSILON	1.192092896e-07F
	void	clipPolygonToPlaneNegative(const DirectX::XMVECTOR& planeN, float planeOffset,
									   const DirectX::XMVECTOR* verts, size_t vertCount,
									   DirectX::XMVECTOR* out, size_t& outCount)
	{
		constexpr size_t START_IN = 0x0;
		constexpr size_t START_ON = 0x1;
		constexpr size_t START_OUT = 0x2;

		constexpr size_t END_IN = 0x0;
		constexpr size_t END_ON = 0x4;
		constexpr size_t END_OUT = 0x8;

		outCount = 0;

		switch (std::min(vertCount, 3llu))
		{
			case 0:
			{
				return;
			}
			case 1:
			{
				float offset = DirectX::XMVectorGetX(DirectX::XMVector3Dot(verts[0], planeN)) - planeOffset;
				if (offset <= FLT_EPSILON)
				{
					out[0] = verts[0];
					outCount = 1;
				}
				break;
			}
			case 2:
			{
				const DirectX::XMVECTOR& start = verts[0];
				const DirectX::XMVECTOR& end = verts[1];
			
				float startOffset = DirectX::XMVectorGetX(DirectX::XMVector3Dot(start, planeN)) - planeOffset;
				float endOffset = DirectX::XMVectorGetX(DirectX::XMVector3Dot(end, planeN)) - planeOffset;

				size_t result = START_IN | END_IN;

				if( fabs(startOffset) <= FLT_EPSILON )
					result |= START_ON;
				else if( startOffset > FLT_EPSILON )
					result |= START_OUT;

				if( fabs(endOffset) <= FLT_EPSILON )
					result |= END_ON;
				else if( endOffset > FLT_EPSILON )
					result |= END_OUT;

				switch (result)
				{
					case (START_IN | END_OUT):
					{
						DirectX::XMVECTOR edge = DirectX::XMVectorSubtract(end, start);
						float d = -startOffset / DirectX::XMVectorGetX(DirectX::XMVector3Dot(edge, planeN));

						out[0] = start;
						out[1] = DirectX::XMVectorAdd(start, DirectX::XMVectorScale(edge, d));
						outCount = 2;
						break;				
					}
					case (START_OUT | END_IN):
					{
						DirectX::XMVECTOR edge = DirectX::XMVectorSubtract(end, start);
						float d = -startOffset / DirectX::XMVectorGetX(DirectX::XMVector3Dot(edge, planeN));

						out[0] = DirectX::XMVectorAdd(start, DirectX::XMVectorScale(edge, d));
						out[1] = end;
						outCount = 2;
					}
					case ( START_OUT | END_OUT ):
					{
						break;
					}
					case ( START_ON | END_OUT ):
					{
						out[0] = start;
						outCount = 1;
						break;
					}
					case ( START_OUT | END_ON ):
					{
						out[0] = end;
						outCount = 1;
					}
					break;
					default:
					{
						out[0] = start;
						out[1] = end;
						outCount = 2;
					}
					break;
				}	
				break;	
			}
			default:
			{
				DirectX::XMVECTOR start = verts[vertCount-1];

				for (size_t index = 0; index < vertCount; ++index)
				{
					const DirectX::XMVECTOR& end = verts[index];

					float startOffset = DirectX::XMVectorGetX(DirectX::XMVector3Dot(start, planeN)) - planeOffset;	
					float endOffset = DirectX::XMVectorGetX(DirectX::XMVector3Dot(end, planeN)) - planeOffset;

					size_t result = START_IN | END_IN;

					if (abs(startOffset) <= FLT_EPSILON)
						result |= START_ON;
					else if (startOffset > FLT_EPSILON)
						result |= START_OUT;

					if (abs(endOffset) <= FLT_EPSILON)
						result |= END_ON;
					else if (endOffset > FLT_EPSILON)
						result |= END_OUT;

					switch (result)
					{
						case (START_IN | END_OUT):
						{
							DirectX::XMVECTOR edge = DirectX::XMVectorSubtract(end, start);
							float d = -startOffset / DirectX::XMVectorGetX(DirectX::XMVector3Dot(edge, planeN));

							out[outCount] = DirectX::XMVectorAdd(start, DirectX::XMVectorScale(edge, d));
							++outCount;					
						}
						break;
						case (START_OUT | END_IN):
						{
							DirectX::XMVECTOR edge = DirectX::XMVectorSubtract(end, start);
							float d = -startOffset / DirectX::XMVectorGetX(DirectX::XMVector3Dot(edge, planeN));

							out[outCount] = DirectX::XMVectorAdd(start, DirectX::XMVectorScale(edge, d));
							++outCount;	

							out[outCount] = end;
							++outCount;
						}
						case ( START_OUT | END_OUT ):
						case ( START_ON | END_OUT ):
						{
							break;
						}
						default:
						{
							out[outCount] = end;
							++outCount;
						}
						break;
					}

					start = end;
				}
			}
			break;
		}
	}

	bool face_contact_obb_to_aabb(const bounding_box& obb, const inner::axis_aligned_bounding_box& aabb, size_t axis_index, DirectX::XMVECTOR& InsertPoint)
	{
		DirectX::XMMATRIX ObbMatrix = obb.get_transeform_matrix();
		DirectX::XMVECTOR Axis = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR Delta = DirectX::XMVectorSubtract(ObbMatrix.r[3], DirectX::XMLoadFloat3(&aabb.get_position()));
	
		Axis = DirectX::XMVectorSetByIndex(Axis, DirectX::XMVectorGetByIndex(Delta, axis_index) < 0.0f ? -1.0f : 1.0f, axis_index);

		DirectX::XMVECTOR LocalAxis = DirectX::XMVector3TransformNormal(Axis, ObbMatrix);

		DirectX::XMVECTOR ScaledAxis = DirectX::XMVectorDivide(DirectX::XMVectorNegate(LocalAxis), DirectX::XMLoadFloat3(&obb.get_local_scale()));

		size_t face_axis = 0;
		if (fabs(DirectX::XMVectorGetX(ScaledAxis)) < fabs(DirectX::XMVectorGetY(ScaledAxis)))
		{
			face_axis = 1;
		}
		if (fabs(DirectX::XMVectorGetByIndex(ScaledAxis, face_axis)) < fabs(DirectX::XMVectorGetZ(ScaledAxis)))
		{
			face_axis = 2;
		}

		DirectX::XMVECTOR Vertices[12];
		size_t face_index = DirectX::XMVectorGetByIndex(ScaledAxis, face_axis) < 0.0f ? face_axis : face_axis + 3;

		static const DirectX::XMVECTOR UnitVertices[8] =
		{
			DirectX::XMVectorSet(-1.0f, -1.0f, -1.0f, 0.0f),
			DirectX::XMVectorSet(-1.0f, -1.0f,  1.0f, 0.0f),
			DirectX::XMVectorSet(-1.0f,  1.0f, -1.0f, 0.0f),
			DirectX::XMVectorSet(-1.0f,  1.0f,  1.0f, 0.0f),
			DirectX::XMVectorSet( 1.0f, -1.0f, -1.0f, 0.0f),
			DirectX::XMVectorSet( 1.0f, -1.0f,  1.0f, 0.0f),
			DirectX::XMVectorSet( 1.0f,  1.0f, -1.0f, 0.0f),
			DirectX::XMVectorSet( 1.0f,  1.0f,  1.0f, 0.0f)
		};
		static const size_t UnitPlaneFace[4 * 6] =
		{
			0, 1, 3, 2,	//	上
			1, 0, 4, 5,	//	前
			0, 2, 6, 4,	//	左
			4, 6, 7, 5,	//	下
			2, 3, 7, 6,	//	後
			1, 5, 7, 3,	//	右
		};
		for (size_t index = 0; index < 4; index++)
		{
			Vertices[index] = DirectX::XMVectorMultiply(UnitVertices[UnitPlaneFace[face_index * 4 + index]], DirectX::XMLoadFloat3(&obb.get_local_scale()));

			Vertices[index] = DirectX::XMVector3TransformCoord(Vertices[index], ObbMatrix);
			Vertices[index] = DirectX::XMVectorSubtract(Vertices[index], DirectX::XMLoadFloat3(&obb.get_position()));
		}

		size_t clip_axes[][2]	=
		{
			{ 1, 2 },
			{ 0, 2 },
			{ 0, 1 }
		};

		DirectX::XMVECTOR Buffer[12];
		DirectX::XMVECTOR* Input = Vertices;
		DirectX::XMVECTOR* Output = Buffer;
		size_t input_count = 4;
		size_t output_count = 0;
		for(int a = 0; a < 2; ++a)
		{
			size_t index = clip_axes[axis_index][a];

			DirectX::XMVECTOR PlaneN = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			PlaneN = DirectX::XMVectorSetByIndex(PlaneN, 1.0f, index);
			float planeOffset = (&aabb.get_local_scale().x)[index];
			
			clipPolygonToPlaneNegative(PlaneN, planeOffset, Input, input_count, Output, output_count);

			input_count = output_count;
			output_count = 0;
			DirectX::XMVECTOR* Temp = Input;
			Input = Output;
			Output = Temp;

			PlaneN = DirectX::XMVectorSetByIndex(PlaneN, -1.0f, index);
			clipPolygonToPlaneNegative(PlaneN, planeOffset, Input, input_count, Output, output_count);

			input_count = output_count;
			output_count = 0;
			Temp = Input;
			Input = Output;
			Output = Temp;
		}

		if (input_count == 0)
			return false;

		int size = (int)input_count;
		float distances[12];
		for (size_t index = 0; index < size; ++index)
		{
			distances[index] = DirectX::XMVectorGetByIndex(Input[index], axis_index) * DirectX::XMVectorGetByIndex(Axis, axis_index);
		}

		for (size_t loop = 0; loop < size; ++loop)
		{
			for (size_t index = 0; index < size - 1; ++index)
			{
				if (distances[index] > distances[index + 1])
				{
					std::swap(distances[index], distances[index + 1]);
					std::swap(Input[index], Input[index + 1]);
				}
			}
		}

		DirectX::XMVECTOR Average = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		size_t count = 0;
		for (; count < input_count; ++count)
		{
			if (fabsf(distances[0] - distances[count]) > 0.01f)
				break;

			Average = DirectX::XMVectorAdd(Average, Input[count]);
		}

		Average = DirectX::XMVectorDivide(Average, DirectX::XMVectorSet(static_cast<float>(count), static_cast<float>(count), static_cast<float>(count), static_cast<float>(count)));
		InsertPoint = DirectX::XMVectorAdd(Average, DirectX::XMLoadFloat3(&aabb.get_position()));
		return true;
	}

    DirectX::XMVECTOR InverseTransformPoint(const DirectX::XMVECTOR& a, const DirectX::XMMATRIX& m)
    {
		return	DirectX::XMVector3TransformNormal(DirectX::XMVectorSubtract(a, m.r[3]), m);
    }
}	//	namespace

////////////////////////////////////////////////////////////////////////////////////////////
DirectX::XMMATRIX bounding_box::get_transeform_matrix() const
{
//	DirectX::XMVECTOR Scale = DirectX::XMLoadFloat3(&get_local_scale());
	DirectX::XMVECTOR Rotation = DirectX::XMLoadFloat4(&get_rotation());
	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	return	//DirectX::XMMatrixScalingFromVector(Scale) *
			DirectX::XMMatrixRotationQuaternion(Rotation) *
			DirectX::XMMatrixTranslationFromVector(Position);
}

DirectX::XMFLOAT4X4 bounding_box::get_transeform_float4x4() const
{
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, get_transeform_matrix());
	return matrix;
}

bounding_box::bounding_box()
: bounding_object()
{
}

bounding_box::bounding_box(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& position)
: bounding_object(scale, rotation, position)
{
}

bounding_box::bounding_box(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& y_axis, float angle, const DirectX::XMFLOAT3& position)
: bounding_object(scale, DirectX::XMFLOAT4(0, 0, 0, 1), position)
{
	DirectX::XMVECTOR YAxis = DirectX::XMLoadFloat3(&y_axis);
	DirectX::XMVECTOR Rotation = DirectX::XMQuaternionRotationAxis(YAxis, angle);
	DirectX::XMFLOAT4 rotation;
	DirectX::XMStoreFloat4(&rotation, Rotation);
	set_rotation(rotation);
}

bounding_box::~bounding_box()
{
}

float bounding_box::calculate_point_to_length(const DirectX::XMFLOAT3& point) const
{
	DirectX::XMVECTOR Vector = DirectX::XMVectorSet(0, 0, 0, 0);   // 最終的に長さを求めるベクトル

	DirectX::XMVECTOR Point = DirectX::XMLoadFloat3(&point);
	DirectX::XMVECTOR PositionObb = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&get_rotation()));

	//	各軸についてはみ出た部分のベクトルを算出
	const float* length = &get_local_scale().x;
	for (int axis = 0; axis < 3; axis++)
	{
		if (length[axis] <= 0)
			continue;	//	L = 0は計算できない

		DirectX::XMVECTOR PointToPositionObb = DirectX::XMVectorSubtract(Point, PositionObb);
		DirectX::XMVECTOR ObbAxis = Rotation.r[axis];
		float s = DirectX::XMVectorGetX(DirectX::XMVector3Dot(PointToPositionObb, ObbAxis)) / length[axis];

		//	sの値から、はみ出した部分があればそのベクトルを加算
		s = fabsf(s);
		if (s > 1)
		{
			s = (1 - s) * length[axis];
			//	はみ出した部分のベクトル算出
			Vector = DirectX::XMVectorAdd(Vector,
									DirectX::XMVectorMultiply(ObbAxis, DirectX::XMVectorSet(s, s, s, s)));
		}
	}

	//	長さを出力
	return	DirectX::XMVectorGetX(DirectX::XMVector3Length(Vector));
}

bool bounding_box::intersect_point(const DirectX::XMFLOAT3& point) const
{
	return calculate_point_to_length(point) <= FLT_EPSILON;
}

bool bounding_box::intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect) const
{
	DirectX::BoundingOrientedBox box;
	box.Center = get_position();
	box.Extents = get_local_scale();
	box.Orientation = get_rotation();
	DirectX::XMVECTOR Start = DirectX::XMLoadFloat3(&start);
	DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&end);
	DirectX::XMVECTOR Direction = DirectX::XMVectorSubtract(End, Start);
	float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(Direction));
	Direction = DirectX::XMVector3Normalize(Direction);
	float new_distance;
	bool hit = box.Intersects(Start, Direction, new_distance);
	if (hit)
	{
		if (distance < new_distance)
		{
			hit = false;
		}
		if (hit && intersect)
		{
			End = DirectX::XMVectorSet(new_distance, new_distance, new_distance, new_distance);
			DirectX::XMFLOAT3 position, direction;
			DirectX::XMStoreFloat3(&position, DirectX::XMVectorMultiplyAdd(Direction, End, Start));
			DirectX::XMStoreFloat3(&direction, DirectX::XMVectorNegate(Direction));
			intersect->add(position, direction, new_distance);
		}
	}
	return hit;
}

bool bounding_box::intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect) const
{
	DirectX::XMMATRIX TransformMatrix = get_transeform_matrix();
	DirectX::XMMATRIX InverseTransformMatrix = DirectX::XMMatrixInverse(nullptr, TransformMatrix);

	DirectX::XMVECTOR LocalPoint;
	LocalPoint = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&point), InverseTransformMatrix);

	DirectX::XMFLOAT3 new_point;
	DirectX::XMStoreFloat3(&new_point, LocalPoint);

	inner::axis_aligned_bounding_box aabb(get_local_scale(), {0.0f, 0.0f, 0.0f});
	bool contact = aabb.intersect_sphere(new_point, radius, intersect);
	if (contact && intersect)
	{
		for (size_t index = 0; index < intersect->size(); ++index)
		{
			intersect->multiply_matrix(index, TransformMatrix);
		}
	}
	return contact;
}

bool bounding_box::intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect) const
{
	return	intersect_sphere(sphere.get_position(), sphere.get_radius(), intersect);
}

bool bounding_box::intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect) const
{
	DirectX::XMMATRIX TransformMatrix = get_transeform_matrix();
	DirectX::XMMATRIX InverseTransformMatrix = DirectX::XMMatrixInverse(nullptr, TransformMatrix);
	DirectX::XMMATRIX CapsuleTransformMatrix, InverseCapsuleRotationMatrix;
	CapsuleTransformMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&capsule.get_rotation()))
						* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&capsule.get_position()));
	InverseCapsuleRotationMatrix = DirectX::XMMatrixMultiply(CapsuleTransformMatrix, InverseTransformMatrix);

	DirectX::XMVECTOR Dummy, LocalCapsuleRotation, LocalCapsulePosition;
	DirectX::XMMatrixDecompose(&Dummy , &LocalCapsuleRotation, &LocalCapsulePosition, InverseCapsuleRotationMatrix);

	DirectX::XMFLOAT3 local_capsule_position;
	DirectX::XMStoreFloat3(&local_capsule_position, LocalCapsulePosition);
	DirectX::XMFLOAT4 local_rotation;
	DirectX::XMStoreFloat4(&local_rotation, LocalCapsuleRotation);
	bounding_capsule local_capsule(local_capsule_position, local_rotation, capsule.get_radius(), capsule.get_length() * 0.5f );
	inner::axis_aligned_bounding_box aabb(get_local_scale(), {0.0f, 0.0f, 0.0f});
	bool contact = aabb.intersect_capsule(local_capsule, intersect);
	if (contact && intersect)
	{
		for (size_t index = 0; index < intersect->size(); ++index)
		{
			intersect->multiply_matrix(index, TransformMatrix);
		}
	}
	return contact;
}

bool bounding_box::intersect_box(const bounding_box& box, intersect_data* intersect) const
{
	DirectX::XMMATRIX xformA = get_transeform_matrix();
	DirectX::XMVECTOR extentsA = DirectX::XMLoadFloat3(&get_local_scale());

	DirectX::XMMATRIX xformB = box.get_transeform_matrix();
	DirectX::XMVECTOR extentsB = DirectX::XMLoadFloat3(&box.get_local_scale());

	DirectX::XMFLOAT3X3 r;
	r.m[0][0] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(xformA.r[0], xformB.r[0]));
	r.m[0][1] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(xformA.r[0], xformB.r[1]));
	r.m[0][2] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(xformA.r[0], xformB.r[2]));

	r.m[1][0] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(xformA.r[1], xformB.r[0]));
	r.m[1][1] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(xformA.r[1], xformB.r[1]));
	r.m[1][2] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(xformA.r[1], xformB.r[2]));

	r.m[2][0] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(xformA.r[2], xformB.r[0]));
	r.m[2][1] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(xformA.r[2], xformB.r[1]));
	r.m[2][2] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(xformA.r[2], xformB.r[2]));

	DirectX::XMVECTOR t = DirectX::XMVectorSubtract(xformB.r[3], xformA.r[3]);
	t = DirectX::XMVectorSet(DirectX::XMVectorGetX(DirectX::XMVector3Dot(t, xformA.r[0])),
							 DirectX::XMVectorGetX(DirectX::XMVector3Dot(t, xformA.r[1])),
							 DirectX::XMVectorGetX(DirectX::XMVector3Dot(t, xformA.r[2])),
							 0.0f);

	DirectX::XMFLOAT3X3 abs_r;
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			abs_r.m[i][j] = abs( r.m[i][j] ) + FLT_EPSILON;
		}
	}

	float penetration_depth = FLT_MAX;
	DirectX::XMFLOAT3 axis(0.0f, 0.0f, 0.0f);
	size_t world_axis = 0;
	for (size_t index = 0; index < 3; ++index)
	{
		float	ra	= DirectX::XMVectorGetByIndex(extentsA, index);
		float	rb	= DirectX::XMVectorGetByIndex(extentsB, 0) * abs_r.m[index][0]
					+ DirectX::XMVectorGetByIndex(extentsB, 1) * abs_r.m[index][1]
					+ DirectX::XMVectorGetByIndex(extentsB, 2) * abs_r.m[index][2];

		float d = fabsf(DirectX::XMVectorGetByIndex(t, index));

		float pd = (ra + rb) - d;
			
		if (pd <= 0.0f)
			return false;

		if (pd < penetration_depth)
		{
			penetration_depth = pd;
			axis = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			(&axis.x)[index] = 1.0f;
			world_axis = index;
		}
	}

	for (size_t index = 0; index < 3; ++index)
	{
		float ra = DirectX::XMVectorGetX(extentsA) * abs_r.m[0][index]
				 + DirectX::XMVectorGetY(extentsA) * abs_r.m[1][index]
				 + DirectX::XMVectorGetZ(extentsA) * abs_r.m[2][index];
		float rb = DirectX::XMVectorGetByIndex(extentsB, index);

		float d = fabsf(DirectX::XMVectorGetX(t) * r.m[0][index] + DirectX::XMVectorGetY(t) * r.m[1][index] + DirectX::XMVectorGetZ(t) * r.m[2][index]);
		float pd = (ra + rb) - d;

		if (pd <= 0.0f)
			return false;

		if (pd < penetration_depth)
		{
			penetration_depth = pd;
			axis = DirectX::XMFLOAT3(r.m[0][index], r.m[1][index], r.m[2][index]);
			world_axis = index + 3;
		}
	}

	DirectX::XMFLOAT3 axes[9];
	float pds[9] = {0.0f};

	for (size_t indexA = 0; indexA < 3; ++indexA)
	{
		DirectX::XMFLOAT3 axisA(0.0f, 0.0f, 0.0f);
		(&axisA.x)[indexA] = 1.0f;

		for (size_t indexB = 0; indexB < 3; ++indexB)
		{
			DirectX::XMFLOAT3 axisB(r.m[0][indexB], r.m[1][indexB], r.m[2][indexB]);

			axisB.x += (axisB.x < 0.0f) ? -FLT_EPSILON : FLT_EPSILON;
			axisB.y += (axisB.y < 0.0f) ? -FLT_EPSILON : FLT_EPSILON;
			axisB.z += (axisB.z < 0.0f) ? -FLT_EPSILON : FLT_EPSILON;

			DirectX::XMFLOAT3& current_axis = axes[indexA * 3 + indexB];
			DirectX::XMVECTOR AxisA = DirectX::XMLoadFloat3(&axisA);
			DirectX::XMVECTOR AxisB = DirectX::XMLoadFloat3(&axisB);
			DirectX::XMStoreFloat3(&current_axis, DirectX::XMVector3Cross(AxisA, AxisB));

			float ra = DirectX::XMVectorGetX(extentsA) * fabsf(current_axis.x)
					 + DirectX::XMVectorGetY(extentsA) * fabsf(current_axis.y)
					 + DirectX::XMVectorGetZ(extentsA) * fabsf(current_axis.z);

			float rb = DirectX::XMVectorGetX(extentsB) * fabsf(current_axis.x * r.m[0][0] + current_axis.y * r.m[1][0] + current_axis.z * r.m[2][0])
					 + DirectX::XMVectorGetY(extentsB) * fabsf(current_axis.x * r.m[0][1] + current_axis.y * r.m[1][1] + current_axis.z * r.m[2][1])
					 + DirectX::XMVectorGetZ(extentsB) * fabsf(current_axis.x * r.m[0][2] + current_axis.y * r.m[1][2] + current_axis.z * r.m[2][2]);

			float& pd = pds[indexA * 3 + indexB];

			pd = (ra + rb) - fabsf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(t, DirectX::XMLoadFloat3(&current_axis))));

			if (pd <= 0.0f)
				return false;
		}
	}

	if (intersect)
	{
		size_t axisA = -1;
		size_t axisB = -1;
		for (size_t indexA = 0; indexA < 3; ++indexA)
		{
			for (size_t indexB = 0; indexB < 3; ++indexB)
			{
				size_t index = indexA * 3 + indexB;

				DirectX::XMFLOAT3& current_axis = axes[index];
				float& pd = pds[index];

				float mag = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&current_axis)));

				if (mag < FLT_EPSILON)
					continue;

				float inv_mag = 1.0f / mag;
				pd *= inv_mag;

				if (pd <= 0.01f)
					return false;
					
				if (pd <= 0.0f)
					return false;

				if (pd < penetration_depth)
				{
					penetration_depth = pd;
					axis.x = current_axis.x * inv_mag;
					axis.y = current_axis.y * inv_mag;
					axis.z = current_axis.z * inv_mag;
					axisA = indexA;
					axisB = indexB;
				}
			}
		}

		if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&axis), t)) > 0.0f)
		{
			DirectX::XMStoreFloat3(&axis, DirectX::XMVectorNegate(DirectX::XMLoadFloat3(&axis)));
		}
			
		//	線と線
		if (axisA != -1)
		{
			size_t world_axisA = axisA;
			size_t world_axisB = axisB;

			DirectX::XMVECTOR PA = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
							  QA = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
							  PB = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
							  QB = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			DirectX::XMVECTOR ScaledAxisA = DirectX::XMVectorSet(-axis.x / DirectX::XMVectorGetX(extentsA),
																 -axis.y / DirectX::XMVectorGetY(extentsA),
																 -axis.z / DirectX::XMVectorGetZ(extentsA),
																 0.0f);
			size_t face_axisA = 0;
			if (fabsf(DirectX::XMVectorGetX(ScaledAxisA)) < fabsf(DirectX::XMVectorGetY(ScaledAxisA)) )
				face_axisA = 1;
			if (fabsf(DirectX::XMVectorGetByIndex(ScaledAxisA, face_axisA)) < fabsf(DirectX::XMVectorGetZ(ScaledAxisA)))
				face_axisA = 2;

			PA = DirectX::XMVectorSetByIndex(PA, +DirectX::XMVectorGetByIndex(extentsA, world_axisA), world_axisA);
			QA = DirectX::XMVectorSetByIndex(QA, -DirectX::XMVectorGetByIndex(extentsA, world_axisA), world_axisA);

			float work = DirectX::XMVectorGetByIndex(ScaledAxisA, face_axisA) < 0.0f ? -DirectX::XMVectorGetByIndex(extentsA, face_axisA) : DirectX::XMVectorGetByIndex(extentsA, face_axisA);
			PA = DirectX::XMVectorSetByIndex(PA, work, face_axisA);
			QA = DirectX::XMVectorSetByIndex(QA, work, face_axisA);

			size_t third_axis = 3 - (world_axisA | face_axisA);

			work = DirectX::XMVectorGetByIndex(ScaledAxisA, third_axis) < 0.0f ? -DirectX::XMVectorGetByIndex(extentsA, third_axis) : DirectX::XMVectorGetByIndex(extentsA, third_axis);
			PA = DirectX::XMVectorSetByIndex(PA, work, third_axis);
			QA = DirectX::XMVectorSetByIndex(QA, work, third_axis);

			DirectX::XMVECTOR ScaledAxisB;
			ScaledAxisA	= DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&axis), xformA);
			ScaledAxisB	= DirectX::XMVector3TransformNormal(ScaledAxisA, xformB);

			ScaledAxisB = DirectX::XMVectorDivide(ScaledAxisB, extentsB);

			size_t face_axisB = 0;
			if (fabsf(DirectX::XMVectorGetX(ScaledAxisB)) < fabsf(DirectX::XMVectorGetY(ScaledAxisB)) )
				face_axisB = 1;
			if (fabsf(DirectX::XMVectorGetByIndex(ScaledAxisB, face_axisB)) < fabsf(DirectX::XMVectorGetZ(ScaledAxisB)))
				face_axisB = 2;

			PB = DirectX::XMVectorSetByIndex(PB, +DirectX::XMVectorGetByIndex(extentsB, world_axisB), world_axisB);
			QB = DirectX::XMVectorSetByIndex(QB, -DirectX::XMVectorGetByIndex(extentsB, world_axisB), world_axisB);

			work = DirectX::XMVectorGetByIndex(ScaledAxisB, face_axisB) < 0.0f ? -DirectX::XMVectorGetByIndex(extentsB, face_axisB) : DirectX::XMVectorGetByIndex(extentsB, face_axisB);
			PB = DirectX::XMVectorSetByIndex(PB, work, face_axisB);
			QB = DirectX::XMVectorSetByIndex(QB, work, face_axisB);

			third_axis = 3 - (world_axisB | face_axisB);

			work = DirectX::XMVectorGetByIndex(ScaledAxisB, third_axis) < 0.0f ? -DirectX::XMVectorGetByIndex(extentsB, third_axis) : DirectX::XMVectorGetByIndex(extentsB, third_axis);
			PB = DirectX::XMVectorSetByIndex(PB, work, third_axis);
			QB = DirectX::XMVectorSetByIndex(QB, work, third_axis);

			PB = DirectX::XMVector3TransformCoord(PB, xformB);
			QB = DirectX::XMVector3TransformCoord(QB, xformB);
			PB = InverseTransformPoint(PB, xformA);
			QB = InverseTransformPoint(QB, xformA);

			DirectX::XMFLOAT3 pA, pB, qA, qB;
			DirectX::XMStoreFloat3(&pA, PA);
			DirectX::XMStoreFloat3(&pB, PB);
			DirectX::XMStoreFloat3(&qA, QA);
			DirectX::XMStoreFloat3(&qB, QB);

			line line1(line::types::segment, pA, qA);
			line line2(line::types::segment, pB, qB);

			DirectX::XMFLOAT3 cpA, cpB;
			float deltaA, deltaB;
			line1.calculate_distance(cpA, deltaA, cpB, deltaB, line2);
			DirectX::XMVECTOR CPA = DirectX::XMLoadFloat3(&cpA);
			DirectX::XMVECTOR CPB = DirectX::XMLoadFloat3(&cpB);
			DirectX::XMStoreFloat3(&cpA, DirectX::XMVector3TransformCoord(CPA, xformA));
			CPB = XMVector3TransformNormal(DirectX::XMLoadFloat3(&axis), xformA);
			DirectX::XMStoreFloat3(&cpB, DirectX::XMVector3Normalize(CPB));
			intersect->add(cpA, cpB, penetration_depth);
		}
		else	//	線と面 / 面と面
		{
			if (world_axis < 3)
			{
				bounding_box local_obb = box.multiply_obb(get_transeform_matrix());
				inner::axis_aligned_bounding_box local_aabb(get_local_scale(), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

				DirectX::XMVECTOR ContactPt;
				if (!face_contact_obb_to_aabb(local_obb, local_aabb, world_axis, ContactPt))
					return	false;

				ContactPt = DirectX::XMVector3TransformCoord(ContactPt, get_transeform_matrix());
				ContactPt = DirectX::XMVectorSubtract(ContactPt, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&axis), penetration_depth));
				DirectX::XMFLOAT3 contactPt, normal;
				DirectX::XMStoreFloat3(&contactPt, ContactPt);
				DirectX::XMStoreFloat3(&normal, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&axis), xformA));
				intersect->add(contactPt, normal, penetration_depth);
			}
			else
			{
				world_axis -= 3;

				bounding_box local_obb = box.multiply_obb(box.get_transeform_matrix());
				inner::axis_aligned_bounding_box local_aabb(box.get_local_scale(), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

				DirectX::XMVECTOR ContactPt;
				if (!face_contact_obb_to_aabb(local_obb, local_aabb, world_axis, ContactPt))
					return	false;

				ContactPt = DirectX::XMVector3TransformCoord(ContactPt, box.get_transeform_matrix());
				DirectX::XMFLOAT3 contactPt, normal;
				DirectX::XMStoreFloat3(&contactPt, ContactPt);
				DirectX::XMStoreFloat3(&normal, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&axis), xformA));
				intersect->add(contactPt, normal, penetration_depth);
			}
		}
	}
	return true;
}

bool bounding_box::intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect) const
{
	return	mesh.intersect_box(*this, intersect);
}

void bounding_box::include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const
{
	DirectX::XMFLOAT3 vertex[8];
	calculate_vertex(vertex, 8);
	min = vertex[0];
	max = vertex[0];
	for (size_t index = 1; index < 8; ++index)
	{
		min.x	= std::min(vertex[index].x, min.x);
		min.y	= std::min(vertex[index].y, min.y);
		min.z	= std::min(vertex[index].z, min.z);
		max.x	= std::max(vertex[index].x, max.x);
		max.y	= std::max(vertex[index].y, max.y);
		max.z	= std::max(vertex[index].z, max.z);
	}
}

bounding_box bounding_box::multiply_obb(const DirectX::XMMATRIX& matrix) const
{
	DirectX::XMMATRIX OBBTransformMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&get_rotation()))
										 * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&get_position()));
	DirectX::XMFLOAT4X4 rotation_matrix;
	DirectX::XMVECTOR Work;
	Work = InverseTransformPoint(OBBTransformMatrix.r[3], matrix);
	rotation_matrix._41	= DirectX::XMVectorGetX(Work);
	rotation_matrix._42	= DirectX::XMVectorGetY(Work);
	rotation_matrix._43	= DirectX::XMVectorGetZ(Work);
	rotation_matrix._44	= 1.0f;

	Work = InverseTransformPoint(OBBTransformMatrix.r[0], matrix);
	rotation_matrix._11	= DirectX::XMVectorGetX(Work);
	rotation_matrix._12	= DirectX::XMVectorGetY(Work);
	rotation_matrix._13	= DirectX::XMVectorGetZ(Work);
	rotation_matrix._14	= 0.0f;

	Work = InverseTransformPoint(OBBTransformMatrix.r[1], matrix);
	rotation_matrix._21	= DirectX::XMVectorGetX(Work);
	rotation_matrix._22	= DirectX::XMVectorGetY(Work);
	rotation_matrix._23	= DirectX::XMVectorGetZ(Work);
	rotation_matrix._24	= 0.0f;

	Work = InverseTransformPoint(OBBTransformMatrix.r[2], matrix);
	rotation_matrix._31	= DirectX::XMVectorGetX(Work);
	rotation_matrix._32	= DirectX::XMVectorGetY(Work);
	rotation_matrix._33	= DirectX::XMVectorGetZ(Work);
	rotation_matrix._34	= 0.0f;

	DirectX::XMVECTOR Scale, Rotation, Position;
	DirectX::XMMatrixDecompose(&Scale, &Rotation, &Position, DirectX::XMLoadFloat4x4(&rotation_matrix));
	DirectX::XMFLOAT4 rotation;
	DirectX::XMStoreFloat4(&rotation, Rotation);
	DirectX::XMFLOAT3 position;
	DirectX::XMStoreFloat3(&position, Position);
	return bounding_box(get_local_scale(), rotation, position);
}

bounding_box bounding_box::multiply_obb(const DirectX::XMFLOAT4X4& matrix) const
{
	return multiply_obb(DirectX::XMLoadFloat4x4(&matrix));
}

void bounding_box::calculate_vertex(DirectX::XMFLOAT3* vertices, size_t vertex_count) const
{
	if (!vertices)
		return;

	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&get_rotation()));
	RotationMatrix.r[0] = DirectX::XMVectorMultiply(RotationMatrix.r[0], DirectX::XMVectorSet(get_local_scale().x, get_local_scale().x, get_local_scale().x, 1.0f));
	RotationMatrix.r[1] = DirectX::XMVectorMultiply(RotationMatrix.r[1], DirectX::XMVectorSet(get_local_scale().y, get_local_scale().y, get_local_scale().y, 1.0f));
	RotationMatrix.r[2] = DirectX::XMVectorMultiply(RotationMatrix.r[2], DirectX::XMVectorSet(get_local_scale().z, get_local_scale().z, get_local_scale().z, 1.0f));

	DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&get_position());
	DirectX::XMVECTOR Vertices[8];
	DirectX::XMVECTOR PnX = DirectX::XMVectorSubtract(Position, RotationMatrix.r[0]);
	DirectX::XMVECTOR PpX = DirectX::XMVectorAdd(Position, RotationMatrix.r[0]);
	DirectX::XMVECTOR YnZ = DirectX::XMVectorSubtract(RotationMatrix.r[1], RotationMatrix.r[2]);
	DirectX::XMVECTOR YpZ = DirectX::XMVectorAdd(RotationMatrix.r[1], RotationMatrix.r[2]);
	Vertices[0]	= DirectX::XMVectorAdd(PnX, YnZ);
	Vertices[1]	= DirectX::XMVectorAdd(PpX, YnZ);
	Vertices[2] = DirectX::XMVectorAdd(PpX, YpZ);
	Vertices[3]	= DirectX::XMVectorAdd(PnX, YpZ);

	Vertices[4] = DirectX::XMVectorSubtract(PnX, YpZ);
	Vertices[5] = DirectX::XMVectorSubtract(PpX, YpZ);
	Vertices[6] = DirectX::XMVectorSubtract(PpX, YnZ);
	Vertices[7] = DirectX::XMVectorSubtract(PnX, YnZ);
	for (size_t index = 0; index < vertex_count; ++index)
	{
		DirectX::XMStoreFloat3(vertices + index, Vertices[index]);
	}
}
