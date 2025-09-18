#include "collision/mesh.h"

#include "collision/sphere.h"
#include "collision/capsule.h"
#include "collision/box.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/unordered_map.hpp"

#include <DirectXCollision.h>
#include <thread>

namespace
{
	inline bool plane_aabb(const DirectX::XMFLOAT3& n, const float d, const DirectX::XMFLOAT3& c, const DirectX::XMFLOAT3& h)
	{
		float e = h.x * fabsf(n.x)
				+ h.y * fabsf(n.y)
				+ h.z * fabsf(n.z);
		float s = (c.x * n.x + c.y * n.y + c.z * n.z) + d;
		if (s - e > 0) return false; // 半空間外(法線の逆側)
		if (s + e < 0) return false; // 半空間内(法線側)
		return true;
	}

	inline bool triangle_aabb(const DirectX::XMFLOAT3& u0,
							  const DirectX::XMFLOAT3& u1,
							  const DirectX::XMFLOAT3& u2,
							  const DirectX::XMFLOAT3& n,
							  const collision::inner::axis_aligned_bounding_box& box)
	{
		// 三角形の頂点をAABB中心を原点とした座標系に移動
		DirectX::XMFLOAT3 v[3];
		v[0].x = u0.x - box.get_position().x;
		v[0].y = u0.y - box.get_position().y;
		v[0].z = u0.z - box.get_position().z;
		v[1].x = u1.x - box.get_position().x;
		v[1].y = u1.y - box.get_position().y;
		v[1].z = u1.z - box.get_position().z;
		v[2].x = u2.x - box.get_position().x;
		v[2].y = u2.y - box.get_position().y;
		v[2].z = u2.z - box.get_position().z;

#if 0
		// 1. 三角形のAABBとAABBの交差判定
		DirectX::XMFLOAT3 bmin, bmax;
		bmin.x = -box.get_local_scale().x;
		bmin.y = -box.get_local_scale().y;
		bmin.z = -box.get_local_scale().z;
		bmax = box.get_local_scale();
		
		DirectX::XMFLOAT3 tmin = v[0], tmax = v[0];
		for(int j = 1; j < 3; ++j){
			if(v[j].x < tmin.x) tmin.x = v[j].x;
			if(v[j].x > tmax.x) tmax.x = v[j].x;
			if(v[j].y < tmin.y) tmin.y = v[j].y;
			if(v[j].y > tmax.y) tmax.y = v[j].y;
			if(v[j].z < tmin.z) tmin.z = v[j].z;
			if(v[j].z > tmax.z) tmax.z = v[j].z;
		}

		// 各軸での重なり判定
		int overlap = 0;
		if(!(tmax.x < bmin.x || tmin.x > bmax.x)) overlap++;
		if(!(tmax.y < bmin.y || tmin.y > bmax.y)) overlap++;
		if(!(tmax.z < bmin.z || tmin.z > bmax.z)) overlap++;

		//	3軸すべてで重なっていたらAABB同士が重なっている
		if(overlap < 3)
			return false;

		// 2. AABBと三角形を含む平面との交差判定
		if(!plane_aabb(n, -(n.x * v[0].x + n.y * v[0].y + n.z * v[0].z), {0.0f, 0.0f, 0.0f}, box.get_local_scale()))
			return false;
#endif
		// 3. AABBの各軸方向とエッジのテスト
		DirectX::XMFLOAT3 e[3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} }; // AABBの各軸方向ベクトル
		DirectX::XMFLOAT3 f[3] = {}; // 三角形のエッジベクトル
		e[0].x = 1.0f;
		f[0].x = v[1].x - v[0].x;
		f[0].y = v[1].y - v[0].y;
		f[0].z = v[1].z - v[0].z;
		e[1].x = 1.0f;
		f[1].x = v[2].x - v[1].x;
		f[1].y = v[2].y - v[1].y;
		f[1].z = v[2].z - v[1].z;
		e[2].x = 1.0f;
		f[2].x = v[0].x - v[2].x;
		f[2].y = v[0].y - v[2].y;
		f[2].z = v[0].z - v[2].z;
		for(int i = 0; i < 3; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				DirectX::XMFLOAT3 a, p;
				DirectX::XMVECTOR A = DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&e[i]), DirectX::XMLoadFloat3(&f[j]));
				DirectX::XMStoreFloat3(&a, A);
				p.x = DirectX::XMVectorGetX(DirectX::XMVector3Dot(A, DirectX::XMLoadFloat3(&v[0])));
				p.y = DirectX::XMVectorGetX(DirectX::XMVector3Dot(A, DirectX::XMLoadFloat3(&v[1])));
				p.z = DirectX::XMVectorGetX(DirectX::XMVector3Dot(A, DirectX::XMLoadFloat3(&v[2])));
				float r = box.get_local_scale().x * fabs(a.x)
						+ box.get_local_scale().y * fabs(a.y)
						+ box.get_local_scale().z * fabs(a.z);
				if(std::min(p.x, std::min(p.y, p.z)) > r || std::max(p.x, std::max(p.y, p.z)) < -r)
					return false;
			}
		}

		return true;
	}

	inline UINT calculate_array_index_from_point(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT3& area_min, const DirectX::XMFLOAT3& area_unit, DirectX::XMUINT3 separate_count)
	{
		DirectX::XMFLOAT3 p;
		p.x = (point.x - area_min.x) / area_unit.x;
		p.y = (point.y - area_min.y) / area_unit.y;
		p.z = (point.z - area_min.z) / area_unit.z;
		return static_cast<UINT>(p.x) + static_cast<UINT>(separate_count.x * p.y) + static_cast<UINT>(separate_count.x * separate_count.y * p.z);
	}

	std::vector<UINT> calculate_check_order_list(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max, const DirectX::XMFLOAT3& area_min, const DirectX::XMFLOAT3& area_unit, DirectX::XMUINT3 separate_count)
	{
		DirectX::XMFLOAT3 vector;
		vector.x = max.x - min.x;
		vector.y = max.y - min.y;
		vector.z = max.z - min.z;

		DirectX::XMUINT3 axis_count;
		axis_count.x = static_cast<UINT>(ceilf((vector.x / area_unit.x + 0.001f)));
		axis_count.y = static_cast<UINT>(ceilf((vector.y / area_unit.y + 0.001f)));
		axis_count.z = static_cast<UINT>(ceilf((vector.z / area_unit.z + 0.001f)));

		UINT square_separate_count = separate_count.x * separate_count.y;
		UINT cube_separate_count = separate_count.x * separate_count.y * separate_count.z;
		std::vector<UINT> order_list;
		order_list.reserve(axis_count.x * axis_count.y * axis_count.z);

		DirectX::XMFLOAT3 point = min, p;
		for (UINT x = 0; x < axis_count.x; ++x)
		{
			p.x = (point.x - area_min.x) / area_unit.x;
			UINT index_x = static_cast<UINT>(p.x);
			for (UINT y = 0; y < axis_count.y; ++y)
			{
				p.y = (point.y - area_min.y) / area_unit.y;
				UINT index_y = static_cast<UINT>(p.y);
				UINT index_xy = index_x + index_y * separate_count.x;
				for (UINT z = 0; z < axis_count.z; ++z)
				{
					p.z = (point.z - area_min.z) / area_unit.z;
					UINT index_z = static_cast<UINT>(p.z);
					UINT index = index_xy + index_z * square_separate_count;
					if (index < cube_separate_count)
					{
						order_list.emplace_back(index);
					}
					point.z += area_unit.z;
				}
				point.y += area_unit.y;
			}
			point.x += area_unit.x;
		}

		return order_list;
	}
}

using namespace collision;

// CEREALバージョン定義
CEREAL_CLASS_VERSION(bounding_mesh::block_data, 1)

template<class Archive>
void bounding_mesh::block_data::serialize(Archive& archive, int version)
{
	archive(CEREAL_NVP(center),
			CEREAL_NVP(local_scale),
			CEREAL_NVP(indices));
}

const DirectX::XMFLOAT3& bounding_mesh::get_local_scale() const
{
	static const DirectX::XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
	return	scale;
}

void bounding_mesh::set_local_scale(const DirectX::XMFLOAT3& scale)
{
	//	何もしない
}

bounding_mesh::bounding_mesh()
: bounding_object({ 1, 1, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0 })
{
}

bounding_mesh::bounding_mesh(std::vector<DirectX::XMFLOAT3>& positions, std::vector<UINT>& indices, UINT separate_level)
: bounding_object({1, 1, 1}, {0, 0, 0, 1}, {0, 0, 0})
, positions(positions)
, indices(indices)

, separate_count(1, 1, 1)
{
	if (separate_level == 0)
		return;

	//	幅算出
	include_corner(area_min, area_max);
	area_min.x -= 1.0f;
	area_min.y -= 1.0f;
	area_min.z -= 1.0f;
	area_max.x += 1.0f;
	area_max.y += 1.0f;
	area_max.z += 1.0f;

	area_size.x = area_max.x - area_min.x;
	area_size.y = area_max.y - area_min.y;
	area_size.z = area_max.z - area_min.z;

	//	インデックス数が一定以下の場合、全体をくくるものとして登録する
	if (indices.size() <= 1024)
		separate_level = 0;

	//	分割レベルに応じてブロック幅算出
	{
		UINT temp_separate_count = 1 << separate_level;
		separate_count.x = temp_separate_count;
		separate_count.y = temp_separate_count;
		separate_count.z = temp_separate_count;
		for (; separate_count.x >= 1; --separate_count.x)
		{
			float temp = area_size.x / static_cast<float>(separate_count.x);
			if (temp >= separate_level)
				break;
		}
		for (; separate_count.y >= 1; --separate_count.y)
		{
			float temp = area_size.y / static_cast<float>(separate_count.y);
			if (temp >= separate_level)
				break;
		}
		for (; separate_count.z >= 1; --separate_count.z)
		{
			float temp = area_size.z / static_cast<float>(separate_count.z);
			if (temp >= separate_level)
				break;
		}
		area_unit.x = area_size.x / static_cast<float>(separate_count.x);
		area_unit.y = area_size.y / static_cast<float>(separate_count.y);
		area_unit.z = area_size.z / static_cast<float>(separate_count.z);
	}

	//	インデックス空間生成
	block_datas.resize(separate_count.x * separate_count.y * separate_count.z);
	auto calculate_boxes = [&](DirectX::XMUINT3 start_index, DirectX::XMUINT3 end_index)
	{
		for (UINT x = start_index.x; x < end_index.x; ++x)
		{
			for (UINT y = start_index.y; y < end_index.y; ++y)
			{
				for (UINT z = start_index.z; z < end_index.z; ++z)
				{
					block_data data;
					data.center = area_min;
					data.local_scale = area_unit;
					data.center.x += area_unit.x * (static_cast<float>(x) + 0.5f);
					data.center.y += area_unit.y * (static_cast<float>(y) + 0.5f);
					data.center.z += area_unit.z * (static_cast<float>(z) + 0.5f);
					data.local_scale.x /= 2;
					data.local_scale.y /= 2;
					data.local_scale.z /= 2;
					data.bounds = inner::axis_aligned_bounding_box(data.local_scale, data.center);

					//	負荷軽減用
					DirectX::XMVECTOR BoxMin, BoxMax;
					data.bounds.include_corner(BoxMin, BoxMax);

					auto p = positions.data();
					auto i = indices.data();
					for (size_t index = 0; index < indices.size(); index += 3)
					{
						UINT ia = i[index + 0];
						UINT ib = i[index + 1];
						UINT ic = i[index + 2];
						DirectX::XMFLOAT3 a = p[ia];
						DirectX::XMFLOAT3 b = p[ib];
						DirectX::XMFLOAT3 c = p[ic];
						DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&a);
						DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&b);
						DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&c);

						DirectX::XMVECTOR TriMin = DirectX::XMVectorMin(DirectX::XMVectorMin(A, B), C);
						DirectX::XMVECTOR TriMax = DirectX::XMVectorMax(DirectX::XMVectorMax(A, B), C);
						DirectX::XMVECTOR Disjoint = DirectX::XMVectorOrInt(DirectX::XMVectorGreater(TriMin, BoxMax), DirectX::XMVectorGreater(BoxMin, TriMax));
						if (DirectX::Internal::XMVector3AnyTrue(Disjoint))
							continue;

						DirectX::XMVECTOR AB = DirectX::XMVectorSubtract(B, A);
						DirectX::XMVECTOR AC = DirectX::XMVectorSubtract(C, A);
						DirectX::XMFLOAT3 n;
						DirectX::XMStoreFloat3(&n, DirectX::XMVector3Normalize(DirectX::XMVector3Cross(AB, AC)));
						if (triangle_aabb(a, b, c, n, data.bounds))
						{
							data.indices.emplace_back(ia);
							data.indices.emplace_back(ib);
							data.indices.emplace_back(ic);
						}
					}

					UINT index = x +
								 y * separate_count.x +
								 z * separate_count.x * separate_count.y;
					block_datas[index] = data;
				}
			}
		}
	};

	if (separate_level > 0)
	{
		std::vector<std::thread> thread_functions;
		UINT X = static_cast<UINT>(ceilf(static_cast<float>(separate_count.x) / separate_level + 0.1f));
		UINT Y = static_cast<UINT>(ceilf(static_cast<float>(separate_count.y) / separate_level + 0.1f));
		UINT Z = static_cast<UINT>(ceilf(static_cast<float>(separate_count.z) / separate_level + 0.1f));
		for (UINT x = 0; x < X; ++x)
		{
			for (UINT y = 0; y < Y; ++y)
			{
				for (UINT z = 0; z < Z; ++z)
				{
					DirectX::XMUINT3 s = { (x + 0) * separate_level, (y + 0) * separate_level, (z + 0) * separate_level };
					DirectX::XMUINT3 e = { (x + 1) * separate_level, (y + 1) * separate_level, (z + 1) * separate_level };
					e = { std::min(separate_count.x, e.x), std::min(separate_count.y, e.y), std::min(separate_count.z, e.z) };
					thread_functions.emplace_back(calculate_boxes, s, e);
				}
			}
		}

		for (auto& thread : thread_functions)
		{
			thread.join();
		}
	}
	else
	{
		DirectX::XMUINT3 s = { 0, 0, 0 };
		DirectX::XMUINT3 e = { 1, 1, 1 };
		calculate_boxes(s, e);
	}
}

bounding_mesh::~bounding_mesh()
{
}

bool bounding_mesh::serialize(std::string_view file_path)
{
	std::ofstream ostream(file_path.data(), std::ios::binary);
	if (ostream.is_open())
	{
		cereal::BinaryOutputArchive archive(ostream);

		try
		{
			archive(CEREAL_NVP(positions),
					CEREAL_NVP(indices),

					CEREAL_NVP(area_min),
					CEREAL_NVP(area_max),
					CEREAL_NVP(area_size),
					CEREAL_NVP(area_unit),
					CEREAL_NVP(separate_count),
					CEREAL_NVP(block_datas));
		}
		catch (...)
		{
			DEBUG_LOG("bounding_mesh serialize failed. %s\n", file_path.data());
			assert(!"bounding_mesh serialize failed.\n");
			return false;
		}

		return true;
	}
	return false;
}

bool bounding_mesh::deserialize(std::string_view file_path)
{
	std::ifstream istream(file_path.data(), std::ios::binary);
	if (istream.is_open())
	{
		cereal::BinaryInputArchive archive(istream);

		try
		{
			archive(CEREAL_NVP(positions),
					CEREAL_NVP(indices),

					CEREAL_NVP(area_min),
					CEREAL_NVP(area_max),
					CEREAL_NVP(area_size),
					CEREAL_NVP(area_unit),
					CEREAL_NVP(separate_count),
					CEREAL_NVP(block_datas));
		}
		catch (...)
		{
			DEBUG_LOG("bounding_mesh deserialize failed. %s\n", file_path.data());
			assert(!"bounding_mesh deserialize failed.\n");
			return false;
		}

		//	インデックスブロック内部のバウンディングボックス生成
		for (auto& node : block_datas)
		{
			node.bounds = inner::axis_aligned_bounding_box(node.local_scale, node.center);
		}
		return true;
	}
	OutputDebugStringA(file_path.data());
	assert(!"material deserialize failed.\n");
	return false;
}

bool bounding_mesh::intersect_point_inner(const DirectX::XMFLOAT3& point, float& nearlest, const std::vector<UINT>& indices) const
{
	if (indices.size() == 0)
		return false;

	DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&point);
	auto p = positions.data();
	auto i = indices.data();
	nearlest = FLT_MAX;
	for (size_t index = 0; index < indices.size(); index += 3)
	{
		DirectX::XMVECTOR A = DirectX::XMLoadFloat3(p + i[index + 0]);
		DirectX::XMVECTOR B = DirectX::XMLoadFloat3(p + i[index + 1]);
		DirectX::XMVECTOR C = DirectX::XMLoadFloat3(p + i[index + 2]);

		DirectX::XMVECTOR NP;
		calculate_nearlest_point(P, A, B, C, NP);
		float t = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(NP, P)));
		if (t < nearlest)
			nearlest = t;
	}
	return nearlest < FLT_EPSILON;
}

bool bounding_mesh::intersect_point(const DirectX::XMFLOAT3& point) const
{
	float nearlest = FLT_MAX;
	if (block_datas.empty())
	{
		return intersect_point_inner(point, nearlest, indices);
	}
	auto index = calculate_array_index_from_point(point, area_min, area_unit, separate_count);
	{
		auto& node = block_datas[index];
		if (node.bounds.intersect_point(point))
		{
			if (intersect_point_inner(point, nearlest, node.indices))
				return true;
		}
	}
	return false;
}

bool bounding_mesh::intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect) const
{
	if (block_datas.empty())
	{
		collision::triangles triangles;
		triangles.positions = positions.data();
		triangles.indices = indices.data();
		triangles.index_start = 0;
		triangles.index_count = indices.size();
		intersect_data data;
		if (ray_cast(start, end, triangles, intersect))
		{
			return true;
		}
		return false;
	}
	DirectX::XMFLOAT3 min = start, max = start;
	min.x = std::min(min.x, end.x);
	min.y = std::min(min.y, end.y);
	min.z = std::min(min.z, end.z);
	max.x = std::max(max.x, end.x);
	max.y = std::max(max.y, end.y);
	max.z = std::max(max.z, end.z);
	auto list = calculate_check_order_list(min, max, area_min, area_unit, separate_count);
	if (list.empty())
		return false;

	max.x = (max.x - min.x) * 0.5f;
	max.y = (max.y - min.y) * 0.5f;
	max.z = (max.z - min.z) * 0.5f;
	min.x = min.x + max.x;
	min.y = min.y + max.y;
	min.z = min.z + max.z;
	bounding_sphere bs = bounding_sphere(min, std::max(max.x, std::max(max.y, max.z)));

	bool hit = false;
	for (auto node_index : list)
	{
		auto& node = block_datas[node_index];
		if (node.indices.size() == 0 || !node.bounds.intersect_sphere(bs))
			continue;

		collision::triangles triangles;
		triangles.positions = positions.data();
		triangles.indices = node.indices.data();
		triangles.index_count = node.indices.size();

		if (intersect)
		{
			intersect_data data;
			if (ray_cast(start, end, triangles, &data))
			{
				if (intersect->size() == 0)
				{
					*intersect = data;
				}
				else
				{
					if (data[0].length < (*intersect)[0].length)
					{
						*intersect = data;
					}
				}
				hit = true;
			}
		}
		else
		{
			if (ray_cast(start, end, triangles, intersect))
				return true;
		}
	}
	return hit;
}

bool bounding_mesh::intersect_sphere_inner(const DirectX::XMFLOAT3& point, float radius, const std::vector<UINT>& indices, intersect_data* intersect) const
{
	if (indices.size() == 0)
		return false;

	DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&point);
	auto p = positions.data();
	auto i = indices.data();
	DirectX::XMVECTOR NealestPoint = {};
	float nearlest = FLT_MAX;
	for (size_t index = 0; index < indices.size(); index += 3)
	{
		DirectX::XMVECTOR A = DirectX::XMLoadFloat3(p + i[index + 0]);
		DirectX::XMVECTOR B = DirectX::XMLoadFloat3(p + i[index + 1]);
		DirectX::XMVECTOR C = DirectX::XMLoadFloat3(p + i[index + 2]);

		DirectX::XMVECTOR NP;
		calculate_nearlest_point(P, A, B, C, NP);
		float t = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(NP, P)));
		if (t < nearlest)
		{
			nearlest = t;
			NealestPoint = NP;
		}
	}
	nearlest = sqrtf(nearlest);
	nearlest -= radius;
	if (nearlest <= 0.0f && intersect)
	{
		P = DirectX::XMLoadFloat3(&point);
		DirectX::XMVECTOR Vector = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(NealestPoint, P));
		DirectX::XMFLOAT3 p, n;
		DirectX::XMStoreFloat3(&p, NealestPoint);
		DirectX::XMStoreFloat3(&n, Vector);
		intersect->add(p, n, fabsf(nearlest));
	}
	return nearlest <= 0.0f;
}

bool bounding_mesh::intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect) const
{
	float nearlest = FLT_MAX;
	if (block_datas.empty())
	{
		return intersect_sphere_inner(point, radius, indices, intersect);
	}
	else
	{
		DirectX::XMFLOAT3 min, max;
		min.x = point.x - radius;
		min.y = point.y - radius;
		min.z = point.z - radius;
		max.x = point.x + radius;
		max.y = point.y + radius;
		max.z = point.z + radius;
		auto list = calculate_check_order_list(min, max, area_min, area_unit, separate_count);
		for (auto node_index : list)
		{
			auto& node = block_datas[node_index];
			if (intersect_sphere_inner(point, radius, node.indices, intersect))
			{
				return true;
			}
		}
	}
	return false;
}

bool bounding_mesh::intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect) const
{
	return	intersect_sphere(sphere.get_position(), sphere.get_radius(), intersect);
}

bool bounding_mesh::intersect_capsule_inner(const bounding_capsule& capsule, const std::vector<UINT>& indices, intersect_data* intersect) const
{
	if (indices.size() == 0)
		return false;

	//	ローカル空間に変換
	bounding_capsule copy_capsule = capsule;
	DirectX::XMMATRIX TransformMatrix = get_transform_matrix();
	{
		DirectX::XMMATRIX CopyTransformMatrix = copy_capsule.get_transform_matrix();
		DirectX::XMMATRIX InverseTransformMatrix = DirectX::XMMatrixInverse(nullptr, TransformMatrix);
		DirectX::XMVECTOR Scale, Rotation, Translation;
		DirectX::XMMatrixDecompose(&Scale, &Rotation, &Translation, CopyTransformMatrix * InverseTransformMatrix);
		DirectX::XMFLOAT4 rotation;
		DirectX::XMFLOAT3 translation;
		DirectX::XMStoreFloat4(&rotation, Rotation);
		DirectX::XMStoreFloat3(&translation, Translation);
		copy_capsule.set_rotation(rotation);
		copy_capsule.set_position(translation);
	}

	inner::axis_aligned_bounding_box capsule_box;
	{
		DirectX::XMFLOAT3 min, max;
		copy_capsule.include_corner(min, max);
		max.x = (max.x - min.x) * 0.5f;
		max.y = (max.y - min.y) * 0.5f;
		max.z = (max.z - min.z) * 0.5f;
		min.x += max.x;
		min.y += max.y;
		min.z += max.z;
		capsule_box = inner::axis_aligned_bounding_box(max, min);
	}

	auto p = positions.data();
	auto i = indices.data();

	DirectX::XMFLOAT3 nearlest_point_segment = {}, nearlest_point_triangle = {};
	bool nearlest_point_edghFlg = false;
	float nearlest = FLT_MAX;
	size_t nealest_face_index = -1;
	for (size_t index = 0; index < indices.size(); index += 3)
	{
		DirectX::XMFLOAT3 a = *(p + i[index + 0]);
		DirectX::XMFLOAT3 b = *(p + i[index + 1]);
		DirectX::XMFLOAT3 c = *(p + i[index + 2]);

		//	ボックス判定
		DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&a);
		DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&b);
		DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&c);
		DirectX::XMVECTOR Center = DirectX::XMVectorScale(DirectX::XMVectorAdd(A, DirectX::XMVectorAdd(B, C)), 1.0f / 3.0f);
		float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(A, Center)));
		DirectX::XMFLOAT3 center;
		DirectX::XMStoreFloat3(&center, Center);
		if (!capsule_box.intersect_sphere(center, length))
			continue;

		//	距離算出
		DirectX::XMFLOAT3 near_point_segment, near_point_triangle;
		bool near_point_edghFlg;
		float t = calculate_segment_distance(copy_capsule.get_line(), a, b, c, near_point_segment, near_point_triangle, near_point_edghFlg);
		if (t < nearlest)
		{
			nearlest = t;
			nealest_face_index = index;
			nearlest_point_segment = near_point_segment;
			nearlest_point_triangle = near_point_triangle;
			nearlest_point_edghFlg = near_point_edghFlg;
		}
	}

	float radius = copy_capsule.get_radius();
	if (nearlest < radius && intersect)
	{
		DirectX::XMFLOAT3 a = *(p + i[nealest_face_index + 0]);
		DirectX::XMFLOAT3 b = *(p + i[nealest_face_index + 1]);
		DirectX::XMFLOAT3 c = *(p + i[nealest_face_index + 2]);
		DirectX::XMVECTOR TriangleA = DirectX::XMLoadFloat3(&a);
		DirectX::XMVECTOR TriangleB = DirectX::XMLoadFloat3(&b);
		DirectX::XMVECTOR TriangleC = DirectX::XMLoadFloat3(&c);
		DirectX::XMVECTOR PointA = DirectX::XMLoadFloat3(&nearlest_point_segment);
		DirectX::XMVECTOR PointB = DirectX::XMLoadFloat3(&nearlest_point_triangle);

		DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&copy_capsule.get_line()[0]);
		DirectX::XMVECTOR End = DirectX::XMLoadFloat3(&copy_capsule.get_line()[1]);
		DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(Position, End));

		collision::intersect_data::infomation infomation;
		infomation.point = nearlest_point_segment;
		infomation.point2 = nearlest_point_triangle;
		DirectX::XMVECTOR Vector = DirectX::XMVectorSubtract(PointA, PointB);

		//	最短点が面上の場合
		DirectX::XMVECTOR Normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(
			DirectX::XMVectorSubtract(TriangleB, TriangleA), DirectX::XMVectorSubtract(TriangleC, TriangleA)));

		if (!nearlest_point_edghFlg)
		{
			//	線分が三角形を貫通している場合
			if (nearlest == 0)
			{
				//	両端の表裏判定
				float d = DirectX::XMVectorGetX(DirectX::XMVector3Dot(Normal, Direction));
				DirectX::XMVECTOR BackPoint = d > 0 ? Position : End;

				//	裏側の点から交点へのベクトルを算出し、カプセルのポジションに加算
				//	さらに法線の単位ベクトルにカプセルの半径を掛けたベクトルを用意し、カプセルのポジションに加算する
				Vector = DirectX::XMVectorAdd(DirectX::XMVectorSubtract(PointB, BackPoint), DirectX::XMVectorScale(Normal, radius));

				infomation.length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Vector));
				DirectX::XMStoreFloat3(&infomation.normal, DirectX::XMVector3Normalize(Vector));
			}
			else
			{
				infomation.length = radius - DirectX::XMVectorGetX(DirectX::XMVector3Length(Vector));
				DirectX::XMStoreFloat3(&infomation.normal, DirectX::XMVector3Normalize(Vector));
			}
		}
		else
		{
			//	最短点が辺上の場合
			infomation.length = radius - nearlest;
		//	infomation.length = radius - DirectX::XMVectorGetX(DirectX::XMVector3Length(Vector));
			DirectX::XMStoreFloat3(&infomation.normal, DirectX::XMVector3Normalize(Vector));
		}

		//	法線が手に入らなかった場合は三角形の面を納入
		if (infomation.normal.x * infomation.normal.x + infomation.normal.y * infomation.normal.y + infomation.normal.z * infomation.normal.z <= FLT_EPSILON)
		{
			DirectX::XMStoreFloat3(&infomation.normal, DirectX::XMVector3Normalize(Normal));
		}

		//	法線を反転しておく
		DirectX::XMStoreFloat3(&infomation.normal, DirectX::XMVectorNegate(DirectX::XMLoadFloat3(&infomation.normal)));

		DirectX::XMStoreFloat3(&infomation.point, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&infomation.point), TransformMatrix));
		DirectX::XMStoreFloat3(&infomation.point2, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&infomation.point2), TransformMatrix));
		DirectX::XMStoreFloat3(&infomation.normal, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&infomation.normal), TransformMatrix));
		intersect->add(infomation);
	}
	return nearlest < radius;
}

bool bounding_mesh::intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect) const
{
	if (block_datas.empty())
	{
		return intersect_capsule_inner(capsule, indices, intersect);
	}

	DirectX::XMFLOAT3 min, max;
	capsule.include_corner(min, max);
	auto list = calculate_check_order_list(min, max, area_min, area_unit, separate_count);

	bool hit = false;
	for (auto node_index : list)
	{
		auto& node = block_datas[node_index];
		if (node.bounds.intersect_capsule(capsule))
		{
			if (intersect)
			{
				intersect_data data;
				if (intersect_capsule_inner(capsule, node.indices, &data))
				{
					if (intersect->size() == 0)
					{
						*intersect = data;
					}
					else
					{
						if (data[0].length < (*intersect)[0].length)
						{
							*intersect = data;
						}
					}
					hit = true;
				}
			}
			else
			{
				if (intersect_capsule_inner(capsule, node.indices, intersect))
					return true;
			}
		}
	}
	return hit;
}

bool bounding_mesh::intersect_box(const bounding_box& box, intersect_data* intersect) const
{
	//	未対応
//	DEBUG_LOG("not corespound mesh vs box\n");
	return false;
}

bool bounding_mesh::intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect) const
{
	//	未対応
//	DEBUG_LOG("not corespound mesh vs mesh\n");
	return false;
}

void bounding_mesh::include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const
{
	min = max = positions[0];
	for (auto& position : positions)
	{
		min.x	= std::min(position.x, min.x);
		min.y	= std::min(position.y, min.y);
		min.z	= std::min(position.z, min.z);
		max.x	= std::max(position.x, max.x);
		max.y	= std::max(position.y, max.y);
		max.z	= std::max(position.z, max.z);
	}
}

bool bounding_mesh::calculate_nearlest_point(const DirectX::XMVECTOR& P, const DirectX::XMVECTOR& A, const DirectX::XMVECTOR& B, const DirectX::XMVECTOR& C, DirectX::XMVECTOR& NearlestPoint) const
{
	DirectX::XMVECTOR ab = DirectX::XMVectorSubtract(B, A);
	DirectX::XMVECTOR ac = DirectX::XMVectorSubtract(C, A);
	DirectX::XMVECTOR ap = DirectX::XMVectorSubtract(P, A);

	float d1 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ab, ap));
	float d2 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ac, ap));
	if (d1 <= 0.0f && d2 <= 0.0f)
	{
		NearlestPoint = A;
		return false;
	}

	DirectX::XMVECTOR bp = DirectX::XMVectorSubtract(P, B);
	float d3 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ab, bp));
	float d4 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ac, bp));
	if (d3 >= 0.0f && d4 <= d3)
	{
		NearlestPoint = B;
		return false;
	}

	DirectX::XMVECTOR cp = DirectX::XMVectorSubtract(P, C);
	float d5 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ab, cp));
	float d6 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ac, cp));
	if (d6 >= 0.0f && d5 <= d6)
	{
		NearlestPoint = C;
		return false;
	}

	float vc = d1 * d4 - d3 * d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
	{
		NearlestPoint = DirectX::XMVectorAdd(DirectX::XMVectorScale(ab, d1 / (d1 - d3)), A);
		return false;
	}

	float vb = d5 * d2 - d1 * d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
	{
		NearlestPoint = DirectX::XMVectorAdd(DirectX::XMVectorScale(ac, d2 / (d2 - d6)), A);
		return false;
	}

	float va = d3 * d6 - d5 * d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
	{
		DirectX::XMVECTOR bc = DirectX::XMVectorSubtract(C, B);
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		NearlestPoint = DirectX::XMVectorAdd(DirectX::XMVectorScale(bc, w), B);
		return false;
	}

	//	最近点が三角形内の点と判断
	float denom = 1.0f / (va + vb + vc);
	ab = DirectX::XMVectorScale(ab, vb * denom);
	ac = DirectX::XMVectorScale(ac, vc * denom);
	NearlestPoint = DirectX::XMVectorAdd(DirectX::XMVectorAdd(ab, ac), A);
	return true;
}

bool bounding_mesh::intersect_ray(const DirectX::XMVECTOR& Start,
								  const DirectX::XMVECTOR& Direction,
								  const DirectX::XMVECTOR& A,
								  const DirectX::XMVECTOR& B,
								  const DirectX::XMVECTOR& C,
								  const DirectX::XMVECTOR& Normal,
								  DirectX::XMVECTOR& InsertPoint,
								  float& distance) const
{
	float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(Direction, Normal));
	if (dot >= 0.0f )
		return	false;

	float dot2 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVectorSubtract(A, Start), Normal));
	float delta = (dot2 == 0.0f) ? 0 : (dot2 / dot);
	if (delta < 0.0f)
		return	false;

	DirectX::XMVECTOR InnerPoint = DirectX::XMVectorAdd(DirectX::XMVectorScale(Direction, delta), Start);

	DirectX::XMVECTOR Work;
	Work = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(B, A), DirectX::XMVectorSubtract(A, InnerPoint));
	if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(Work, Normal)) < 0.0f)
		return false;
	Work = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(C, B), DirectX::XMVectorSubtract(B, InnerPoint));
	if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(Work, Normal)) < 0.0f)
		return false;
	Work = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(A, C), DirectX::XMVectorSubtract(C, InnerPoint));
	if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(Work, Normal)) < 0.0f)
		return false;

	InsertPoint = InnerPoint;
	distance = delta;
	return	true;
}

float bounding_mesh::calculate_segment_distance(const collision::line& line,
												const DirectX::XMFLOAT3& a,
												const DirectX::XMFLOAT3& b,
												const DirectX::XMFLOAT3& c,
												DirectX::XMFLOAT3& nearlest_point_segment,
												DirectX::XMFLOAT3& nearlest_point_triangle,
												bool& nearlest_point_edghFlg) const
{
	nearlest_point_edghFlg = false;

	//	線分と三角形の交差判定
	DirectX::XMVECTOR PointA = DirectX::XMLoadFloat3(&line[0]);
	DirectX::XMVECTOR PointB = DirectX::XMLoadFloat3(&line[1]);
	DirectX::XMVECTOR TriangleA = DirectX::XMLoadFloat3(&a);
	DirectX::XMVECTOR TriangleB = DirectX::XMLoadFloat3(&b);
	DirectX::XMVECTOR TriangleC = DirectX::XMLoadFloat3(&c);
	DirectX::XMVECTOR TriangleAB = DirectX::XMVectorSubtract(TriangleB, TriangleA);
	DirectX::XMVECTOR TriangleAC = DirectX::XMVectorSubtract(TriangleC, TriangleA);
	DirectX::XMVECTOR TriangleNormal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(TriangleAB, TriangleAC));

	DirectX::XMVECTOR RayDirection = DirectX::XMVectorSubtract(PointB, PointA);
	DirectX::XMVECTOR IntersectPoint;
	float distance;
	if (intersect_ray(PointA, RayDirection, TriangleA, TriangleB, TriangleC, TriangleNormal, IntersectPoint, distance))
	{
		DirectX::XMStoreFloat3(&nearlest_point_segment, IntersectPoint);
		DirectX::XMStoreFloat3(&nearlest_point_triangle, IntersectPoint);
		return 0.0f;
	}
	RayDirection = DirectX::XMVectorSubtract(PointA, PointB);
	if (intersect_ray(PointB, RayDirection, TriangleA, TriangleB, TriangleC, TriangleNormal, IntersectPoint, distance))
	{
		DirectX::XMStoreFloat3(&nearlest_point_segment, IntersectPoint);
		DirectX::XMStoreFloat3(&nearlest_point_triangle, IntersectPoint);
		return 0.0f;
	}

	//	線分の各頂点に対して三角形の最近点を求める
	DirectX::XMVECTOR NearPosA, NearPosB;
	bool insideA = calculate_nearlest_point(PointA, TriangleA, TriangleB, TriangleC, NearPosA);
	bool insideB = calculate_nearlest_point(PointB, TriangleA, TriangleB, TriangleC, NearPosB);

	float nearlest_distance = FLT_MAX;
	if (insideA || insideB)
	{
		float distance_squareA, distance_squareB;
		distance_squareA = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(NearPosA, PointA)));
		distance_squareB = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(NearPosB, PointB)));

		if (distance_squareA < distance_squareB)
		{
			nearlest_distance = distance_squareA;
			DirectX::XMStoreFloat3(&nearlest_point_segment, PointA);
			DirectX::XMStoreFloat3(&nearlest_point_triangle, NearPosA);
		}
		else
		{
			nearlest_distance = distance_squareB;
			DirectX::XMStoreFloat3(&nearlest_point_segment, PointB);
			DirectX::XMStoreFloat3(&nearlest_point_triangle, NearPosB);
		}

		//	両端とも射影点が三角形内部
		if (insideA && insideB)
		{
			return nearlest_distance;
		}
	}

	//	最近傍点は頂点と線分上に存在(カプセル用に必須)
	nearlest_point_edghFlg = true;

	//	三角形外部に線分（または端）が存在し、最近点が辺上になる場合、線分と三角形の３辺の距離を全て計測し比較する
	DirectX::XMFLOAT3 horizon1, horizon2;
	float delta1, delta2;

	collision::line ab = collision::line(collision::line::types::segment, a, b);
	float distance_edge = line.calculate_distance(horizon1, delta1, horizon2, delta2, ab);
	if (distance_edge < nearlest_distance)
	{
		nearlest_distance = distance_edge;
		DirectX::XMStoreFloat3(&nearlest_point_segment, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&horizon1), delta1));
		DirectX::XMStoreFloat3(&nearlest_point_triangle, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&horizon2), delta2));
	}

	collision::line bc = collision::line(collision::line::types::segment, b, c);
	distance_edge = line.calculate_distance(horizon1, delta1, horizon2, delta2, bc);
	if (distance_edge < nearlest_distance)
	{
		nearlest_distance = distance_edge;
		DirectX::XMStoreFloat3(&nearlest_point_segment, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&horizon1), delta1));
		DirectX::XMStoreFloat3(&nearlest_point_triangle, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&horizon2), delta2));
	}

	collision::line ca = collision::line(collision::line::types::segment, c, a);
	distance_edge = line.calculate_distance(horizon1, delta1, horizon2, delta2, ca);
	if (distance_edge < nearlest_distance)
	{
		nearlest_distance = distance_edge;
		DirectX::XMStoreFloat3(&nearlest_point_segment, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&horizon1), delta1));
		DirectX::XMStoreFloat3(&nearlest_point_triangle, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&horizon2), delta2));
	}

	return nearlest_distance;
}

////////////////////////////////////////////////////////////////////////////////////////////////
bool collision::generate_bounding_mesh(bounding_mesh& out_mesh, model_resource_ptr resource, size_t index, DirectX::XMMATRIX WorldMatrix, bool reverse_flag, UINT separate_level)
{
	auto& mesh = resource->get_meshes()[index];
	if (mesh.index_buffer_view.count() == 0)
		return false;

	auto& nodes = resource->get_nodes();
	std::function<DirectX::XMMATRIX(size_t index)> calculate_transform_matrix = [&](size_t node_index)
	{
		if (node_index == model_resource::invalid_id)
			return DirectX::XMMatrixIdentity();

		auto& node = nodes[node_index];
		DirectX::XMMATRIX transform;
		transform = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z)
				  * DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation))
				  * DirectX::XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z);
		if (node.parent_index != model_resource::invalid_id)
			transform *= calculate_transform_matrix(node.parent_index);

		return transform;
	};

	DirectX::XMMATRIX transform_matrix = calculate_transform_matrix(mesh.node_index) * WorldMatrix;

	auto it = mesh.vertex_buffer_views.find(semantic("POSITION", 0));
	if (it == mesh.vertex_buffer_views.end())
		return false;

	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<UINT> indices;
	{
		auto p = reinterpret_cast<const DirectX::XMFLOAT3*>(it->second.row_data.data());
		for (UINT index = 0; index < it->second.count(); ++index)
		{
			DirectX::XMVECTOR P = DirectX::XMLoadFloat3(p + index);
			positions.emplace_back();
			DirectX::XMStoreFloat3(&positions.back(), DirectX::XMVector3TransformCoord(P, transform_matrix));
		}
		auto i = reinterpret_cast<const UINT*>(mesh.index_buffer_view.row_data.data());
		UINT index1 = reverse_flag ? 2 : 0;
		UINT index2 = reverse_flag ? 1 : 1;
		UINT index3 = reverse_flag ? 0 : 2;
		for (UINT index = 0; index < mesh.index_buffer_view.count(); index += 3)
		{
			indices.push_back(i[index + index1]);
			indices.push_back(i[index + index2]);
			indices.push_back(i[index + index3]);
		}
	}
	out_mesh = collision::bounding_mesh(positions, indices, separate_level);
	return true;
}
