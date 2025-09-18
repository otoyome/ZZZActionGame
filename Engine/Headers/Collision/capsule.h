#pragma once

#include	"collision.h"
#include	"line.h"

namespace collision
{
	class bounding_capsule final : public bounding_object
	{
	public:
		types get_type() const override {return types::capsule;}

		const line& get_line() const {return line;}

		float get_radius() const;
		void set_radius(float radius);
		float get_length() const;
		void set_length(float length);

		DirectX::XMVECTOR get_axis_vector() const;
		DirectX::XMFLOAT3 get_axis_float3() const;

	public:
		bounding_capsule();
		bounding_capsule(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& rotation, float radius, float length);
		~bounding_capsule() override;

		bool intersect_point(const DirectX::XMFLOAT3& point) const override;
		bool intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect = nullptr) const override;
		bool intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect = nullptr) const;
		bool intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect = nullptr) const override;
		bool intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect = nullptr) const override;
		bool intersect_box(const bounding_box& box, intersect_data* intersect = nullptr) const override;
		bool intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect = nullptr) const override;

		void include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const override;

	protected:
		void on_change_parameter() override;

	private:
		line line;
	};
}	//	namespace collision