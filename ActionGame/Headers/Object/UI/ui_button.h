#pragma once

#include "sprite.h"
#include "object/ui/ui_image.h"

//	ボタン制御用オブジェクト
class ui_button : public game_object
{
	GAME_OBJECT_DECRLEAR(ui_button, game_object)

public:
	enum class color_type
	{
		normal,
		pressed,
		disabled,
	};

public:
	DirectX::XMFLOAT4 get_color(color_type type) const;
	void set_color(color_type type, const DirectX::XMFLOAT4& color);
	void set_color(color_type type, const DirectX::XMFLOAT3& color);
	void set_alpha(float alpha);

	bool get_enabled();
	void set_enabled(bool enable);

public:
	ui_button(std::string_view name, std::shared_ptr<ui_image> image);
	~ui_button() override;

	void fixed_update(float elapsed_time) override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

	void press();

private:
	std::shared_ptr<ui_image> image;
	DirectX::XMFLOAT4 color[magic_enum::enum_count<color_type>()];
	bool enable = false;
	bool pressed = false;
	float fade_duration = 0.1f;
	float alpha = 1.0f;
};
