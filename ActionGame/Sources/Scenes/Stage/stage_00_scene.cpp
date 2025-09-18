#include "Scenes/Stage/stage_00_scene.h"

#include "editor/editor_camera.h"

#include "model_instance.h"
#include "camera.h"

#include "object/light_object.h"
#include "object/model_object.h"
#include "object/stage_object.h"

#include "object/decal_object.h"

stage_00_scene::stage_00_scene()
: scene_base()
{
}

stage_00_scene::~stage_00_scene()
{
}

bool stage_00_scene::initialize_core()
{
	//	平行光源生成
	register_object(std::make_shared<directional_light_object>("平行光源"));

	//	その他光源設定
	if (0)
	{
		auto point = std::make_shared<point_light_object>("点光源");
		point->set_flag(light::flags::use_shadow, false);
		point->set_position(system_units::convert_m_to_unit({ 0, 5.0f, 0 }));
		point->set_range(system_units::convert_m_to_unit(10.0f));
		point->set_color({ 1.0f, 1.0f, 1.0f, 100.0f });
		register_object(point);

		auto spot = std::make_shared<spot_light_object>("スポットライト");
		spot->set_flag(light::flags::use_shadow, true);
		spot->set_position(system_units::convert_m_to_unit({ 0, 5.0f, -10 }));
		spot->set_rotation({DirectX::XM_PI * 0.5f, 0, 0});
		spot->set_range(system_units::convert_m_to_unit(20.0f));
		spot->set_inner_cone(30.0f);
		spot->set_outer_cone(60.0f);
		spot->set_color({ 1.0f, 1.0f, 1.0f, 10.0f });
		register_object(spot);
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

	//	モデル
	//	モデル関係は当たり判定の事もあるので原点ベースで作成しておく
	{
		auto file_path = "Assets/Resources/宮殿ステージ/宮殿ステージ.emo";
		UINT separate_level = 6;
		std::filesystem::path path(file_path);
		if (!std::filesystem::exists(path))
		{
			//	変換済みファイルが無いのでpmxをemoに変換
			path.replace_extension("pmx");
			pmx_model model = pmx_model(render_data::instance()->get_device().Get(), path.string());
			std::shared_ptr<model_resource> mr = std::make_shared<model_resource>();
			mr->convert_from_pmx(model);
			//	ファイル保存
			mr->serialize(file_path);
			path.replace_extension("emo");
		}

		//	モデル準備
		{
			auto model = std::make_shared<model_object>("ステージ", path.string());
			model->get_model_instance()->set_flag(model_instance::flags::culling_target_mesh, true);
			model->get_model_instance()->set_flag(model_instance::flags::shadow_caster, false);
			register_object(model);

			auto stage = std::make_shared<stage_object>("ステージ制御", model);
			register_object(stage);

			//	メッシュ分当たり判定生成
			DirectX::XMMATRIX transform;
			transform = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&stage->get_scale()))
					  * DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&stage->get_rotation().convert_quaternion()))
					  * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&stage->get_position()));
			generate_model_collision(model->get_model_instance()->get_resource(), transform, separate_level);
		}

		//	透明の当たり判定オブジェクト
		//	ただし、本来は別シーンで設置できるようにしておくと管理がしやすくなる
		{
			{
				auto stage = std::make_shared<collision_object>();
				stage->set_bounding_object(std::make_shared<collision::bounding_box>());
			#if defined(USE_IMGUI)
				stage->set_name("ステージ Collision Left Wall");
			#endif	//	defined(USE_IMGUI)
				stage->set_filter("stage");
				stage->set_local_scale({80, 100, 1000});
				stage->set_position({-140, 75, 0});
				stage->set_flag(collision_object::flags::static_object, true);
				register_object(stage);
			}
			{
				auto stage = std::make_shared<collision_object>();
				stage->set_bounding_object(std::make_shared<collision::bounding_box>());
			#if defined(USE_IMGUI)
				stage->set_name("ステージ Collision Right Wall");
			#endif	//	defined(USE_IMGUI)
				stage->set_filter("stage");
				stage->set_local_scale({80, 100, 1000});
				stage->set_position({+140, 75, 0});
				stage->set_flag(collision_object::flags::static_object, true);
				register_object(stage);
			}
		}
	}

	return true;
}

bool stage_00_scene::uninitialize_core()
{
	return true;
}

void stage_00_scene::update_core(float elapsed_time)
{
}

#if defined(USE_IMGUI)
void stage_00_scene::imgui_render_core()
{
	if (ImGui::TreeNode("stage_00_scene"))
	{
		ImGui::TreePop();
	}
}
#endif	//	defined(USE_IMGUI)

void stage_00_scene::generate_model_collision(model_resource_ptr resource, DirectX::XMMATRIX& transform, UINT separate_level)
{
	std::string file_path = std::string(resource->get_file_path());
	std::filesystem::path collision_mesh_path(file_path);

	for (size_t mesh_index = 0; mesh_index < resource->get_meshes().size(); ++mesh_index)
	{
		std::shared_ptr<collision::bounding_mesh> mesh = std::make_shared<collision::bounding_mesh>();

		//	シリアライズ済みファイルがあるか確認
		std::string extension = std::to_string(mesh_index) + ".ebmo";
		collision_mesh_path = file_path;
		collision_mesh_path.replace_extension(extension);
		if (!(std::filesystem::exists(collision_mesh_path) &&
			mesh->deserialize(collision_mesh_path.string())))
		{
			collision::generate_bounding_mesh(*mesh, resource, mesh_index, transform, true, separate_level);
			mesh->serialize(collision_mesh_path.string());
		}
		collision_object_ptr collision = std::make_shared<collision_object>();
#if defined(USE_IMGUI)
		collision->set_name(resource->get_materials()[resource->get_meshes()[mesh_index].material_index].get_name());
#endif	//	defined(USE_IMGUI)
		collision->set_bounding_object(mesh);
		collision->set_filter("stage");
		collision->set_flag(collision_object::flags::static_object, true);
		collision->set_physical_data(resource->get_materials()[mesh_index].get_physical_data());
		register_object(collision);
	}
}