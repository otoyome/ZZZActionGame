#if 0

#include "gltf_model.h"
#include <stack>
#include <functional>

#define TINYGLTF_IMPLEMENTATION
#include "tinygltf-release/tiny_gltf.h"

#include "misc.h"
#include "shader.h"
#include "texture.h"

#include "raycast.h"

bool null_load_image_data(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
	return true;
}
gltf_model::gltf_model(ID3D11Device* device, const std::string& filename) : filename(filename)
{
	tinygltf::TinyGLTF tiny_gltf;
	tiny_gltf.SetImageLoader(null_load_image_data, nullptr);

	tinygltf::Model gltf_model;
	std::string error, warning;
	bool succeeded{ false };
	if (filename.find(".glb") != std::string::npos)
	{
		succeeded = tiny_gltf.LoadBinaryFromFile(&gltf_model, &error, &warning, filename.c_str());
	}
	else if (filename.find(".gltf") != std::string::npos)
	{
		succeeded = tiny_gltf.LoadASCIIFromFile(&gltf_model, &error, &warning, filename.c_str());
	}

	_ASSERT_EXPR_A(warning.empty(), warning.c_str());
	_ASSERT_EXPR_A(error.empty(), warning.c_str());
	_ASSERT_EXPR_A(succeeded, L"Failed to load glTF file");

	for (std::vector<tinygltf::Scene>::const_reference gltf_scene : gltf_model.scenes)
	{
		scene& scene{ scenes.emplace_back() };
		scene.name = gltf_model.scenes.at(0).name;
		scene.nodes = gltf_model.scenes.at(0).nodes;
	}

	fetch_nodes(gltf_model);
	fetch_meshes(device, gltf_model);

	fetch_materials(device, gltf_model);
	fetch_textures(device, gltf_model);

	//	デフォルト描画用シェーダー登録
	{
		gltf_model::pipeline_state_desc desc;
		desc.name = DefaultShaderName;
		desc.vs_path = "gltf_model_forward_vs.cso";
		desc.ps_path = "gltf_model_forward_ps.cso";
		gltf_model::add_pipeline_state(device, desc);
		//	
		desc.name = DefaultInstancingShaderName;
		desc.vs_path = "gltf_model_forward_instancing_vs.cso";
		gltf_model::add_pipeline_state_for_instancing(device, desc);
	}

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(primitive_constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr;
	hr = device->CreateBuffer(&buffer_desc, nullptr, primitive_cbuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

}
void gltf_model::fetch_nodes(const tinygltf::Model& gltf_model)
{
	for (std::vector<tinygltf::Node>::const_reference gltf_node : gltf_model.nodes)
	{
		node& node{ nodes.emplace_back() };
		node.name = gltf_node.name;
		node.skin = gltf_node.skin;
		node.mesh = gltf_node.mesh;
		node.children = gltf_node.children;
		if (!gltf_node.matrix.empty())
		{
			DirectX::XMFLOAT4X4 matrix;
			for (size_t row = 0; row < 4; row++)
			{
				for (size_t column = 0; column < 4; column++)
				{
					matrix(row, column) = static_cast<float>(gltf_node.matrix.at(4 * row + column));
				}
			}

			DirectX::XMVECTOR S, T, R;
			bool succeed = DirectX::XMMatrixDecompose(&S, &R, &T, DirectX::XMLoadFloat4x4(&matrix));
			_ASSERT_EXPR(succeed, L"Failed to decompose matrix.");

			DirectX::XMStoreFloat3(&node.scale, S);
			DirectX::XMStoreFloat4(&node.rotation, R);
			DirectX::XMStoreFloat3(&node.translation, T);
		}
		else
		{
			if (gltf_node.scale.size() > 0)
			{
				node.scale.x = static_cast<float>(gltf_node.scale.at(0));
				node.scale.y = static_cast<float>(gltf_node.scale.at(1));
				node.scale.z = static_cast<float>(gltf_node.scale.at(2));
			}
			if (gltf_node.translation.size() > 0)
			{
				node.translation.x = static_cast<float>(gltf_node.translation.at(0));
				node.translation.y = static_cast<float>(gltf_node.translation.at(1));
				node.translation.z = static_cast<float>(gltf_node.translation.at(2));
			}
			if (gltf_node.rotation.size() > 0)
			{
				node.rotation.x = static_cast<float>(gltf_node.rotation.at(0));
				node.rotation.y = static_cast<float>(gltf_node.rotation.at(1));
				node.rotation.z = static_cast<float>(gltf_node.rotation.at(2));
				node.rotation.w = static_cast<float>(gltf_node.rotation.at(3));
			}
		}
	}
	cumulate_transforms(nodes);
}
void gltf_model::cumulate_transforms(std::vector<node>& nodes)
{
	using namespace DirectX;

	std::stack<XMFLOAT4X4> parent_global_transforms;
	std::function<void(int)> traverse{ [&](int node_index)->void
	{
		node& node{nodes.at(node_index)};
		XMMATRIX S{ XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z) };
		XMMATRIX R{ XMMatrixRotationQuaternion(XMVectorSet(node.rotation.x, node.rotation.y, node.rotation.z, node.rotation.w)) };
		XMMATRIX T{ XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z) };
		XMStoreFloat4x4(&node.global_transform, S * R * T * XMLoadFloat4x4(&parent_global_transforms.top()));
		for (int child_index : node.children)
		{
			parent_global_transforms.push(node.global_transform);
			traverse(child_index);
			parent_global_transforms.pop();
		}
	} };
	for (std::vector<int>::value_type node_index : scenes.at(0).nodes)
	{
		parent_global_transforms.push({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 });
		traverse(node_index);
		parent_global_transforms.pop();
	}
}
gltf_model::buffer_view gltf_model::make_buffer_view(const tinygltf::Accessor& accessor)
{
	buffer_view buffer_view;
	switch (accessor.type)
	{
	case TINYGLTF_TYPE_SCALAR:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			buffer_view.format = DXGI_FORMAT_R16_UINT;
			buffer_view.stride_in_bytes = sizeof(USHORT);
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			buffer_view.format = DXGI_FORMAT_R32_UINT;
			buffer_view.stride_in_bytes = sizeof(UINT);
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;
	case TINYGLTF_TYPE_VEC2:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			buffer_view.format = DXGI_FORMAT_R32G32_FLOAT;
			buffer_view.stride_in_bytes = sizeof(FLOAT) * 2;
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;
	case TINYGLTF_TYPE_VEC3:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			buffer_view.format = DXGI_FORMAT_R32G32B32_FLOAT;
			buffer_view.stride_in_bytes = sizeof(FLOAT) * 3;
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;
	case TINYGLTF_TYPE_VEC4:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			buffer_view.format = DXGI_FORMAT_R16G16B16A16_UINT;
			buffer_view.stride_in_bytes = sizeof(USHORT) * 4;
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			buffer_view.format = DXGI_FORMAT_R32G32B32A32_UINT;
			buffer_view.stride_in_bytes = sizeof(UINT) * 4;
			break;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			buffer_view.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			buffer_view.stride_in_bytes = sizeof(FLOAT) * 4;
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;
	default:
		_ASSERT_EXPR(FALSE, L"This accessor type is not supported.");
		break;
	}
	buffer_view.size_in_bytes = static_cast<UINT>(accessor.count * buffer_view.stride_in_bytes);
	return buffer_view;
}
void gltf_model::fetch_meshes(ID3D11Device* device, const tinygltf::Model& gltf_model)
{
	HRESULT hr;
	for (std::vector<tinygltf::Mesh>::const_reference gltf_mesh : gltf_model.meshes)
	{
		mesh& mesh{ meshes.emplace_back() };
		mesh.name = gltf_mesh.name;
		for (std::vector<tinygltf::Primitive>::const_reference gltf_primitive : gltf_mesh.primitives)
		{
			mesh::primitive& primitive{ mesh.primitives.emplace_back() };
			primitive.material = gltf_primitive.material;

			// Create index buffer
			const tinygltf::Accessor& gltf_accessor{ gltf_model.accessors.at(gltf_primitive.indices) };
			const tinygltf::BufferView& gltf_buffer_view{ gltf_model.bufferViews.at(gltf_accessor.bufferView) };

			primitive.index_buffer_view = make_buffer_view(gltf_accessor);

			D3D11_BUFFER_DESC buffer_desc{};
			buffer_desc.ByteWidth = static_cast<UINT>(primitive.index_buffer_view.size_in_bytes);
			buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			D3D11_SUBRESOURCE_DATA subresource_data{};
			subresource_data.pSysMem = gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset;
			hr = device->CreateBuffer(&buffer_desc, &subresource_data, primitive.index_buffer_view.buffer.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

			//	レイキャスト用にコピー
			{
				primitive.indices.resize(static_cast<UINT>(primitive.index_buffer_view.size_in_bytes) / primitive.index_buffer_view.stride_in_bytes);
				primitive.index_stride = primitive.index_buffer_view.stride_in_bytes;
				//	何某からUINT32に変換
				switch (primitive.index_buffer_view.stride_in_bytes)
				{
				case 2:
				{
					const unsigned short* p = reinterpret_cast<const unsigned short*>(subresource_data.pSysMem);
					for (size_t index = 0; index < primitive.indices.size(); ++index)
					{
						primitive.indices[index] = *(p++);
					}
					break;
				}
				case 4:
				{
					const uint32_t* p = reinterpret_cast<const uint32_t*>(subresource_data.pSysMem);
					for (size_t index = 0; index < primitive.indices.size(); ++index)
					{
						primitive.indices[index] = *(p++);
					}
					break;
				}
				default:
					assert(!"Not compatible.");
				}
			}

			// Create vertex buffers
			for (std::map<std::string, int>::const_reference gltf_attribute : gltf_primitive.attributes)
			{
				const tinygltf::Accessor& gltf_accessor{ gltf_model.accessors.at(gltf_attribute.second) };
				const tinygltf::BufferView& gltf_buffer_view{ gltf_model.bufferViews.at(gltf_accessor.bufferView) };

				buffer_view vertex_buffer_view{ make_buffer_view(gltf_accessor) };

				D3D11_BUFFER_DESC buffer_desc{};
				buffer_desc.ByteWidth = static_cast<UINT>(vertex_buffer_view.size_in_bytes);
				buffer_desc.Usage = D3D11_USAGE_DEFAULT;
				buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				D3D11_SUBRESOURCE_DATA subresource_data{};
				subresource_data.pSysMem = gltf_model.buffers.at(gltf_buffer_view.buffer).data.data() + gltf_buffer_view.byteOffset + gltf_accessor.byteOffset;
				hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer_view.buffer.ReleaseAndGetAddressOf());
				_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

				//	レイキャスト用にコピー
				if (gltf_attribute.first == "POSITION")
				{
					primitive.positions.resize(static_cast<UINT>(vertex_buffer_view.size_in_bytes));
					primitive.position_stride = vertex_buffer_view.stride_in_bytes;
					memcpy_s(primitive.positions.data(), primitive.positions.size(),
						subresource_data.pSysMem, static_cast<UINT>(vertex_buffer_view.size_in_bytes));
				}

				primitive.vertex_buffer_views.emplace(std::make_pair(gltf_attribute.first, vertex_buffer_view));
			}

			// Add dummy attributes if any are missing.
			const std::unordered_map<std::string, buffer_view> attributes{
				{ "TANGENT", { DXGI_FORMAT_R32G32B32A32_FLOAT } },
				{ "TEXCOORD_0", { DXGI_FORMAT_R32G32_FLOAT } },
				{ "JOINTS_0", { DXGI_FORMAT_R16G16B16A16_UINT } },
				{ "WEIGHTS_0", { DXGI_FORMAT_R32G32B32A32_FLOAT } },
			};
			for (std::unordered_map<std::string, buffer_view>::const_reference attribute : attributes)
			{
				if (primitive.vertex_buffer_views.find(attribute.first) == primitive.vertex_buffer_views.end())
				{
					primitive.vertex_buffer_views.insert(std::make_pair(attribute.first, attribute.second));
				}
			}
		}
	}
}

void gltf_model::render(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world, const char* pipeline_state_name)
{
	using namespace DirectX;

	immediate_context->PSSetShaderResources(0, 1, material_resource_view.GetAddressOf());

	auto pipeline_state = pipeline_states.find(std::string(pipeline_state_name));
	if(pipeline_state == pipeline_states.end())
	{
		// シェーダーが見つからない場合はデフォルトを使用
		auto pipeline_state = pipeline_states.find(std::string(DefaultShaderName));
	}
	immediate_context->VSSetShader(pipeline_state->second.vertex_shader.Get(), nullptr, 0);
	immediate_context->HSSetShader(pipeline_state->second.hull_shader.Get(), nullptr, 0);
	immediate_context->DSSetShader(pipeline_state->second.domain_shader.Get(), nullptr, 0);
	immediate_context->GSSetShader(pipeline_state->second.geometry_shader.Get(), nullptr, 0);
	immediate_context->PSSetShader(pipeline_state->second.pixel_shader.Get(), nullptr, 0);
	immediate_context->IASetInputLayout(pipeline_state->second.input_layout.Get());
	immediate_context->IASetPrimitiveTopology(pipeline_state->second.primitive_toporogy);

	std::function<void(int)> traverse{ [&](int node_index)->void {
		const node& node{nodes.at(node_index)};
		if (node.mesh > -1)
		{
			const mesh& mesh{ meshes.at(node.mesh) };
			for (std::vector<mesh::primitive>::const_reference primitive : mesh.primitives)
			{
				ID3D11Buffer* vertex_buffers[]{
					primitive.vertex_buffer_views.at("POSITION").buffer.Get(),
					primitive.vertex_buffer_views.at("NORMAL").buffer.Get(),
					primitive.vertex_buffer_views.at("TANGENT").buffer.Get(),
					primitive.vertex_buffer_views.at("TEXCOORD_0").buffer.Get(),
					primitive.vertex_buffer_views.at("JOINTS_0").buffer.Get(),
					primitive.vertex_buffer_views.at("WEIGHTS_0").buffer.Get(),
				};
				UINT strides[]{
					static_cast<UINT>(primitive.vertex_buffer_views.at("POSITION").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("NORMAL").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("TANGENT").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("TEXCOORD_0").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("JOINTS_0").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("WEIGHTS_0").stride_in_bytes),
				};
				UINT offsets[_countof(vertex_buffers)]{ 0 };
				immediate_context->IASetVertexBuffers(0, _countof(vertex_buffers), vertex_buffers, strides, offsets);
				immediate_context->IASetIndexBuffer(primitive.index_buffer_view.buffer.Get(), primitive.index_buffer_view.format, 0);

				primitive_constants primitive_data{};
				primitive_data.material = primitive.material;
				primitive_data.has_tangent = primitive.vertex_buffer_views.at("TANGENT").buffer != NULL;
				primitive_data.skin = node.skin;
				XMStoreFloat4x4(&primitive_data.world, XMLoadFloat4x4(&node.global_transform) * XMLoadFloat4x4(&world));
				XMStoreFloat4x4(&primitive_data.previous_world, XMLoadFloat4x4(&node.global_transform) * XMLoadFloat4x4(&world));
				immediate_context->UpdateSubresource(primitive_cbuffer.Get(), 0, 0, &primitive_data, 0, 0);
				immediate_context->VSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->HSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->DSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->GSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->PSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());

				const material& material{ materials.at(primitive.material) };
				const int texture_indices[]
				{
					material.data.pbr_metallic_roughness.basecolor_texture.index,
					material.data.pbr_metallic_roughness.metallic_roughness_texture.index,
					material.data.normal_texture.index,
					material.data.emissive_texture.index,
					material.data.occlusion_texture.index,
				};
				ID3D11ShaderResourceView* null_shader_resource_view{};
				std::vector<ID3D11ShaderResourceView*> shader_resource_views(_countof(texture_indices));
				for (int texture_index = 0; texture_index < shader_resource_views.size(); ++texture_index)
				{
					shader_resource_views.at(texture_index) = texture_indices[texture_index] > -1 ? texture_resource_views.at(textures.at(texture_indices[texture_index]).source).Get() : null_shader_resource_view;
				}
				immediate_context->PSSetShaderResources(1, static_cast<UINT>(shader_resource_views.size()), shader_resource_views.data());

				immediate_context->DrawIndexed(static_cast<UINT>(primitive.index_buffer_view.count()), 0, 0);
			}
		}
		for (std::vector<int>::value_type child_index : node.children)
		{
			traverse(child_index);
		}
	} };
	for (std::vector<int>::value_type node_index : scenes.at(0).nodes)
	{
		traverse(node_index);
	}

	immediate_context->VSSetShader(nullptr, nullptr, 0);
	immediate_context->HSSetShader(nullptr, nullptr, 0);
	immediate_context->DSSetShader(nullptr, nullptr, 0);
	immediate_context->GSSetShader(nullptr, nullptr, 0);
	immediate_context->PSSetShader(nullptr, nullptr, 0);
	immediate_context->IASetInputLayout(nullptr);
}

void gltf_model::render(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4X4& previous_world, const char* pipeline_state_name)
{
	using namespace DirectX;

	immediate_context->PSSetShaderResources(0, 1, material_resource_view.GetAddressOf());

	auto pipeline_state = pipeline_states.find(std::string(pipeline_state_name));
	if (pipeline_state == pipeline_states.end())
	{
		// シェーダーが見つからない場合はデフォルトを使用
		auto pipeline_state = pipeline_states.find(std::string(DefaultShaderName));
	}
	immediate_context->VSSetShader(pipeline_state->second.vertex_shader.Get(), nullptr, 0);
	immediate_context->HSSetShader(pipeline_state->second.hull_shader.Get(), nullptr, 0);
	immediate_context->DSSetShader(pipeline_state->second.domain_shader.Get(), nullptr, 0);
	immediate_context->GSSetShader(pipeline_state->second.geometry_shader.Get(), nullptr, 0);
	immediate_context->PSSetShader(pipeline_state->second.pixel_shader.Get(), nullptr, 0);
	immediate_context->IASetInputLayout(pipeline_state->second.input_layout.Get());
	immediate_context->IASetPrimitiveTopology(pipeline_state->second.primitive_toporogy);

	std::function<void(int)> traverse{ [&](int node_index)->void {
		const node& node{nodes.at(node_index)};
		if (node.mesh > -1)
		{
			const mesh& mesh{ meshes.at(node.mesh) };
			for (std::vector<mesh::primitive>::const_reference primitive : mesh.primitives)
			{
				ID3D11Buffer* vertex_buffers[]{
					primitive.vertex_buffer_views.at("POSITION").buffer.Get(),
					primitive.vertex_buffer_views.at("NORMAL").buffer.Get(),
					primitive.vertex_buffer_views.at("TANGENT").buffer.Get(),
					primitive.vertex_buffer_views.at("TEXCOORD_0").buffer.Get(),
					primitive.vertex_buffer_views.at("JOINTS_0").buffer.Get(),
					primitive.vertex_buffer_views.at("WEIGHTS_0").buffer.Get(),
				};
				UINT strides[]{
					static_cast<UINT>(primitive.vertex_buffer_views.at("POSITION").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("NORMAL").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("TANGENT").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("TEXCOORD_0").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("JOINTS_0").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("WEIGHTS_0").stride_in_bytes),
				};
				UINT offsets[_countof(vertex_buffers)]{ 0 };
				immediate_context->IASetVertexBuffers(0, _countof(vertex_buffers), vertex_buffers, strides, offsets);
				immediate_context->IASetIndexBuffer(primitive.index_buffer_view.buffer.Get(), primitive.index_buffer_view.format, 0);

				primitive_constants primitive_data{};
				primitive_data.material = primitive.material;
				primitive_data.has_tangent = primitive.vertex_buffer_views.at("TANGENT").buffer != NULL;
				primitive_data.skin = node.skin;
				XMStoreFloat4x4(&primitive_data.world, XMLoadFloat4x4(&node.global_transform) * XMLoadFloat4x4(&world));
				XMStoreFloat4x4(&primitive_data.previous_world, XMLoadFloat4x4(&node.global_transform) * XMLoadFloat4x4(&previous_world));
				immediate_context->UpdateSubresource(primitive_cbuffer.Get(), 0, 0, &primitive_data, 0, 0);
				immediate_context->VSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->HSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->DSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->GSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->PSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());

				const material& material{ materials.at(primitive.material) };
				const int texture_indices[]
				{
					material.data.pbr_metallic_roughness.basecolor_texture.index,
					material.data.pbr_metallic_roughness.metallic_roughness_texture.index,
					material.data.normal_texture.index,
					material.data.emissive_texture.index,
					material.data.occlusion_texture.index,
				};
				ID3D11ShaderResourceView* null_shader_resource_view{};
				std::vector<ID3D11ShaderResourceView*> shader_resource_views(_countof(texture_indices));
				for (int texture_index = 0; texture_index < shader_resource_views.size(); ++texture_index)
				{
					shader_resource_views.at(texture_index) = texture_indices[texture_index] > -1 ? texture_resource_views.at(textures.at(texture_indices[texture_index]).source).Get() : null_shader_resource_view;
				}
				immediate_context->PSSetShaderResources(1, static_cast<UINT>(shader_resource_views.size()), shader_resource_views.data());

				immediate_context->DrawIndexed(static_cast<UINT>(primitive.index_buffer_view.count()), 0, 0);
			}
		}
		for (std::vector<int>::value_type child_index : node.children)
		{
			traverse(child_index);
		}
	} };
	for (std::vector<int>::value_type node_index : scenes.at(0).nodes)
	{
		traverse(node_index);
	}

	immediate_context->VSSetShader(nullptr, nullptr, 0);
	immediate_context->HSSetShader(nullptr, nullptr, 0);
	immediate_context->DSSetShader(nullptr, nullptr, 0);
	immediate_context->GSSetShader(nullptr, nullptr, 0);
	immediate_context->PSSetShader(nullptr, nullptr, 0);
	immediate_context->IASetInputLayout(nullptr);
}

void gltf_model::render(ID3D11DeviceContext* immediate_context, ID3D11Buffer* world_matrices_buffer, UINT instancing_count, const char* pipeline_state_name, UINT start_instance_location)
{
	using namespace DirectX;

	immediate_context->PSSetShaderResources(0, 1, material_resource_view.GetAddressOf());

	auto pipeline_state = pipeline_states.find(std::string(pipeline_state_name));
	if (pipeline_state == pipeline_states.end())
	{
		// シェーダーが見つからない場合はデフォルトを使用
		auto pipeline_state = pipeline_states.find(std::string(DefaultInstancingShaderName));
	}
	immediate_context->VSSetShader(pipeline_state->second.vertex_shader.Get(), nullptr, 0);
	immediate_context->HSSetShader(pipeline_state->second.hull_shader.Get(), nullptr, 0);
	immediate_context->DSSetShader(pipeline_state->second.domain_shader.Get(), nullptr, 0);
	immediate_context->GSSetShader(pipeline_state->second.geometry_shader.Get(), nullptr, 0);
	immediate_context->PSSetShader(pipeline_state->second.pixel_shader.Get(), nullptr, 0);
	immediate_context->IASetInputLayout(pipeline_state->second.input_layout.Get());
	immediate_context->IASetPrimitiveTopology(pipeline_state->second.primitive_toporogy);

	std::function<void(int)> traverse{ [&](int node_index)->void {
		const node& node{nodes.at(node_index)};
		if (node.mesh > -1)
		{
			const mesh& mesh{ meshes.at(node.mesh) };
			for (std::vector<mesh::primitive>::const_reference primitive : mesh.primitives)
			{
				ID3D11Buffer* vertex_buffers[]{
					primitive.vertex_buffer_views.at("POSITION").buffer.Get(),
					primitive.vertex_buffer_views.at("NORMAL").buffer.Get(),
					primitive.vertex_buffer_views.at("TANGENT").buffer.Get(),
					primitive.vertex_buffer_views.at("TEXCOORD_0").buffer.Get(),
					primitive.vertex_buffer_views.at("JOINTS_0").buffer.Get(),
					primitive.vertex_buffer_views.at("WEIGHTS_0").buffer.Get(),
					world_matrices_buffer,
					world_matrices_buffer,
				};
				UINT strides[]{
					static_cast<UINT>(primitive.vertex_buffer_views.at("POSITION").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("NORMAL").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("TANGENT").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("TEXCOORD_0").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("JOINTS_0").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("WEIGHTS_0").stride_in_bytes),
					sizeof(DirectX::XMFLOAT4X4),
					sizeof(DirectX::XMFLOAT4X4),
				};
				UINT offsets[_countof(vertex_buffers)]{ 0 };
				immediate_context->IASetVertexBuffers(0, _countof(vertex_buffers), vertex_buffers, strides, offsets);
				immediate_context->IASetIndexBuffer(primitive.index_buffer_view.buffer.Get(), primitive.index_buffer_view.format, 0);

				primitive_constants primitive_data{};
				primitive_data.material = primitive.material;
				primitive_data.has_tangent = primitive.vertex_buffer_views.at("TANGENT").buffer != NULL;
				primitive_data.skin = node.skin;
				primitive_data.world = node.global_transform;
				primitive_data.previous_world = node.global_transform;
				immediate_context->UpdateSubresource(primitive_cbuffer.Get(), 0, 0, &primitive_data, 0, 0);
				immediate_context->VSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->HSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->DSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->GSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->PSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());

				const material& material{ materials.at(primitive.material) };
				const int texture_indices[]
				{
					material.data.pbr_metallic_roughness.basecolor_texture.index,
					material.data.pbr_metallic_roughness.metallic_roughness_texture.index,
					material.data.normal_texture.index,
					material.data.emissive_texture.index,
					material.data.occlusion_texture.index,
				};
				ID3D11ShaderResourceView* null_shader_resource_view{};
				std::vector<ID3D11ShaderResourceView*> shader_resource_views(_countof(texture_indices));
				for (int texture_index = 0; texture_index < shader_resource_views.size(); ++texture_index)
				{
					shader_resource_views.at(texture_index) = texture_indices[texture_index] > -1 ? texture_resource_views.at(textures.at(texture_indices[texture_index]).source).Get() : null_shader_resource_view;
				}
				immediate_context->PSSetShaderResources(1, static_cast<UINT>(shader_resource_views.size()), shader_resource_views.data());

				immediate_context->DrawIndexedInstanced(static_cast<UINT>(primitive.index_buffer_view.count()), instancing_count, 0, 0, start_instance_location);
			}
		}
		for (std::vector<int>::value_type child_index : node.children)
		{
			traverse(child_index);
		}
	} };
	for (std::vector<int>::value_type node_index : scenes.at(0).nodes)
	{
		traverse(node_index);
	}

	immediate_context->VSSetShader(nullptr, nullptr, 0);
	immediate_context->HSSetShader(nullptr, nullptr, 0);
	immediate_context->DSSetShader(nullptr, nullptr, 0);
	immediate_context->GSSetShader(nullptr, nullptr, 0);
	immediate_context->PSSetShader(nullptr, nullptr, 0);
	immediate_context->IASetInputLayout(nullptr);
}

void gltf_model::render(ID3D11DeviceContext* immediate_context, ID3D11Buffer* world_matrices_buffer, ID3D11Buffer* previous_world_matrices_buffer, UINT instancing_count, const char* pipeline_state_name, UINT start_instance_location)
{
	using namespace DirectX;

	immediate_context->PSSetShaderResources(0, 1, material_resource_view.GetAddressOf());

	auto pipeline_state = pipeline_states.find(std::string(pipeline_state_name));
	if (pipeline_state == pipeline_states.end())
	{
		// シェーダーが見つからない場合はデフォルトを使用
		auto pipeline_state = pipeline_states.find(std::string(DefaultInstancingShaderName));
	}
	immediate_context->VSSetShader(pipeline_state->second.vertex_shader.Get(), nullptr, 0);
	immediate_context->HSSetShader(pipeline_state->second.hull_shader.Get(), nullptr, 0);
	immediate_context->DSSetShader(pipeline_state->second.domain_shader.Get(), nullptr, 0);
	immediate_context->GSSetShader(pipeline_state->second.geometry_shader.Get(), nullptr, 0);
	immediate_context->PSSetShader(pipeline_state->second.pixel_shader.Get(), nullptr, 0);
	immediate_context->IASetInputLayout(pipeline_state->second.input_layout.Get());
	immediate_context->IASetPrimitiveTopology(pipeline_state->second.primitive_toporogy);

	std::function<void(int)> traverse{ [&](int node_index)->void {
		const node& node{nodes.at(node_index)};
		if (node.mesh > -1)
		{
			const mesh& mesh{ meshes.at(node.mesh) };
			for (std::vector<mesh::primitive>::const_reference primitive : mesh.primitives)
			{
				ID3D11Buffer* vertex_buffers[]{
					primitive.vertex_buffer_views.at("POSITION").buffer.Get(),
					primitive.vertex_buffer_views.at("NORMAL").buffer.Get(),
					primitive.vertex_buffer_views.at("TANGENT").buffer.Get(),
					primitive.vertex_buffer_views.at("TEXCOORD_0").buffer.Get(),
					primitive.vertex_buffer_views.at("JOINTS_0").buffer.Get(),
					primitive.vertex_buffer_views.at("WEIGHTS_0").buffer.Get(),
					world_matrices_buffer,
					previous_world_matrices_buffer,
				};
				UINT strides[]{
					static_cast<UINT>(primitive.vertex_buffer_views.at("POSITION").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("NORMAL").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("TANGENT").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("TEXCOORD_0").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("JOINTS_0").stride_in_bytes),
					static_cast<UINT>(primitive.vertex_buffer_views.at("WEIGHTS_0").stride_in_bytes),
					sizeof(DirectX::XMFLOAT4X4),
					sizeof(DirectX::XMFLOAT4X4),
				};
				UINT offsets[_countof(vertex_buffers)]{ 0 };
				immediate_context->IASetVertexBuffers(0, _countof(vertex_buffers), vertex_buffers, strides, offsets);
				immediate_context->IASetIndexBuffer(primitive.index_buffer_view.buffer.Get(), primitive.index_buffer_view.format, 0);

				primitive_constants primitive_data{};
				primitive_data.material = primitive.material;
				primitive_data.has_tangent = primitive.vertex_buffer_views.at("TANGENT").buffer != NULL;
				primitive_data.skin = node.skin;
				primitive_data.world = node.global_transform;
				primitive_data.previous_world = node.global_transform;
				immediate_context->UpdateSubresource(primitive_cbuffer.Get(), 0, 0, &primitive_data, 0, 0);
				immediate_context->VSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->HSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->DSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->GSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());
				immediate_context->PSSetConstantBuffers(0, 1, primitive_cbuffer.GetAddressOf());

				const material& material{ materials.at(primitive.material) };
				const int texture_indices[]
				{
					material.data.pbr_metallic_roughness.basecolor_texture.index,
					material.data.pbr_metallic_roughness.metallic_roughness_texture.index,
					material.data.normal_texture.index,
					material.data.emissive_texture.index,
					material.data.occlusion_texture.index,
				};
				ID3D11ShaderResourceView* null_shader_resource_view{};
				std::vector<ID3D11ShaderResourceView*> shader_resource_views(_countof(texture_indices));
				for (int texture_index = 0; texture_index < shader_resource_views.size(); ++texture_index)
				{
					shader_resource_views.at(texture_index) = texture_indices[texture_index] > -1 ? texture_resource_views.at(textures.at(texture_indices[texture_index]).source).Get() : null_shader_resource_view;
				}
				immediate_context->PSSetShaderResources(1, static_cast<UINT>(shader_resource_views.size()), shader_resource_views.data());

				immediate_context->DrawIndexedInstanced(static_cast<UINT>(primitive.index_buffer_view.count()), instancing_count, 0, 0, start_instance_location);
			}
		}
		for (std::vector<int>::value_type child_index : node.children)
		{
			traverse(child_index);
		}
	} };
	for (std::vector<int>::value_type node_index : scenes.at(0).nodes)
	{
		traverse(node_index);
	}

	immediate_context->VSSetShader(nullptr, nullptr, 0);
	immediate_context->HSSetShader(nullptr, nullptr, 0);
	immediate_context->DSSetShader(nullptr, nullptr, 0);
	immediate_context->GSSetShader(nullptr, nullptr, 0);
	immediate_context->PSSetShader(nullptr, nullptr, 0);
	immediate_context->IASetInputLayout(nullptr);
}

bool gltf_model::add_pipeline_state(ID3D11Device* device, pipeline_state_desc desc)
{
	auto it = pipeline_states.find(desc.name);
	if (it != pipeline_states.end())
		return false;

	// TODO: This is a force-brute programming, may cause bugs.
	const std::map<std::string, buffer_view>& vertex_buffer_views{ meshes.at(0).primitives.at(0).vertex_buffer_views };
	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{ "POSITION", 0, vertex_buffer_views.at("POSITION").format, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, vertex_buffer_views.at("NORMAL").format, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, vertex_buffer_views.at("TANGENT").format, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, vertex_buffer_views.at("TEXCOORD_0").format, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "JOINTS", 0, vertex_buffer_views.at("JOINTS_0").format, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0,vertex_buffer_views.at("WEIGHTS_0").format, 5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	pipeline_state state;
	if(!desc.vs_path.empty())
		create_vs_from_cso(device, desc.vs_path.data(), state.vertex_shader.ReleaseAndGetAddressOf(), state.input_layout.ReleaseAndGetAddressOf(), input_element_desc, _countof(input_element_desc));
	if (!desc.hs_path.empty())
		create_hs_from_cso(device, desc.hs_path.data(), state.hull_shader.ReleaseAndGetAddressOf());
	if (!desc.ds_path.empty())
		create_ds_from_cso(device, desc.ds_path.data(), state.domain_shader.ReleaseAndGetAddressOf());
	if (!desc.gs_path.empty())
		create_gs_from_cso(device, desc.gs_path.data(), state.geometry_shader.ReleaseAndGetAddressOf());
	if (!desc.ps_path.empty())
		create_ps_from_cso(device, desc.ps_path.data(), state.pixel_shader.ReleaseAndGetAddressOf());
	state.primitive_toporogy = desc.primitive_toporogy;

	pipeline_states.insert_or_assign(desc.name, state);
	return true;
}

bool gltf_model::add_pipeline_state_for_instancing(ID3D11Device* device, pipeline_state_desc desc)
{
	auto it = pipeline_states.find(desc.name);
	if (it != pipeline_states.end())
		return false;

	// TODO: This is a force-brute programming, may cause bugs.
	const std::map<std::string, buffer_view>& vertex_buffer_views{ meshes.at(0).primitives.at(0).vertex_buffer_views };
	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{ "POSITION", 0, vertex_buffer_views.at("POSITION").format, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, vertex_buffer_views.at("NORMAL").format, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, vertex_buffer_views.at("TANGENT").format, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, vertex_buffer_views.at("TEXCOORD_0").format, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "JOINTS", 0, vertex_buffer_views.at("JOINTS_0").format, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0,vertex_buffer_views.at("WEIGHTS_0").format, 5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WORLD_MATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD_MATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD_MATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD_MATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PREVIOUS_WORLD_MATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 7,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PREVIOUS_WORLD_MATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PREVIOUS_WORLD_MATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PREVIOUS_WORLD_MATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 7, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	pipeline_state state;
	if (!desc.vs_path.empty())
		create_vs_from_cso(device, desc.vs_path.data(), state.vertex_shader.ReleaseAndGetAddressOf(), state.input_layout.ReleaseAndGetAddressOf(), input_element_desc, _countof(input_element_desc));
	if (!desc.hs_path.empty())
		create_hs_from_cso(device, desc.hs_path.data(), state.hull_shader.ReleaseAndGetAddressOf());
	if (!desc.ds_path.empty())
		create_ds_from_cso(device, desc.ds_path.data(), state.domain_shader.ReleaseAndGetAddressOf());
	if (!desc.gs_path.empty())
		create_gs_from_cso(device, desc.gs_path.data(), state.geometry_shader.ReleaseAndGetAddressOf());
	if (!desc.ps_path.empty())
		create_ps_from_cso(device, desc.ps_path.data(), state.pixel_shader.ReleaseAndGetAddressOf());
	state.primitive_toporogy = desc.primitive_toporogy;

	pipeline_states.insert_or_assign(desc.name, state);
	return true;
}

void gltf_model::fetch_materials(ID3D11Device* device, const tinygltf::Model& gltf_model)
{
	for (std::vector<tinygltf::Material>::const_reference gltf_material : gltf_model.materials)
	{
		std::vector<material>::reference material = materials.emplace_back();

		material.name = gltf_material.name;

		material.data.emissive_factor[0] = static_cast<float>(gltf_material.emissiveFactor.at(0));
		material.data.emissive_factor[1] = static_cast<float>(gltf_material.emissiveFactor.at(1));
		material.data.emissive_factor[2] = static_cast<float>(gltf_material.emissiveFactor.at(2));

		material.data.alpha_mode = gltf_material.alphaMode == "OPAQUE" ? 0 : gltf_material.alphaMode == "MASK" ? 1 : gltf_material.alphaMode == "BLEND" ? 2 : 0;
		material.data.alpha_cutoff = static_cast<float>(gltf_material.alphaCutoff);
		material.data.double_sided = gltf_material.doubleSided ? 1 : 0;

		material.data.pbr_metallic_roughness.basecolor_factor[0] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(0));
		material.data.pbr_metallic_roughness.basecolor_factor[1] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(1));
		material.data.pbr_metallic_roughness.basecolor_factor[2] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(2));
		material.data.pbr_metallic_roughness.basecolor_factor[3] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor.at(3));
		material.data.pbr_metallic_roughness.basecolor_texture.index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
		material.data.pbr_metallic_roughness.basecolor_texture.texcoord = gltf_material.pbrMetallicRoughness.baseColorTexture.texCoord;
		material.data.pbr_metallic_roughness.metallic_factor = static_cast<float>(gltf_material.pbrMetallicRoughness.metallicFactor);
		material.data.pbr_metallic_roughness.roughness_factor = static_cast<float>(gltf_material.pbrMetallicRoughness.roughnessFactor);
		material.data.pbr_metallic_roughness.metallic_roughness_texture.index = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
		material.data.pbr_metallic_roughness.metallic_roughness_texture.texcoord = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

		material.data.normal_texture.index = gltf_material.normalTexture.index;
		material.data.normal_texture.texcoord = gltf_material.normalTexture.texCoord;
		material.data.normal_texture.scale = static_cast<float>(gltf_material.normalTexture.scale);

		material.data.occlusion_texture.index = gltf_material.occlusionTexture.index;
		material.data.occlusion_texture.texcoord = gltf_material.occlusionTexture.texCoord;
		material.data.occlusion_texture.strength = static_cast<float>(gltf_material.occlusionTexture.strength);

		material.data.emissive_texture.index = gltf_material.emissiveTexture.index;
		material.data.emissive_texture.texcoord = gltf_material.emissiveTexture.texCoord;
	}

	// Create material data as shader resource view on GPU
	std::vector<material::cbuffer> material_data;
	for (std::vector<material>::const_reference material : materials)
	{
		material_data.emplace_back(material.data);
	}

	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> material_buffer;
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(material::cbuffer) * material_data.size());
	buffer_desc.StructureByteStride = sizeof(material::cbuffer);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	D3D11_SUBRESOURCE_DATA subresource_data{};
	subresource_data.pSysMem = material_data.data();
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, material_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
	shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(material_data.size());
	hr = device->CreateShaderResourceView(material_buffer.Get(), &shader_resource_view_desc, material_resource_view.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void gltf_model::fetch_textures(ID3D11Device* device, const tinygltf::Model& gltf_model)
{
	HRESULT hr{ S_OK };
	for (const tinygltf::Texture& gltf_texture : gltf_model.textures)
	{
		texture& texture{ textures.emplace_back() };
		texture.name = gltf_texture.name;
		texture.source = gltf_texture.source;
	}
	for (const tinygltf::Image& gltf_image : gltf_model.images)
	{
		image& image{ images.emplace_back() };
		image.name = gltf_image.name;
		image.width = gltf_image.width;
		image.height = gltf_image.height;
		image.component = gltf_image.component;
		image.bits = gltf_image.bits;
		image.pixel_type = gltf_image.pixel_type;
		image.buffer_view = gltf_image.bufferView;
		image.mime_type = gltf_image.mimeType;
		image.uri = gltf_image.uri;
		image.as_is = gltf_image.as_is;

		if (gltf_image.bufferView > -1)
		{
			const tinygltf::BufferView& buffer_view{ gltf_model.bufferViews.at(gltf_image.bufferView) };
			const tinygltf::Buffer& buffer{ gltf_model.buffers.at(buffer_view.buffer) };
			const byte* data = buffer.data.data() + buffer_view.byteOffset;

			ID3D11ShaderResourceView* texture_resource_view{};
			hr = load_texture_from_memory(device, data, buffer_view.byteLength, &texture_resource_view);
			if (hr == S_OK)
			{
				texture_resource_views.emplace_back().Attach(texture_resource_view);
			}
		}
		else
		{
			const std::filesystem::path path(filename);
			ID3D11ShaderResourceView* shader_resource_view{};
			D3D11_TEXTURE2D_DESC texture2d_desc;
			std::wstring imagepanme{ std::wstring(gltf_image.uri.begin(), gltf_image.uri.end()) };
			if (imagepanme.empty())
			{
				//	生成できない場合はダミーを生成
				texture_resource_views.emplace_back().Attach(nullptr);
				continue;
			}
			std::wstring filepath{ path.parent_path().concat(L"/").wstring() + imagepanme };
			hr = load_texture_from_file(device, filepath.c_str(), &shader_resource_view, &texture2d_desc);
			if (hr == S_OK)
			{
				texture_resource_views.emplace_back().Attach(shader_resource_view);
			}
		}
	}
}

bool gltf_model::ray_cast(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction, float& distance, const DirectX::XMFLOAT4X4& world_transform, DirectX::XMFLOAT3& intersect_point, DirectX::XMFLOAT3& intersected_normal, std::string& intersected_mesh, std::string& intersected_material)
{
	bool hit = false;
	float closest_distance{ FLT_MAX };
	std::function<void(int)> traverse{ [&](int node_index)->void {
		const node& node{nodes.at(node_index)};
		if (node.mesh > -1)
		{
			const mesh& mesh{ meshes.at(node.mesh) };
			for (std::vector<mesh::primitive>::const_reference primitive : mesh.primitives)
			{
				DirectX::XMFLOAT4 ray_position;
				ray_position.x = position.x;
				ray_position.y = position.y;
				ray_position.z = position.z;
				ray_position.w = 1;
				DirectX::XMFLOAT4 ray_direction;
				ray_direction.x = direction.x;
				ray_direction.y = direction.y;
				ray_direction.z = direction.z;
				ray_direction.w = 0;

				DirectX::XMMATRIX concatenated_matrix{ DirectX::XMLoadFloat4x4(&node.global_transform) * DirectX::XMLoadFloat4x4(&world_transform) };
				DirectX::XMMATRIX inverse_concatenated_matrix{ DirectX::XMMatrixInverse(nullptr, concatenated_matrix) };
				DirectX::XMStoreFloat4(&ray_position, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&ray_position), inverse_concatenated_matrix));
				DirectX::XMStoreFloat4(&ray_direction, DirectX::XMVector3Normalize(XMVector3TransformNormal(DirectX::XMLoadFloat4(&ray_direction), inverse_concatenated_matrix)));

				//	
				float work_distance{ 1.0e+7f };
				DirectX::XMFLOAT4 intersection_point{};
				DirectX::XMFLOAT3 intersection_normal{};
				const float* vertex_positions{ reinterpret_cast<const float*>(primitive.positions.data()) };
				const uint32_t* indices{ primitive.indices.data() };
				const size_t index_count{ primitive.indices.size() };

				const int intersected_triangle_index{ intersect_ray_triangles(vertex_positions, 0L, sizeof(DirectX::XMFLOAT3), indices, index_count, ray_position, ray_direction, intersection_point, intersection_normal, work_distance) };
				if (intersected_triangle_index >= 0)
				{
					DirectX::XMVECTOR WorldIntersectionPoint;
					WorldIntersectionPoint = XMVector3TransformCoord(XMLoadFloat4(&intersection_point), concatenated_matrix);
					work_distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(WorldIntersectionPoint, DirectX::XMLoadFloat3(&position))));

					if (closest_distance > work_distance)
					{
						closest_distance = work_distance;
						distance = work_distance;
						intersection_point.w = 1;
						XMStoreFloat3(&intersect_point, WorldIntersectionPoint);
						XMStoreFloat3(&intersected_normal, XMVector3TransformNormal(XMLoadFloat3(&intersection_normal), concatenated_matrix));
						intersected_mesh = mesh.name;
						intersected_material = materials[primitive.material].name;
					}
					hit = true;
				}
			}
		}
		for (std::vector<int>::value_type child_index : node.children)
		{
			traverse(child_index);
		}
	} };
	//
	for (std::vector<int>::value_type node_index : scenes.at(0).nodes)
	{
		traverse(node_index);
	}
	return hit;
}

#endif
