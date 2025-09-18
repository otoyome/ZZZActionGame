#pragma once
#include "camera.h"
#include "object/camera_object.h"

class editor_camera : public game_object
{
public:
	editor_camera(std::shared_ptr<camera_object> camera);
	~editor_camera();

	void update(float elapsed_time) override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

	inline std::shared_ptr<camera_object> get_camera() {return camera;}

private:
	std::shared_ptr<camera_object> camera;

	POINT cursor_position{ 0, 0 };
	float rotation_speed = 0.0125f;
	float move_speed = system_units::convert_m_to_unit(0.2f);
};
