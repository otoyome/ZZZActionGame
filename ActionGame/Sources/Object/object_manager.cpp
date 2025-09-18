#include "Object/object_manager.h"

namespace
{
	static constexpr float fixed_per_second = system_units::convert_fps_to_sec(0.25f);
	static constexpr float addjust_pushing_factor = 0.05f;
}

object_manager::object_manager()
: game_objects()
, game_object_add()
, game_object_removes()

, collision_objects()
, collision_object_add()
, collision_object_removes()

, total_time(0)
, remain_time(0)

#if defined(USE_IMGUI)
, draw_collision(true)
#endif // defined(USE_IMGUI)
{
}

object_manager::~object_manager()
{
}

void object_manager::updates(float elapsed_time)
{
	//	追加
	add_objects();

#if defined(USE_IMGUI)
	if (pause)
	{
		if (step)
		{
			elapsed_time = fixed_per_second + 0.00001f;
			step = false;
		}
		else
		{
			return;
		}
	}
#endif // defined(USE_IMGUI)

	//	更新処理
	{
#if defined(_DEBUG)
		//	デバッグ更新
		for (auto& object : game_objects)
		{
			if (!object->is_active())
				continue;
			object->debug_start_of_frame();
		}
#endif	//	defined(_DEBUG)

		total_time += elapsed_time;
		float temporary_elapsed_time = elapsed_time + remain_time;
		int loop_count = static_cast<int>(temporary_elapsed_time / fixed_per_second);
#if defined(USE_IMGUI)
		fixed_update_count = loop_count;
#endif // defined(USE_IMGUI)
		int limit_loop_count = std::min(loop_count, static_cast<int>(1.0f / fixed_per_second + 0.0001f));
		temporary_elapsed_time -= (loop_count - limit_loop_count) * fixed_per_second;
		for (int count = 0; count < limit_loop_count; ++count)
		{
			add_objects();

			//	固定更新処理
			for (auto& object : game_objects)
			{
				if (!object->is_active())
					continue;
				object->fixed_update(fixed_per_second);
			}

			remove_objects();
			add_objects();

			//	当たり判定処理
			for (auto& object : collision_objects)
			{
				if (!object->is_flag(collision_object::flags::active))
					continue;

				object->pre_intersect();
			}

			remove_objects();
			add_objects();

			//	ヒット処理
			float pushing_factor = 1.0f + addjust_pushing_factor;
			do
			{
				bool is_pushing = false;
				auto* collision_array = collision_objects.data();
				for (size_t index1 = 0; index1 < collision_objects.size(); ++index1)
				{
					auto& object1 = collision_objects[index1];
					if (!object1->is_flag(collision_object::flags::active))
						continue;

					for (size_t index2 = index1 + 1; index2 < collision_objects.size(); ++index2)
					{
						auto& object2 = collision_objects[index2];
						if (!object2->is_flag(collision_object::flags::active))
							continue;

						//	両方staticの場合は除外
						bool is_static_object = object1->is_flag(collision_object::flags::static_object) &&
												object2->is_flag(collision_object::flags::static_object);
						if (is_static_object)
							continue;

						//	カテゴリーチェック
						if (!object1->get_filter()->is_flag(object2->get_filter()->get_category()))
							continue;

						collision::intersect_data intersect_data;
						if (object1->intersect(object2, intersect_data))
						{
							collision::intersect_data data1 = intersect_data;
							collision::intersect_data data2 = intersect_data;
							//	押し合い処理
							bool is_push = !object1->is_flag(collision_object::flags::detect_only) &&
											!object2->is_flag(collision_object::flags::detect_only);
							if (is_push && !is_static_object)
							{
								data1.clear();
								data2.clear();
								//	押し合い係数生成
								float rate1 = 0, rate2 = 0;
								if (object1->is_flag(collision_object::flags::static_object))
								{
									rate2 = 1;
								}
								else if (object2->is_flag(collision_object::flags::static_object))
								{
									rate1 = 1;
								}
								else
								{
									float total_mass = object1->get_mass() + object2->get_mass();
									rate2 = object1->get_mass() / total_mass;
									rate1 = object2->get_mass() / total_mass;
								}

								bool is_pushing = false;
								for (size_t index = 0; index < intersect_data.size(); ++index)
								{
									auto data = intersect_data[index];
									if (fabsf(data.length) <= FLT_EPSILON)
										continue;

									auto temp = data;
									temp.length = temp.length * rate1 * pushing_factor;
									data1.add(temp);

									temp.normal.x = -data.normal.x;
									temp.normal.y = -data.normal.y;
									temp.normal.z = -data.normal.z;
									temp.length = data.length * rate2 * pushing_factor;
									data2.add(temp);
									is_pushing = true;
								}
								object1->solve_pushing_objects(data1);
								object2->solve_pushing_objects(data2);
								if (is_pushing)
									is_pushing = true;
							}
							else
							{
								//	法線を反転しておく
								for (size_t index = 0; index < data2.size(); ++index)
								{
									auto& data = data2[index];
									if (fabsf(data.length) <= FLT_EPSILON)
										continue;

									data.normal.x = -data.normal.x;
									data.normal.y = -data.normal.y;
									data.normal.z = -data.normal.z;
								}

							}

							//	ヒットオブジェクトを記録
							object1->update_hit_objects(data1, object2);
							object2->update_hit_objects(data2, object1);
						}
					}
				}
				//	係数更新
				pushing_factor = is_pushing ? pushing_factor + addjust_pushing_factor : 0.0f;
			} while (1.0f <= pushing_factor && 1.0f <= addjust_pushing_factor * 10);

			remove_objects();
			add_objects();

			for (auto& object : collision_objects)
			{
				if (!object->is_flag(collision_object::flags::active))
					continue;

				object->post_intersect();
			}

			remove_objects();
			add_objects();

			temporary_elapsed_time -= fixed_per_second;
		}
		remain_time = temporary_elapsed_time;

		//	更新処理
		for (auto& object : game_objects)
		{
			if (!object->is_active())
				continue;
			object->update(elapsed_time);
		}

		remove_objects();
		add_objects();

		//	更新後処理
		for (auto& object : game_objects)
		{
			if (!object->is_active())
				continue;
			object->post_update(elapsed_time);
		}

		remove_objects();
		add_objects();

#if defined(_DEBUG)
		//	デバッグ更新
		for (auto& object : game_objects)
		{
			if (!object->is_active())
				continue;
			object->debug_pre_update(elapsed_time);
		}
		for (auto& object : game_objects)
		{
			if (!object->is_active())
				continue;
			object->debug_update(elapsed_time);
		}
		for (auto& object : game_objects)
		{
			if (!object->is_active())
				continue;
			object->debug_post_update(elapsed_time);
		}
#endif	//	defined(_DEBUG)
	}

#if defined(USE_IMGUI)
	//	デバッグライン描画
	if (draw_collision)
	{
		for (auto& object : collision_objects)
		{
			object->debug_draw();
		}
	}
#endif // defined(USE_IMGUI)
}

void object_manager::pre_render()
{
	remove_objects();
	add_objects();

	//	描画前処理
	for (auto& object : game_objects)
	{
		if (!object->is_active())
			continue;

		object->pre_render();
	}

	remove_objects();
	add_objects();
}

void object_manager::post_render()
{
	remove_objects();
	add_objects();

	//	描画前処理
	for (auto& object : game_objects)
	{
		if (!object->is_active())
			continue;

		object->post_render();
	}

	remove_objects();
	add_objects();
}

void object_manager::finalize()
{
	for (auto& object : game_objects)
	{
		object->on_finalize();
	}
	system_utility::std_vector_swap_clear(game_objects);
	for (auto& object : game_object_add)
	{
		object->on_finalize();
	}
	system_utility::std_vector_swap_clear(game_object_add);
	for (auto& object : game_object_removes)
	{
		object->on_finalize();
	}
	system_utility::std_vector_swap_clear(game_object_removes);

	system_utility::std_vector_swap_clear(collision_objects);
	system_utility::std_vector_swap_clear(collision_object_add);
	system_utility::std_vector_swap_clear(collision_object_removes);
}

#if defined(USE_IMGUI)
void object_manager::imgui_render()
{
	if (ImGui::Begin("objects"))
	{
		for (auto& object : game_objects)
		{
			object->imgui_render();
		}
	}
	ImGui::End();

	if (ImGui::Begin("collisions"))
	{
		for (auto& object : collision_objects)
		{
			object->imgui_render();
		}
	}
	ImGui::End();
}

bool object_manager::get_draw_collision() const
{
	return draw_collision;
}

void object_manager::set_draw_collision(bool draw_collision)
{
	this->draw_collision = draw_collision;
}

bool object_manager::get_fixed_update_count() const
{
	return fixed_update_count;
}

bool object_manager::get_pause() const
{
	return pause;
}

void object_manager::set_pause(bool pause)
{
	this->pause = pause;
}

void object_manager::called_step()
{
	step = true;
}

#endif // defined(USE_IMGUI)

bool object_manager::register_object(game_object_ptr object)
{
	if (!object)
		return false;
	if (std::find(game_object_add.begin(), game_object_add.end(), object) != game_object_add.end())
		return false;
	if (std::find(game_objects.begin(), game_objects.end(), object) != game_objects.end())
		return false;

	auto it = std::find(game_object_removes.begin(), game_object_removes.end(), object);
	if (it != game_object_removes.end())
	{
		game_object_removes.erase(it);
	}
	game_object_add.emplace_back(object);
	return true;
}

bool object_manager::register_object(collision_object_ptr object)
{
	if (!object)
		return false;
	if (std::find(collision_object_add.begin(), collision_object_add.end(), object) != collision_object_add.end())
		return false;
	if (std::find(collision_objects.begin(), collision_objects.end(), object) != collision_objects.end())
		return false;

	auto it = std::find(collision_object_removes.begin(), collision_object_removes.end(), object);
	if (it != collision_object_removes.end())
	{
		collision_object_removes.erase(it);
	}
	collision_object_add.emplace_back(object);
	return true;
}

bool object_manager::unregister_object(game_object_ptr object)
{
	if (!object)
		return false;
	if (std::find(game_object_removes.begin(), game_object_removes.end(), object) != game_object_removes.end())
		return false;
	if (std::find(game_objects.begin(), game_objects.end(), object) == game_objects.end())
		return false;

	auto it = std::find(game_object_add.begin(), game_object_add.end(), object);
	if (it != game_object_add.end())
	{
		game_object_add.erase(it);
	}
	game_object_removes.emplace_back(object);
	return true;
}

bool object_manager::unregister_object(collision_object_ptr object)
{
	if (!object)
		return false;
	if (std::find(collision_object_removes.begin(), collision_object_removes.end(), object) != collision_object_removes.end())
		return false;
	if (std::find(collision_objects.begin(), collision_objects.end(), object) == collision_objects.end())
		return false;

	auto it = std::find(collision_object_add.begin(), collision_object_add.end(), object);
	if (it != collision_object_add.end())
	{
		collision_object_add.erase(it);
	}
	collision_object_removes.emplace_back(object);
	return true;
}

bool object_manager::ray_cast(const ray_cast_descs& desc, collision::intersect_data& intersect_data, collision_object_ptr& out_collision)
{
	for (auto& object : collision_objects)
	{
		if (!object->is_flag(collision_object::flags::active) ||
			object->is_flag(collision_object::flags::detect_only))
			continue;

		//	カテゴリーチェック
		if (!desc.filter->is_flag(object->get_filter()->get_category()))
			continue;

		//	除外オブジェクトチェック
		{
			bool find = false;
			for (auto& exclude_object : desc.exclude_objects)
			{
				if (exclude_object == object)
				{
					find = true;
					break;
				}
			}
			if (find)
				continue;
		}

		collision::intersect_data data;
		if (object->get_bounding_object()->intersect_segment(desc.start, desc.end, &data))
		{
			if (intersect_data.size() == 0)
			{
				intersect_data = data;
				out_collision = object;
			}
			else if (data[0].length < intersect_data[0].length)
			{
				intersect_data = data;
				out_collision = object;
			}
		}
	}
	return intersect_data.size() > 0;
}

void object_manager::add_objects()
{
	for (auto& object : game_object_add)
	{
		game_objects.emplace_back(object->shared_from_this());
		game_objects.back()->on_initialize();
	}
	system_utility::std_vector_swap_clear(game_object_add);

	for (auto& object : collision_object_add)
	{
		collision_objects.emplace_back(object->shared_from_this());
	}
	system_utility::std_vector_swap_clear(collision_object_add);
}

void object_manager::remove_objects()
{
	for (auto& object : game_object_removes)
	{
		auto it = std::find(game_objects.begin(), game_objects.end(), object);
		if (it != game_objects.end())
		{
			(*it)->on_finalize();
			game_objects.erase(it);
		}
	}
	system_utility::std_vector_swap_clear(game_object_removes);

	for (auto& object : collision_object_removes)
	{
		collision_objects.erase(std::find(collision_objects.begin(), collision_objects.end(), object));
	}
	system_utility::std_vector_swap_clear(collision_object_removes);
}
