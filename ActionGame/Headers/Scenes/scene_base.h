#pragma once

#include "render_data.h"
#include "light_data.h"

#include "sprite.h"
#include "geometric_primitive.h"
#include "gltf_model.h"
#include "pmx_model.h"

#include "shader.h"
#include "texture.h"

#include "nameof/include/nameof.hpp"
#include "magic_enum/include/magic_enum/magic_enum.hpp"

struct scene_generates
{
	template<typename Type>
	static void add_scene()
	{
		scene_manager::instance()->add_scene<Type>();
	}
	template<typename Type, typename ... Lists>
	static void add_scenes()
	{
		add_scene<Type>();
		if constexpr (sizeof...(Lists) > 0)
			add_scenes<Lists...>();
	}
};

class scene_base
{
public:
	static constexpr std::string_view type_name(){ return NAMEOF_TYPE(scene_base); }
	virtual std::string_view get_type_name() const { return type_name(); }
	virtual bool check_type(std::string_view text) const { return type_name() == text; }

public:
	scene_base();
	scene_base(const scene_base&) = delete;
	scene_base(scene_base&&) = delete;
	virtual ~scene_base();

	bool initialize();
	bool uninitialize();
	void update(float elapsed_time);
#if defined(USE_IMGUI)
	void imgui_render();
#endif	//	defined(USE_IMGUI)

protected:
	virtual bool initialize_core() { return true; }
	virtual bool uninitialize_core() { return true; }
	virtual void update_core(float elapsed_time) {}
#if defined(USE_IMGUI)
	virtual void imgui_render_core() {}
#endif	//	defined(USE_IMGUI)

protected:
	void register_object(game_object_ptr object);
	void register_object(collision_object_ptr object);

private:
	std::vector<std::weak_ptr<game_object>> game_objects;
	std::vector<std::weak_ptr<collision_object>> collision_objects;
};

class scene_manager
{
	SINGLETON(scene_manager)

private:
	scene_manager();
	~scene_manager();

public:
	void updates(float elapsed_time);

	void remove_all_scene();

	template<typename scene_type>
	void create_scene_function()
	{
		scenes.insert_or_assign(scene_type::type_name(), std::make_unique<scene_type>());
		auto& scene = scenes.find(scene_type::type_name())->second;
		scene->initialize();
	}

	//	全てのシーンを破棄して一つのシーンを読み込む
	template<typename scene_type>
	bool change_scene()
	{
		for (auto& scene : scenes)
		{
			if (std::find(remove_scenes.begin(), remove_scenes.end(), scene.second->get_type_name()) != remove_scenes.end())
				continue;

			remove_scenes.emplace_back(scene.second->get_type_name());
		}

		add_scene<scene_type>();

		return true;
	}

	template<typename scene_type>
	bool add_scene()
	{
		if (scenes.find(scene_type::type_name()) != scenes.end())
		{
			return false;
		}
		if (std::find_if(add_scenes.begin(),
			add_scenes.end(),
			[&](add_scene_data& data) {return data.name == scene_type::type_name(); }) != add_scenes.end())
		{
			return false;
		}

		add_scenes.emplace_back(scene_type::type_name(), scene_type::generate_order(), [this](){ create_scene_function<scene_type>(); });
		scene_type::on_scene_children();
		return true;
	}

	template<typename scene_type>
	bool remove_scene()
	{
		if (std::find_if(remove_scenes.begin(),
			remove_scenes.end(),
			[&](std::string_view name) {return name == scene_type::type_name(); }) != remove_scenes.end())
		{
			return false;
		}

		if (scenes.find(scene_type::type_name()) == scenes.end())
			return false;

		remove_scenes.emplace_back(scene_type::type_name());
		return true;
	}

public:
#if defined(USE_IMGUI)
	void imgui_render();

	//	全てのシーンを破棄して一つのシーンを読み込む
	template<typename scene_type>
	bool change_scene_for_imgui()
	{
		for (auto& scene : scenes)
		{
			if (std::find(remove_scenes.begin(), remove_scenes.end(), scene.second->get_type_name()) != remove_scenes.end())
				continue;

			remove_scenes.emplace_back(scene.second->get_type_name());
		}
		updates(0);

		add_scene<scene_type>();

		return true;
	}
#endif	//	defined(USE_IMGUI)

private:
	struct add_scene_data
	{
		std::string_view name;
		int order;
		std::function<void()> func;
		add_scene_data(std::string_view name, int order, std::function<void()> func)
		: name(name), order(order), func(func)
		{
		}
	};
	std::vector<add_scene_data> add_scenes;
	std::vector<std::string_view> remove_scenes;
	std::unordered_map<std::string_view, std::unique_ptr<scene_base>> scenes;
};

#define	__SCENE_DECRLEAR_INNER__(ABSTRACT_TYPE, BASE_TYPE)	\
public:	\
using base = BASE_TYPE;	\
static constexpr std::string_view type_name(){return NAMEOF_TYPE(ABSTRACT_TYPE);}	\
std::string_view get_type_name() const override {return type_name();}	\
bool check_type(std::string_view text) const override {return type_name() == text ? true : BASE_TYPE::check_type(text);}

#define	SCENE_DECRLEAR(ABSTRACT_TYPE, BASE_TYPE) \
__SCENE_DECRLEAR_INNER__(ABSTRACT_TYPE, BASE_TYPE) \
static constexpr int generate_order(){return 0;} \
static void on_scene_children() {} \
private:

#define	SCENE_DECRLEAR_AND_SET_ORDER(ABSTRACT_TYPE, ORDER, BASE_TYPE)	\
__SCENE_DECRLEAR_INNER__(ABSTRACT_TYPE, BASE_TYPE) \
static constexpr int generate_order(){return ORDER;} \
static void on_scene_children() {} \
private:

#define	SCENE_ADD_CHILDREN_DECRLEAR(ABSTRACT_TYPE, BASE_TYPE, ... )	\
__SCENE_DECRLEAR_INNER__(ABSTRACT_TYPE, BASE_TYPE) \
static constexpr int generate_order(){return 0;} \
static void on_scene_children() { scene_generates::add_scenes<__VA_ARGS__>(); } \
private:

#define	SCENE_ADD_CHILDREN_DECRLEAR_AND_SET_ORDER(ABSTRACT_TYPE, ORDER, BASE_TYPE, ... )	\
__SCENE_DECRLEAR_INNER__(ABSTRACT_TYPE, BASE_TYPE) \
static constexpr int generate_order(){return ORDER;} \
static void on_scene_children() { scene_generates::add_scenes<__VA_ARGS__>(); } \
private:
