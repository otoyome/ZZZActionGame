#include "PipelineState/ui_rendering_pipeline_states.h"

namespace
{
	//	マテリアルインデックスバッファ
	constexpr int PrimitiveCBVIndex = 1;
	//	テクスチャ
	constexpr UINT BaseMapIndex = 0;

	//	マテリアル定数バッファ構造体
	struct primitive_constant_buffer
	{
		DirectX::XMFLOAT4X4 matrix;
	};

	//	☆ﾃｷﾄｰ('ω')ｰﾄｷﾃ☆
	void register_input_element_vertex(std::vector<D3D11_INPUT_ELEMENT_DESC>& descs, LPCSTR semantic_name, UINT semantic_index, DXGI_FORMAT format, UINT& byte_offset)
	{
		descs.emplace_back(D3D11_INPUT_ELEMENT_DESC{ semantic_name, semantic_index, format, 0, byte_offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		byte_offset += graphics_utility::get_byte_offset_from_format(format);
	}
	void register_input_element_stream_vertex(std::vector<D3D11_INPUT_ELEMENT_DESC>& descs, LPCSTR semantic_name, UINT semantic_index, DXGI_FORMAT format, UINT& stream_index)
	{
		descs.emplace_back(D3D11_INPUT_ELEMENT_DESC{ semantic_name, semantic_index, format, stream_index, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		++stream_index;
	}
}

ui_pipeline_state::ui_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/sprite_vs.cso";
	{
		UINT stream_index = 0;
		register_input_element_stream_vertex(vs_desc.input_element_desc, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, stream_index);
	}
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/sprite_ps.cso";
	shader_manager::instance()->load_shader<pixel_shader>(ps, ps_desc);

	//	定数バッファの生成
	{
		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		{
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::scene_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, scbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(primitive_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, tcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
	}

	//	深度ステンシルステート
	{
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
		depth_stencil_desc.DepthEnable = TRUE;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		hr = render_data::instance()->get_device()->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	// ブレンドステートの生成
	{
		D3D11_BLEND_DESC blend_desc{};
		blend_desc.AlphaToCoverageEnable = FALSE;
		blend_desc.IndependentBlendEnable = FALSE;
		blend_desc.RenderTarget[0].BlendEnable = true;
		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = render_data::instance()->get_device()->CreateBlendState(&blend_desc, blend_states.GetAddressOf());
	}

	// ラスタライザーステートの生成
	{
		D3D11_RASTERIZER_DESC rasterizer_desc{};
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		rasterizer_desc.FrontCounterClockwise = TRUE;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = TRUE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = FALSE;
		rasterizer_desc.AntialiasedLineEnable = FALSE;
		hr = render_data::instance()->get_device()->CreateRasterizerState(&rasterizer_desc, rasterizer_states.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}

ui_pipeline_state::~ui_pipeline_state()
{
}

void ui_pipeline_state::begin(const graphics_context& context)
{
	//	シェーダー設定
	context.device_context->VSSetShader(vs->get_shader().Get(), nullptr, 0);
	context.device_context->HSSetShader(nullptr, nullptr, 0);
	context.device_context->DSSetShader(nullptr, nullptr, 0);
	context.device_context->GSSetShader(nullptr, nullptr, 0);
	context.device_context->PSSetShader(ps->get_shader().Get(), nullptr, 0);
	context.device_context->IASetInputLayout(vs->get_input_layout().Get());
	context.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	ステート設定
	context.device_context->OMSetBlendState(blend_states.Get(), nullptr, 0xFFFFFFFF);
	context.device_context->OMSetDepthStencilState(depth_stencil_states.Get(), 0);
	context.device_context->RSSetState(rasterizer_states.Get());

	//	シーン定数バッファ設定
	context.device_context->UpdateSubresource(scbv.Get(), 0, 0, &context.scene_constant, 0, 0);
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
}

void ui_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
	texture_ptr texture;
	if (material.get_texture("base_texture", texture))
	{
		context.device_context->PSSetShaderResources(BaseMapIndex, 1, texture->get_shader_resource_view().GetAddressOf());
		context.device_context->PSSetSamplers(BaseMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR_CLAMP).GetAddressOf());
	}
}

void ui_pipeline_state::draw_indexed(const graphics_context& context, const buffer_datas& buffer_data)
{
	{
		primitive_constant_buffer cbv;
		cbv.matrix = buffer_data.transform_matrices[0];
		context.device_context->UpdateSubresource(tcbv.Get(), 0, 0, &cbv, 0, 0);
		context.device_context->VSSetConstantBuffers(PrimitiveCBVIndex, 1, tcbv.GetAddressOf());
	}
	UINT stream_index = 0;
	for (auto& element_desc : vs->get_input_element_desc())
	{
		auto it = buffer_data.vertex_buffer_views->find(semantic(element_desc.SemanticName, element_desc.SemanticIndex));
		if (it != buffer_data.vertex_buffer_views->end())
		{
			UINT stride = graphics_utility::get_byte_offset_from_format(element_desc.Format);
			UINT offset = static_cast<UINT>(it->second.offset()) * stride;
			context.device_context->IASetVertexBuffers(stream_index, 1, it->second.buffer.GetAddressOf(), &stride, &offset);
		}
		else
		{
			UINT offset = 0;
			UINT stride = 0;
			context.device_context->IASetVertexBuffers(stream_index, 1, nullptr, &stride, &offset);
		}
		++stream_index;
	}
	context.device_context->IASetIndexBuffer(buffer_data.index_buffer_view->buffer.Get(), buffer_data.index_buffer_view->format, 0);

	context.device_context->DrawIndexed(static_cast<UINT>(buffer_data.index_buffer_view->count()), static_cast<UINT>(buffer_data.index_buffer_view->offset()), 0);
}

void ui_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}
