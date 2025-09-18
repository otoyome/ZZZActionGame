#pragma once

#define	NOMINMAX
#include <type_traits>

#include "texture.h"
#include "material.h"
#include "pmx_model.h"
#include "collision/capsule.h"
#include "collision/box.h"

namespace animation
{
	struct node
	{
		float time;
		DirectX::XMFLOAT3 scale{ 1, 1, 1 };
		DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 };
		DirectX::XMFLOAT3 translation{ 0, 0, 0 };
		template<class Archive>
		void serialize(Archive& archive, int version)
		{
			archive(CEREAL_NVP(time),
				CEREAL_NVP(scale),
				CEREAL_NVP(rotation),
				CEREAL_NVP(translation));
		}
	};

	struct camera_spherical
	{
		float time;
		float distance;
		float view_angle;
		unsigned char is_parth;
		DirectX::XMFLOAT3 focus{ 0, 0, 0 };
		DirectX::XMFLOAT3 rotation{ 0, 0, 0 };
		template<class Archive>
		void serialize(Archive& archive, int version)
		{
			archive(CEREAL_NVP(time),
				CEREAL_NVP(distance),
				CEREAL_NVP(view_angle),
				CEREAL_NVP(is_parth),
				CEREAL_NVP(focus),
				CEREAL_NVP(rotation));
		}
	};

	struct event
	{
		enum class variable_type
		{
			None,
			Int,
			Float,
			String,
		};
		template<typename Type> void get_variable(Type& value) const {}
		template<> void get_variable<int>(int& value) const
		{
			if (type != variable_type::Int)
				return;

			value = std::atoi(variable.data());
		}
		template<> void get_variable<float>(float& value) const
		{
			if (type != variable_type::Float)
				return;

			value = static_cast<float>(std::atof(variable.data()));
		}
		template<> void get_variable<std::string>(std::string& value) const
		{
			if (type != variable_type::String)
				return;

			value = variable;
		}

		float time;
		variable_type type;
		std::string function_name;
		std::string variable;
		template<class Archive>
		void serialize(Archive& archive, int version)
		{
			archive(CEREAL_NVP(time),
				CEREAL_NVP(type),
				CEREAL_NVP(function_name),
				CEREAL_NVP(variable));
		}
	};

	struct variable
	{
		float time;
		std::vector<BYTE> buffer;
		template<typename Type>
		void set_data(const Type& data)
		{
			static_assert(std::is_pointer<Type>());
			static_assert(std::is_array<Type>());

			buffer.resize(sizeof(Type));
			*static_cast<Type*>(buffer.data()) = data;
		}

		template<typename Type>
		bool get_data(Type& data)
		{
			static_assert(std::is_pointer<Type>());
			static_assert(std::is_array<Type>());

			data = *static_cast<Type*>(buffer.data());
		}
		template<class Archive>
		void serialize(Archive& archive, int version)
		{
			archive(CEREAL_NVP(time), CEREAL_NVP(buffer));
		}
	};

	struct data
	{
		std::string name;
		float end_time;
		bool root_motion_xz{ false };
		bool root_motion_y{ false };
		bool root_rotation{ false };
		std::string root_motion_node_name;
		std::vector<std::vector<node>> nodes;
		std::vector<camera_spherical> camera_sphericals;
		std::vector<event> events;

		std::unordered_map<std::string, std::vector<variable>> variables;	//	あんまりよくはない

		bool compute_animation(std::vector<animation::node>& outs, float time) const;
		bool compute_animation(animation::node& out, size_t node_index, float time) const;

		bool compute_animation(animation::camera_spherical& out, float time) const;

		using variable_function = std::function<void(float, std::string_view, const animation::variable&, const animation::variable&)>;
		bool compute_animation(float time, std::string_view identifier, variable_function function) const;
		bool compute_animation(float time, variable_function function) const;

		bool call_events(DirectX::XMFLOAT2 time_range, std::function<void(const event& event)> function) const;

		template<class Archive>
		void serialize(Archive& archive, int version)
		{
			switch (version)
			{
				case 0:
				case 1:
				{
					archive(CEREAL_NVP(name),
						CEREAL_NVP(end_time),
						CEREAL_NVP(root_motion_xz),
						CEREAL_NVP(root_motion_y),
						CEREAL_NVP(root_rotation),
						CEREAL_NVP(root_motion_node_name),
						CEREAL_NVP(nodes));
					break;
				}
				case 2:
				{
					archive(CEREAL_NVP(name),
						CEREAL_NVP(end_time),
						CEREAL_NVP(root_motion_xz),
						CEREAL_NVP(root_motion_y),
						CEREAL_NVP(root_rotation),
						CEREAL_NVP(root_motion_node_name),
						CEREAL_NVP(nodes),
						CEREAL_NVP(camera_sphericals));
					break;
				}
				case 3:
				{
					archive(CEREAL_NVP(name),
						CEREAL_NVP(end_time),
						CEREAL_NVP(root_motion_xz),
						CEREAL_NVP(root_motion_y),
						CEREAL_NVP(root_rotation),
						CEREAL_NVP(root_motion_node_name),
						CEREAL_NVP(nodes),
						CEREAL_NVP(camera_sphericals),
						CEREAL_NVP(events));
					break;
				}
				case 4:
				{
					archive(CEREAL_NVP(name),
						CEREAL_NVP(end_time),
						CEREAL_NVP(root_motion_xz),
						CEREAL_NVP(root_motion_y),
						CEREAL_NVP(root_rotation),
						CEREAL_NVP(root_motion_node_name),
						CEREAL_NVP(nodes),
						CEREAL_NVP(camera_sphericals),
						CEREAL_NVP(events),
						CEREAL_NVP(variables));
					break;
				}
			}
		}
	};
}	//	namespace animation

//	アニメーションリソース
class animation_resource
{
public:
	static constexpr size_t invalid_id = -1;

public:
	inline std::string_view get_file_path() const {return file_path;}
	inline const std::vector<animation::data>& get_animation() const {return animations;}

	size_t get_index_from_name(std::string_view name) const;

#if defined(_DEBUG)
	inline std::vector<animation::data>& get_animations_for_debug() { return animations; }
#endif	//	defined(_DEBUG)

public:
	animation_resource();
	~animation_resource();

	bool convert_from_vmd(std::string_view animation_name, const vmd_loader& loader);

	bool add_animation_data(const animation::data& data);

	bool serialize(std::string_view file_path);
	bool deserialize(std::string_view file_path);

private:
	std::string file_path;

	std::vector<animation::data> animations;
};

