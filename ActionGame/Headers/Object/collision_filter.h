#pragma once

//	コリジョンカテゴリー
enum class collision_category
{
	player = 0,
	player_projectile,

	enemy,
	enemy_projectile,

	stage,

	invisible,

	camera,

	//	最大32個まで
};

//	コリジョンフィルター
class collision_filter
{
public:
	std::string_view get_name() const;
	void set_name(std::string_view name);

	collision_category get_category() const;
	void set_category(collision_category category);

	int get_mask() const;
	bool is_flag(collision_category category) const;
	void set_flag(collision_category category, bool value);

public:
	collision_filter();
	~collision_filter();

private:
	std::string name; // 名前
	collision_category category; // 所属カテゴリー
	int mask; // 衝突するカテゴリー
};

using collision_filter_ptr = std::shared_ptr<collision_filter>;

//	コリジョンフィルター
class collision_filter_manager
{
	SINGLETON(collision_filter_manager)

private:
	collision_filter_manager();
	~collision_filter_manager();

public:
	bool register_filter(collision_filter_ptr& filter);
	bool unregister_filter(collision_filter_ptr& filter);
	bool unregister_filter(std::string_view name);
	collision_filter_ptr search_filter(std::string_view name);

private:
	std::unordered_map<std::string, collision_filter_ptr> filters;
};
