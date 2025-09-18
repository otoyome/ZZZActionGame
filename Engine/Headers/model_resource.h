#pragma once

#define	NOMINMAX
#include "texture.h"
#include "material.h"
#include "pmx_model.h"
#include "animation_resource.h"
#include "collision/capsule.h"
#include "collision/box.h"

//	モデルリソース
class model_resource
{
public:
	static constexpr size_t invalid_id = -1;

	struct node
	{
		std::string name;
		size_t parent_index = invalid_id;

		DirectX::XMFLOAT3 scale{ 1, 1, 1 };
		DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 };
		DirectX::XMFLOAT3 translation{ 0, 0, 0 };
		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct bone
	{
		size_t node_index;
		DirectX::XMFLOAT4X4 offset_matrix{1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1};
		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct mesh
	{
		std::vector<bone> bones;
		size_t material_index = invalid_id;
		size_t node_index = invalid_id;

		std::unordered_map<semantic, buffer_view> vertex_buffer_views;
		buffer_view index_buffer_view;

		DirectX::XMFLOAT3 bounds_min;
		DirectX::XMFLOAT3 bounds_max;

		template<class Archive>
		void serialize(Archive& archive, int version);
	};

	struct capsule_shadow
	{
		size_t node_index;
		DirectX::XMFLOAT4 rotation;
		DirectX::XMFLOAT3 center;
		float radius;
		float length;
		template<class Archive>
		void serialize(Archive& archive, int version);
	};

public:
	inline std::string_view get_file_path() const {return file_path;}
	inline const std::vector<material>& get_materials() const {return materials;}
	inline const std::vector<node>& get_nodes() const {return nodes;}
	inline const std::vector<mesh>& get_meshes() const {return  meshes;}
	inline const std::vector<animation::data>& get_animations() const {return animations;}
	inline const std::vector<capsule_shadow>& get_capsule_shadows() const {return capsule_shadows;}

#if defined(_DEBUG)
	inline std::vector<material>& get_materials_for_debug() { return materials; }
	inline std::vector<animation::data>& get_animations_for_debug() { return animations; }
#endif	//	defined(_DEBUG)

public:
	model_resource();
	model_resource(std::string_view file_path);
	~model_resource();

	bool compute_animation_nodes(std::vector<animation::node>& nodes, size_t animation_index, float time) const;
	bool compute_animation_node(animation::node& node, size_t animation_index, size_t node_index, float time) const;
	size_t get_animation_index_from_name(std::string_view name) const;

public:
	//	pmx_modelからこのモデル形式に変換
	bool convert_from_pmx(pmx_model& model);
	bool add_animation_from_pmx(std::string_view animation_name, pmx_model& model);

	bool serialize(std::string_view file_path);
	bool deserialize(std::string_view file_path);

	void paste_materials(const std::vector<material>& materials);

private:
	void build_model();

private:
	std::string file_path;

	std::vector<material> materials;
	std::vector<node> nodes;
	std::vector<mesh> meshes;
	std::vector<animation::data> animations;
	std::vector<capsule_shadow> capsule_shadows;
};

using model_resource_ptr	= std::shared_ptr<model_resource>;

class model_resource_manageer
{
	SINGLETON(model_resource_manageer)

private:
	model_resource_manageer();
	~model_resource_manageer();

public:
	bool load_model(model_resource_ptr& resource, std::string_view file_path);

};
