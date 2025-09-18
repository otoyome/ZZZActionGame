#pragma once

#include "sprite.h"
#include "object/ui/ui_object.h"

//	UI制御用オブジェクト
class ui_image : public ui_object
{
	GAME_OBJECT_DECRLEAR(ui_image, ui_object)

public:
	enum class draw_modes
	{
		single,
		sliced,
	};

public:
	draw_modes get_draw_mode() const;
	void set_draw_mode(draw_modes draw_mode);

	DirectX::XMFLOAT4 get_border() const;
	void set_border(DirectX::XMFLOAT4 border);

	texture_ptr get_texture() const;
	void set_texture(std::string_view texture_path);
	void set_texture(texture_ptr texture);

public:
	ui_image(std::string_view name);
	~ui_image() override;

	void pre_render() override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

private:
	draw_modes draw_mode;

	DirectX::XMFLOAT4 border;
};
