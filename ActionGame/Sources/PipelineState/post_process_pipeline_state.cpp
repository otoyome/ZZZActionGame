#include "PipelineState/post_process_pipeline_state.h"

namespace
{
	//	定数バッファ
	static constexpr UINT SceneCBVIndex = 0;
	static constexpr int GameColorFilterCBVIndex = 11;

	//	テクスチャ
	static constexpr UINT GBufferMapIndex = 0;
	static constexpr UINT OverwriteTextMapIndex = 11;

	//	サンプラステート
	static constexpr UINT GBufferSamplerIndex = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
write_back_pipeline_state::write_back_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/fullscreen_quad_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/fullscreen_quad_ps.cso";
	shader_manager::instance()->load_shader<pixel_shader>(ps, ps_desc);

	// ブレンドステートの生成
	{
		D3D11_BLEND_DESC blend_desc{};
		blend_desc.AlphaToCoverageEnable = FALSE;
		blend_desc.IndependentBlendEnable = FALSE;
		blend_desc.RenderTarget[0].BlendEnable = FALSE;
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

write_back_pipeline_state::~write_back_pipeline_state()
{
}

void write_back_pipeline_state::begin(const graphics_context& context)
{
	//	シェーダー設定
	context.device_context->VSSetShader(vs->get_shader().Get(), nullptr, 0);
	context.device_context->HSSetShader(nullptr, nullptr, 0);
	context.device_context->DSSetShader(nullptr, nullptr, 0);
	context.device_context->GSSetShader(nullptr, nullptr, 0);
	context.device_context->PSSetShader(ps->get_shader().Get(), nullptr, 0);
	context.device_context->IASetInputLayout(vs->get_input_layout().Get());
	context.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//	ステート設定
	context.device_context->OMSetBlendState(blend_states.Get(), nullptr, 0xFFFFFFFF);
	context.device_context->OMSetDepthStencilState(depth_stencil_states.Get(), 0);
	context.device_context->RSSetState(rasterizer_states.Get());

	context.device_context->PSSetShaderResources(0, 1, context.scene_texture->get_shader_resource_view().GetAddressOf());
	context.device_context->PSSetSamplers(0, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());
}

void write_back_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void write_back_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->Draw(4, 0);
}

void write_back_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
color_filter_pre_effect_pipeline_state::color_filter_pre_effect_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/fullscreen_quad_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/color_filter_pre_effect_ps.cso";
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
		depth_stencil_desc.DepthEnable = FALSE;
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

color_filter_pre_effect_pipeline_state::~color_filter_pre_effect_pipeline_state()
{
}

void color_filter_pre_effect_pipeline_state::begin(const graphics_context& context)
{
	//	シェーダー設定
	context.device_context->VSSetShader(vs->get_shader().Get(), nullptr, 0);
	context.device_context->HSSetShader(nullptr, nullptr, 0);
	context.device_context->DSSetShader(nullptr, nullptr, 0);
	context.device_context->GSSetShader(nullptr, nullptr, 0);
	context.device_context->PSSetShader(ps->get_shader().Get(), nullptr, 0);
	context.device_context->IASetInputLayout(vs->get_input_layout().Get());
	context.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//	ステート設定
	context.device_context->OMSetBlendState(blend_states.Get(), nullptr, 0xFFFFFFFF);
	context.device_context->OMSetDepthStencilState(depth_stencil_states.Get(), 0);
	context.device_context->RSSetState(rasterizer_states.Get());

	//	シーン定数バッファ設定
	context.device_context->UpdateSubresource(scbv.Get(), 0, 0, &context.scene_constant, 0, 0);
	context.device_context->VSSetConstantBuffers(SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(SceneCBVIndex, 1, scbv.GetAddressOf());

	//	GBufferテクスチャ設定
	ID3D11ShaderResourceView* g_buffer_srvs[static_cast<UINT>(GBufferId::Max)];
	for (UINT index = 0; index < static_cast<UINT>(GBufferId::Max); ++index)
	{
		g_buffer_srvs[index] = context.g_buffer_textures[index]->get_shader_resource_view().Get();
	}
	context.device_context->PSSetShaderResources(GBufferMapIndex, ARRAYSIZE(g_buffer_srvs), g_buffer_srvs);
	context.device_context->PSSetSamplers(GBufferMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());

	//	カラーバッファだけ入れ替えておく
	context.device_context->PSSetShaderResources(static_cast<UINT>(GBufferId::BaseColor), 1, context.scene_texture->get_shader_resource_view().GetAddressOf());
}

void color_filter_pre_effect_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void color_filter_pre_effect_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->Draw(4, 0);
}

void color_filter_pre_effect_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
color_filter_after_effect_pipeline_state::color_filter_after_effect_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/fullscreen_quad_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/color_filter_after_effect_ps.cso";
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
			buffer_desc.ByteWidth = sizeof(color_filter_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, cgcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
	}

	//	深度ステンシルステート
	{
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
		depth_stencil_desc.DepthEnable = FALSE;
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

color_filter_after_effect_pipeline_state::~color_filter_after_effect_pipeline_state()
{
}

void color_filter_after_effect_pipeline_state::begin(const graphics_context& context)
{
	//	シェーダー設定
	context.device_context->VSSetShader(vs->get_shader().Get(), nullptr, 0);
	context.device_context->HSSetShader(nullptr, nullptr, 0);
	context.device_context->DSSetShader(nullptr, nullptr, 0);
	context.device_context->GSSetShader(nullptr, nullptr, 0);
	context.device_context->PSSetShader(ps->get_shader().Get(), nullptr, 0);
	context.device_context->IASetInputLayout(vs->get_input_layout().Get());
	context.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//	ステート設定
	context.device_context->OMSetBlendState(blend_states.Get(), nullptr, 0xFFFFFFFF);
	context.device_context->OMSetDepthStencilState(depth_stencil_states.Get(), 0);
	context.device_context->RSSetState(rasterizer_states.Get());

	//	シーン定数バッファ設定
	context.device_context->UpdateSubresource(scbv.Get(), 0, 0, &context.scene_constant, 0, 0);
	context.device_context->VSSetConstantBuffers(SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(SceneCBVIndex, 1, scbv.GetAddressOf());

	color_filter_constant_buffer cb;
	cb.color_grading = context.post_process_data.color_grading;
	cb.radial_blur = context.post_process_data.radial_blur;
	cb.separate_color = context.post_process_data.separate_color;
	cb.overwrite_text = context.post_process_data.overwrite_text;
	cb.concentrated_line_work = context.post_process_data.concentrated_line_work;
	cb.distortion_data = context.post_process_data.distortion_data;
	cb.sin_distortion_datas[0] = context.post_process_data.sin_distortion_datas[0];
	cb.sin_distortion_datas[1] = context.post_process_data.sin_distortion_datas[1];
	cb.voronoi_distortion_data = context.post_process_data.voronoi_distortion_data;

	context.device_context->UpdateSubresource(cgcbv.Get(), 0, 0, &cb, 0, 0);
	context.device_context->VSSetConstantBuffers(GameColorFilterCBVIndex, 1, cgcbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(GameColorFilterCBVIndex, 1, cgcbv.GetAddressOf());

	//	GBufferテクスチャ設定
	ID3D11ShaderResourceView* g_buffer_srvs[static_cast<UINT>(GBufferId::Max)];
	for (UINT index = 0; index < static_cast<UINT>(GBufferId::Max); ++index)
	{
		g_buffer_srvs[index] = context.g_buffer_textures[index]->get_shader_resource_view().Get();
	}
	context.device_context->PSSetShaderResources(GBufferMapIndex, ARRAYSIZE(g_buffer_srvs), g_buffer_srvs);
	context.device_context->PSSetSamplers(GBufferMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());

	//	カラーバッファだけ入れ替えておく
	context.device_context->PSSetShaderResources(static_cast<UINT>(GBufferId::BaseColor), 1, context.scene_texture->get_shader_resource_view().GetAddressOf());

	//	文字テクスチャを入れる
	if (context.post_process_data.overwrite_text_texture)
		context.device_context->PSSetShaderResources(OverwriteTextMapIndex, 1, context.post_process_data.overwrite_text_texture->get_shader_resource_view().GetAddressOf());
}

void color_filter_after_effect_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void color_filter_after_effect_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->Draw(4, 0);
}

void color_filter_after_effect_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}
