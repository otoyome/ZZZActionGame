#pragma once

#include "object/ui/ui_image.h"

namespace fade
{
	enum class states
	{
		idle,
		processing,
	};

	enum class directions
	{
		in,
		out,
	};

	enum class modes
	{
		black,
		white,
	};
}

//	フェード制御用オブジェクト
class fade_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(fade_controller_object, game_object)

public:
	bool is_processing();

public:
	fade_controller_object(std::string_view name, std::shared_ptr<ui_image> image);
	~fade_controller_object() override;

	void post_update(float elapsed_time) override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

public:
	bool play(fade::directions direction, float fade_time, fade::modes mode);

private:
	std::shared_ptr<ui_image> image;
	fade::states state;
	fade::directions direction;
	fade::modes mode;
	float total_time;
	float fade_time;
};
