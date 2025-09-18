#pragma once

#include "object/ui/ui_object.h"
#include "font_data.h"

//	UI制御用オブジェクト
class ui_text : public ui_objects
{
	GAME_OBJECT_DECRLEAR(ui_text, ui_objects)

public:
	enum class alignment_horizontals
	{
		left,
		center,
		right,
	};
	enum class alignment_verticals
	{
		top,
		middle,
		bottom
	};

public:
	std::string_view get_texts() const;
	void set_texts(std::string_view texts);

public:
	ui_text(std::string_view name, font_data_ptr font_data);
	~ui_text() override;

	void pre_render() override;

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

private:
	font_data_ptr font_data;
	alignment_horizontals alignment_horizontal;
	alignment_verticals alignment_vertical;

	std::string texts;
};
