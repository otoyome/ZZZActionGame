#include "object/ui/ui_image.h"

ui_image::draw_modes ui_image::get_draw_mode() const
{
	return draw_mode;
}

void ui_image::set_draw_mode(draw_modes draw_mode)
{
	this->draw_mode = draw_mode;
	set_dirty();
}

DirectX::XMFLOAT4 ui_image::get_border() const
{
	return border;
}

void ui_image::set_border(DirectX::XMFLOAT4 border)
{
	this->border = math::saturate(border);
	set_dirty();
}

texture_ptr ui_image::get_texture() const
{
	texture_ptr texture;
	get_sprite().get_material().get_texture(sprite::base_texture_name, texture);
	return texture;
}

void ui_image::set_texture(std::string_view texture_path)
{
	texture_ptr texture;
	texture_manager::instance()->load_texture_from_file(texture, texture_path);
	get_sprite().get_material().set_texture(sprite::base_texture_name, texture);
}

void ui_image::set_texture(texture_ptr texture)
{
	get_sprite().get_material().set_texture(sprite::base_texture_name, texture);
}

ui_image::ui_image(std::string_view name)
: ui_object(name)
, draw_mode(draw_modes::single)
, border(0.25f, 0.25f, 0.5f, 0.5f)
{
}

ui_image::~ui_image()
{
}

void ui_image::pre_render()
{
	//	頂点修正
	if (is_dirty())
	{
		DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		std::vector<sprite::vertex> vertices;
		std::vector<UINT> indices;
		switch (draw_mode)
		{
			case draw_modes::single:
			{
				vertices.push_back({{ 0.0f, 0.0f, 0.0f }, color, { 0.0f, 0.0f }});
				vertices.push_back({{ 1.0f, 0.0f, 0.0f }, color, { 1.0f, 0.0f }});
				vertices.push_back({{ 0.0f, 1.0f, 0.0f }, color, { 0.0f, 1.0f }});
				vertices.push_back({{ 1.0f, 1.0f, 0.0f }, color, { 1.0f, 1.0f }});
				indices.emplace_back(0);
				indices.emplace_back(1);
				indices.emplace_back(2);
				indices.emplace_back(1);
				indices.emplace_back(3);
				indices.emplace_back(2);
				break;
			}
			case draw_modes::sliced:
			{
				//	テクスチャ空間での境界を算出
				texture_ptr texture;
				get_sprite().get_material().get_texture(sprite::base_texture_name, texture);
				DirectX::XMFLOAT2 texture_size = { SCREEN_WIDTH, SCREEN_HEIGHT };
				if (texture2d_ptr texture_2d = std::static_pointer_cast<texture2d>(texture))
				{
					texture_size.x = static_cast<float>(texture_2d->get_width());
					texture_size.y = static_cast<float>(texture_2d->get_height());
				}

				DirectX::XMFLOAT2 screen_rect_size = { get_screen_rect().z, get_screen_rect().w };
				DirectX::XMFLOAT2 size_rate = { screen_rect_size.x / texture_size.x, screen_rect_size.y / texture_size.y };
				//	座標
				DirectX::XMFLOAT2 corner_size[2];
				{
					float rate[2] = {border.x, math::saturate(1.0f - (border.x + border.z))};
					float total_rate = rate[0] + rate[1];
					{
						float w = std::max(total_rate - 1.0f, 0.0f);
						float w0 = rate[0] / total_rate;
						float w1 = rate[1] / total_rate;
						rate[0] -= w * w0;
						rate[1] -= w * w1;
						total_rate = rate[0] + rate[1];
					}
					float l = rate[0] * texture_size.x;
					float r = rate[1] * texture_size.x;
					float remain = texture_size.x * (std::min(size_rate.x, total_rate) - total_rate);
					l = std::max(0.0f, l + remain * (rate[0] / total_rate));
					r = std::max(0.0f, r + remain * (rate[1] / total_rate));

					corner_size[0].x = std::max(0.0f, l / screen_rect_size.x);
					corner_size[1].x = std::max(0.0f, (screen_rect_size.x - r) / screen_rect_size.x);
				}
				{
					float rate[2] = { border.y, math::saturate(1.0f - (border.y + border.w))};
					float total_rate = rate[0] + rate[1];
					{
						float w = std::max(total_rate - 1.0f, 0.0f);
						float w0 = rate[0] / total_rate;
						float w1 = rate[1] / total_rate;
						rate[0] -= w * w0;
						rate[1] -= w * w1;
						total_rate = rate[0] + rate[1];
					}
					float l = rate[0] * texture_size.y;
					float r = rate[1] * texture_size.y;
					float remain = texture_size.y * (std::min(size_rate.y, total_rate) - total_rate);
					l = std::max(0.0f, l + remain * (rate[0] / total_rate));
					r = std::max(0.0f, r + remain * (rate[1] / total_rate));

					corner_size[0].y = std::max(0.0f, l / screen_rect_size.y);
					corner_size[1].y = std::max(0.0f, (screen_rect_size.y - r) / screen_rect_size.y);
				}
				//	UV
				float llx = 0.0f;
				float lrx = 1.0f;
				float lly = 0.0f;
				float lry = 1.0f;
				float clx = corner_size[0].x;
				float crx = corner_size[1].x;
				float cly = corner_size[0].y;
				float cry = corner_size[1].y;
				vertices.push_back({{ llx, lly, 0.0f }, color, {                0.0f, 0.0f }});
				vertices.push_back({{ clx, lly, 0.0f }, color, {            border.x, 0.0f }});
				vertices.push_back({{ crx, lly, 0.0f }, color, { border.x + border.z, 0.0f }});
				vertices.push_back({{ lrx, lly, 0.0f }, color, {                1.0f, 0.0f }});

				vertices.push_back({{ llx, cly, 0.0f }, color, {                0.0f, border.y }});
				vertices.push_back({{ clx, cly, 0.0f }, color, {            border.x, border.y }});
				vertices.push_back({{ crx, cly, 0.0f }, color, { border.x + border.z, border.y }});
				vertices.push_back({{ lrx, cly, 0.0f }, color, {                1.0f, border.y }});

				vertices.push_back({{ llx, cry, 0.0f }, color, {                0.0f, border.y + border.w }});
				vertices.push_back({{ clx, cry, 0.0f }, color, {            border.x, border.y + border.w }});
				vertices.push_back({{ crx, cry, 0.0f }, color, { border.x + border.z, border.y + border.w }});
				vertices.push_back({{ lrx, cry, 0.0f }, color, {                1.0f, border.y + border.w }});

				vertices.push_back({{ llx, lry, 0.0f }, color, {                0.0f, 1.0f }});
				vertices.push_back({{ clx, lry, 0.0f }, color, {            border.x, 1.0f }});
				vertices.push_back({{ crx, lry, 0.0f }, color, { border.x + border.z, 1.0f }});
				vertices.push_back({{ lrx, lry, 0.0f }, color, {                1.0f, 1.0f }});

				indices.emplace_back(0);	indices.emplace_back(1);	indices.emplace_back(4);
				indices.emplace_back(1);	indices.emplace_back(5);	indices.emplace_back(4);

				indices.emplace_back(1);	indices.emplace_back(2);	indices.emplace_back(5);
				indices.emplace_back(2);	indices.emplace_back(6);	indices.emplace_back(5);

				indices.emplace_back(2);	indices.emplace_back(3);	indices.emplace_back(6);
				indices.emplace_back(3);	indices.emplace_back(7);	indices.emplace_back(6);

				indices.emplace_back(4);	indices.emplace_back(5);	indices.emplace_back(8);
				indices.emplace_back(5);	indices.emplace_back(9);	indices.emplace_back(8);

				indices.emplace_back(5);	indices.emplace_back(6);	indices.emplace_back(9);
				indices.emplace_back(6);	indices.emplace_back(10);	indices.emplace_back(9);

				indices.emplace_back(6);	indices.emplace_back(7);	indices.emplace_back(10);
				indices.emplace_back(7);	indices.emplace_back(11);	indices.emplace_back(10);

				indices.emplace_back(8);	indices.emplace_back(9);	indices.emplace_back(12);
				indices.emplace_back(9);	indices.emplace_back(13);	indices.emplace_back(12);

				indices.emplace_back(9);	indices.emplace_back(10);	indices.emplace_back(13);
				indices.emplace_back(10);	indices.emplace_back(14);	indices.emplace_back(13);

				indices.emplace_back(10);	indices.emplace_back(11);	indices.emplace_back(14);
				indices.emplace_back(11);	indices.emplace_back(15);	indices.emplace_back(14);
				break;
			}
		}
		get_sprite().update_vertices(vertices, indices);
	}

	base::pre_render();
}

#if defined(USE_IMGUI)
void ui_image::imgui_render_core()
{
	base::imgui_render_core();

	const char* draw_mode_names[] =
	{
		"single",
		"sliced",
	};
	static_assert(ARRAYSIZE(draw_mode_names) == magic_enum::enum_count<draw_modes>());

	int index = static_cast<int>(draw_mode);
	if (ImGui::Combo("draw_mode", &index, draw_mode_names, ARRAYSIZE(draw_mode_names)))
	{
		set_draw_mode(static_cast<draw_modes>(index));
	}

	auto rect = get_border();
	if (ImGui::DragFloat4("border", &rect.x, 0.01f, 0.0f, 1.0f))
	{
		set_border(rect);
	}
}
#endif	//	defined(USE_IMGUI)

