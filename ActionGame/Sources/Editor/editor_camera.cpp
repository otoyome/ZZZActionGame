#include "Editor/editor_camera.h"

editor_camera::editor_camera(std::shared_ptr<camera_object> camera)
: game_object("editor_camera")
, camera(camera)
{
}

editor_camera::~editor_camera()
{
}

void editor_camera::update(float elapsed_time)
{
	auto mouse_state = mouse::instance()->get_state();

	//	キーボード操作
	if (mouse_state.is_key_state(mouse::key_states::right) == input::state::hold)
	{
		int x = 0, y = 0, z = 0;
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::w) == input::state::hold)
		{
			++z;
		}
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::s) == input::state::hold)
		{
			--z;
		}
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::d) == input::state::hold)
		{
			++x;
		}
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::a) == input::state::hold)
		{
			--x;
		}
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::e) == input::state::hold)
		{
			++y;
		}
		if (keyboard::instance()->get_state().is_key_state(keyboard::key_states::q) == input::state::hold)
		{
			--y;
		}

		int wheel = mouse::instance()->get_wheel_value();
		if (wheel != 0)
		{
			move_speed += move_speed * 0.001f * wheel;
		}

		float speed = move_speed * system_units::convert_sec_to_fps(elapsed_time);
		DirectX::XMFLOAT3 position = camera->get_position();
		DirectX::XMFLOAT3 front = camera->get_front();
		DirectX::XMFLOAT3 right = camera->get_right();
		DirectX::XMFLOAT3 up = camera->get_up();
		position.x += +front.x * speed * z + right.x * speed * x - up.x * speed * y;
		position.y += +front.y * speed * z + right.y * speed * x - up.y * speed * y;
		position.z += +front.z * speed * z + right.z * speed * x - up.z * speed * y;
		camera->set_position(position);
	}

	// マウス操作
#if USE_IMGUI
	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
#endif // USE_IMGUI
	{
		DirectX::XMVECTOR Up{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

		DirectX::XMFLOAT2 move;
		move.x = static_cast<float>(mouse::instance()->get_move_cursol().x);
		move.y = static_cast<float>(mouse::instance()->get_move_cursol().y);
		int wheel = mouse::instance()->get_wheel_value();
		move.x *= math::step(1.0f, fabsf(move.x));
		move.y *= math::step(1.0f, fabsf(move.y));
		if (mouse_state.is_key_state(mouse::key_states::right) == input::state::hold)
		{
			float speed = rotation_speed * system_units::convert_sec_to_fps(elapsed_time);
			math::rotator camera_rotator = camera->get_rotation();
			camera_rotator.x -= move.y * speed;
			camera_rotator.y -= move.x * (camera_rotator.get_up().y < 0 ? -speed : speed);
			camera->set_rotation(camera_rotator);
		}

		if (mouse_state.is_key_state(mouse::key_states::middle) == input::state::hold ||
			(mouse_state.is_key_state(mouse::key_states::right) != input::state::hold && wheel != 0))
		{
			float speed = system_units::convert_m_to_unit(0.02f) * system_units::convert_sec_to_fps(elapsed_time);
			DirectX::XMFLOAT3 position = camera->get_position();
			DirectX::XMFLOAT3 front = camera->get_front();
			DirectX::XMFLOAT3 right = camera->get_right();
			DirectX::XMFLOAT3 up = camera->get_up();
			position.x += +front.x * speed * wheel + right.x * speed * move.x - up.x * speed * move.y;
			position.y += +front.y * speed * wheel + right.y * speed * move.x - up.y * speed * move.y;
			position.z += +front.z * speed * wheel + right.z * speed * move.x - up.z * speed * move.y;
			camera->set_position(position);
		}
	}
}

#if defined(USE_IMGUI)
void editor_camera::imgui_render_core()
{
	auto rotation = camera->get_rotation();
	ImGui::DragFloat3("rotation", &rotation.x, 0.01f);
	camera->set_rotation(rotation);

	auto position = camera->get_position();
	ImGui::DragFloat3("position", &position.x, 0.01f);
	camera->set_position(position);

	ImGui::DragFloat("move_speed", &move_speed, 0.01f);
	ImGui::DragFloat("rotation_speed", &rotation_speed, 0.01f);
}
#endif	//	defined(USE_IMGUI)


