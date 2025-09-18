#include "object/ui/ui_object.h"

int ui_object_base::get_sort_order() const
{
	return sort_order;
}

void ui_object_base::set_sort_order(int sort_order)
{
	this->sort_order = sort_order;
	set_dirty();
}

float ui_object_base::get_rotation() const
{
	return DirectX::XMConvertToDegrees(rotation);
}

void ui_object_base::set_rotation(float angle)
{
	this->rotation = DirectX::XMConvertToRadians(angle);
	set_dirty();
}

DirectX::XMFLOAT4 ui_object_base::get_screen_rect() const
{
	return ui::calculate_screen_rect(screen_rect);
}

void ui_object_base::set_screen_rect(const DirectX::XMFLOAT4& screen_rect)
{
	this->screen_rect = ui::calculate_screen_rect_rate(screen_rect);
	this->screen_rect.x = std::max(this->screen_rect.x, 0.0f);
	this->screen_rect.y = std::max(this->screen_rect.y, 0.0f);
	set_dirty();
}

const DirectX::XMFLOAT2& ui_object_base::get_pivot() const
{
	return pivot;
}

void ui_object_base::set_pivot(const DirectX::XMFLOAT2& position)
{
	this->pivot = pivot;
	set_dirty();
}

const DirectX::XMFLOAT4& ui_object_base::get_uv_rect() const
{
	return uv_rect;
}

void ui_object_base::set_uv_rect(const DirectX::XMFLOAT4& uv_rect)
{
	this->uv_rect = uv_rect;
	set_dirty();
}

const DirectX::XMFLOAT4& ui_object_base::get_color() const
{
	return color;
}

void ui_object_base::set_color(const DirectX::XMFLOAT4& color)
{
	this->color = color;
	set_dirty();
}

void ui_object_base::set_color(const DirectX::XMFLOAT3& color)
{
	this->color.x = color.x;
	this->color.y = color.y;
	this->color.z = color.z;
	set_dirty();
}

bool ui_object_base::is_dirty() const
{
	return dirty;
}

void ui_object_base::set_dirty()
{
	dirty = true;
}

DirectX::XMFLOAT4 ui_object_base::get_screen_rect_inner() const
{
	return screen_rect;
}

ui_object_base::ui_object_base(std::string_view name)
: game_object(name)
, dirty(true)
, sort_order(0)
, rotation(0)
, pivot(0.5f, 0.5f)
, screen_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
, uv_rect(0, 0, 1, 1)
, color(1, 1, 1, 1)
{
}

ui_object_base::~ui_object_base()
{
}

void ui_object_base::post_render()
{
	dirty = false;
}

#if defined(USE_IMGUI)
void ui_object_base::imgui_render_core()
{
	auto rect = get_screen_rect();
	if (ImGui::DragFloat4("screen_rect", &rect.x, 1.0f))
	{
		set_screen_rect(rect);
	}

	auto rotation = get_rotation();
	if (ImGui::DragFloat("rotation", &rotation, 0.1f))
	{
		set_rotation(rotation);
	}

	auto pivot = get_pivot();
	if (ImGui::DragFloat2("pivot", &pivot.x, 0.01f))
	{
		set_pivot(pivot);
	}

	auto uv_rect = get_uv_rect();
	if (ImGui::DragFloat4("uv_rect", &uv_rect.x, 0.01f, 0.0f, 1.0f))
	{
		set_uv_rect(uv_rect);
	}

	auto color = get_color();
	if (ImGui::ColorEdit4("color", &color.x))
	{
		set_color(color);
	}

	auto order = get_sort_order();
	if (ImGui::DragInt("sort_order", &order, 0.1f))
	{
		set_sort_order(order);
	}
}
#endif	//	defined(USE_IMGUI)

/////////////////////////////////////////////////////////////////////////////////////////////////////
ui_object::ui_object(std::string_view name)
: ui_object_base(name)
{
}

ui_object::~ui_object()
{
}

void ui_object::pre_render()
{
	if (is_dirty())
	{
		auto screen_rect = get_screen_rect_inner();
		auto color = get_color();
		auto rotation = get_rotation();
		auto pivot = get_pivot();
		auto uv_rect = get_uv_rect();
		sprite.update_transform(screen_rect.x, screen_rect.y, screen_rect.z, screen_rect.w,
			color.x, color.y, color.z, color.w,
			rotation, pivot.x, pivot.y,
			uv_rect.x, uv_rect.y, uv_rect.z, uv_rect.w);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
ui_objects::ui_objects(std::string_view name)
: ui_object_base(name)
{
}

ui_objects::~ui_objects()
{
}


void ui_objects::pre_render()
{
	if (is_dirty())
	{
		auto screen_rect = get_screen_rect_inner();
		auto color = get_color();
		auto rotation = get_rotation();
		auto pivot = get_pivot();
		auto uv_rect = get_uv_rect();
		for (auto& sprite : sprites)
		{
			sprite.update_transform(screen_rect.x, screen_rect.y, screen_rect.z, screen_rect.w,
				color.x, color.y, color.z, color.w,
				rotation, pivot.x, pivot.y,
				uv_rect.x, uv_rect.y, uv_rect.z, uv_rect.w);
		}
	}
}
