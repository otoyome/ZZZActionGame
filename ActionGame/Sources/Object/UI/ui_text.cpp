#include "object/ui/ui_text.h"

std::string_view ui_text::get_texts() const
{
	return texts;
}

void ui_text::set_texts(std::string_view texts)
{
	this->texts = texts;
	set_dirty();
}

ui_text::ui_text(std::string_view name, font_data_ptr font_data)
: ui_objects(name)
, font_data(font_data)
, alignment_horizontal(alignment_horizontals::left)
, alignment_vertical(alignment_verticals::top)
, texts("text")
{
	get_sprites().resize(font_data->get_textures().size());
	int texture_index = 0;
	for (auto& sprite : get_sprites())
	{
		sprite.get_material().set_texture(sprite::base_texture_name, font_data->get_textures().at(texture_index++));
	}
}

ui_text::~ui_text()
{
}

void ui_text::pre_render()
{
	//	頂点修正
	if (is_dirty())
	{
		DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		std::vector<std::vector<sprite::vertex>> page_vertices(get_sprites().size());
		std::vector<std::vector<UINT>> page_indices(get_sprites().size());

		float start_x = 0.0f;
		float start_y = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float x_scale = 1.0f;
		float y_scale = 1.0f;
		float space = font_data->get_font_width() * x_scale;

		std::wstring utf16_texts;
		utf16_texts.resize(1024);
		system_utility::convert_shift_jis_to_utf16(texts.data(), utf16_texts.data(), utf16_texts.size());
		for (size_t index = 0; index < utf16_texts.size(); ++index)
		{
			// 文字値から文字情報が格納されているコードを取得
			WORD word = static_cast<WORD>(utf16_texts[index]);
			WORD code = font_data->get_character_indices().at(word);

			// 特殊制御用コードの処理
			if (code == font_data::EndCode)
			{
				break;
			}
			else if (code == font_data::ReturnCode)
			{
				x = start_x;
				y += font_data->get_font_height() * y_scale;
				continue;
			}
			else if (code == font_data::TabCode)
			{
				x += space * 4;
				continue;
			}
			else if (code == font_data::SpaceCode)
			{
				x += space;
				continue;
			}

			// 文字情報を取得し、頂点データを編集
			const font_data::character_info& info = font_data->get_character_infos().at(code);
			float xoffset = info.xoffset * x_scale;
			float yoffset = info.yoffset * y_scale;
			float width = info.width * x_scale;
			float height = info.height * y_scale;
			float xadvance = info.xadvance * x_scale;
			float positionX = x + xoffset;// + 0.5f;
			float positionY = y + yoffset;// + 0.5f;

			auto& vertices = page_vertices[info.page];
			auto& indices = page_indices[info.page];

			UINT previous_vertex_size = static_cast<UINT>(vertices.size());
			vertices.emplace_back();
			vertices.back().position.x = positionX;
			vertices.back().position.y = positionY;
			vertices.back().position.z = 0.0f;
			vertices.back().texcoord.x = info.left;
			vertices.back().texcoord.y = info.top;
			vertices.back().color = color;

			vertices.emplace_back();
			vertices.back().position.x = positionX + width;
			vertices.back().position.y = positionY;
			vertices.back().position.z = 0.0f;
			vertices.back().texcoord.x = info.right;
			vertices.back().texcoord.y = info.top;
			vertices.back().color = color;

			vertices.emplace_back();
			vertices.back().position.x = positionX;
			vertices.back().position.y = positionY + height;
			vertices.back().position.z = 0.0f;
			vertices.back().texcoord.x = info.left;
			vertices.back().texcoord.y = info.bottom;
			vertices.back().color = color;

			vertices.emplace_back();
			vertices.back().position.x = positionX + width;
			vertices.back().position.y = positionY + height;
			vertices.back().position.z = 0.0f;
			vertices.back().texcoord.x = info.right;
			vertices.back().texcoord.y = info.bottom;
			vertices.back().color = color;

			x += xadvance;

			indices.push_back(previous_vertex_size + 0);
			indices.push_back(previous_vertex_size + 1);
			indices.push_back(previous_vertex_size + 2);
			indices.push_back(previous_vertex_size + 1);
			indices.push_back(previous_vertex_size + 3);
			indices.push_back(previous_vertex_size + 2);
		}

		int index = 0;
		for (auto& sprite : get_sprites())
		{
			for (auto& vertex : page_vertices[index])
			{
				vertex.position.x /= get_screen_rect().z;
				vertex.position.y /= get_screen_rect().w;
			}
			sprite.update_vertices(page_vertices[index], page_indices[index]);
			++index;
		}
	}

	base::pre_render();
}

#if defined(USE_IMGUI)
void ui_text::imgui_render_core()
{
	base::imgui_render_core();

	std::string temp_text = texts;
	temp_text.resize(1024);
	if (ImGui::InputText("Text", temp_text.data(), temp_text.size()))
	{
		set_texts(temp_text);
	}
}
#endif	//	defined(USE_IMGUI)

