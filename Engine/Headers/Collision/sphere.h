#pragma once

#include	"collision.h"

namespace collision
{
	class bounding_sphere final : public bounding_object
	{
	public:
		types get_type() const override {return types::sphere;}

		float get_radius() const;
		void set_radius(float radius);

	public:
		bounding_sphere();
		bounding_sphere(const DirectX::XMFLOAT3& position, float radius);
		~bounding_sphere() override;

		bool intersect_point(const DirectX::XMFLOAT3& point) const override;
		bool intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect = nullptr) const override;
		bool intersect_segment(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& End, intersect_data* intersect = nullptr) const;
		bool intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect = nullptr) const;
		bool intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect = nullptr) const override;
		bool intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect = nullptr) const override;
		bool intersect_box(const bounding_box& box, intersect_data* intersect = nullptr) const override;
		bool intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect = nullptr) const override;

		void include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const override;
	};
}	//	namespace collision
