#pragma once

#include "collision.h"
#include "plane.h"

namespace collision
{
	class frustum_culling
	{
		enum surface
		{
			surface_near,
			surface_left,
			surface_right,
			surface_bottom,
			surface_top,
			surface_far,

			surface_max,
		};

	public:
		frustum_culling();
		frustum_culling(const DirectX::XMFLOAT4X4& projection_matrix);
		virtual ~frustum_culling();

		bool intersection(const bounding_object* object) const;

		//	指定の点が視錐台内部にあるかどうか
		bool to_point(const DirectX::XMFLOAT3& point) const;

		//	球との交差判定
		bool to_sphere(const DirectX::XMFLOAT3& point, float radius) const;
		bool to_sphere(const bounding_sphere& sphere) const;

		//	カプセルとの交差判定
		bool to_capsule(const bounding_capsule& capsule) const;

		//	箱との交差判定
		bool to_box(const bounding_box& box) const;

		//	AABBとの交差判定
		bool to_aabb(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max) const;

		//	6平面を算出する
		void create_planes(const DirectX::XMFLOAT4X4& view_projection_matrix);

		//	8頂点算出
		void create_frustum_corners(const DirectX::XMFLOAT4X4& inverse_view_projection_matrix);
		void create_frustum_corners(const DirectX::XMMATRIX& inverse_view_projection_matrix);

	private:
		plane planes[surface_max];	//	視錐台の面
		DirectX::XMFLOAT4 corners[8];	//	視錐台の頂点
	};

}	//	namespace collision


