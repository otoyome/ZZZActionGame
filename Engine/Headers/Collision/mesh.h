#pragma once

#include	"collision.h"
#include	"model_resource.h"

namespace collision
{
	using UINT = unsigned int;

	class bounding_mesh : public bounding_object
	{
	public:
		struct block_data
		{
			DirectX::XMFLOAT3 center;
			DirectX::XMFLOAT3 local_scale;
			std::vector<UINT> indices;

			inner::axis_aligned_bounding_box bounds;
			template<class Archive>
			void serialize(Archive& archive, int version);
		};

	public:
		types get_type() const override {return types::mesh;}

		const DirectX::XMFLOAT3& get_local_scale() const override;
		void set_local_scale(const DirectX::XMFLOAT3& scale) override;

		const std::vector<DirectX::XMFLOAT3>& get_positions() const {return positions;}
		const std::vector<UINT>& get_indices() const {return indices;}
		const std::vector<block_data>& get_block_datas() const {return block_datas;}
		const DirectX::XMUINT3& get_separate_count() const {return separate_count;}

	public:
		bounding_mesh();
		bounding_mesh(std::vector<DirectX::XMFLOAT3>& positions, std::vector<UINT>& indices, UINT separate_level = 4);
		~bounding_mesh() override;

		bool serialize(std::string_view file_path);
		bool deserialize(std::string_view file_path);

		bool intersect_point(const DirectX::XMFLOAT3& point) const override;
		bool intersect_segment(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, intersect_data* intersect = nullptr) const override;
		bool intersect_sphere(const DirectX::XMFLOAT3& point, float radius, intersect_data* intersect = nullptr) const;
		bool intersect_sphere(const bounding_sphere& sphere, intersect_data* intersect = nullptr) const override;
		bool intersect_capsule(const bounding_capsule& capsule, intersect_data* intersect = nullptr) const override;
		bool intersect_box(const bounding_box& box, intersect_data* intersect = nullptr) const override;
		bool intersect_mesh(const bounding_mesh& mesh, intersect_data* intersect = nullptr) const override;

		void include_corner(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max) const override;

	private:
		bool intersect_point_inner(const DirectX::XMFLOAT3& point, float& nearlest, const std::vector<UINT>& indices) const;
		bool intersect_sphere_inner(const DirectX::XMFLOAT3& point, float radius, const std::vector<UINT>& indices, intersect_data* intersect = nullptr) const;
		bool intersect_capsule_inner(const bounding_capsule& capsule, const std::vector<UINT>& indices, intersect_data* intersect = nullptr) const;

	private:
		bool calculate_nearlest_point(const DirectX::XMVECTOR& P, const DirectX::XMVECTOR& A, const DirectX::XMVECTOR& B, const DirectX::XMVECTOR& C, DirectX::XMVECTOR& NearlestPoint) const;
		bool intersect_ray(const DirectX::XMVECTOR& Start, const DirectX::XMVECTOR& Direction, const DirectX::XMVECTOR& A, const DirectX::XMVECTOR& B, const DirectX::XMVECTOR& C, const DirectX::XMVECTOR& Normal, DirectX::XMVECTOR& InsertPoint, float& distance) const;
		float calculate_segment_distance(const collision::line& line, const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, const DirectX::XMFLOAT3& c, DirectX::XMFLOAT3& nearlest_point_segment, DirectX::XMFLOAT3& nearlest_point_triangle, bool& nearlest_point_edghFlg) const;

	private:
		std::vector<DirectX::XMFLOAT3> positions;
		std::vector<UINT> indices;

		DirectX::XMFLOAT3 area_min;
		DirectX::XMFLOAT3 area_max;
		DirectX::XMFLOAT3 area_size;
		DirectX::XMFLOAT3 area_unit;
		DirectX::XMUINT3 separate_count;
		std::vector<block_data> block_datas;
	};

	bool generate_bounding_mesh(bounding_mesh& out_mesh, model_resource_ptr resource, size_t index, DirectX::XMMATRIX WorldMatrix, bool reverse_flag = true, UINT separate_level = 4);
}	//	namespace collision
