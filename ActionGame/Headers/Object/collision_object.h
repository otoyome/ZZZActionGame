#pragma once

#include "collision/collision.h"
#include "collision_filter.h"

#include <mutex>

//	当たり判定オブジェクト
class collision_object : public std::enable_shared_from_this<collision_object>
{
public:
	static constexpr size_t invalid_id = 0;

public:
	enum class flags
	{
		active,
		detect_only, // 検知のみフラグ
		static_object, // 衝突検知後、押し合いする時に動かないオブジェクトフラグ

		dirty,
		frame_hit, // 1フレーム内に1回当たったかどうかを判定

		draw_line, // デバッグ時にラインを描画するかどうか
	};

	//	ユーザー情報
	struct user_datas
	{
		std::string identifier; // 識別子
		std::vector<BYTE> buffer; // データバッファ
		template<typename type>
		const type* get_data() const {return reinterpret_cast<const type*>(buffer.data());}
		template<typename type>
		type* get_data(){ return reinterpret_cast<type*>(buffer.data()); }
	};

	//	判定情報
	struct data
	{
		std::shared_ptr<collision_object> from;
		std::shared_ptr<collision_object> to;
		collision::intersect_data intersect_data; // 交差情報
	};

	using on_collision_enter = std::function<void(const data& data)>;
	using on_collision_stay = std::function<void(const data& data)>;
	using on_collision_end = std::function<void(const data& data)>;

public:
	collision_filter_ptr get_filter() const;
	void set_filter(std::string_view name);

	bool is_flag(flags flag) const { return system_utility::is_bitflag(flag_data, 1 << static_cast<UINT>(flag)); }
	void set_flag(flags flag, bool value) { system_utility::set_bitflag(flag_data, 1 << static_cast<UINT>(flag), value); }

	user_datas get_user_data() const;
	void set_user_data(const user_datas& user_data);

	float get_mass() const;
	void set_mass(float mass);

	const material::physical_datas& get_physical_data() const;
	void set_physical_data(const material::physical_datas& data);

	std::shared_ptr<collision::bounding_object> get_bounding_object();
	void set_bounding_object(std::shared_ptr<collision::bounding_object> object);

	const math::rotator& get_rotation() const;
	void set_rotation(const math::rotator& rotation);

	const DirectX::XMFLOAT3& get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);

	const DirectX::XMFLOAT3& get_local_scale() const;
	void set_local_scale(const DirectX::XMFLOAT3& scale);

	const math::rotator& get_local_rotation() const;
	void set_local_rotation(const math::rotator& rotation);

	const DirectX::XMFLOAT3& get_local_offset() const;
	void set_local_offset(const DirectX::XMFLOAT3& offset);

	void set_draw_normal_color(DirectX::XMFLOAT4 color);
	void set_draw_hit_color(DirectX::XMFLOAT4 color);

public:
	collision_object();
	~collision_object();

	void pre_intersect();
	bool intersect(std::shared_ptr<collision_object>& object, collision::intersect_data& intersect_data);
	void solve_pushing_objects(collision::intersect_data& intersect_data);
	void update_hit_objects(collision::intersect_data& intersect_data, std::weak_ptr<collision_object> to);
	void post_intersect();

	void set_collision_enter(on_collision_enter collision_enter);
	void set_collision_stay(on_collision_stay collision_stay);
	void set_collision_end(on_collision_end collision_end);

	void debug_draw();

#if defined(USE_IMGUI)
	void imgui_render();
#endif	//	defined(USE_IMGUI)

private:
	void update();

#if defined(USE_IMGUI)
	std::string name;
	bool draw_mesh_color = false;
	INT draw_mesh_collision_x_index = -1;
	INT draw_mesh_collision_y_index = -1;
	INT draw_mesh_collision_z_index = -1;

public:
	std::string_view get_name() const {return name;}
	void set_name(std::string_view name){this->name = std::string(name);}
#endif	//	defined(USE_IMGUI)

private:
	size_t id;
	collision_filter_ptr filter;
	int flag_data;
	user_datas user_data;
	float mass; // 本体の質量
	material::physical_datas physical_data; // 物理マテリアル
	DirectX::XMFLOAT3 local_scale; // ローカルスケール情報
	math::rotator local_rotation; // ローカル回転
	DirectX::XMFLOAT3 local_offset; // ローカルオフセット

	math::rotator rotation;
	DirectX::XMFLOAT3 position;

	DirectX::XMFLOAT4 draw_normal_color;
	DirectX::XMFLOAT4 draw_hit_color;

	std::shared_ptr<collision::bounding_object> bounding_object;

	on_collision_enter collision_enter;
	on_collision_stay collision_stay;
	on_collision_end collision_end;

	struct hit_object
	{
		enum class flags
		{
			added,
			aleady,
		};

		UINT flag_data = true;
		std::weak_ptr<collision_object> to;
		collision::intersect_data intersect_data;

		hit_object(std::weak_ptr<collision_object> to)
		: flag_data(0)
		, to(to)
		, intersect_data()
		{
			set_flag(flags::added, true);
			set_flag(flags::aleady, false);
		}

		bool is_flag(flags flag) const { return system_utility::is_bitflag(flag_data, 1 << static_cast<UINT>(flag)); }
		void set_flag(flags flag, bool value) { system_utility::set_bitflag(flag_data, 1 << static_cast<UINT>(flag), value); }
	};
	std::vector<hit_object> hit_objects;

	std::mutex mutex;
	inline static size_t id_counter;
};
