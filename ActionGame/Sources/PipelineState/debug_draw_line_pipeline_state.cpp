#include "PipelineState/debug_draw_line_pipeline_state.h"

namespace
{
	//	定数バッファ
	static constexpr UINT SceneCBVIndex = 0;

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

///////////////////////////////////////////////////////////////////////////////////////////////////////
debug_draw_line_pipeline_state::debug_draw_line_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/debug_draw_line_vs.cso";
	{
		UINT byte_offset = 0;
		register_input_element_vertex(vs_desc.input_element_desc, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, byte_offset);
		register_input_element_vertex(vs_desc.input_element_desc, "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, byte_offset);
	}
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/debug_draw_line_ps.cso";
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
	}

	//	深度ステンシルステート
	{
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
		depth_stencil_desc.DepthEnable = TRUE;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		hr = render_data::instance()->get_device()->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	// ブレンドステートの生成
	{
		D3D11_BLEND_DESC blend_desc{};
		blend_desc.AlphaToCoverageEnable = FALSE;
		blend_desc.IndependentBlendEnable = FALSE;
		blend_desc.RenderTarget[0].BlendEnable = TRUE;
		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
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

debug_draw_line_pipeline_state::~debug_draw_line_pipeline_state()
{
}

void debug_draw_line_pipeline_state::begin(const graphics_context& context)
{
	//	シェーダー設定
	context.device_context->VSSetShader(vs->get_shader().Get(), nullptr, 0);
	context.device_context->HSSetShader(nullptr, nullptr, 0);
	context.device_context->DSSetShader(nullptr, nullptr, 0);
	context.device_context->GSSetShader(nullptr, nullptr, 0);
	context.device_context->PSSetShader(ps->get_shader().Get(), nullptr, 0);
	context.device_context->IASetInputLayout(vs->get_input_layout().Get());
	context.device_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

	//	ステート設定
	context.device_context->OMSetBlendState(blend_states.Get(), nullptr, 0xFFFFFFFF);
	context.device_context->OMSetDepthStencilState(depth_stencil_states.Get(), 0);
	context.device_context->RSSetState(rasterizer_states.Get());

	//	シーン定数バッファ設定
	context.device_context->UpdateSubresource(scbv.Get(), 0, 0, &context.scene_constant, 0, 0);
	context.device_context->VSSetConstantBuffers(SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(SceneCBVIndex, 1, scbv.GetAddressOf());
}

void debug_draw_line_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void debug_draw_line_pipeline_state::draw_indexed(const graphics_context& context, const buffer_datas& buffer_data)
{
	//	VERTEX扱いで登録されている事前提
	auto it = buffer_data.vertex_buffer_views->begin();
	UINT stride = static_cast<UINT>(it->second.stride_in_bytes);
	UINT offset = static_cast<UINT>(it->second.offset());
	context.device_context->IASetVertexBuffers(0, 1, it->second.buffer.GetAddressOf(), &stride, &offset);

	//	LINE LIST であつかう
	context.device_context->Draw(static_cast<UINT>(it->second.count()), 0);
}

void debug_draw_line_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}
