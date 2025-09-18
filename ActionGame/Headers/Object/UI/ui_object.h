#pragma once

#include "sprite.h"
#include "object/game_object.h"

namespace ui
{
	constexpr DirectX::XMFLOAT2 get_pixel_size()
	{
		return { 1.0f / SCREEN_WIDTH, 1.0f / SCREEN_HEIGHT };
	}
	constexpr DirectX::XMFLOAT4 calculate_screen_rect_rate(DirectX::XMFLOAT4 rect)
	{
		//return rect;
		return { rect.x / SCREEN_WIDTH, rect.y / SCREEN_HEIGHT, rect.z / SCREEN_WIDTH, rect.w / SCREEN_HEIGHT };
	}
	constexpr DirectX::XMFLOAT4 calculate_screen_rect(DirectX::XMFLOAT4 rect_rate)
	{
		//return rect;
		return { rect_rate.x * SCREEN_WIDTH, rect_rate.y * SCREEN_HEIGHT, rect_rate.z * SCREEN_WIDTH, rect_rate.w * SCREEN_HEIGHT };
	}
}	//	namespace ui

//	UI制御用オブジェクト
class ui_object_base : public game_object
{
	GAME_OBJECT_DECRLEAR(ui_object_base, game_object)

public:
	int get_sort_order() const;
	void set_sort_order(int sort_order);

	float get_rotation() const;
	void set_rotation(float rotation);

	DirectX::XMFLOAT4 get_screen_rect() const;
	void set_screen_rect(const DirectX::XMFLOAT4& screen_rect);

	const DirectX::XMFLOAT2& get_pivot() const;
	void set_pivot(const DirectX::XMFLOAT2& position);

	const DirectX::XMFLOAT4& get_uv_rect() const;
	void set_uv_rect(const DirectX::XMFLOAT4& uv_rect);

	const DirectX::XMFLOAT4& get_color() const;
	void set_color(const DirectX::XMFLOAT4& color);
	void set_color(const DirectX::XMFLOAT3& color);

protected:
	bool is_dirty() const;
	void set_dirty();

	virtual DirectX::XMFLOAT4 get_screen_rect_inner() const;

public:
	ui_object_base(std::string_view name);
	~ui_object_base() override;

	void post_render() override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

private:
	bool dirty;

	int sort_order;
	float rotation;
	DirectX::XMFLOAT2 pivot;
	DirectX::XMFLOAT4 screen_rect;
	DirectX::XMFLOAT4 uv_rect;
	DirectX::XMFLOAT4 color;
};

//	UI制御用オブジェクト
class ui_object : public ui_object_base
{
	GAME_OBJECT_DECRLEAR(ui_object, ui_object_base)

public:
	const sprite& get_sprite() const { return sprite; }
	sprite& get_sprite() { return sprite; }

private:
	DirectX::XMFLOAT4 get_screen_rect_inner() const override { return base::get_screen_rect_inner(); }

public:
	ui_object(std::string_view name);
	~ui_object() override;

	void pre_render() override;

private:
	sprite sprite;
};

//	UI制御用オブジェクト
class ui_objects : public ui_object_base
{
	GAME_OBJECT_DECRLEAR(ui_objects, ui_object_base)

public:
	const std::vector<sprite>& get_sprites() const { return sprites; }
	std::vector<sprite>& get_sprites() { return sprites; }

private:
	DirectX::XMFLOAT4 get_screen_rect_inner() const override { return base::get_screen_rect_inner(); }

public:
	ui_objects(std::string_view name);
	~ui_objects() override;

	void pre_render() override;

private:
	std::vector<sprite> sprites;
};
