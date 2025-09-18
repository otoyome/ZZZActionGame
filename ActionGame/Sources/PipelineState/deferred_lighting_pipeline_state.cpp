#include "PipelineState/deferred_lighting_pipeline_state.h"

namespace
{
	//	テクスチャ
	static constexpr UINT GBufferMapIndex = 0;
	static constexpr UINT ShadowMapIndex = 10;
	static constexpr UINT DiffuseIBLMapIndex = 33;
	static constexpr UINT SpecularIBLMapIndex = 34;
	static constexpr UINT LUTGGXMapIndex = 35;
	static constexpr UINT SceneColorMapIndex = 0;

	//	サンプラステート
	static constexpr UINT GBufferSamplerIndex = 0;
	static constexpr UINT IBLSamplerIndex = 1;
	static constexpr UINT SceneSamplerIndex = 0;
	static constexpr UINT ShadowMapSamplerIndex = 10;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
deferred_indirect_lighting_pipeline_state::deferred_indirect_lighting_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/fullscreen_quad_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/deferred_rendering_indirect_ps.cso";
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
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::light_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, lcbv.GetAddressOf());
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

deferred_indirect_lighting_pipeline_state::~deferred_indirect_lighting_pipeline_state()
{
}

void deferred_indirect_lighting_pipeline_state::begin(const graphics_context& context)
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
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());

	//	GBufferテクスチャ設定
	ID3D11ShaderResourceView* g_buffer_srvs[static_cast<UINT>(GBufferId::Max)];
	for (UINT index = 0; index < static_cast<UINT>(GBufferId::Max); ++index)
	{
		g_buffer_srvs[index] = context.g_buffer_textures[index]->get_shader_resource_view().Get();
	}
	context.device_context->PSSetShaderResources(GBufferMapIndex, ARRAYSIZE(g_buffer_srvs), g_buffer_srvs);
	context.device_context->PSSetSamplers(GBufferMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());

	//	IBL設定
	context.device_context->PSSetShaderResources(DiffuseIBLMapIndex, 1, context.diffuse_ibl_texture->get_shader_resource_view().GetAddressOf());
	context.device_context->PSSetShaderResources(SpecularIBLMapIndex, 1, context.specluar_ibl_texture->get_shader_resource_view().GetAddressOf());
	context.device_context->PSSetShaderResources(LUTGGXMapIndex, 1, context.lut_ggx_texture->get_shader_resource_view().GetAddressOf());
	context.device_context->PSSetSamplers(IBLSamplerIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());

	//	ライト情報設定
	context.device_context->UpdateSubresource(lcbv.Get(), 0, 0, &context.light_constant, 0, 0);
	context.device_context->PSSetConstantBuffers(common_pipeline_states::LightCBVIndex, 1, lcbv.GetAddressOf());
}

void deferred_indirect_lighting_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void deferred_indirect_lighting_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->Draw(4, 0);
}

void deferred_indirect_lighting_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
deferred_emissive_lighting_pipeline_state::deferred_emissive_lighting_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/fullscreen_quad_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/deferred_rendering_emissive_ps.cso";
	shader_manager::instance()->load_shader<pixel_shader>(ps, ps_desc);

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

deferred_emissive_lighting_pipeline_state::~deferred_emissive_lighting_pipeline_state()
{
}

void deferred_emissive_lighting_pipeline_state::begin(const graphics_context& context)
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

	//	GBufferテクスチャ設定
	ID3D11ShaderResourceView* g_buffer_srvs[static_cast<UINT>(GBufferId::Max)];
	for (UINT index = 0; index < static_cast<UINT>(GBufferId::Max); ++index)
	{
		g_buffer_srvs[index] = context.g_buffer_textures[index]->get_shader_resource_view().Get();
	}
	context.device_context->PSSetShaderResources(GBufferMapIndex, ARRAYSIZE(g_buffer_srvs), g_buffer_srvs);
	context.device_context->PSSetSamplers(GBufferMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());
}

void deferred_emissive_lighting_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void deferred_emissive_lighting_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->Draw(4, 0);
}

void deferred_emissive_lighting_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
deferred_directional_lighting_pipeline_state::deferred_directional_lighting_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/fullscreen_quad_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/deferred_rendering_directional_ps.cso";
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
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::light_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, lcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::shadow_map_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, smcbv.GetAddressOf());
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

deferred_directional_lighting_pipeline_state::~deferred_directional_lighting_pipeline_state()
{
}

void deferred_directional_lighting_pipeline_state::begin(const graphics_context& context)
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
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());

	//	GBufferテクスチャ設定
	ID3D11ShaderResourceView* g_buffer_srvs[static_cast<UINT>(GBufferId::Max)];
	for (UINT index = 0; index < static_cast<UINT>(GBufferId::Max); ++index)
	{
		g_buffer_srvs[index] = context.g_buffer_textures[index]->get_shader_resource_view().Get();
	}
	context.device_context->PSSetShaderResources(GBufferMapIndex, ARRAYSIZE(g_buffer_srvs), g_buffer_srvs);
	context.device_context->PSSetSamplers(GBufferMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());

	//	ライト情報設定
	context.device_context->UpdateSubresource(lcbv.Get(), 0, 0, &context.light_constant, 0, 0);
	context.device_context->PSSetConstantBuffers(common_pipeline_states::LightCBVIndex, 1, lcbv.GetAddressOf());

	//	シャドウマップ設定
	{
		context.device_context->UpdateSubresource(smcbv.Get(), 0, 0, &context.shadow_map_constant, 0, 0);
		context.device_context->PSSetConstantBuffers(common_pipeline_states::ShadowmapCBVIndex, 1, smcbv.GetAddressOf());
		context.device_context->PSSetShaderResources(ShadowMapIndex, 1, context.shadow_map_texture->get_shader_resource_view().GetAddressOf());
		context.device_context->PSSetSamplers(ShadowMapSamplerIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());
	}
}

void deferred_directional_lighting_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void deferred_directional_lighting_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->Draw(4, 0);
}

void deferred_directional_lighting_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
deferred_point_lighting_pipeline_state::deferred_point_lighting_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/fullscreen_quad_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/deferred_rendering_point_ps.cso";
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
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::light_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, lcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::shadow_map_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, smcbv.GetAddressOf());
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

deferred_point_lighting_pipeline_state::~deferred_point_lighting_pipeline_state()
{
}

void deferred_point_lighting_pipeline_state::begin(const graphics_context& context)
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
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());

	//	GBufferテクスチャ設定
	ID3D11ShaderResourceView* g_buffer_srvs[static_cast<UINT>(GBufferId::Max)];
	for (UINT index = 0; index < static_cast<UINT>(GBufferId::Max); ++index)
	{
		g_buffer_srvs[index] = context.g_buffer_textures[index]->get_shader_resource_view().Get();
	}
	context.device_context->PSSetShaderResources(GBufferMapIndex, ARRAYSIZE(g_buffer_srvs), g_buffer_srvs);
	context.device_context->PSSetSamplers(GBufferMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());

	//	ライト情報設定
	context.device_context->UpdateSubresource(lcbv.Get(), 0, 0, &context.light_constant, 0, 0);
	context.device_context->PSSetConstantBuffers(common_pipeline_states::LightCBVIndex, 1, lcbv.GetAddressOf());

	//	シャドウマップ設定
	{
		context.device_context->UpdateSubresource(smcbv.Get(), 0, 0, &context.shadow_map_constant, 0, 0);
		context.device_context->PSSetConstantBuffers(common_pipeline_states::ShadowmapCBVIndex, 1, smcbv.GetAddressOf());
		context.device_context->PSSetShaderResources(ShadowMapIndex, 1, context.shadow_map_texture->get_shader_resource_view().GetAddressOf());
		context.device_context->PSSetSamplers(ShadowMapSamplerIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());
	}
}

void deferred_point_lighting_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void deferred_point_lighting_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->Draw(4, 0);
}

void deferred_point_lighting_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
deferred_spot_lighting_pipeline_state::deferred_spot_lighting_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/fullscreen_quad_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/deferred_rendering_spot_ps.cso";
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
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::light_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, lcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::shadow_map_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, smcbv.GetAddressOf());
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

deferred_spot_lighting_pipeline_state::~deferred_spot_lighting_pipeline_state()
{
}

void deferred_spot_lighting_pipeline_state::begin(const graphics_context& context)
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
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());

	//	GBufferテクスチャ設定
	ID3D11ShaderResourceView* g_buffer_srvs[static_cast<UINT>(GBufferId::Max)];
	for (UINT index = 0; index < static_cast<UINT>(GBufferId::Max); ++index)
	{
		g_buffer_srvs[index] = context.g_buffer_textures[index]->get_shader_resource_view().Get();
	}
	context.device_context->PSSetShaderResources(GBufferMapIndex, ARRAYSIZE(g_buffer_srvs), g_buffer_srvs);
	context.device_context->PSSetSamplers(GBufferMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());

	//	ライト情報設定
	context.device_context->UpdateSubresource(lcbv.Get(), 0, 0, &context.light_constant, 0, 0);
	context.device_context->PSSetConstantBuffers(common_pipeline_states::LightCBVIndex, 1, lcbv.GetAddressOf());

	//	シャドウマップ設定
	{
		context.device_context->UpdateSubresource(smcbv.Get(), 0, 0, &context.shadow_map_constant, 0, 0);
		context.device_context->PSSetConstantBuffers(common_pipeline_states::ShadowmapCBVIndex, 1, smcbv.GetAddressOf());
		context.device_context->PSSetShaderResources(ShadowMapIndex, 1, context.shadow_map_texture->get_shader_resource_view().GetAddressOf());
		context.device_context->PSSetSamplers(ShadowMapSamplerIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());
	}
}

void deferred_spot_lighting_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void deferred_spot_lighting_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->Draw(4, 0);
}

void deferred_spot_lighting_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
deferred_shadow_light_pipeline_state::deferred_shadow_light_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/fullscreen_quad_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/deferred_rendering_shadow_spot_light_ps.cso";
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
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::light_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, lcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::shadow_map_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, smcbv.GetAddressOf());
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
		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO; //D3D11_BLEND_DEST_COLOR
		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR; //D3D11_BLEND_SRC_COLOR
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
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

deferred_shadow_light_pipeline_state::~deferred_shadow_light_pipeline_state()
{
}

void deferred_shadow_light_pipeline_state::begin(const graphics_context& context)
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
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());

	//	GBufferテクスチャ設定
	ID3D11ShaderResourceView* g_buffer_srvs[static_cast<UINT>(GBufferId::Max)];
	for (UINT index = 0; index < static_cast<UINT>(GBufferId::Max); ++index)
	{
		g_buffer_srvs[index] = context.g_buffer_textures[index]->get_shader_resource_view().Get();
	}
	context.device_context->PSSetShaderResources(GBufferMapIndex, ARRAYSIZE(g_buffer_srvs), g_buffer_srvs);
	context.device_context->PSSetSamplers(GBufferMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());

	//	ライト情報設定
	context.device_context->UpdateSubresource(lcbv.Get(), 0, 0, &context.light_constant, 0, 0);
	context.device_context->PSSetConstantBuffers(common_pipeline_states::LightCBVIndex, 1, lcbv.GetAddressOf());

	//	シャドウマップ設定
	{
		context.device_context->UpdateSubresource(smcbv.Get(), 0, 0, &context.shadow_map_constant, 0, 0);
		context.device_context->PSSetConstantBuffers(common_pipeline_states::ShadowmapCBVIndex, 1, smcbv.GetAddressOf());
		context.device_context->PSSetShaderResources(ShadowMapIndex, 1, context.shadow_map_texture->get_shader_resource_view().GetAddressOf());
		context.device_context->PSSetSamplers(ShadowMapSamplerIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR_BORDER_WHITE).GetAddressOf());
	}
}

void deferred_shadow_light_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void deferred_shadow_light_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->Draw(4, 0);
}

void deferred_shadow_light_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
skybox_pipeline_state::skybox_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/sky_box_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/sky_box_gbuffer_ps.cso";
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
			buffer_desc.ByteWidth = sizeof(common_pipeline_states::light_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, lcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
	}

	//	深度ステンシルステート
	{
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
		depth_stencil_desc.DepthEnable = TRUE;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		hr = render_data::instance()->get_device()->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_states.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	// ブレンドステートの生成
	{
		D3D11_BLEND_DESC blend_desc{};
		blend_desc.AlphaToCoverageEnable = FALSE;
		blend_desc.IndependentBlendEnable = FALSE;
		blend_desc.RenderTarget[0].BlendEnable = FALSE;
		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
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

skybox_pipeline_state::~skybox_pipeline_state()
{
}

void skybox_pipeline_state::begin(const graphics_context& context)
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
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());

	//	IBLテクスチャ処理
	static constexpr UINT SkyboxIndex = 0;
	static constexpr UINT SkyboxSamplerIndex = 0;
	context.device_context->PSSetShaderResources(SkyboxIndex, 1, context.specluar_ibl_texture->get_shader_resource_view().GetAddressOf());
	context.device_context->PSSetSamplers(SkyboxSamplerIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());

	//	ライト情報設定
	context.device_context->UpdateSubresource(lcbv.Get(), 0, 0, &context.light_constant, 0, 0);
	context.device_context->PSSetConstantBuffers(common_pipeline_states::LightCBVIndex, 1, lcbv.GetAddressOf());
}

void skybox_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void skybox_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	context.device_context->Draw(4, 0);
}

void skybox_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}
