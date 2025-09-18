#include "pipeline_state.h"
#include "misc.h"

#include "render_data.h"


bool pipeline_state_manager::register_pipeline_state(std::string_view key, pipeline_state_ptr& pipeline_state)
{
	std::string key_name = std::string(key);
	auto it = ps_map.find(key_name);
	if (it != ps_map.end())
	{
		return false;
	}

	ps_map.insert(std::make_pair(key_name, pipeline_state));
	return true;
}

pipeline_state_ptr	pipeline_state_manager::search(std::string_view key)
{
	std::string key_name = std::string(key);
	auto it = ps_map.find(key_name);
	if (it != ps_map.end())
	{
		return it->second;
	}

	return	nullptr;
}
