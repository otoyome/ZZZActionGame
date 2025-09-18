#pragma once

class renderer
{
public:
	enum class domain
	{
		depth_prepass,
		g_buffer,
		g_buffer_decal,
		deffered_lighting,
		post_process,
		effect_rendering,
		final_pass,

#if defined(_DEBUG)
		debug_draw,
#endif // defined(_DEBUG)

		ui_rendering,
	};
	virtual domain get_domain() const = 0;

public:
	renderer(){}
	virtual ~renderer() {}

	virtual void render(graphics_context& context) = 0;
	virtual void resize_buffer(int width, int height) = 0;
};
