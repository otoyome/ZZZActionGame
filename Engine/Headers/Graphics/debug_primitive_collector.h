#pragma once

#if defined(_DEBUG)
#include "system_utility.h"
#include "graphics_context.h"
#include "render_data.h"

class debug_primitive_manager
{
	SINGLETON(debug_primitive_manager);

	static constexpr size_t VertexCount = 0x000FFFFF;

	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	struct line
	{
		DirectX::XMFLOAT3 start;
		DirectX::XMFLOAT3 end;
		line(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end) : start(start), end(end) {}
	};

private:
	debug_primitive_manager();
	~debug_primitive_manager();

public:
	void get_container(debug_draw_primitive_proxy_container& container);
	void push_line(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color);
	void push_sphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color);
	void push_capsule(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT4& rotation, float radius, float length, const DirectX::XMFLOAT4& color);
	void push_box(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT4& rotation, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color);

private:
	std::vector<vertex> vertices;

	std::unordered_map<semantic, buffer_view> vertex_buffer_views;
};

#define PUSH_DEBUG_PRIMITIVE_LINE(START, END, COLOR) debug_primitive_manager::instance()->push_line(START, END, COLOR)
#define PUSH_DEBUG_PRIMITIVE_SPHERE(CENTER, RADIUS, COLOR) debug_primitive_manager::instance()->push_sphere(CENTER, RADIUS, COLOR)
#define PUSH_DEBUG_PRIMITIVE_CAPSULE(CENTER, ROTATION, RADIUS, LENGTH, COLOR) debug_primitive_manager::instance()->push_capsule(CENTER, ROTATION, RADIUS, LENGTH, COLOR)
#define PUSH_DEBUG_PRIMITIVE_BOX(SCALE, ROTATION, POSITION, COLOR) debug_primitive_manager::instance()->push_box(SCALE, ROTATION, POSITION, COLOR)

#else // defined(_DEBUG)

#define PUSH_DEBUG_PRIMITIVE_LINE(START, END, COLOR)
#define PUSH_DEBUG_PRIMITIVE_SPHERE(CENTER, RADIUS, COLOR)
#define PUSH_DEBUG_PRIMITIVE_CAPSULE(CENTER, RADIUS, LENGTH, COLOR)
#define PUSH_DEBUG_PRIMITIVE_BOX(SCALE, ROTATION, POSITION, COLOR)

#endif // defined(_DEBUG)
