#include "graphics_context.h"

void graphics_context::set_constant_buffer(int start_slot, int num, ID3D11Buffer* const* constant_buffers) const
{
	device_context->VSSetConstantBuffers(start_slot, num, constant_buffers);
	device_context->HSSetConstantBuffers(start_slot, num, constant_buffers);
	device_context->DSSetConstantBuffers(start_slot, num, constant_buffers);
	device_context->GSSetConstantBuffers(start_slot, num, constant_buffers);
	device_context->PSSetConstantBuffers(start_slot, num, constant_buffers);
	device_context->CSSetConstantBuffers(start_slot, num, constant_buffers);
}

void graphics_context::set_shader_resource(int start_slot, int num, ID3D11ShaderResourceView* const* shader_resources) const
{
	device_context->VSSetShaderResources(start_slot, num, shader_resources);
	device_context->HSSetShaderResources(start_slot, num, shader_resources);
	device_context->DSSetShaderResources(start_slot, num, shader_resources);
	device_context->GSSetShaderResources(start_slot, num, shader_resources);
	device_context->PSSetShaderResources(start_slot, num, shader_resources);
	device_context->CSSetShaderResources(start_slot, num, shader_resources);
}

void graphics_context::set_sampler(int start_slot, int num, ID3D11SamplerState* const* sampler_state) const
{
	device_context->VSSetSamplers(start_slot, num, sampler_state);
	device_context->HSSetSamplers(start_slot, num, sampler_state);
	device_context->DSSetSamplers(start_slot, num, sampler_state);
	device_context->GSSetSamplers(start_slot, num, sampler_state);
	device_context->PSSetSamplers(start_slot, num, sampler_state);
	device_context->CSSetSamplers(start_slot, num, sampler_state);
}

void graphics_context::clear_shader_slots() const
{
	device_context->VSSetShader(nullptr, nullptr, 0);
	device_context->HSSetShader(nullptr, nullptr, 0);
	device_context->DSSetShader(nullptr, nullptr, 0);
	device_context->GSSetShader(nullptr, nullptr, 0);
	device_context->PSSetShader(nullptr, nullptr, 0);
	device_context->CSSetShader(nullptr, nullptr, 0);
}

void graphics_context::clear_vertex_buffer() const
{
	ID3D11Buffer* vbs[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = {};
	UINT dummy[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = {};
	device_context->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, vbs, dummy, dummy);
}

void graphics_context::clear_index_buffer() const
{
	device_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
}

void graphics_context::clear_render_target_slots() const
{
	device_context->OMSetRenderTargets(0, nullptr, nullptr);
}

void graphics_context::clear_constant_buffers(int start_slot, int num) const
{
	ID3D11Buffer* clear_constant_bufferes[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT]{};
	device_context->VSSetConstantBuffers(start_slot, num, clear_constant_bufferes);
	device_context->HSSetConstantBuffers(start_slot, num, clear_constant_bufferes);
	device_context->DSSetConstantBuffers(start_slot, num, clear_constant_bufferes);
	device_context->GSSetConstantBuffers(start_slot, num, clear_constant_bufferes);
	device_context->PSSetConstantBuffers(start_slot, num, clear_constant_bufferes);
	device_context->CSSetConstantBuffers(start_slot, num, clear_constant_bufferes);
}

void graphics_context::clear_shader_resorce_views(int start_slot, int num) const
{
	ID3D11ShaderResourceView* clear_shader_resource_view[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	device_context->VSSetShaderResources(start_slot, num, clear_shader_resource_view);
	device_context->HSSetShaderResources(start_slot, num, clear_shader_resource_view);
	device_context->DSSetShaderResources(start_slot, num, clear_shader_resource_view);
	device_context->GSSetShaderResources(start_slot, num, clear_shader_resource_view);
	device_context->PSSetShaderResources(start_slot, num, clear_shader_resource_view);
	device_context->CSSetShaderResources(start_slot, num, clear_shader_resource_view);
}

void graphics_context::clear_unordered_access_views(int start_slot, int num) const
{
	ID3D11UnorderedAccessView* clear_unordered_access_view[8]{};
	device_context->CSSetUnorderedAccessViews(start_slot, num, clear_unordered_access_view, nullptr);
}

void graphics_context::clear_sampler(int start_slot, int num) const
{
	ID3D11SamplerState* clear_sampler[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT]{};
	device_context->VSSetSamplers(start_slot, num, clear_sampler);
	device_context->HSSetSamplers(start_slot, num, clear_sampler);
	device_context->DSSetSamplers(start_slot, num, clear_sampler);
	device_context->GSSetSamplers(start_slot, num, clear_sampler);
	device_context->PSSetSamplers(start_slot, num, clear_sampler);
	device_context->CSSetSamplers(start_slot, num, clear_sampler);
}
