#pragma once

#include "Effekseer.h"

class effect_instance;

class effect_resource
{
public:
	::Effekseer::EffectRef get_resource() {return resource;}

public:
	effect_resource();
	~effect_resource();

	bool deserialize(::Effekseer::ManagerRef& manager, std::string_view file_path);

	std::shared_ptr<effect_instance> play();

private:
	::Effekseer::EffectRef resource;
	std::vector<std::shared_ptr<effect_instance>> instances;
};
