#pragma once

#include <mutex>

//	ゲーム用オブジェクト
class game_object : public std::enable_shared_from_this<game_object>
{
public:
	static constexpr size_t invalid_id = 0;

public:
	static constexpr std::string_view type_name() {return NAMEOF_TYPE(game_object);}
	virtual std::string_view get_type_name() const {return type_name();}
	virtual bool check_type(std::string_view text) const {return type_name() == text;}

public:
	size_t get_id() const;
	std::string_view get_name() const;
	void set_name(std::string_view name);
	bool is_active() const;
	void set_active(bool active);
	std::mutex& get_mutex();

public:
	game_object(std::string_view name);
	virtual ~game_object();
	virtual void fixed_update(float elapsed_time);
	virtual void update(float elapsed_time);
	virtual void post_update(float elapsed_time);

	virtual void pre_render();
	virtual void post_render();

#if defined(_DEBUG)
	virtual void debug_start_of_frame() {}
	virtual void debug_pre_update(float elapsed_time) {}
	virtual void debug_update(float elapsed_time) {}
	virtual void debug_post_update(float elapsed_time) {}
#endif	//	defined(_DEBUG)

#if defined(USE_IMGUI)
	void imgui_render();
	virtual void imgui_render_core();
#endif	//	defined(USE_IMGUI)

	virtual void on_initialize();
	virtual void on_finalize();

protected:
	virtual void on_active();
	virtual void on_deactive();

private:
	size_t id;
	std::string name;
	bool active;
#if defined(_DEBUG)
	bool debug_object;
#endif	//	defined(_DEBUG)
	std::mutex mutex;
	inline static size_t id_counter;
};

#define	GAME_OBJECT_DECRLEAR(ABSTRACT_TYPE, BASE_TYPE)	\
public:	\
using base = BASE_TYPE;	\
	static constexpr std::string_view type_name() {return NAMEOF_TYPE(ABSTRACT_TYPE);}	\
	std::string_view get_type_name() const override {return type_name();}	\
	bool check_type(std::string_view text) const override {return type_name() == text ? true : BASE_TYPE::check_type(text);}	\
private:
