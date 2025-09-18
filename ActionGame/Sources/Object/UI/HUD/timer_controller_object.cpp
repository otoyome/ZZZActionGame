#include "object/ui/hud/timer_controller_object.h"

namespace
{
	enum ui_id
	{
		base,
		text,
	};
}

timer_controller_object::timer_controller_object(const object_descs& object_desc)
: game_object(object_desc.name)
{
	image = object_desc.image;
	if (image)
		origin_colors[ui_id::base] = image->get_color();
	text = object_desc.text;
	if (text)
		origin_colors[ui_id::text] = text->get_color();
}

timer_controller_object::~timer_controller_object()
{
}

void timer_controller_object::set_time(float time)
{
	text->set_texts(system_utility::to_display_time_string(time));
}

void timer_controller_object::set_alpha(float alpha)
{
	if (image)
	{
		DirectX::XMFLOAT4 color = origin_colors[ui_id::base];
		color.w *= alpha;
		image->set_color(color);
	}
	if (text)
	{
		DirectX::XMFLOAT4 color = origin_colors[ui_id::text];
		color.w *= alpha;
		text->set_color(color);
	}
}



