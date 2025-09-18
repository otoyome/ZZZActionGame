#include "object/decal_object.h"

const DirectX::XMFLOAT3& decal_object::get_scale() const
{
	return scale;
}

void decal_object::set_scale(const DirectX::XMFLOAT3& scale)
{
	this->scale = scale;
}

const math::rotator& decal_object::get_rotation() const
{
	return rotation;
}

void decal_object::set_rotation(const math::rotator& rotator)
{
	this->rotation = rotator;
}

const DirectX::XMFLOAT3& decal_object::get_position() const
{
	return position;
}

void decal_object::set_position(const DirectX::XMFLOAT3& position)
{
	this->position = position;
}

decal_datas decal_object::convert_decal_data() const
{
	decal_datas data;
	data.scale = scale;
	data.rotation = rotation;
	data.position = position;
	data.material.material = &material;
	return data;
}

decal_object::decal_object(std::string_view name, math::compute_fade::calculator fade_time)
: game_object(name)
, scale(1.0f, 1.0f, 1.0f)
, rotation(0.0f, 0.0f, 0.0f)
, position(0.0f, 0.0f, 0.0f)
, time(0.0f)
, fade_time(fade_time)
{
	CONST char* shader_names[] =
	{
		"deferred_decal_stencil_write",
		"deferred_decal_primitive",
		"deferred_decal_fullscreen",
	};
	material.set_domain(material::domains::surface);
	material.set_blend_mode(material::blend_modes::mask);
	material.set_pipeline_state(material::surface_paths::depth_write_pass, shader_names[0]);
	material.set_pipeline_state(material::surface_paths::g_buffer_pass, shader_names[1]);
	material.set_pipeline_state(material::surface_paths::shadow_write_pass, shader_names[2]);

	//	マテリアルの必要な設定を行っておく
	material.set_float4(material_variable_name_base_color, { 1.0f, 1.0f, 1.0f, 1.0f });

	//	ダミーテクスチャ設定
	texture_ptr white_dummy = texture_manager::instance()->search(texture_manager::white_texture2d_name);
	material.set_texture(material_variable_name_base_texture, white_dummy);
}

decal_object::~decal_object()
{
}

void decal_object::post_update(float elapsed_time)
{
	//	自殺
	time += elapsed_time;
	bool end_flag = false;
	float rate = fade_time.compute(time, &end_flag);
	if (end_flag)
	{
		object_manager::instance()->unregister_object(shared_from_this());
		return;
	}
}

void decal_object::pre_render()
{
	bool end_flag = false;
	float rate = fade_time.compute(time, &end_flag);
	//	マテリアル側にパラメーター設定(行列などもここで設定しておく)
	material.set_float4(material_variable_name_base_color, { 1, 1, 1, rate });
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation.convert_quaternion()));
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMMATRIX V = DirectX::XMMatrixInverse(nullptr, S * R * T);
	DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicLH(1, 1, 0, 1);
	DirectX::XMFLOAT4 decal_direction;
	DirectX::XMStoreFloat4(&decal_direction, R.r[2]);
	decal_direction.w = 0;
	material.set_float4(decal_material_parameter_name::decal_direction, decal_direction);
	DirectX::XMFLOAT4X4 decal_inverse_transform;
	DirectX::XMStoreFloat4x4(&decal_inverse_transform, V * P);
	material.set_float4x4(decal_material_parameter_name::decal_inverse_transform, decal_inverse_transform);
}

#if defined(USE_IMGUI)
void decal_object::imgui_render_core()
{
	bool end_flag = false;
	float rate = fade_time.compute(time, &end_flag);
	ImGui::DragFloat("time", &time, 0.01f);
	ImGui::DragFloat("rate", &rate, 0.01f);
	ImGui::Checkbox("end_flag", &end_flag);

	auto scale = get_scale();
	ImGui::DragFloat3("scale", &scale.x, 0.01f);
	set_scale(scale);

	auto rotation = get_rotation();
	ImGui::DragFloat3("rotation", &rotation.x, 0.01f);
	set_rotation(rotation);

	auto position = get_position();
	ImGui::DragFloat3("position", &position.x, 0.01f);
	set_position(position);

}
#endif	//	defined(USE_IMGUI)
