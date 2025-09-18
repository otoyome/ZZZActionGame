#include "object/character/person/kuroko_object.h"

#include "collision/capsule.h"

#include "collision/sphere.h"

namespace setup_data
{
	constexpr std::string_view character_model_file_path = "Assets/Resources/Kuroko/Kuroko.emo";
	constexpr std::string_view character_model_object_name = "モノ・クロコ モデル";

	//	武器モデル
	constexpr std::string_view weapon_model_file_path = "Assets/Resources/Kuroko/Weapon.emo";
	constexpr std::string_view weapon_model_object_name = "モノ・クロコ 武器 モデル";
}

std::shared_ptr<model_object> kuroko_object::generate_character_model_object()
{
	//	変換済みファイルがあるか？
	std::filesystem::path path(setup_data::character_model_file_path);
	if (!std::filesystem::exists(path))
	{
#if defined(USE_IMGUI)
		//	無いので変換
		path.replace_extension("pmx");
		pmx_model model = pmx_model(render_data::instance()->get_device().Get(), path.string());
		model.add_vmd_animation("Idle", "Assets/Resources/Kuroko/Animation/Idle.vmd");
		model.add_vmd_animation("Walking", "Assets/Resources/Kuroko/Animation/Walking.vmd");
		model.add_vmd_animation("BackWalking", "Assets/Resources/Kuroko/Animation/BackWalking.vmd");
		model.add_vmd_animation("RunningNormal", "Assets/Resources/Kuroko/Animation/RunningNormal.vmd");
		model.add_vmd_animation("RunningFast", "Assets/Resources/Kuroko/Animation/RunningFast.vmd");
		model.add_vmd_animation("RunningFastFlip", "Assets/Resources/Kuroko/Animation/RunningFastFlip.vmd");
		model.add_vmd_animation("RunStop", "Assets/Resources/Kuroko/Animation/RunStop.vmd");
		model.add_vmd_animation("Step-Front-Split1", "Assets/Resources/Kuroko/Animation/Step-Front-Split1.vmd");
		model.add_vmd_animation("Step-Front-Split2", "Assets/Resources/Kuroko/Animation/Step-Front-Split2.vmd");
		model.add_vmd_animation("Step-Back-Split1", "Assets/Resources/Kuroko/Animation/Step-Back-Split1.vmd");
		model.add_vmd_animation("Step-Back-Split2", "Assets/Resources/Kuroko/Animation/Step-Back-Split2.vmd");
		model.add_vmd_animation("Attack1", "Assets/Resources/Kuroko/Animation/Attack1.vmd");
		model.add_vmd_animation("Attack2", "Assets/Resources/Kuroko/Animation/Attack2.vmd");
		model.add_vmd_animation("Attack3", "Assets/Resources/Kuroko/Animation/Attack3.vmd");
		model.add_vmd_animation("Attack4", "Assets/Resources/Kuroko/Animation/Attack4.vmd");
		model.add_vmd_animation("Attack5", "Assets/Resources/Kuroko/Animation/Attack5.vmd");
		model.add_vmd_animation("Attack6-1", "Assets/Resources/Kuroko/Animation/Attack6-1.vmd");
		model.add_vmd_animation("Attack6-2", "Assets/Resources/Kuroko/Animation/Attack6-2.vmd");
		model.add_vmd_animation("Attack6-3", "Assets/Resources/Kuroko/Animation/Attack6-3.vmd");
		model.add_vmd_animation("AttackDodge", "Assets/Resources/Kuroko/Animation/AttackDodge.vmd");
		model.add_vmd_animation("DamageNormal", "Assets/Resources/Kuroko/Animation/DamageNormal.vmd");
		model.add_vmd_animation("DamageBlown", "Assets/Resources/Kuroko/Animation/DamageBlown.vmd");
		model.add_vmd_animation("Dead", "Assets/Resources/Kuroko/Animation/Dead.vmd");
		std::shared_ptr<model_resource> mr = std::make_shared<model_resource>();
		mr->convert_from_pmx(model);
		//	ルートモーション情報設定
		{
			size_t index;
			index = mr->get_animation_index_from_name("RunningFastFlip");
			auto& animations = mr->get_animations_for_debug();
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_rotation = true;
			}
			index = mr->get_animation_index_from_name("Step-Front-Split1");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("Step-Front-Split2");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("Step-Back-Split1");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("Step-Back-Split2");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("Attack1");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("Attack2");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("Attack3");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("Attack4");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("Attack5");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("Attack6-1");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
				animations[index].root_motion_y = true;
			}
			index = mr->get_animation_index_from_name("AttackDodge");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("DamageNormal");
			if (index != model_resource::invalid_id)
			{
				animations[index].root_motion_node_name = "センター";
				animations[index].root_motion_xz = true;
			}
			index = mr->get_animation_index_from_name("DamageBlown");
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

	return std::make_shared<model_object>(setup_data::character_model_object_name, path.string());
}

std::shared_ptr<model_object> kuroko_object::generate_weapon_model_object()
{
	//	変換済みファイルがあるか？
	std::filesystem::path path(setup_data::weapon_model_file_path);
	if (!std::filesystem::exists(path))
	{
#if defined(USE_IMGUI)
		//	無いので変換
		path.replace_extension("pmx");
		pmx_model model = pmx_model(render_data::instance()->get_device().Get(), path.string());
		std::shared_ptr<model_resource> mr = std::make_shared<model_resource>();
		mr->convert_from_pmx(model);
		//	ファイル保存
		mr->serialize(setup_data::weapon_model_file_path);
		path.replace_extension("emo");
#endif // defined(USE_IMGUI)
	}
	return std::make_shared<model_object>(setup_data::weapon_model_object_name, path.string());
}


void kuroko_object::register_effects()
{
	//	エフェクトリソース読込
	register_effect(static_cast<size_t>(custom_effect_id::grand_viper_frame), "Assets/Resources/effect/tktk02/grand_viperr_frame.efk");
	register_effect(static_cast<size_t>(custom_effect_id::attack6_3), "Assets/Resources/effect/tktk02/attack6_3.efk");
	register_effect(static_cast<size_t>(custom_effect_id::ground_break_effect), "Assets/Resources/effect/Pierre01/ground_break_effect.efk");

//	register_effect(static_cast<size_t>(custom_effect_id::break_attack_impact), "Assets/Resources/effect/tktk02/break_attack2.efk");
	register_effect(static_cast<size_t>(custom_effect_id::break_attack_impact), "Assets/Resources/effect/tktk01/Blow13.efk");

	//	攻撃用デカールテクスチャ読込
	texture_manager::instance()->load_texture_from_file(attack_6_decal, "Assets/Resources/ground_crack.png");
}
