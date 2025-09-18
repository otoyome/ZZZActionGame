#include "shader.h"
#include "misc.h"
#include "geometric_primitive.h"
#include "graphics_utility.h"

namespace
{
	//	頂点フォーマット
	struct format_data
	{
		semantic s;
		DXGI_FORMAT f;
	};
	enum format_id
	{
		position,
		texcoord_0,
	};
	const format_data format_datas[] =
	{
		{{"POSITION", 0}, DXGI_FORMAT_R32G32B32_FLOAT},
		{{"TEXCOORD", 0}, DXGI_FORMAT_R32G32_FLOAT},
	};
}
geometric_primitive::geometric_primitive(ID3D11Device* device)
{
}

void geometric_primitive::create_com_buffers(ID3D11Device* device,
	DirectX::XMFLOAT3* positions,
	DirectX::XMFLOAT2* texcoords,
	size_t vertex_count,
	uint32_t* indices,
	size_t index_count)
{
	HRESULT hr{ S_OK };

	//	頂点バッファビュー生成
	std::vector<buffer_view> views(ARRAYSIZE(format_datas));
	for (size_t index = 0; index < views.size(); ++index)
	{
		auto& view = views[index];
		view.format = format_datas[index].f;
		view.stride_in_bytes = graphics_utility::get_byte_offset_from_format(view.format);
		view.size_in_bytes = view.stride_in_bytes * vertex_count;
	}

	//	インデックスバッファビュー生成
	{
		auto& view = index_buffer_view;
		view.format = DXGI_FORMAT_R32_UINT;
		view.stride_in_bytes = graphics_utility::get_byte_offset_from_format(view.format);
		view.size_in_bytes = view.stride_in_bytes * index_count;
	}

	D3D11_BUFFER_DESC buffer_desc{};
	D3D11_SUBRESOURCE_DATA subresource_data{};
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	{
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.ByteWidth = static_cast<UINT>(views[format_id::position].size_in_bytes);
		subresource_data.pSysMem = positions;
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, views[format_id::position].buffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		vertex_buffer_views.insert_or_assign(format_datas[format_id::position].s, views[format_id::position]);

		buffer_desc.ByteWidth = static_cast<UINT>(views[format_id::texcoord_0].size_in_bytes);
		subresource_data.pSysMem = texcoords;
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, views[format_id::texcoord_0].buffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		vertex_buffer_views.insert_or_assign(format_datas[format_id::texcoord_0].s, views[format_id::texcoord_0]);
	}

	{
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffer_desc.ByteWidth = static_cast<UINT>(index_buffer_view.size_in_bytes);
		subresource_data.pSysMem = indices;
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, index_buffer_view.buffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}

geometric_plane::geometric_plane(ID3D11Device* device, bool x_flip) : geometric_primitive(device)
{
	// create a mesh for a cube
	DirectX::XMFLOAT3 positions[4];
	DirectX::XMFLOAT2 texcoords[4];
	uint32_t indices[6]{};
	if (x_flip)
	{
		positions[1] = { +0.5f, +0.0f, +0.5f };
		positions[0] = { -0.5f, +0.0f, +0.5f };
		positions[3] = { +0.5f, +0.0f, -0.5f };
		positions[2] = { -0.5f, +0.0f, -0.5f };
		indices[0] = 0;
		indices[1] = 2;
		indices[2] = 1;
		indices[3] = 1;
		indices[4] = 2;
		indices[5] = 3;
	}
	else
	{
		positions[0] = { +0.5f, +0.0f, +0.5f };
		positions[1] = { -0.5f, +0.0f, +0.5f };
		positions[2] = { +0.5f, +0.0f, -0.5f };
		positions[3] = { -0.5f, +0.0f, -0.5f };
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 1;
		indices[4] = 3;
		indices[5] = 2;
	}
	texcoords[0] = { +0.0f, +0.0f };
	texcoords[1] = { +1.0f, +0.0f };
	texcoords[2] = { +0.0f, +1.0f };
	texcoords[3] = { +1.0f, +1.0f };
	create_com_buffers(device, positions, texcoords, 4, indices, 6);
}

geometric_cube::geometric_cube(ID3D11Device* device) : geometric_primitive(device)
{
	// create a mesh for a cube
	DirectX::XMFLOAT3 positions[24];
	DirectX::XMFLOAT2 texcoords[24];
	uint32_t indices[36]{};

	uint32_t face{ 0 };

	// top-side
	face = 0;
	positions[face * 4 + 0] = { +0.5f, +0.5f, +0.5f };
	positions[face * 4 + 1] = { -0.5f, +0.5f, +0.5f };
	positions[face * 4 + 2] = { +0.5f, +0.5f, -0.5f };
	positions[face * 4 + 3] = { -0.5f, +0.5f, -0.5f };
	texcoords[face * 4 + 0] = { +0.0f, +0.0f };
	texcoords[face * 4 + 1] = { +1.0f, +0.0f };
	texcoords[face * 4 + 2] = { +0.0f, +1.0f };
	texcoords[face * 4 + 3] = { +1.0f, +1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	// bottom-side
	face += 1;
	positions[face * 4 + 0] = { -0.5f, -0.5f, +0.5f };
	positions[face * 4 + 1] = { +0.5f, -0.5f, +0.5f };
	positions[face * 4 + 2] = { -0.5f, -0.5f, -0.5f };
	positions[face * 4 + 3] = { +0.5f, -0.5f, -0.5f };
	texcoords[face * 4 + 0] = { +0.0f, +0.0f };
	texcoords[face * 4 + 1] = { +1.0f, +0.0f };
	texcoords[face * 4 + 2] = { +0.0f, +1.0f };
	texcoords[face * 4 + 3] = { +1.0f, +1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	// front-side
	face += 1;
	positions[face * 4 + 0] = { +0.5f, +0.5f, -0.5f };
	positions[face * 4 + 1] = { -0.5f, +0.5f, -0.5f };
	positions[face * 4 + 2] = { +0.5f, -0.5f, -0.5f };
	positions[face * 4 + 3] = { -0.5f, -0.5f, -0.5f };
	texcoords[face * 4 + 0] = { +0.0f, +0.0f };
	texcoords[face * 4 + 1] = { +1.0f, +0.0f };
	texcoords[face * 4 + 2] = { +0.0f, +1.0f };
	texcoords[face * 4 + 3] = { +1.0f, +1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	// back-side
	face += 1;
	positions[face * 4 + 0] = { -0.5f, +0.5f, +0.5f };
	positions[face * 4 + 1] = { +0.5f, +0.5f, +0.5f };
	positions[face * 4 + 2] = { -0.5f, -0.5f, +0.5f };
	positions[face * 4 + 3] = { +0.5f, -0.5f, +0.5f };
	texcoords[face * 4 + 0] = { +0.0f, +0.0f };
	texcoords[face * 4 + 1] = { +1.0f, +0.0f };
	texcoords[face * 4 + 2] = { +0.0f, +1.0f };
	texcoords[face * 4 + 3] = { +1.0f, +1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	// right-side
	face += 1;
	positions[face * 4 + 1] = { +0.5f, +0.5f, -0.5f };
	positions[face * 4 + 0] = { +0.5f, +0.5f, +0.5f };
	positions[face * 4 + 3] = { +0.5f, -0.5f, -0.5f };
	positions[face * 4 + 2] = { +0.5f, -0.5f, +0.5f };
	texcoords[face * 4 + 0] = { +0.0f, +0.0f };
	texcoords[face * 4 + 1] = { +1.0f, +0.0f };
	texcoords[face * 4 + 2] = { +0.0f, +1.0f };
	texcoords[face * 4 + 3] = { +1.0f, +1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	// left-side
	face += 1;
	positions[face * 4 + 0] = { -0.5f, +0.5f, -0.5f };
	positions[face * 4 + 1] = { -0.5f, +0.5f, +0.5f };
	positions[face * 4 + 2] = { -0.5f, -0.5f, -0.5f };
	positions[face * 4 + 3] = { -0.5f, -0.5f, +0.5f };
	texcoords[face * 4 + 0] = { +0.0f, +0.0f };
	texcoords[face * 4 + 1] = { +1.0f, +0.0f };
	texcoords[face * 4 + 2] = { +0.0f, +1.0f };
	texcoords[face * 4 + 3] = { +1.0f, +1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	create_com_buffers(device, positions, texcoords, 24, indices, 36);
}

geometric_sphere::geometric_sphere(ID3D11Device* device, uint32_t slices, uint32_t stacks) : geometric_primitive(device)
{
	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT2> texcoords;
	std::vector<uint32_t> indices;

	float r{ 0.5f };

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	vertex top_vertex{};
	top_vertex.position = { 0.0f, +r, 0.0f };
	top_vertex.texcoord = { 0.5f, +0.0f };

	vertex bottom_vertex{};
	bottom_vertex.position = { 0.0f, -r, 0.0f };
	bottom_vertex.texcoord = { 0.5f, +1.0f };

	positions.emplace_back(top_vertex.position);
	texcoords.emplace_back(top_vertex.texcoord);

	float phi_step{ DirectX::XM_PI / stacks };
	float theta_step{ 2.0f * DirectX::XM_PI / slices };

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32_t i = 1; i <= stacks - 1; ++i)
	{
		float phi{ i * phi_step };

		// Vertices of ring.
		for (uint32_t j = 0; j <= slices; ++j)
		{
			float theta{ j * theta_step };

			vertex v{};

			// spherical to cartesian
			v.position.x = r * sinf(phi) * cosf(theta);
			v.position.y = r * cosf(phi);
			v.position.z = r * sinf(phi) * sinf(theta);

			v.texcoord.x = theta / DirectX::XM_2PI;
			v.texcoord.y = phi / DirectX::XM_PI;

			positions.emplace_back(v.position);
			texcoords.emplace_back(v.texcoord);
		}
	}

	positions.emplace_back(bottom_vertex.position);
	texcoords.emplace_back(bottom_vertex.texcoord);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//
	for (uint32_t i = 1; i <= slices; ++i)
	{
		indices.emplace_back(0);
		indices.emplace_back(i);
		indices.emplace_back(i + 1);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint32_t base_index{ 1 };
	uint32_t ring_vertex_count{ slices + 1 };
	for (uint32_t i = 0; i < stacks - 2; ++i)
	{
		for (uint32_t j = 0; j < slices; ++j)
		{
			indices.emplace_back(base_index + i * ring_vertex_count + j);
			indices.emplace_back(base_index + (i + 1) * ring_vertex_count + j);
			indices.emplace_back(base_index + i * ring_vertex_count + j + 1);

			indices.emplace_back(base_index + (i + 1) * ring_vertex_count + j);
			indices.emplace_back(base_index + (i + 1) * ring_vertex_count + j + 1);
			indices.emplace_back(base_index + i * ring_vertex_count + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32_t south_pole_index{ static_cast<uint32_t>(positions.size() - 1) };

	// Offset the indices to the index of the first vertex in the last ring.
	base_index = south_pole_index - ring_vertex_count;

	for (uint32_t i = 0; i < slices; ++i)
	{
		indices.emplace_back(south_pole_index);
		indices.emplace_back(base_index + i + 1);
		indices.emplace_back(base_index + i);
	}
	create_com_buffers(device, positions.data(), texcoords.data(), positions.size(), indices.data(), indices.size());
}
