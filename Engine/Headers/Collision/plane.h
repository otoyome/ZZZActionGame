#pragma once

#include	"collision.h"
#include	"line.h"

namespace collision
{
	class plane
	{
	public:
		DirectX::XMFLOAT3 get_plane_position() const;
		DirectX::XMFLOAT3 get_normal() const;
		float get_distance() const;
		const DirectX::XMFLOAT4& get_parameter() const;
		void set_parameter(const DirectX::XMFLOAT4& parameter);

	public:
		plane(const DirectX::XMFLOAT4& parameter = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
		plane(const DirectX::XMFLOAT3& normal, float distance);

		void initialize(const DirectX::XMFLOAT4& parameter);
		void initialize(const DirectX::XMFLOAT3& normal, float distance);

		//	³‹K‰»
		void normlize();

		//	“_‚Æ•½–Ê‚Ì‹——£
		float calculate_distance(const DirectX::XMFLOAT3& point) const;

		//	ü‚Æ•½–Ê‚ÌŒğ·”»’è
		bool to_line(const line& line, float* distance = nullptr, DirectX::XMFLOAT3* insert_point = nullptr) const;

	private:
		DirectX::XMFLOAT4 parameter; //	xyz : –@ü, w : ‹——£
	};
}	//	namespace collision

