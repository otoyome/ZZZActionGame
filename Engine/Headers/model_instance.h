#pragma once

#include "model_resource.h"
#include "collision/frustum_culling.h"

#include "collision/sphere.h"
#include "collision/capsule.h"
#include "collision/box.h"

//	モデルインスタンス
class model_instance
{
public:
	static constexpr size_t invalid_id = -1;

	enum class flags
	{
		active,						//	表示フラグ
		color_render,				//	色描画
		shadow_caster,				//	影生成
		culling,					//	カリング
		culling_target_mesh,		//	メッシュ単位でカリング
		self_shadow_off,			//	セルフシャドウオフ
		use_capsule_shadow,			//	カプセルシャドウ使用

		debug_draw_culling_object,	//	カリングオブジェクトの表示フラグ
	};

	struct node
	{
		std::string name;
		size_t parent_index = invalid_id;

		DirectX::XMFLOAT4X4 local_transform{1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1};
		DirectX::XMFLOAT4X4 global_transform{1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1};
		DirectX::XMFLOAT4X4 world_transform{1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1};
	};

	using node_animation = animation::node;

public:
	bool is_flag(flags flag) const { return system_utility::is_bitflag(system_flags, 1 << static_cast<UINT>(flag)); }
	void set_flag(flags flag, bool value) { system_utility::set_bitflag(system_flags, 1 << static_cast<UINT>(flag), value); }

	inline model_resource_ptr get_resource() const { return resource; }
	inline const std::vector<node>& get_nodes() const { return nodes; }
	inline std::vector<node>& get_nodes(){ return nodes; }
	inline const std::vector<material>& get_materials() const { return materials; }
	inline std::vector<material>& get_materials(){ return materials; }

#if defined(USE_IMGUI)
	inline void set_materials_for_editor(const std::vector<material>& materials) { this->materials = materials; }
#endif	//	defined(USE_IMGUI)

	size_t get_node_index(std::string_view node_name) const;

public:
	model_instance();
	model_instance(model_resource_ptr resource);
	~model_instance();

	bool compute_animation_nodes(std::vector<node_animation>& nodes, size_t animation_index, float time) const;
	bool compute_animation_node(node_animation& node, size_t animation_index, size_t node_index, float time) const;
	void set_node_poses(const std::vector<node_animation>& nodes);
	void set_node_pose(size_t node_index, const node_animation& node);
	void update_transform(const DirectX::XMFLOAT4X4& world_transform, float random_offset_power = 0.0f);
	void update_transform(const DirectX::XMMATRIX& world_transform, float random_offset_power = 0.0f);

private:
	void build_model();

private:
	UINT system_flags;
	model_resource_ptr resource;

	std::vector<material> materials;
	std::vector<node> nodes;
};

using model_instance_ptr = std::shared_ptr<model_instance>;

//	モデルインスタンスマネージャー
class model_instance_maanger
{
	SINGLETON(model_instance_maanger)

	using	model_instances	= std::vector<std::weak_ptr<model_instance>>;

public:
	inline const model_instances& get_instances() const { return instances; }

#if defined(_DEBUG)
	size_t get_draw_mesh_count() const { return draw_mesh_count; }
	size_t get_draw_shadow_mesh_count() const { return draw_shadow_mesh_count; }
	size_t get_max_draw_mesh_count() const { return max_draw_mesh_count; }
	bool get_draw_culling_mesh_aabb() const {return draw_culling_mesh_aabb;}
	void set_draw_culling_mesh_aabb(bool flag){draw_culling_mesh_aabb = flag;}
#endif	//	defined(_DEBUG)

private:
	model_instance_maanger();
	~model_instance_maanger();

public:
	void registered(std::weak_ptr<model_instance> instance);
	void get_container(mesh_container& container, const collision::frustum_culling& frustum_culling);

private:
	model_instances instances;
#if defined(_DEBUG)
	size_t draw_mesh_count = 0;
	size_t draw_shadow_mesh_count = 0;
	size_t max_draw_mesh_count = 0;
	bool draw_culling_mesh_aabb = false;
#endif	//	defined(_DEBUG)
};
