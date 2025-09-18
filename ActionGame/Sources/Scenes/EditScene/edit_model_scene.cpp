#include "scenes/editscene/edit_model_scene.h"

#include "object/debug/debug_menu.h"

#include "model_instance.h"
#include "camera.h"

#include "object/light_object.h"

#include "object/model_object.h"

#include "object/character/person/boss_seraphim_object.h"
#include "object/character/person/kuroko_object.h"

#include "object/character/player/player_controller_object.h"
#include "object/character/player/player_camera_controller_object.h"

#include "object/cinematics/cinematics_object.h"

#if defined(USE_IMGUI)
namespace
{
	const char* domain_texts[] =
	{
		"surface",		//	モデル
		"postprocess",	//	ポストプロセス
	};

	const char* blend_mode_texts[] =
	{
		"opaque",		//	不透明描画
		"mask",			//	不透明描画+アルファマスク
	};

	const char* material_parameter_type_texts[] =
	{
		"Unknown",
		"Int",
		"Int2",
		"Int3",
		"Int4",
		"UInt",
		"UInt2",
		"UInt3",
		"UInt4",
		"Float",
		"Float2",
		"Float3",
		"Float4",
		"Float4x4",

		"Texture",
	};

	constexpr float edit_item_speed = 0.1f;

	template<typename Type>
	void set_parameter(std::string_view key_name, Type& value, material& material)
	{
	}

	template<> void set_parameter<int>(std::string_view key_name, int& value, material& material)
	{
		material.set_int(key_name, value);
	}
	template<> void set_parameter<DirectX::XMINT2>(std::string_view key_name, DirectX::XMINT2& value, material& material)
	{
		material.set_int2(key_name, value);
	}
	template<> void set_parameter<DirectX::XMINT3>(std::string_view key_name, DirectX::XMINT3& value, material& material)
	{
		material.set_int3(key_name, value);
	}
	template<> void set_parameter<DirectX::XMINT4>(std::string_view key_name, DirectX::XMINT4& value, material& material)
	{
		material.set_int4(key_name, value);
	}

	template<> void set_parameter<UINT>(std::string_view key_name, UINT& value, material& material)
	{
		material.set_uint(key_name, value);
	}
	template<> void set_parameter<DirectX::XMUINT2>(std::string_view key_name, DirectX::XMUINT2& value, material& material)
	{
		material.set_uint2(key_name, value);
	}
	template<> void set_parameter<DirectX::XMUINT3>(std::string_view key_name, DirectX::XMUINT3& value, material& material)
	{
		material.set_uint3(key_name, value);
	}
	template<> void set_parameter<DirectX::XMUINT4>(std::string_view key_name, DirectX::XMUINT4& value, material& material)
	{
		material.set_uint4(key_name, value);
	}

	template<> void set_parameter<float>(std::string_view key_name, float& value, material& material)
	{
		material.set_float(key_name, value);
	}
	template<> void set_parameter<DirectX::XMFLOAT2>(std::string_view key_name, DirectX::XMFLOAT2& value, material& material)
	{
		material.set_float2(key_name, value);
	}
	template<> void set_parameter<DirectX::XMFLOAT3>(std::string_view key_name, DirectX::XMFLOAT3& value, material& material)
	{
		material.set_float3(key_name, value);
	}
	template<> void set_parameter<DirectX::XMFLOAT4>(std::string_view key_name, DirectX::XMFLOAT4& value, material& material)
	{
		material.set_float4(key_name, value);
	}

	template<> void set_parameter<DirectX::XMFLOAT4X4>(std::string_view key_name, DirectX::XMFLOAT4X4& value, material& material)
	{
		material.set_float4x4(key_name, value);
	}

	void set_parameter(std::string_view key_name, material::parameter::type type, material::parameter::data* data, material& material)
	{
		switch (type)
		{
		case material::parameter::type::Int: set_parameter<int>(key_name, data->Int, material); break;
		case material::parameter::type::Int2: set_parameter<DirectX::XMINT2>(key_name, data->Int2, material); break;
		case material::parameter::type::Int3: set_parameter<DirectX::XMINT3>(key_name, data->Int3, material); break;
		case material::parameter::type::Int4: set_parameter<DirectX::XMINT4>(key_name, data->Int4, material); break;

		case material::parameter::type::UInt: set_parameter<UINT>(key_name, data->UInt, material); break;
		case material::parameter::type::UInt2: set_parameter<DirectX::XMUINT2>(key_name, data->UInt2, material); break;
		case material::parameter::type::UInt3: set_parameter<DirectX::XMUINT3>(key_name, data->UInt3, material); break;
		case material::parameter::type::UInt4: set_parameter<DirectX::XMUINT4>(key_name, data->UInt4, material); break;

		case material::parameter::type::Float: set_parameter<float>(key_name, data->Float, material); break;
		case material::parameter::type::Float2: set_parameter<DirectX::XMFLOAT2>(key_name, data->Float2, material); break;
		case material::parameter::type::Float3: set_parameter<DirectX::XMFLOAT3>(key_name, data->Float3, material); break;
		case material::parameter::type::Float4: set_parameter<DirectX::XMFLOAT4>(key_name, data->Float4, material); break;

		case material::parameter::type::Float4x4: set_parameter<DirectX::XMFLOAT4X4>(key_name, data->Float4x4, material); break;
		}
	}

	template<typename Type>
	bool edit_item(std::string_view key_name, material::parameter::data* data)
	{
		return false;
	}
	template<>bool edit_item<int>(std::string_view key_name, material::parameter::data* data)
	{
		return ImGui::DragInt(key_name.data(), &data->Int, edit_item_speed, -1000, 1000);
	}
	template<>bool edit_item<DirectX::XMINT2>(std::string_view key_name, material::parameter::data* data)
	{
		return ImGui::DragInt2(key_name.data(), &data->Int2.x, edit_item_speed, -1000, 1000);
	}
	template<>bool edit_item<DirectX::XMINT3>(std::string_view key_name, material::parameter::data* data)
	{
		return ImGui::DragInt3(key_name.data(), &data->Int3.x, edit_item_speed, -1000, 1000);
	}
	template<>bool edit_item<DirectX::XMINT4>(std::string_view key_name, material::parameter::data* data)
	{
		return ImGui::DragInt4(key_name.data(), &data->Int4.x, edit_item_speed, -1000, 1000);
	}

	template<>bool edit_item<UINT>(std::string_view key_name, material::parameter::data* data)
	{
		UINT d = data->UInt;
		int d2 = static_cast<int>(d);
		bool f = ImGui::DragInt(key_name.data(), &d2, edit_item_speed, 0, 2000);
		data->UInt = d2;
		return f;
	}
	template<>bool edit_item<DirectX::XMUINT2>(std::string_view key_name, material::parameter::data* data)
	{
		DirectX::XMINT2 d = { static_cast<int>(data->UInt2.x), static_cast<int>(data->UInt2.y) };
		bool f = ImGui::DragInt2(key_name.data(), &d.x, edit_item_speed, 0, 2000);
		data->UInt2 = { static_cast<UINT>(d.x), static_cast<UINT>(d.y) };
		return f;
	}
	template<>bool edit_item<DirectX::XMUINT3>(std::string_view key_name, material::parameter::data* data)
	{
		DirectX::XMINT3 d = { static_cast<int>(data->UInt3.x), static_cast<int>(data->UInt3.y), static_cast<int>(data->UInt3.z) };
		bool f = ImGui::DragInt3(key_name.data(), &d.x, edit_item_speed, 0, 2000);
		data->UInt3 = { static_cast<UINT>(d.x), static_cast<UINT>(d.y), static_cast<UINT>(d.z) };
		return f;
	}
	template<>bool edit_item<DirectX::XMUINT4>(std::string_view key_name, material::parameter::data* data)
	{
		DirectX::XMINT4 d = { static_cast<int>(data->UInt4.x), static_cast<int>(data->UInt4.y), static_cast<int>(data->UInt4.z), static_cast<int>(data->UInt4.w)};
		bool f = ImGui::DragInt4(key_name.data(), &d.x, edit_item_speed, 0, 2000);
		data->UInt4 = { static_cast<UINT>(d.x), static_cast<UINT>(d.y), static_cast<UINT>(d.z), static_cast<UINT>(d.w) };
		return f;
	}

	template<>bool edit_item<float>(std::string_view key_name, material::parameter::data* data)
	{
		return ImGui::DragFloat(key_name.data(), &data->Float, edit_item_speed, -1000, 1000);
	}
	template<>bool edit_item<DirectX::XMFLOAT2>(std::string_view key_name, material::parameter::data* data)
	{
		return ImGui::DragFloat2(key_name.data(), &data->Float2.x, edit_item_speed, -1000, 1000);
	}
	template<>bool edit_item<DirectX::XMFLOAT3>(std::string_view key_name, material::parameter::data* data)
	{
		return ImGui::DragFloat3(key_name.data(), &data->Float3.x, edit_item_speed, -1000, 1000);
	}
	template<>bool edit_item<DirectX::XMFLOAT4>(std::string_view key_name, material::parameter::data* data)
	{
		return ImGui::DragFloat4(key_name.data(), &data->Float4.x, edit_item_speed, -1000, 1000);
	}

	template<>bool edit_item<DirectX::XMFLOAT4X4>(std::string_view key_name, material::parameter::data* data)
	{
		bool f = false;
		std::string a = std::string(key_name) + "[0]";
		if (ImGui::DragFloat4(a.data(), data->Float4x4.m[0], edit_item_speed, -1000, 1000))
			f = true;
		a = std::string(key_name) + "[1]";
		if (ImGui::DragFloat4(a.data(), data->Float4x4.m[1], edit_item_speed, -1000, 1000))
			f = true;
		a = std::string(key_name) + "[2]";
		if (ImGui::DragFloat4(a.data(), data->Float4x4.m[2], edit_item_speed, -1000, 1000))
			f = true;
		a = std::string(key_name) + "[3]";
		if (ImGui::DragFloat4(a.data(), data->Float4x4.m[3], edit_item_speed, -1000, 1000))
			f = true;
		return f;
	}

	bool edit_item(std::string_view key_name, material::parameter::type type, material::parameter::data* data)
	{
		bool edit_flag = false;
		switch (type)
		{
		case material::parameter::type::Int: edit_flag = edit_item<int>(key_name, data); break;
		case material::parameter::type::Int2: edit_flag = edit_item<DirectX::XMINT2>(key_name, data); break;
		case material::parameter::type::Int3: edit_flag = edit_item<DirectX::XMINT3>(key_name, data); break;
		case material::parameter::type::Int4: edit_flag = edit_item<DirectX::XMINT4>(key_name, data); break;

		case material::parameter::type::UInt: edit_flag = edit_item<UINT>(key_name, data); break;
		case material::parameter::type::UInt2: edit_flag = edit_item<DirectX::XMUINT2>(key_name, data); break;
		case material::parameter::type::UInt3: edit_flag = edit_item<DirectX::XMUINT3>(key_name, data); break;
		case material::parameter::type::UInt4: edit_flag = edit_item<DirectX::XMUINT4>(key_name, data); break;

		case material::parameter::type::Float: edit_flag = edit_item<float>(key_name, data); break;
		case material::parameter::type::Float2: edit_flag = edit_item<DirectX::XMFLOAT2>(key_name, data); break;
		case material::parameter::type::Float3: edit_flag = edit_item<DirectX::XMFLOAT3>(key_name, data); break;
		case material::parameter::type::Float4: edit_flag = edit_item<DirectX::XMFLOAT4>(key_name, data); break;

		case material::parameter::type::Float4x4: edit_flag = edit_item<DirectX::XMFLOAT4X4>(key_name, data); break;
		}
		return edit_flag;
	}

	void add_material_parameter(std::string_view key_name, material& material, material::parameter::type type)
	{
		switch (type)
		{
		case material::parameter::type::Int: material.set_int(key_name, 0); break;
		case material::parameter::type::Int2: material.set_int2(key_name, {}); break;
		case material::parameter::type::Int3: material.set_int3(key_name, {}); break;
		case material::parameter::type::Int4: material.set_int4(key_name, {}); break;

		case material::parameter::type::UInt: material.set_uint(key_name, 0); break;
		case material::parameter::type::UInt2: material.set_uint2(key_name, {}); break;
		case material::parameter::type::UInt3: material.set_uint3(key_name, {}); break;
		case material::parameter::type::UInt4: material.set_uint4(key_name, {}); break;

		case material::parameter::type::Float: material.set_float(key_name, 0); break;
		case material::parameter::type::Float2: material.set_float2(key_name, {}); break;
		case material::parameter::type::Float3: material.set_float3(key_name, {}); break;
		case material::parameter::type::Float4: material.set_float4(key_name, {}); break;

		case material::parameter::type::Float4x4: material.set_float4x4(key_name, {}); break;
		}
	}

}

edit_model_scene::edit_model_scene()
: scene_base()
{
}

edit_model_scene::~edit_model_scene()
{
}

bool edit_model_scene::initialize_core()
{
	//	メインカメラ生成
	std::shared_ptr<camera_object> main_camera;
	{
		main_camera = std::make_shared<camera_object>("メインカメラ");
		register_object(main_camera);
		mouse::instance()->set_center_lock(false);
		mouse::instance()->set_cursol_hide(false);
	}

	//	演出オブジェクト生成
	std::shared_ptr<cinematics_object> main_cinematics;
	{
		main_cinematics = std::make_shared<cinematics_object>("演出管理オブジェクト", main_camera);
		register_object(main_cinematics);
	}

	//	ライト生成
	{
		//	平行光源生成
		{
			auto light = std::make_shared<directional_light_object>("平行光源");
			light->set_flag(light::flags::use_shadow, false);
			register_object(light);
		}

		//	間接光生成
		{
			auto sky_light = std::make_shared<sky_light_object>("スカイライト");
			texture_ptr ibl_texture;
			texture_manager::instance()->load_texture_from_file(ibl_texture, "Assets/Resources/hilly_terrain_01_puresky_4k/diffuse_iem.dds");
			sky_light->set_diffuse_ibl_texture(ibl_texture);
			texture_manager::instance()->load_texture_from_file(ibl_texture, "Assets/Resources/hilly_terrain_01_puresky_4k/specular_pmrem.dds");
			sky_light->set_specular_ibl_texture(ibl_texture);
			texture_manager::instance()->load_texture_from_file(ibl_texture, "Assets/Resources/lut_ggx.dds");
			sky_light->set_lut_ggx_texture(ibl_texture);
			register_object(sky_light);
		}
	}

	//	モデル生成
	{
		auto generate_model_objects = [&](std::string_view file_path)
		{
			std::filesystem::path path(file_path);
			if (!std::filesystem::exists(path))
			{
#if defined(USE_IMGUI)
				path.replace_extension("pmx");
				pmx_model model = pmx_model(render_data::instance()->get_device().Get(), path.string());
				//	ファイル保存
				std::shared_ptr<model_resource> mr = std::make_shared<model_resource>();
				mr->convert_from_pmx(model);
				mr->serialize(file_path);
				path.replace_extension("emo");
#endif // defined(USE_IMGUI)
			}
			return std::make_shared<model_object>(file_path, path.string());
		};
		edit_models.push_back(kuroko_object::generate_character_model_object());
		edit_models.push_back(kuroko_object::generate_weapon_model_object());
		edit_models.push_back(boss_seraphim_object::generate_character_model_object());
		edit_models.push_back(generate_model_objects("Assets/Resources/宮殿ステージ/宮殿ステージ.emo"));
		edit_models.push_back(generate_model_objects("Assets/Resources/バーダス・コロシアム/バーダス・コロシアム.emo"));
	//	edit_models.push_back(generate_model_objects("Assets/Resources/box/box.emo"));
		for (auto& model_object : edit_models)
		{
			model_object->set_active(false);
			register_object(model_object);
		}
		edit_model = edit_models[0];
		edit_model->set_active(true);
	}

#if defined(_DEBUG)
	//	デバッグカメラ生成
	{
		auto camera = std::make_shared<debug_camera>(main_camera);
		camera->set_position(system_units::convert_m_to_unit({0, 1, 3}));
		camera->set_rotation({0, DirectX::XM_PI, 0});
		register_object(camera);
	//	game_objects.emplace_back(std::make_shared<debug_menu>(camera));
	}
#endif	//	defined(_DEBUG)

	return true;
}

bool edit_model_scene::uninitialize_core()
{
	return true;
}

void edit_model_scene::update_core(float elapsed_time)
{
}

// Shift-JISからUTF-8へ変換する関数
std::string convert_sjis_to_utf8(std::string_view sjisStr)
{
	// Shift-JISからUTF-16へ変換
	int utf16Size = MultiByteToWideChar(CP_ACP, 0, sjisStr.data(), -1, NULL, 0);
	std::vector<wchar_t> utf16Str(utf16Size);
	MultiByteToWideChar(CP_ACP, 0, sjisStr.data(), -1, utf16Str.data(), utf16Size);

	// UTF-16からUTF-8へ変換
	int utf8Size = WideCharToMultiByte(CP_UTF8, 0, utf16Str.data(), -1, NULL, 0, NULL, NULL);
	std::vector<char> utf8Str(utf8Size);
	WideCharToMultiByte(CP_UTF8, 0, utf16Str.data(), -1, utf8Str.data(), utf8Size, NULL, NULL);

	return std::string(utf8Str.data());
}

bool edit_model_scene::imgui_all_item_changer()
{
	bool flag = false;
	int ididid = 0x87435184;
	ImGui::PushID(ididid++);

	ImGui::Text("all material add");
	if (ImGui::InputText("variable name", all_variable_name_texts, MAX_PATH))
		flag = true;
	if (ImGui::Combo("variable type", &all_variable_type, material_parameter_type_texts, ARRAYSIZE(material_parameter_type_texts)))
		flag = true;

	if (ImGui::Button("add"))
	{
		for (auto& material : edit_model->get_model_instance()->get_materials())
		{
			add_material_parameter(all_variable_name_texts, material, static_cast<material::parameter::type>(all_variable_type));
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("delete"))
	{
		for (auto& material : edit_model->get_model_instance()->get_materials())
		{
			material.remove_parameter(all_variable_name_texts);
		}
		flag = true;
	}
	ImGui::Separator();

	edit_item(all_variable_name_texts, static_cast<material::parameter::type>(all_variable_type), &all_edit_item);
	if (ImGui::Button("apply parameter"))
	{
		for (auto& material : edit_model->get_model_instance()->get_materials())
		{
			set_parameter(all_variable_name_texts, static_cast<material::parameter::type>(all_variable_type), &all_edit_item, material);
		}
		flag = true;
	}
	ImGui::Separator();
	ImGui::PopID();

	return flag;
}

void edit_model_scene::imgui_render_core()
{
	if (ImGui::TreeNode("edit_model_scene"))
	{
		//	モデル選択
		if (ImGui::TreeNode("select model"))
		{
			for (auto& model_object : edit_models)
			{
				if (ImGui::Button(convert_sjis_to_utf8(model_object->get_name()).data()))
				{
					edit_model->set_active(false);
					edit_model = model_object;
					edit_model->set_active(true);
				}
			}
			ImGui::TreePop();
			ImGui::Separator();
		}

		//	モデル保存
		if (ImGui::Button("Save"))
		{
			edit_model->get_model_instance()->get_resource()->serialize(edit_model->get_model_instance()->get_resource()->get_file_path());
		}

		//	マテリアル編集
		bool edit_flag = false;
		if (ImGui::TreeNode("materials"))
		{
			if (imgui_all_item_changer())
				edit_flag = true;

			int ididid = 0x87435184;
			for (auto& material : edit_model->get_model_instance()->get_materials())
			{
				ImGui::PushID(ididid++);

				if (ImGui::TreeNode(convert_sjis_to_utf8(material.get_name()).data()))
				{
					float alpha_clip_threshold = material.get_alpha_clip_threshold();
					if (ImGui::SliderFloat("alpha_clip_threshold", &alpha_clip_threshold, 0.0f, 1.0f))
					{
						material.set_alpha_clip_threshold(alpha_clip_threshold);
						edit_flag = true;
					}

					//	物理マテリアル
					if (ImGui::TreeNode("physical material"))
					{
						material::physical_datas physical_data = material.get_physical_data();

						if (ImGui::DragFloat("friction", &physical_data.friction, 0.0f, 1000.0f))
						{
							edit_flag = true;
						}

						material.set_physical_data(physical_data);
						ImGui::TreePop();
					}

					//	マテリアルフラグ
					if (ImGui::TreeNode("flags"))
					{
						for (auto data : magic_enum::enum_values<material::flags>())
						{
							bool flag = material.is_flag(data);
							if (ImGui::Checkbox(magic_enum::enum_name(data).data(), &flag))
							{
								material.set_flag(data, flag);
								edit_flag = true;
							}
						}
						ImGui::TreePop();
					}

					//	指定のマテリアルに変数を追加
					ImGui::InputText("variable name", variable_name_texts, MAX_PATH);
					ImGui::Combo("variable type", &variable_type, material_parameter_type_texts, ARRAYSIZE(material_parameter_type_texts));

					if (ImGui::Button("add variable"))
					{
						add_material_parameter(variable_name_texts, material, static_cast<material::parameter::type>(variable_type));
						edit_flag = true;
					}
					ImGui::Separator();

					//	パラメーター追加
					std::string_view delete_key;
					for (auto& parameter : material.get_parameters())
					{
						ImGui::PushID(ididid++);
						ImGui::Text(convert_sjis_to_utf8(parameter.first).data());
						ImGui::SameLine();
						if (ImGui::Button("Delete"))
						{
							delete_key = parameter.first;
							edit_flag = true;
						}
						if (edit_item(parameter.first, parameter.second.Type, &parameter.second.Data))
							edit_flag = true;
						ImGui::Separator();
						ImGui::PopID();
					}

					//	パラメーター破棄
					material.remove_parameter(delete_key);

					ImGui::TreePop();
				}

				ImGui::PopID();
			}
			if (edit_flag)
			{
				edit_model->get_model_instance()->get_resource()->paste_materials(edit_model->get_model_instance()->get_materials());
			}

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
}
#endif	//	defined(USE_IMGUI)
