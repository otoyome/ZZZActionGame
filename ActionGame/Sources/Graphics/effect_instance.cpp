#include "graphics/effect_instance.h"

#include "graphics/effect_manager.h"

#include "render_data.h"

bool effect_instance::is_exist() const
{
	return effect_manager::instance()->get_effekseer_manager()->Exists(handle);
}

DirectX::XMFLOAT3 effect_instance::get_scale() const
{
	::Effekseer::Matrix43 base_matrix = effect_manager::instance()->get_effekseer_manager()->GetBaseMatrix(handle);
	::Effekseer::Vector3D scale;
	base_matrix.GetScale(scale);
	return effect_utility::to_float3(scale);
}

void effect_instance::set_scale(const DirectX::XMFLOAT3& scale)
{
	effect_manager::instance()->get_effekseer_manager()->SetScale(handle, scale.x, scale.y, scale.z);
}

math::rotator effect_instance::get_rotation() const
{
	::Effekseer::Matrix43 base_matrix = effect_manager::instance()->get_effekseer_manager()->GetBaseMatrix(handle);
	::Effekseer::Matrix43 rotation_matrix43;
	base_matrix.GetRotation(rotation_matrix43);
	::Effekseer::Matrix44 rotation_matrix44;
	rotation_matrix43.ToMatrix44(rotation_matrix44);

	DirectX::XMFLOAT4X4 rotation_matrix = effect_utility::to_float4x4(rotation_matrix44);
	DirectX::XMVECTOR Dummy, Rotation;
	DirectX::XMMatrixDecompose(&Dummy, &Rotation, &Dummy, DirectX::XMLoadFloat4x4(&rotation_matrix));
	DirectX::XMFLOAT4 rotation;
	DirectX::XMStoreFloat4(&rotation, Rotation);
	return math::rotator(rotation);
}

void effect_instance::set_rotation(const math::rotator& rotation)
{
	effect_manager::instance()->get_effekseer_manager()->SetRotation(handle, rotation.x, rotation.y, rotation.z);
}

DirectX::XMFLOAT3 effect_instance::get_position() const
{
	return effect_utility::to_float3(effect_manager::instance()->get_effekseer_manager()->GetLocation(handle));
}

void effect_instance::set_position(const DirectX::XMFLOAT3& position)
{
	effect_manager::instance()->get_effekseer_manager()->SetLocation(handle, effect_utility::to_vector3(position));
}

float effect_instance::get_animation_speed() const
{
	return effect_manager::instance()->get_effekseer_manager()->GetSpeed(handle);
}

void effect_instance::set_animation_speed(float animation_speed)
{
	effect_manager::instance()->get_effekseer_manager()->SetSpeed(handle, animation_speed);
}

effect_instance::effect_instance()
: handle(-1)
{
}

effect_instance::effect_instance(Effekseer::Handle handle)
: handle(handle)
{
	//set_scale({system_units::convert_m_to_unit(1.0f), system_units::convert_m_to_unit(1.0f), system_units::convert_m_to_unit(1.0f)});
}

effect_instance::~effect_instance()
{
	stop();
}

void effect_instance::stop()
{
	effect_manager::instance()->get_effekseer_manager()->StopEffect(handle);
	handle = -1;
}
