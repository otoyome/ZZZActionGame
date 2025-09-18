#include "object/character/person/boss_seraphim_object.h"

#include "collision/capsule.h"

namespace setup_data
{
	constexpr std::string_view character_model_file_path = "Assets/Resources/Seraphim/Seraphim.emo";
	constexpr std::string_view character_model_object_name = "セラフィム モデル";
	constexpr std::string_view model_control_object_name = "セラフィム 制御";
}

std::shared_ptr<model_object> boss_seraphim_object::generate_character_model_object()
{
	//	変換済みファイルがあるか？
	std::filesystem::path path(setup_data::character_model_file_path);
	if (!std::filesystem::exists(path))
	{
#if defined(USE_IMGUI)
		//	無いので変換
		path.replace_extension("pmx");
		pmx_model model = pmx_model(render_data::instance()->get_device().Get(), path.string());
		model.add_vmd_animation("Idle", "Assets/Resources/Seraphim/Animation/Idle.vmd");

		model.add_vmd_animation("Walking", "Assets/Resources/Seraphim/Animation/Idle.vmd");
		model.add_vmd_animation("BackWalking", "Assets/Resources/Seraphim/Animation/Idle.vmd");
		model.add_vmd_animation("RunningNormal", "Assets/Resources/Seraphim/Animation/Idle.vmd");
		model.add_vmd_animation("RunningFast", "Assets/Resources/Seraphim/Animation/Idle.vmd");
		model.add_vmd_animation("RunStop", "Assets/Resources/Seraphim/Animation/RunStop.vmd");

		model.add_vmd_animation("Attack1", "Assets/Resources/Seraphim/Animation/Attack1.vmd");
		model.add_vmd_animation("Attack2", "Assets/Resources/Seraphim/Animation/Attack2.vmd");
		model.add_vmd_animation("Attack3", "Assets/Resources/Seraphim/Animation/Attack3.vmd");
		model.add_vmd_animation("Attack4", "Assets/Resources/Seraphim/Animation/Attack4.vmd");
		std::shared_ptr<model_resource> mr = std::make_shared<model_resource>();
		mr->convert_from_pmx(model);
		//	ルートモーション情報設定
		{
			size_t index;
			index = mr->get_animation_index_from_name("Attack1");
			auto& animations = mr->get_animations_for_debug();
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
		}
		//	ファイル保存
		mr->serialize(setup_data::character_model_file_path);
		path.replace_extension("emo");
#endif // defined(USE_IMGUI)
	}

#if 0 // defined(USE_IMGUI)
	{
		model_resource_ptr resource;
		if (model_resource_manageer::instance()->load_model(resource, path.string()))
		{
			path.replace_extension("pmx");
			pmx_model model = pmx_model(render_data::instance()->get_device().Get(), path.string());

			bool save_flag = false;
			auto& animations = resource->get_animations_for_debug();
			auto add_animation = [&](std::string_view animation_name, std::string_view animation_path, bool overrwrite, bool& rewrite_flag)
			{
				rewrite_flag = false;
				size_t index = resource->get_animation_index_from_name(animation_name);
				if (overrwrite || index == model_resource::invalid_id)
				{
					index = model_resource::invalid_id;
					if (model.add_vmd_animation(animation_name.data(), animation_path.data()))
					{
						if (resource->add_animation_from_pmx(animation_name, model))
						{
							save_flag = true;
							rewrite_flag = true;
							index = resource->get_animation_index_from_name(animation_name);
						}
					}
				}
				return index;
			};

			//	それぞれモーションを読み込んで保存
			bool rewrite_flag;
			size_t index;
			index = add_animation("Walking", "Assets/Resources/Seraphim/Animation/Idle.vmd", false, rewrite_flag);
			index = add_animation("BackWalking", "Assets/Resources/Seraphim/Animation/Idle.vmd", false, rewrite_flag);
			index = add_animation("RunningNormal", "Assets/Resources/Seraphim/Animation/RunningNormal.vmd", false, rewrite_flag);
			if (rewrite_flag && index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = add_animation("RunningFast", "Assets/Resources/Seraphim/Animation/RunningFast.vmd", false, rewrite_flag);
			if (rewrite_flag && index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = add_animation("DamageBlown", "Assets/Resources/Seraphim/Animation/DamageBlown.vmd", false, rewrite_flag);
			if (rewrite_flag && index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = add_animation("DamageNormal", "Assets/Resources/Seraphim/Animation/DamageNormal.vmd", false, rewrite_flag);
			if (rewrite_flag && index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = add_animation("StaggerStart", "Assets/Resources/Seraphim/Animation/StaggerStart.vmd", false, rewrite_flag);
			index = add_animation("Stagger", "Assets/Resources/Seraphim/Animation/Stagger.vmd", false, rewrite_flag);
			index = add_animation("StaggerComplete", "Assets/Resources/Seraphim/Animation/StaggerComplete.vmd", false, rewrite_flag);

			index = add_animation("Entry", "Assets/Resources/Seraphim/Animation/Entry.vmd", true, rewrite_flag);
			index = add_animation("Guard", "Assets/Resources/Seraphim/Animation/Guard.vmd", false, rewrite_flag);
			index = add_animation("Parrying", "Assets/Resources/Seraphim/Animation/Parrying.vmd", false, rewrite_flag);
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = add_animation("LeftMove", "Assets/Resources/Seraphim/Animation/LeftMove.vmd", false, rewrite_flag);
			index = add_animation("RightMove", "Assets/Resources/Seraphim/Animation/RightMove.vmd", false, rewrite_flag);
			index = add_animation("BackMove", "Assets/Resources/Seraphim/Animation/BackMove.vmd", false, rewrite_flag);
			index = add_animation("FrontMove", "Assets/Resources/Seraphim/Animation/FrontMove.vmd", false, rewrite_flag);
			index = add_animation("Attack1", "Assets/Resources/Seraphim/Animation/Attack1.vmd", true, rewrite_flag);
			index = add_animation("Attack2", "Assets/Resources/Seraphim/Animation/Attack2.vmd", true, rewrite_flag);
			index = add_animation("Attack3", "Assets/Resources/Seraphim/Animation/Attack3.vmd", true, rewrite_flag);
			index = add_animation("SpecialAttack1", "Assets/Resources/Seraphim/Animation/SpecialAttack1.vmd", false, rewrite_flag);
			index = add_animation("SpecialAttack2-1", "Assets/Resources/Seraphim/Animation/SpecialAttack2-1.vmd", false, rewrite_flag);
			index = add_animation("SpecialAttack2-2", "Assets/Resources/Seraphim/Animation/SpecialAttack2-2.vmd", false, rewrite_flag);

#if 0
			//	マテリアルに特殊シェーダー設定
			//	主にあとからやる場合に必要
			for (auto& material : resource->get_materials_for_debug())
			{
				material.set_pipeline_state(material::surface_paths::shadow_write_pass, "toon_csm_write");
			}
#endif

			//	ファイル保存
			if (save_flag)
			{
				resource->serialize(setup_data::character_model_file_path);
			}
			path.replace_extension("emo");
		}
	}
#endif
	return std::make_shared<model_object>(setup_data::character_model_object_name, path.string());
}

void boss_seraphim_object::register_effects()
{
	register_effect(static_cast<size_t>(custom_effect_id::omen), "Assets/Resources/effect/Pierre01/omen.efk");
	register_effect(static_cast<size_t>(custom_effect_id::monster_roar), "Assets/Resources/effect/Pierre01/monster_roar.efk");
	register_effect(static_cast<size_t>(custom_effect_id::special_attack_convergence), "Assets/Resources/effect/MAGICALxSPIRAL/special_attack_convergence.efk");
	
	//	エフェクトリソース読込
//	register_effect()
//	effect_manager::instance()->load_resource(effect_resources[static_cast<int>(effect_id::dash)], "Assets/Resources/effect/Effekseer01/sonic_boom.efk");

	//	攻撃用デカールテクスチャ読込
//	texture_manager::instance()->load_texture_from_file(attack_6_decal, "Assets/Resources/ground_crack.png");
}

