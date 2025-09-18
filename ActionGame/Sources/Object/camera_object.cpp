#include "object/camera_object.h"

const math::rotator& camera_object::get_rotation() const
{
	return rotation;
}

void camera_object::set_rotation(const math::rotator& rotator)
{
	this->rotation = rotator;
}

const DirectX::XMFLOAT3& camera_object::get_position() const
{
	return position;
}

void camera_object::set_position(const DirectX::XMFLOAT3& position)
{
	this->position = position;
}

const DirectX::XMFLOAT3& camera_object::get_front() const
{
	return camera->get_front();
}

const DirectX::XMFLOAT3& camera_object::get_right() const
{
	return camera->get_right();
}

const DirectX::XMFLOAT3& camera_object::get_up() const
{
	return camera->get_up();
}

camera_object::camera_object(std::string_view name)
: game_object(name)
, rotation(0, DirectX::XM_PI, 0)
, position(0, 0, 0)
, shake_desc()
{
	camera_manager::instance()->generate_camera(camera);
	camera->set_rotation(rotation);
	camera->set_position(position);
	camera->set_screen_rect({0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT});
}

camera_object::~camera_object()
{
}

void camera_object::post_update(float elapsed_time)
{
	math::rotator new_rotation = rotation;
	DirectX::XMFLOAT3 new_position = position;
	if (shake_current_time > 0.0f)
	{
		float value = math::ease::calculate(shake_current_time / shake_desc.time, math::ease::type::out_bounce) * shake_desc.power;
		new_position.x += (rand() % 3 - 1) * value;
		new_position.y += (rand() % 3 - 1) * value;
		new_position.z += (rand() % 3 - 1) * value;
		shake_current_time -= elapsed_time;
	}
	camera->set_rotation(new_rotation);
	camera->set_position(new_position);
}

#if defined(USE_IMGUI)
void camera_object::imgui_render_core()
{
	auto rotation = camera->get_rotation();
	ImGui::DragFloat3("rotation", &rotation.x, 0.01f);
	camera->set_rotation(rotation);

	auto position = camera->get_position();
	ImGui::DragFloat3("position", &position.x, 0.01f);
	camera->set_position(position);
}
#endif	//	defined(USE_IMGUI)


void camera_object::shake(shake_descs desc)
{
	shake_desc = desc;
	shake_current_time = desc.time;
}
