#pragma once

#include	"collision.h"

namespace collision
{
	class bounding_box : public bounding_object
	{
	public:
		types get_type() const override {return types::box;}

		DirectX::XMMATRIX get_transeform_matrix() const;
		DirectX::XMFLOAT4X4 get_transeform_float4x4() const;

	public:
		bounding_box();
		bounding_box(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& position);
		bounding_box(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& y_axis, float angle, const DirectX::XMFLOAT3& position);
		~bounding_box() override;

		float calculate_point_to_length(const DirectX::XMFLOAT3& point) const;

		bool intersect_point(const DirectX::XMFLOAT3& point) const override;
		bool intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect = nullptr) const override;
		bool intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect = nullptr) const;
		bool intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect = nullptr) const override;
		bool intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect = nullptr) const override;
		bool intersect_box(const bounding_box& box, intersect_data* intersect = nullptr) const override;
		bool intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect = nullptr) const override;

		void include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const override;

	public:
		bounding_box multiply_obb(const DirectX::XMMATRIX& matrix) const;
		bounding_box multiply_obb(const DirectX::XMFLOAT4X4& matrix) const;

		void calculate_vertex(DirectX::XMFLOAT3* vertices, size_t vertex_count) const;
	};

	namespace inner
	{
		//	AABB
		//	íºê⁄égópÇ∑ÇÈÇ±Ç∆ÇÕã÷é~
		class axis_aligned_bounding_box : public bounding_object
		{
		public:
			types get_type() const override {return types::box;}

			DirectX::XMMATRIX get_transeform_matrix() const;
			DirectX::XMFLOAT4X4 get_transeform_float4x4() const;

		public:
			axis_aligned_bounding_box();
			axis_aligned_bounding_box(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& position);
			~axis_aligned_bounding_box() override;

			float calculate_point_to_length(const DirectX::XMFLOAT3& point) const;
			bool intersect_point(const DirectX::XMFLOAT3& point) const override;

			bool intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect = nullptr) const override;

			bool intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect = nullptr) const;

			bool intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect = nullptr) const override;

			bool intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect = nullptr) const override;

			bool intersect_box(const bounding_box& box, intersect_data* intersect = nullptr) const override;

			bool intersect_box(const axis_aligned_bounding_box& box) const;

			bool intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect = nullptr) const override;

			void include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const;
			void include_corner(DirectX::XMVECTOR& Min, DirectX::XMVECTOR& Max) const;

			void calculate_vertex(DirectX::XMFLOAT3* vertices, size_t vertex_count) const;
		};
	}	//	namespace inner
}	//	namespace collision
