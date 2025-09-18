#include "Renderer/ui_renderer.h"

#include "sprite.h"

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
		color_0,
		texcoord_0,
	};
	const format_data format_datas[] =
	{
		{{"POSITION", 0}, DXGI_FORMAT_R32G32B32_FLOAT},
		{{"COLOR", 0}, DXGI_FORMAT_R32G32B32A32_FLOAT},
		{{"TEXCOORD", 0}, DXGI_FORMAT_R32G32_FLOAT},
	};
}

ui_renderer::ui_renderer(std::shared_ptr<scene_render_targets> render_targets)
: renderer()
, render_targets(render_targets)
, viewport({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f })
{
	HRESULT hr;
	resize_buffer(0, 0);

	//	頂点バッファビュー生成
	{
		std::vector<buffer_view> views(ARRAYSIZE(format_datas));
		for (size_t index = 0; index < views.size(); ++index)
		{
			auto& view = views[index];
			view.format = format_datas[index].f;
			view.stride_in_bytes = graphics_utility::get_byte_offset_from_format(view.format);
			view.size_in_bytes = view.stride_in_bytes * vertex_max;
			view.row_data.resize(view.size_in_bytes);
		}
		for (size_t index = 0; index < views.size(); ++index)
		{
			auto& view = views[index];

			D3D11_BUFFER_DESC buffer_desc{};
			buffer_desc.ByteWidth = static_cast<UINT>(view.size_in_bytes);
			buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
			buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA subresource_data{};
			subresource_data.pSysMem = view.row_data.data();
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, &subresource_data, view.buffer.ReleaseAndGetAddressOf());

			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			vertex_buffer_views.insert(std::make_pair(format_datas[index].s, views[index]));
		}
	}

	//	インデックスバッファビュー生成
	{
		auto& view = index_buffer_view;
		view.format = DXGI_FORMAT_R32_UINT;
		view.stride_in_bytes = graphics_utility::get_byte_offset_from_format(view.format);
		view.size_in_bytes = view.stride_in_bytes * index_max;
		view.row_data.resize(view.size_in_bytes);

		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = static_cast<UINT>(view.size_in_bytes);
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA subresource_data{};
		subresource_data.pSysMem = view.row_data.data();
		hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, &subresource_data, view.buffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}

ui_renderer::~ui_renderer()
{
}

void ui_renderer::render(graphics_context& context)
{
	HRESULT hr;
	//	描画先設定
	rendering_pass.activate(context.device_context);

	//	ビューポートの設定
	context.device_context->RSSetViewports(1, &viewport);

	//	バッファ生成
	std::vector<std::unordered_map<semantic, buffer_view>> vbvs;
	std::vector<buffer_view> ibvs;
	{
		D3D11_MAPPED_SUBRESOURCE mapped_vertices[3];
		D3D11_MAPPED_SUBRESOURCE mapped_indices;
		for (auto& it : vertex_buffer_views)
		{
			if (it.first == format_datas[format_id::position].s)
			{
				hr = context.device_context->Map(it.second.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_vertices[format_id::position]);
			}
			else if (it.first == format_datas[format_id::color_0].s)
			{
				hr = context.device_context->Map(it.second.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_vertices[format_id::color_0]);
			}
			else if (it.first == format_datas[format_id::texcoord_0].s)
			{
				hr = context.device_context->Map(it.second.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_vertices[format_id::texcoord_0]);
			}
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		hr = context.device_context->Map(index_buffer_view.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_indices);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		DirectX::XMFLOAT3* positions = static_cast<DirectX::XMFLOAT3*>(mapped_vertices[format_id::position].pData);
		DirectX::XMFLOAT4* colors = static_cast<DirectX::XMFLOAT4*>(mapped_vertices[format_id::color_0].pData);
		DirectX::XMFLOAT2* texcoords = static_cast<DirectX::XMFLOAT2*>(mapped_vertices[format_id::texcoord_0].pData);
		UINT* indices = static_cast<UINT*>(mapped_indices.pData);

		//	情報バッファに纏める
		UINT vertex_size = 0, index_size = 0;
		for (auto& sprite : context.sprite_container.sprite_datas)
		{
			//	頂点情報を書き込む
			size_t vertex_offset = vertex_size;
			vertex_size += static_cast<UINT>(sprite.sprite->get_positions().size());
			assert(vertex_size < vertex_max);
			{
				const DirectX::XMFLOAT3* positions_ptr = sprite.sprite->get_positions().data();
				memcpy_s(positions + vertex_offset, sizeof(DirectX::XMFLOAT3) * (vertex_max - vertex_offset),
							positions_ptr, sizeof(DirectX::XMFLOAT3) * (vertex_size - vertex_offset));
				const DirectX::XMFLOAT4* colors_ptr = sprite.sprite->get_colors().data();
				memcpy_s(colors + vertex_offset, sizeof(DirectX::XMFLOAT4) * (vertex_max - vertex_offset),
							colors_ptr, sizeof(DirectX::XMFLOAT4) * (vertex_size - vertex_offset));
				const DirectX::XMFLOAT2* texcoords_ptr = sprite.sprite->get_texcoords().data();
				memcpy_s(texcoords + vertex_offset, sizeof(DirectX::XMFLOAT2) * (vertex_max - vertex_offset),
							texcoords_ptr, sizeof(DirectX::XMFLOAT2) * (vertex_size - vertex_offset));
			}
			//	インデックス情報を書き込む
			size_t index_offset = index_size;
			index_size += static_cast<UINT>(sprite.sprite->get_indices().size());
			assert(index_size < index_max);
			{
				const UINT* indices_ptr = sprite.sprite->get_indices().data();
				memcpy_s(indices + index_offset, sizeof(UINT) * (index_max - index_offset),
						 indices_ptr, sizeof(UINT) * (index_size - index_offset));
			}
			//	各情報に纏める
			{
				std::unordered_map<semantic, buffer_view> map;
				for (auto& it : vertex_buffer_views)
				{
					buffer_view view;
					view.format = it.second.format;
					view.buffer = it.second.buffer;
					view.stride_in_bytes = it.second.stride_in_bytes;
					view.size_in_bytes = (vertex_size - vertex_offset) * view.stride_in_bytes;
					view.offset_in_bytes = vertex_offset * view.stride_in_bytes;
					map.insert_or_assign(it.first, view);
				}
				vbvs.emplace_back(map);
				{
					ibvs.emplace_back();
					ibvs.back().format = index_buffer_view.format;
					ibvs.back().buffer = index_buffer_view.buffer;
					ibvs.back().stride_in_bytes = index_buffer_view.stride_in_bytes;
					ibvs.back().size_in_bytes = (index_size - index_offset) * ibvs.back().stride_in_bytes;
					ibvs.back().offset_in_bytes = index_offset * ibvs.back().stride_in_bytes;
				}
			}
		}

		for (auto& it : vertex_buffer_views)
		{
			context.device_context->Unmap(it.second.buffer.Get(), 0);
		}
		context.device_context->Unmap(index_buffer_view.buffer.Get(), 0);
	}

	//	描画
	sprite_render(material::blend_modes::translucent, context, vbvs, ibvs);

	rendering_pass.deactivate(context.device_context);
}

void ui_renderer::sprite_render(material::blend_modes blend_mode, graphics_context& context, const std::vector<std::unordered_map<semantic, buffer_view>>& vbvs, const std::vector<buffer_view>& ibvs)
{
	//	描画
	UINT buffer_index = 0;
	pipeline_state_ptr current_pipeline_state;
	for (auto& sprite_data : context.sprite_container.sprite_datas)
	{
		auto& vbs = vbvs[buffer_index];
		auto& ibs = ibvs[buffer_index];
		++buffer_index;

		auto& sprite = sprite_data.sprite;
		//	ドメイン&ブレンドモードチェック
		if (sprite->get_material().get_domain() != material::domains::surface ||
			sprite->get_material().get_blend_mode() != blend_mode)
			continue;

		if (auto pipeline_state = sprite->get_material().get_pipeline_state(material::surface_paths::ui_pass))
		{
			if (pipeline_state != current_pipeline_state)
			{
				if (current_pipeline_state)
					current_pipeline_state->end(context);

				current_pipeline_state = pipeline_state;
				current_pipeline_state->begin(context);
			}
			current_pipeline_state->set_parameters(context, sprite->get_material());

			buffer_datas buffer_data;
			buffer_data.vertex_buffer_views = &vbs;
			buffer_data.index_buffer_view = &ibs;
			buffer_data.transform_matrices.push_back(sprite->get_transform_matrix());
			buffer_data.stride_transform_by_index = 1;
			current_pipeline_state->draw_indexed(context, buffer_data);
		}
	}
	if (current_pipeline_state)
		current_pipeline_state->end(context);
	}

void ui_renderer::resize_buffer(int width, int height)
{
	rendering_pass.render_targets.emplace_back(render_data::instance()->get_swap_chain());
	rendering_pass.render_targets.back().clear_color = false;

	DXGI_SWAP_CHAIN_DESC desc;
	render_data::instance()->get_swap_chain()->GetDesc(&desc);

	// ビューポートの設定
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(desc.BufferDesc.Width);
	viewport.Height = static_cast<float>(desc.BufferDesc.Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}
