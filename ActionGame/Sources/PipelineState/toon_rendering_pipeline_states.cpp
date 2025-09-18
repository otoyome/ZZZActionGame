#include "PipelineState/toon_rendering_pipeline_states.h"

namespace
{
	//	マテリアルインデックスバッファ
	static constexpr UINT MaterialCBVIndex = 5;
	//	テクスチャ
	static constexpr UINT BaseMapIndex = 0;
	static constexpr UINT IlmMapIndex = 1;
	static constexpr UINT SssMapIndex = 2;
	static constexpr UINT EmissiveMapIndex = 3;
	static constexpr UINT DecalMapIndex = 4;
	static constexpr UINT TextMapIndex = 5;
	static constexpr UINT DisolveMapIndex = 6;
	static constexpr UINT FaceThresholdMapIndex = 7;
	static constexpr UINT TextureMax = 8;

	//  マテリアルフラグとG-Bufferステンシルの対応表
	constexpr std::pair<material::flags, UINT> g_buffer_flag_to_material_flag[] =
	{
		{material::flags::indirect_lighting_off, common_pipeline_states::g_buffer_stencil_indirect_lighting_off},
		{material::flags::filter_off, common_pipeline_states::g_buffer_stencil_filter_off},
		{material::flags::character_flag, common_pipeline_states::g_buffer_stencil_character_flag},

		{material::flags::normal_depth_outline_off, common_pipeline_states::g_buffer_stencil_normal_depth_outline_off},
		{material::flags::color_line_off, common_pipeline_states::g_buffer_stencil_color_outline_off},
		{material::flags::noise_off, common_pipeline_states::g_buffer_stencil_noise_off},
		{material::flags::color_postarize_off, common_pipeline_states::g_buffer_stencil_color_posterize_off},
		{material::flags::light_direction_ignore_y, common_pipeline_states::g_buffer_stencil_light_direction_ignore_y},
		{material::flags::contact_shadow_on, common_pipeline_states::g_buffer_stencil_contact_shadow_on},
		{material::flags::camera_distance_translucent, common_pipeline_states::g_buffer_stencil_camera_distance_translucent},
	};

	//	プリミティブ定数バッファ構造体
	static constexpr UINT MeshBoneMax = 512;
	struct primitive_constant_buffer
	{
		DirectX::XMFLOAT4X4 bone_transforms[MeshBoneMax];
	};

	struct material_flag_to_g_buffer_stencil
	{
		material::flags flag;
		UINT stencil_flags;
	};
	static constexpr material_flag_to_g_buffer_stencil material_flag_pair[] =
	{
		{material::flags::filter_off, }
	};

	//	マテリアル定数バッファ構造体
	struct material_constant_buffer
	{
		DirectX::XMINT4		texture_index[TextureMax / 4];
		DirectX::XMFLOAT4	material_color[TextureMax];

		UINT				texture_srgb_flags;
		UINT				material_attribute;
		UINT				game_material_attribute;
		float				alpha_clip_threshold;

		//  材質パラメーター
		DirectX::XMFLOAT4	material_parameters;			// r : 粗さ, g : 金属質, b : 陰影諧調数, a : ノイズ係数

		//  背面法による輪郭線情報
		float				outline_z_offset_max;           //	Zオフセット距離
		float				outline_depth_factor;           //	膨張係数
		DirectX::XMFLOAT2	outline_depth_range;            //	膨張範囲

		float				outline_size;                   //  輪郭線の基本サイズ
		//	影関係
		UINT				shadow_category;	            //	シャドウカテゴリー
		DirectX::XMFLOAT2	shadow_threshold_uv_offset;	    //	影閾値マップのオフセット

		DirectX::XMFLOAT4	mesh_direction;	                //	メッシュの方向(xy : 右ベクトル, zw : 左ベクトル)

		//  ポストエフェクトの色差によるライン算出係数
		float				color_diff_line_factor;         //	アウトライン係数
		float				color_diff_line_threshold;      //	検知係数
		float				color_line_adjust_saturate;     //	ライン彩度調整係数
		float				color_line_adjust_brightness;   //	ライン明度調整係数

		float				color_posterize_factor;         //	色のポスタライズ数
		float				depth_normal_outline_factor;    //	ポストエフェクトの法線/深度によるアウトライン算出係数
		DirectX::XMUINT2	material_constant_dummy;

		DirectX::XMFLOAT4	outline_adjust_parameter;		//	輪郭線調整距離パラメーター
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

	material_constant_buffer set_material_parameters_for_toon(const graphics_context& context, const material& material)
	{
		//	マテリアル定数バッファ設定
		material_constant_buffer	data{};
		for (auto& d : data.texture_index)
			d.x = d.y = d.z = d.w = -1;
		data.texture_srgb_flags = 0;

		texture_ptr texture;
		if (material.get_texture("base_texture", texture))
		{
			data.texture_index[0].x = 0;
			if (texture->get_type() == texture::types::color)
			{
				system_utility::set_bitflag(data.texture_srgb_flags, 1 << BaseMapIndex, !texture->is_flag(texture::flags::srgb));
			}
			context.device_context->PSSetShaderResources(BaseMapIndex, 1, texture->get_shader_resource_view().GetAddressOf());
			context.device_context->PSSetSamplers(BaseMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());
		}
		material.get_float4("base_color", data.material_color[BaseMapIndex]);

		if (material.get_texture("sss_texture", texture))
		{
			data.texture_index[0].x = 0;
			if (texture->get_type() == texture::types::color)
			{
				system_utility::set_bitflag(data.texture_srgb_flags, 1 << SssMapIndex, !texture->is_flag(texture::flags::srgb));
			}
			context.device_context->PSSetShaderResources(SssMapIndex, 1, texture->get_shader_resource_view().GetAddressOf());
			context.device_context->PSSetSamplers(SssMapIndex, 1, render_data::instance()->get_sampler_states(render_data::SAMPLER_STATE::LINEAR).GetAddressOf());
		}
		material.get_float4("sss_color", data.material_color[SssMapIndex]);

		data.material_attribute = 0;
		for (auto& pair : g_buffer_flag_to_material_flag)
		{
			if (material.is_flag(pair.first))
				system_utility::set_bitflag(data.material_attribute, pair.second, true);
		}

		data.game_material_attribute = 0;	//	未使用
		data.alpha_clip_threshold = material.get_alpha_clip_threshold();

		material.get_float("roughness", data.material_parameters.x);
		material.get_float("metalness", data.material_parameters.y);
		material.get_float("shading_steps", data.material_parameters.z);
		material.get_float("noise_factor", data.material_parameters.w);

		material.get_float("outline_z_offset_max", data.outline_z_offset_max);
		material.get_float("outline_depth_factor", data.outline_depth_factor);
		material.get_float2("outline_depth_range", data.outline_depth_range);

		material.get_float("outline_size", data.outline_size);
		material.get_uint("shadow_category", data.shadow_category);
		material.get_float2("shadow_threshold_uv_offset", data.shadow_threshold_uv_offset);

		material.get_float4("mesh_direction", data.mesh_direction);

		material.get_float("color_diff_line_factor", data.color_diff_line_factor);
		material.get_float("color_diff_line_threshold", data.color_diff_line_threshold);
		material.get_float("color_line_adjust_saturate", data.color_line_adjust_saturate);
		material.get_float("color_line_adjust_brightness", data.color_line_adjust_brightness);

		material.get_float("color_posterize_factor", data.color_posterize_factor);
		material.get_float("depth_normal_outline_factor", data.depth_normal_outline_factor);

		material.get_float4("outline_adjust_parameter", data.outline_adjust_parameter);

		return data;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
toon_depth_write_pipeline_state::toon_depth_write_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/toon_depth_vs.cso";
	{
		UINT stream_index = 0;
		register_input_element_stream_vertex(vs_desc.input_element_desc, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "COLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "JOINTS", 0, DXGI_FORMAT_R32G32B32A32_UINT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "JOINTS", 1, DXGI_FORMAT_R32G32B32A32_UINT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "WEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
	}
	shader_manager::instance()->load_shader(vs, vs_desc);

	load_shader_cso gs_desc;
	gs_desc.file_path = "Assets/CSO/toon_gbuffer_outline_gs.cso";
	shader_manager::instance()->load_shader(gs, gs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/toon_depth_ps.cso";
	shader_manager::instance()->load_shader(ps, ps_desc);

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
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, pcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(primitive_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, ppcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(material_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, mcbv.GetAddressOf());
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
		rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = TRUE;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = TRUE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = FALSE;
		rasterizer_desc.AntialiasedLineEnable = FALSE;
		hr = render_data::instance()->get_device()->CreateRasterizerState(&rasterizer_desc, rasterizer_states[0].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		hr = render_data::instance()->get_device()->CreateRasterizerState(&rasterizer_desc, rasterizer_states[1].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}

toon_depth_write_pipeline_state::~toon_depth_write_pipeline_state()
{
}

void toon_depth_write_pipeline_state::begin(const graphics_context& context)
{
	//	シェーダー設定
	context.device_context->VSSetShader(vs->get_shader().Get(), nullptr, 0);
	context.device_context->HSSetShader(nullptr, nullptr, 0);
	context.device_context->DSSetShader(nullptr, nullptr, 0);
	context.device_context->PSSetShader(ps->get_shader().Get(), nullptr, 0);
	context.device_context->IASetInputLayout(vs->get_input_layout().Get());
	context.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	ステート設定
	context.device_context->OMSetBlendState(blend_states.Get(), nullptr, 0xFFFFFFFF);
	context.device_context->OMSetDepthStencilState(depth_stencil_states.Get(), 0);

	//	シーン定数バッファ設定
	context.device_context->UpdateSubresource(scbv.Get(), 0, 0, &context.scene_constant, 0, 0);
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->GSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
}

void toon_depth_write_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
	//	定数バッファ設定
	material_constant_buffer data = set_material_parameters_for_toon(context, material);
	context.device_context->UpdateSubresource(mcbv.Get(), 0, 0, &data, 0, 0);
	context.device_context->VSSetConstantBuffers(MaterialCBVIndex, 1, mcbv.GetAddressOf());
	context.device_context->GSSetConstantBuffers(MaterialCBVIndex, 1, mcbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(MaterialCBVIndex, 1, mcbv.GetAddressOf());

	//	背面法によるアウトライン使用
	if (material.is_flag(material::flags::use_reverse_outline))
	{
		context.device_context->GSSetShader(gs->get_shader().Get(), nullptr, 0);
	}
	else
	{
		context.device_context->GSSetShader(nullptr, nullptr, 0);
	}

	//	両面描画
	if (material.is_flag(material::flags::two_sided))
	{
		context.device_context->RSSetState(rasterizer_states[1].Get());
	}
	else
	{
		context.device_context->RSSetState(rasterizer_states[0].Get());
	}
}

void toon_depth_write_pipeline_state::draw_indexed(const graphics_context& context, const buffer_datas& buffer_data)
{
	{
		primitive_constant_buffer pcb;
		memcpy_s(pcb.bone_transforms,
				 sizeof(pcb.bone_transforms),
				 buffer_data.transform_matrices.data(),
				 buffer_data.transform_matrices.size() * sizeof(DirectX::XMFLOAT4X4));
		context.device_context->UpdateSubresource(pcbv.Get(), 0, 0, &pcb, 0, 0);
		context.device_context->VSSetConstantBuffers(common_pipeline_states::PrimitiveCBVIndex, 1, pcbv.GetAddressOf());
		context.device_context->GSSetConstantBuffers(common_pipeline_states::PrimitiveCBVIndex, 1, pcbv.GetAddressOf());
		context.device_context->PSSetConstantBuffers(common_pipeline_states::PrimitiveCBVIndex, 1, pcbv.GetAddressOf());
	}
	{
		primitive_constant_buffer pcb;
		memcpy_s(pcb.bone_transforms,
			sizeof(pcb.bone_transforms),
			buffer_data.transform_matrices.data(),
			buffer_data.transform_matrices.size() * sizeof(DirectX::XMFLOAT4X4));
		context.device_context->UpdateSubresource(ppcbv.Get(), 0, 0, &pcb, 0, 0);
		context.device_context->VSSetConstantBuffers(common_pipeline_states::PreviousPrimitiveCBVIndex, 1, ppcbv.GetAddressOf());
		context.device_context->GSSetConstantBuffers(common_pipeline_states::PreviousPrimitiveCBVIndex, 1, ppcbv.GetAddressOf());
		context.device_context->PSSetConstantBuffers(common_pipeline_states::PreviousPrimitiveCBVIndex, 1, ppcbv.GetAddressOf());
	}

	UINT stream_index = 0;
	for (auto& element_desc : vs->get_input_element_desc())
	{
		auto it = buffer_data.vertex_buffer_views->find(semantic(element_desc.SemanticName, element_desc.SemanticIndex));
		if (it != buffer_data.vertex_buffer_views->end())
		{
			UINT offset = static_cast<UINT>(it->second.offset());
			UINT stride = graphics_utility::get_byte_offset_from_format(element_desc.Format);
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
	context.device_context->IASetIndexBuffer(buffer_data.index_buffer_view->buffer.Get(), buffer_data.index_buffer_view->format, static_cast<UINT>(buffer_data.index_buffer_view->offset()));

	context.device_context->DrawIndexed(static_cast<UINT>(buffer_data.index_buffer_view->count()), 0, 0);
}

void toon_depth_write_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
toon_csm_write_pipeline_state::toon_csm_write_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/toon_csm_vs.cso";
	{
		UINT stream_index = 0;
		register_input_element_stream_vertex(vs_desc.input_element_desc, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "COLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "JOINTS", 0, DXGI_FORMAT_R32G32B32A32_UINT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "JOINTS", 1, DXGI_FORMAT_R32G32B32A32_UINT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "WEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
	}
	shader_manager::instance()->load_shader(vs, vs_desc);

	load_shader_cso gs_desc;
	gs_desc.file_path = "Assets/CSO/toon_csm_gs.cso";
	shader_manager::instance()->load_shader(gs, gs_desc);

//	load_shader_cso ps_desc;
//	ps_desc.file_path = "Assets/CSO/toon_depth_ps.cso";
//	shader_manager::instance()->load_shader(ps, ps_desc);

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
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, pcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(primitive_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, ppcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(material_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, mcbv.GetAddressOf());
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
		rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = TRUE;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = TRUE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = FALSE;
		rasterizer_desc.AntialiasedLineEnable = FALSE;
		hr = render_data::instance()->get_device()->CreateRasterizerState(&rasterizer_desc, rasterizer_states[0].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		hr = render_data::instance()->get_device()->CreateRasterizerState(&rasterizer_desc, rasterizer_states[1].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}

toon_csm_write_pipeline_state::~toon_csm_write_pipeline_state()
{
}

void toon_csm_write_pipeline_state::begin(const graphics_context& context)
{
	//	シェーダー設定
	context.device_context->VSSetShader(vs->get_shader().Get(), nullptr, 0);
	context.device_context->HSSetShader(nullptr, nullptr, 0);
	context.device_context->DSSetShader(nullptr, nullptr, 0);
	context.device_context->GSSetShader(gs->get_shader().Get(), nullptr, 0);
	context.device_context->PSSetShader(nullptr, nullptr, 0);
	context.device_context->IASetInputLayout(vs->get_input_layout().Get());
	context.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	ステート設定
	context.device_context->OMSetBlendState(blend_states.Get(), nullptr, 0xFFFFFFFF);
	context.device_context->OMSetDepthStencilState(depth_stencil_states.Get(), 0);

	//	シーン定数バッファ設定
	context.device_context->UpdateSubresource(scbv.Get(), 0, 0, &context.scene_constant, 0, 0);
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->GSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());

	//	シャドウマップ定数バッファ設定
	context.device_context->UpdateSubresource(smcbv.Get(), 0, 0, &context.shadow_map_constant, 0, 0);
	context.device_context->VSSetConstantBuffers(common_pipeline_states::ShadowmapCBVIndex, 1, smcbv.GetAddressOf());
	context.device_context->GSSetConstantBuffers(common_pipeline_states::ShadowmapCBVIndex, 1, smcbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::ShadowmapCBVIndex, 1, smcbv.GetAddressOf());
}

void toon_csm_write_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
	//	定数バッファ設定
	material_constant_buffer data = set_material_parameters_for_toon(context, material);

	//	書き込み判定
	render_shadow = system_utility::is_bitflag(context.shadow_map_constant.shadow_mask, 1 << data.shadow_category);
	if (!render_shadow)
		return;

	context.device_context->UpdateSubresource(mcbv.Get(), 0, 0, &data, 0, 0);
	context.device_context->VSSetConstantBuffers(MaterialCBVIndex, 1, mcbv.GetAddressOf());
	context.device_context->GSSetConstantBuffers(MaterialCBVIndex, 1, mcbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(MaterialCBVIndex, 1, mcbv.GetAddressOf());

	//	両面描画
	if (material.is_flag(material::flags::two_sided))
	{
		context.device_context->RSSetState(rasterizer_states[1].Get());
	}
	else
	{
		context.device_context->RSSetState(rasterizer_states[0].Get());
	}
}

void toon_csm_write_pipeline_state::draw_indexed(const graphics_context& context, const buffer_datas& buffer_data)
{
	if (!render_shadow)
		return;

	{
		primitive_constant_buffer pcb;
		memcpy_s(pcb.bone_transforms,
			sizeof(pcb.bone_transforms),
			buffer_data.transform_matrices.data(),
			buffer_data.transform_matrices.size() * sizeof(DirectX::XMFLOAT4X4));
		context.device_context->UpdateSubresource(pcbv.Get(), 0, 0, &pcb, 0, 0);
		context.device_context->VSSetConstantBuffers(common_pipeline_states::PrimitiveCBVIndex, 1, pcbv.GetAddressOf());
		context.device_context->GSSetConstantBuffers(common_pipeline_states::PrimitiveCBVIndex, 1, pcbv.GetAddressOf());
		context.device_context->PSSetConstantBuffers(common_pipeline_states::PrimitiveCBVIndex, 1, pcbv.GetAddressOf());
	}
	{
		primitive_constant_buffer pcb;
		memcpy_s(pcb.bone_transforms,
			sizeof(pcb.bone_transforms),
			buffer_data.transform_matrices.data(),
			buffer_data.transform_matrices.size() * sizeof(DirectX::XMFLOAT4X4));
		context.device_context->UpdateSubresource(ppcbv.Get(), 0, 0, &pcb, 0, 0);
		context.device_context->VSSetConstantBuffers(common_pipeline_states::PreviousPrimitiveCBVIndex, 1, ppcbv.GetAddressOf());
		context.device_context->GSSetConstantBuffers(common_pipeline_states::PreviousPrimitiveCBVIndex, 1, ppcbv.GetAddressOf());
		context.device_context->PSSetConstantBuffers(common_pipeline_states::PreviousPrimitiveCBVIndex, 1, ppcbv.GetAddressOf());
	}

	UINT stream_index = 0;
	for (auto& element_desc : vs->get_input_element_desc())
	{
		auto it = buffer_data.vertex_buffer_views->find(semantic(element_desc.SemanticName, element_desc.SemanticIndex));
		if (it != buffer_data.vertex_buffer_views->end())
		{
			UINT offset = static_cast<UINT>(it->second.offset());
			UINT stride = graphics_utility::get_byte_offset_from_format(element_desc.Format);
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
	context.device_context->IASetIndexBuffer(buffer_data.index_buffer_view->buffer.Get(), buffer_data.index_buffer_view->format, static_cast<UINT>(buffer_data.index_buffer_view->offset()));

	context.device_context->DrawIndexed(static_cast<UINT>(buffer_data.index_buffer_view->count()), 0, 0);
}

void toon_csm_write_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
toon_gbuffer_pipeline_state::toon_gbuffer_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/toon_vs.cso";
	{
		UINT stream_index = 0;
		register_input_element_stream_vertex(vs_desc.input_element_desc, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "COLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "JOINTS", 0, DXGI_FORMAT_R32G32B32A32_UINT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "JOINTS", 1, DXGI_FORMAT_R32G32B32A32_UINT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
		register_input_element_stream_vertex(vs_desc.input_element_desc, "WEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, stream_index);
	}
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso gs_desc;
	gs_desc.file_path = "Assets/CSO/toon_gbuffer_outline_gs.cso";
	shader_manager::instance()->load_shader<geometry_shader>(gs, gs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/toon_gbuffer_ps.cso";
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
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, pcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(primitive_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, ppcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		{
			buffer_desc.ByteWidth = sizeof(material_constant_buffer);
			hr = render_data::instance()->get_device()->CreateBuffer(&buffer_desc, nullptr, mcbv.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
	}

	//	深度ステンシルステート
	{
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
		depth_stencil_desc.DepthEnable = TRUE;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_EQUAL;	//	D3D11_COMPARISON_GREATER_EQUAL;
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
		rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = TRUE;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0;
		rasterizer_desc.SlopeScaledDepthBias = 0;
		rasterizer_desc.DepthClipEnable = TRUE;
		rasterizer_desc.ScissorEnable = FALSE;
		rasterizer_desc.MultisampleEnable = FALSE;
		rasterizer_desc.AntialiasedLineEnable = FALSE;
		hr = render_data::instance()->get_device()->CreateRasterizerState(&rasterizer_desc, rasterizer_states[0].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		hr = render_data::instance()->get_device()->CreateRasterizerState(&rasterizer_desc, rasterizer_states[1].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}

toon_gbuffer_pipeline_state::~toon_gbuffer_pipeline_state()
{
}

void toon_gbuffer_pipeline_state::begin(const graphics_context& context)
{
	//	シェーダー設定
	context.device_context->VSSetShader(vs->get_shader().Get(), nullptr, 0);
	context.device_context->HSSetShader(nullptr, nullptr, 0);
	context.device_context->DSSetShader(nullptr, nullptr, 0);
	context.device_context->GSSetShader(gs->get_shader().Get(), nullptr, 0);
	context.device_context->PSSetShader(ps->get_shader().Get(), nullptr, 0);
	context.device_context->IASetInputLayout(vs->get_input_layout().Get());
	context.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	ステート設定
	context.device_context->OMSetBlendState(blend_states.Get(), nullptr, 0xFFFFFFFF);
	context.device_context->OMSetDepthStencilState(depth_stencil_states.Get(), 0);

	//	シーン定数バッファ設定
	context.device_context->UpdateSubresource(scbv.Get(), 0, 0, &context.scene_constant, 0, 0);
	context.device_context->VSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->GSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(common_pipeline_states::SceneCBVIndex, 1, scbv.GetAddressOf());
}

void toon_gbuffer_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
	//	定数バッファ設定
	material_constant_buffer data = set_material_parameters_for_toon(context, material);
	context.device_context->UpdateSubresource(mcbv.Get(), 0, 0, &data, 0, 0);
	context.device_context->VSSetConstantBuffers(MaterialCBVIndex, 1, mcbv.GetAddressOf());
	context.device_context->GSSetConstantBuffers(MaterialCBVIndex, 1, mcbv.GetAddressOf());
	context.device_context->PSSetConstantBuffers(MaterialCBVIndex, 1, mcbv.GetAddressOf());

	//	背面法によるアウトライン使用
	if (material.is_flag(material::flags::use_reverse_outline))
	{
		context.device_context->GSSetShader(gs->get_shader().Get(), nullptr, 0);
	}
	else
	{
		context.device_context->GSSetShader(nullptr, nullptr, 0);
	}

	//	両面描画
	if (material.is_flag(material::flags::two_sided))
	{
		context.device_context->RSSetState(rasterizer_states[1].Get());
	}
	else
	{
		context.device_context->RSSetState(rasterizer_states[0].Get());
	}
}

void toon_gbuffer_pipeline_state::draw_indexed(const graphics_context& context, const buffer_datas& buffer_data)
{
	{
		primitive_constant_buffer pcb;
		memcpy_s(pcb.bone_transforms,
			sizeof(pcb.bone_transforms),
			buffer_data.transform_matrices.data(),
			buffer_data.transform_matrices.size() * sizeof(DirectX::XMFLOAT4X4));

		context.device_context->UpdateSubresource(pcbv.Get(), 0, 0, &pcb, 0, 0);
		context.device_context->VSSetConstantBuffers(common_pipeline_states::PrimitiveCBVIndex, 1, pcbv.GetAddressOf());
		context.device_context->GSSetConstantBuffers(common_pipeline_states::PrimitiveCBVIndex, 1, pcbv.GetAddressOf());
		context.device_context->PSSetConstantBuffers(common_pipeline_states::PrimitiveCBVIndex, 1, pcbv.GetAddressOf());
	}
	{
		primitive_constant_buffer pcb;
		memcpy_s(pcb.bone_transforms,
			sizeof(pcb.bone_transforms),
			buffer_data.transform_matrices.data(),
			buffer_data.transform_matrices.size() * sizeof(DirectX::XMFLOAT4X4));

		context.device_context->UpdateSubresource(ppcbv.Get(), 0, 0, &pcb, 0, 0);
		context.device_context->VSSetConstantBuffers(common_pipeline_states::PreviousPrimitiveCBVIndex, 1, ppcbv.GetAddressOf());
		context.device_context->GSSetConstantBuffers(common_pipeline_states::PreviousPrimitiveCBVIndex, 1, ppcbv.GetAddressOf());
		context.device_context->PSSetConstantBuffers(common_pipeline_states::PreviousPrimitiveCBVIndex, 1, ppcbv.GetAddressOf());
	}

	UINT stream_index = 0;
	for (auto& element_desc : vs->get_input_element_desc())
	{
		UINT stride = graphics_utility::get_byte_offset_from_format(element_desc.Format);

		auto it = buffer_data.vertex_buffer_views->find(semantic(element_desc.SemanticName, element_desc.SemanticIndex));
		if (it != buffer_data.vertex_buffer_views->end())
		{
			UINT offset = static_cast<UINT>(it->second.offset());
			context.device_context->IASetVertexBuffers(stream_index, 1, it->second.buffer.GetAddressOf(), &stride, &offset);
		}
		else
		{
			UINT offset = 0;
			context.device_context->IASetVertexBuffers(stream_index, 1, nullptr, &stride, &offset);
		}
		++stream_index;
	}
	context.device_context->IASetIndexBuffer(buffer_data.index_buffer_view->buffer.Get(), buffer_data.index_buffer_view->format, static_cast<UINT>(buffer_data.index_buffer_view->offset()));

	context.device_context->DrawIndexed(static_cast<UINT>(buffer_data.index_buffer_view->count()), 0, 0);
}

void toon_gbuffer_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
toon_skybox_pipeline_state::toon_skybox_pipeline_state()
: pipeline_state()
{
	HRESULT hr{ S_OK };

	load_shader_cso vs_desc;
	vs_desc.file_path = "Assets/CSO/sky_box_vs.cso";
	vs_desc.input_element_desc;
	shader_manager::instance()->load_shader<vertex_shader>(vs, vs_desc);

	load_shader_cso ps_desc;
	ps_desc.file_path = "Assets/CSO/toon_sky_box_gbuffer_ps.cso";
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

toon_skybox_pipeline_state::~toon_skybox_pipeline_state()
{
}

void toon_skybox_pipeline_state::begin(const graphics_context& context)
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

void toon_skybox_pipeline_state::set_parameters(const graphics_context& context, const material& material)
{
}

void toon_skybox_pipeline_state::draw(const graphics_context& context)
{
	context.device_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	context.device_context->Draw(4, 0);
}

void toon_skybox_pipeline_state::end(const graphics_context& context)
{
	context.clear_vertex_buffer();
	context.clear_index_buffer();
	context.clear_shader_slots();
	context.clear_shader_resorce_views();
	context.clear_sampler();
	context.clear_constant_buffers();
}
