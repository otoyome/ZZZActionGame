#include "object/ui/hud/face_image_controller_object.h"

namespace
{
	enum ui_id
	{
		cover,
		base,
		main,
	};
}

texture_ptr face_image_controller_object::get_image() const
{
	return uis[ui_id::main]->get_texture();
}

DirectX::XMFLOAT4 face_image_controller_object::get_uv_rect() const
{
	return uis[ui_id::main]->get_uv_rect();
}

void face_image_controller_object::set_image(texture_ptr image, DirectX::XMFLOAT4 uv_rect)
{
	uis[ui_id::main]->set_texture(image);
	uis[ui_id::main]->set_uv_rect(uv_rect);
}

face_image_controller_object::face_image_controller_object(const object_descs& object_desc)
: game_object(object_desc.name)
{
	uis[ui_id::cover] = object_desc.cover;
	uis[ui_id::base] = object_desc.base;
	uis[ui_id::main] = object_desc.main;
	for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
	{
		if (uis[index])
		{
			origin_rects[index] = uis[index]->get_screen_rect();
			origin_colors[index] = uis[index]->get_color();
		}
	}
}

face_image_controller_object::~face_image_controller_object()
{
}

void face_image_controller_object::post_update(float elapsed_time)
{
	shake_desc.time -= elapsed_time;
	if (shake_desc.time <= 0.0f)
	{
		if (shake_desc.time > -10000.0f)
		{
			for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
			{
				if (uis[index])
				{
					auto rect = uis[index]->get_screen_rect();
					rect.x -= shake_offset.x;
					rect.y -= shake_offset.y;
					uis[index]->set_screen_rect(rect);
				}
			}
			shake_offset.x = 0;
			shake_offset.y = 0;
		}
		shake_desc.time = -10000.0f;
		return;
	}

	float direction_x = shake_desc.power * static_cast<float>(rand() % 3 - 1);
	float direction_y = shake_desc.power * static_cast<float>(rand() % 3 - 1);
	for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
	{
		if (uis[index])
		{
			auto rect = uis[index]->get_screen_rect();
			rect.x -= shake_offset.x;
			rect.y -= shake_offset.y;
			rect.x += direction_x;
			rect.y += direction_y;
			uis[index]->set_screen_rect(rect);
		}
	}
	shake_offset.x = direction_x;
	shake_offset.y = direction_y;
}

void face_image_controller_object::call_shake(const shake_descs& shake_desc)
{
	this->shake_desc = shake_desc;
	this->shake_desc.power = (this->shake_desc.power * 1.0f);
}

void face_image_controller_object::set_alpha(float alpha)
{
	for (int index = 0; index < ARRAYSIZE(origin_rects); ++index)
	{
		if (!uis[index])
			continue;

		DirectX::XMFLOAT4 color = origin_colors[index];
		color.w *= alpha;
		uis[index]->set_color(color);
	}
}



