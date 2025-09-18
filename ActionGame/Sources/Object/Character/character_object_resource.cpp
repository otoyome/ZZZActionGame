#include "object/character/character_object.h"

namespace
{
	constexpr std::string_view common_effect_paths[] =
	{
		"Assets/Resources/effect/Effekseer01/sonic_boom.efk",	// shock_wave
		"Assets/Resources/effect/tktk01/parry.efk",	// parry
	};
}

void character_object::register_common_effects()
{
	register_effect(static_cast<size_t>(effect_ids::shock_wave),
					common_effect_paths[static_cast<int>(effect_ids::shock_wave)]);
	register_effect(static_cast<size_t>(effect_ids::parry),
					common_effect_paths[static_cast<int>(effect_ids::parry)]);
}

bool character_object::register_effect(size_t effect_id, std::string_view file_path)
{
	if (effect_resources.find(effect_id) != effect_resources.end())
		return false;

	std::shared_ptr<effect_resource> resource;
	if (!effect_manager::instance()->load_resource(resource, file_path))
	{
		assert(!"Not Found EffectResource");
		return false;
	}
	effect_resources.insert_or_assign(effect_id, resource);
	return true;
}

effect_instance_ptr character_object::play_effect(const play_effect_descs& desc)
{
	int effect_id = static_cast<int>(desc.id);
	auto it = effect_resources.find(effect_id);
	if (it == effect_resources.end())
		return effect_instance_ptr();

	auto effect_instance = it->second->play();
	effect_instance->set_scale(desc.scale);
	effect_instance->set_rotation(desc.rotation);
	effect_instance->set_position(desc.position);
	return effect_instance;
}


