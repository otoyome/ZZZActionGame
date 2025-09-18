#pragma once

#include "render_pass.h"

//	レンダーターゲット集
//	☆ﾃｷﾄｰ('ω')ｰﾄｷﾃ☆
struct scene_render_targets
{
	texture_ptr scene_color_texture;
	texture_ptr scene_depth_stencil_texture;
	std::vector<texture_ptr> g_buffer_textures;
	texture_ptr shadowmap_depth_texture;

	texture_ptr work_buffer_scene_textures[2];
	texture_ptr work_buffer_rgba32float_texture;
};
