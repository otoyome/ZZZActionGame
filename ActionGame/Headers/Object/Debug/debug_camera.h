#pragma once

#if defined(_DEBUG)
#include "camera.h"
#include "object/camera_object.h"

class debug_camera : public game_object
{
public:
	debug_camera(std::shared_ptr<camera_object> camera);
	~debug_camera();

	void fixed_update(float elapsed_time) override;
	void debug_update(float elapsed_time) override;

	void on_execute();

	inline std::shared_ptr<camera_object> get_camera() {return camera;}
	void set_rotation(const math::rotator& rotation) {this->rotation = rotation;}
	const math::rotator& get_rotation() {return rotation;}
	void set_position(const DirectX::XMFLOAT3& position) { this->position = position; }
	const DirectX::XMFLOAT3& get_position() { return position; }

private:
	std::shared_ptr<camera_object> camera;

	bool execute;

	math::rotator rotation;
	DirectX::XMFLOAT3 position;
	math::rotator restore_rotation;
	DirectX::XMFLOAT3 restore_position;

	POINT cursor_position{ 0, 0 };
	float rotation_speed = 0.0125f;
	float move_speed = system_units::convert_m_to_unit(0.2f);
};

#endif	//	defined(_DEBUG)
