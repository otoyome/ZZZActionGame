#pragma once

#include "collision.h"

namespace collision
{
	class line
	{
	public:
		enum class types
		{
			line,
			segment,
		};

	public:
		line();
		line(types type, const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& vector);

		void initialize(types type, const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& vector);

		float calculate_distance(DirectX::XMFLOAT3& horizon, float& delta, const DirectX::XMFLOAT3& point) const;
		float calculate_distance(DirectX::XMFLOAT3& horizon1, float& delta1, DirectX::XMFLOAT3& horizon2, float& delta2, const line& line) const;

		bool is_parallel(const line& line) const;

		float get_length() const;
		types get_type() const;
		DirectX::XMFLOAT3 get_point(float t) const;

	public:
		inline const DirectX::XMFLOAT3& operator[](size_t index) const { return	point[index]; }

	private:
		float calculate_line_distance(DirectX::XMFLOAT3& horizon, float& delta, const DirectX::XMFLOAT3& point) const;
		float calculate_segment_distamce(DirectX::XMFLOAT3& horizon, float& delta, const DirectX::XMFLOAT3& point) const;
		float calculate_line_distance(DirectX::XMFLOAT3& horizon1, float& delta1, DirectX::XMFLOAT3& horizon2, float& delta2, const line& line) const;
		float calculate_segment_distamce(DirectX::XMFLOAT3& horizon1, float& delta1, DirectX::XMFLOAT3& horizon2, float& delta2, const line& line) const;

	private:
		types type;
		DirectX::XMFLOAT3 point[2]; //  [0]：始点, [1]：[直線]正規化済みの方向ベクトル,[線分]終点
	};
}	//	namespace collision


