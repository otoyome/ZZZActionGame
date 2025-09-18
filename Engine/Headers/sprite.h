#pragma once

#define	NOMINMAX
#include "texture.h"
#include "material.h"

class sprite
{
public:
	static constexpr std::string_view base_texture_name = "base_texture";

public:
	const std::vector<DirectX::XMFLOAT3>& get_positions() const { return positions; }
	const std::vector<DirectX::XMFLOAT4>& get_colors() const { return colors; }
	const std::vector<DirectX::XMFLOAT2>& get_texcoords() const { return texcoords; }
	const std::vector<UINT>& get_indices() const {return indices;}
	DirectX::XMFLOAT4X4 get_transform_matrix() const { return transform_matrix; }
	material& get_material() {return material;}
	const material& get_material() const {return material;}

public:
	sprite();
	~sprite();

	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};
	void update_vertices(const std::vector<vertex>& vertices, const std::vector<UINT>& indices);

	void update_transform(
		float dx, float dy, float dw, float dh,
		float r, float g, float b, float a,
		float angle, float pivot_x, float pivot_y,
		float sx, float sy, float sw, float sh);

private:
	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT4> colors;
	std::vector<DirectX::XMFLOAT2> texcoords;
	std::vector<UINT> indices;
	DirectX::XMFLOAT4X4 transform_matrix;
	material material;
};
