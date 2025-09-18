#include "graphics/effect_resource.h"

#include "graphics/effect_instance.h"

#include "graphics/effect_manager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
effect_resource::effect_resource()
: resource(nullptr)
{
}

effect_resource::~effect_resource()
{
	for (auto& instance : instances)
	{
		instance->stop();
	}
	system_utility::std_vector_swap_clear(instances);
}

bool effect_resource::deserialize(::Effekseer::ManagerRef& manager, std::string_view file_path)
{
	wchar_t file_path_utf16[MAX_PATH];
	system_utility::convert_shift_jis_to_utf16(file_path.data(), file_path_utf16, sizeof(file_path_utf16));
	std::wstring text = file_path_utf16;
    std::u16string u16_string(text.begin(), text.end());

	resource = Effekseer::Effect::Create(manager, u16_string.data());
	return resource != nullptr;
}

std::shared_ptr<effect_instance> effect_resource::play()
{
	auto handle = effect_manager::instance()->get_effekseer_manager()->Play(get_resource(), 0.0f, 0.0f, 0.0f);
	instances.emplace_back(std::make_shared<effect_instance>(handle));
	return instances.back();
}
