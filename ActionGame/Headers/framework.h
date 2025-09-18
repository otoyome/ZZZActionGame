#pragma once

#if defined(USE_IMGUI)
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif

#include "render_pass.h"

#include "Scenes/scene_base.h"
#include "Renderer/renderer.h"
#include "Renderer/scene_render_targets.h"

class framework
{
public:
	static constexpr WCHAR* APPLICATION_NAME = L"アクションゲーム";

public:
	framework(HWND hwnd);
	~framework();

	framework(const framework&) = delete;
	framework& operator=(const framework&) = delete;
	framework(framework&&) noexcept = delete;
	framework& operator=(framework&&) noexcept = delete;

	int run();

	LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	bool initialize();
	void update(float elapsed_time);
	void render(float elapsed_time);
	bool uninitialize();

	void calculate_frame_stats();

	bool create_buffers();
	bool create_collision_filters();

private:
	CONST HWND hwnd;
	std::shared_ptr<scene_render_targets> render_targets;
	std::vector<std::unique_ptr<renderer>> scene_renderers;
	std::vector<std::unique_ptr<renderer>> ui_renderers;

	float wheel{ 0 };
	high_resolution_timer tictoc;
	uint32_t frames{ 0 };
	float current_time{ 0.0f };
	float previous_time{ 0.0f };

	render_pass back_buffer_clear_rendering_pass;

#if defined(USE_IMGUI)
	bool display_grid_xz{ true };
	bool display_grid_xy{ false };
	bool display_grid_yz{ false };
	int display_grid_length{ 100 };
	render_pass imgui_rendering_pass;
	D3D11_VIEWPORT imgui_viewport;

	void imgui_scene_change();
#endif // defined(USE_IMGUI)
};

