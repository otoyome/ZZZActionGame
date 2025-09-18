#pragma once

#include "collision/collision.h"
#include "collision_object.h"
#include "game_object.h"

using collision_object_ptr = std::shared_ptr<collision_object>;
using game_object_ptr = std::shared_ptr<game_object>;

//	オブジェクトマネージャー
class object_manager
{
	SINGLETON(object_manager)

private:
	object_manager();
	~object_manager();

public:
	void updates(float elapsed_time);
	void pre_render();
	void post_render();

	void finalize();

#if defined(USE_IMGUI)
	void imgui_render();

	bool get_draw_collision() const;
	void set_draw_collision(bool draw_collision);

	bool get_fixed_update_count() const;

	bool get_pause() const;
	void set_pause(bool pause);

	void called_step();
#endif // defined(USE_IMGUI)

public:
	bool register_object(game_object_ptr object);
	bool register_object(collision_object_ptr object);
	bool unregister_object(game_object_ptr object);
	bool unregister_object(collision_object_ptr object);

	template<typename type>
	std::shared_ptr<type> search_object()
	{
		for (auto& object : game_objects)
		{
			if (object->check_type(NAMEOF_TYPE(type)))
			{
				return std::static_pointer_cast<type>(object);
			}
		}
		for (auto& object : game_object_add)
		{
			if (object->check_type(NAMEOF_TYPE(type)))
			{
				return std::static_pointer_cast<type>(object);
			}
		}
		return nullptr;
	}

	template<typename type>
	std::shared_ptr<type> search_object(std::string_view name)
	{
		for (auto& object : game_objects)
		{
			if (object->check_type(NAMEOF_TYPE(type)))
			{
				if (object->get_name() == name)
				{
					return std::static_pointer_cast<type>(object);
				}
			}
		}
		for (auto& object : game_object_add)
		{
			if (object->check_type(NAMEOF_TYPE(type)))
			{
				if (object->get_name() == name)
				{
					return std::static_pointer_cast<type>(object);
				}
			}
		}
		return nullptr;
	}

	template<typename type>
	std::vector<std::shared_ptr<type>> search_objects()
	{
		std::vector<std::shared_ptr<type>> datas;
		for (auto& object : game_objects)
		{
			if (object->check_type(NAMEOF_TYPE(type)))
			{
				datas.emplace_back(std::static_pointer_cast<type>(object));
			}
		}
		for (auto& object : game_object_add)
		{
			if (object->check_type(NAMEOF_TYPE(type)))
			{
				datas.emplace_back(std::static_pointer_cast<type>(object));
			}
		}
		return datas;
	}

	template<typename type>
	std::vector<std::shared_ptr<game_object>> search_game_objects()
	{
		std::vector<std::shared_ptr<game_object>> datas;
		for (auto& object : game_objects)
		{
			if (object->check_type(NAMEOF_TYPE(type)))
			{
				datas.emplace_back(object);
			}
		}
		for (auto& object : game_object_add)
		{
			if (object->check_type(NAMEOF_TYPE(type)))
			{
				datas.emplace_back(object);
			}
		}
		return datas;
	}

public:
	struct ray_cast_descs
	{
		DirectX::XMFLOAT3 start;
		DirectX::XMFLOAT3 end;
		collision_filter_ptr filter;
		std::vector<collision_object_ptr> exclude_objects;
	};
	bool ray_cast(const ray_cast_descs& desc, collision::intersect_data& intersect_data, collision_object_ptr& out_collision);

private:
	void add_objects();
	void remove_objects();

private:
	std::vector<game_object_ptr> game_objects;
	std::vector<game_object_ptr> game_object_add;
	std::vector<game_object_ptr> game_object_removes;

	std::vector<collision_object_ptr> collision_objects;
	std::vector<collision_object_ptr> collision_object_add;
	std::vector<collision_object_ptr> collision_object_removes;

	float total_time;
	float remain_time;

#if defined(USE_IMGUI)
	bool pause;
	bool step;
	int fixed_update_count;
	bool draw_collision;
	bool debug_stop_time;
#endif // defined(USE_IMGUI)
};
