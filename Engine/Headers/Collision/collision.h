#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "math_utility.h"

namespace collision
{
	class intersect_data
	{
	public:
		struct infomation
		{
			DirectX::XMFLOAT3 point;
			DirectX::XMFLOAT3 point2;
			DirectX::XMFLOAT3 normal;
			float length;
		};

	public:
		intersect_data();
		virtual ~intersect_data();

		void add(const infomation& infomation);
		void add(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT3& normal, float length);
		void clear();

		bool multiply_matrix(std::size_t index, DirectX::XMFLOAT4X4 matrix);
		bool multiply_matrix(std::size_t index, DirectX::XMMATRIX matrix);

		std::size_t size();

	public:
		const infomation& operator[](std::size_t index) const;
		infomation& operator[](std::size_t index);

	private:
		std::vector<infomation>	infomations;
	};

	class bounding_sphere;
	class bounding_box;
	class bounding_capsule;
	class bounding_mesh;

	class bounding_object
	{
	public:
		enum class types
		{
			sphere	= 0,
			capsule,
			box,

			mesh,
		};

	public:
		virtual types get_type() const = 0;

		virtual const DirectX::XMFLOAT3& get_local_scale() const;
		virtual void set_local_scale(const DirectX::XMFLOAT3& scale);

		const DirectX::XMFLOAT4& get_rotation() const;
		void set_rotation(const DirectX::XMFLOAT4& rotation);

		const DirectX::XMFLOAT3& get_position() const;
		void set_position(const DirectX::XMFLOAT3& position);

		DirectX::XMFLOAT4X4 get_transform_float4x4() const;
		DirectX::XMMATRIX get_transform_matrix() const;

	public:
		bounding_object();
		bounding_object(const DirectX::XMFLOAT3& local_scale, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& position);
		virtual	~bounding_object();

		bool intersection(const bounding_object* object, intersect_data* intersect = nullptr) const;
		virtual bool intersect_point(const DirectX::XMFLOAT3& point) const = 0;
		virtual bool intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect = nullptr) const = 0;
		virtual bool intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect = nullptr) const = 0;
		virtual bool intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect = nullptr) const = 0;
		virtual bool intersect_box(const bounding_box& box, intersect_data* intersect = nullptr) const = 0;
		virtual bool intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect = nullptr) const = 0;

		virtual void include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const = 0;

	protected:
		virtual	void	on_change_parameter(){}

	private:
		DirectX::XMFLOAT3 local_scale;
		DirectX::XMFLOAT4 rotation;
		DirectX::XMFLOAT3 position;
	};

	//	レイキャスト
	struct triangles
	{
		const DirectX::XMFLOAT3* positions = nullptr;
		const unsigned int* indices = nullptr;
		size_t index_start = 0;
		size_t index_count = 0;
		bool reverse_index = false;
	};
	bool ray_cast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const triangles& triangles, intersect_data* result);
	bool ray_cast(const DirectX::XMVECTOR& start, const DirectX::XMVECTOR& end, const triangles& triangles, intersect_data* result);
}	//	namespace collision
