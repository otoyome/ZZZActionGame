#pragma	once

#include "BehaviorTree/behavior_data.h"

std::weak_ptr<node_base> behavior_data::pop_sequence_node()
{
	if (sequence_stack.empty() != 0)
	{
		return std::weak_ptr<node_base>();
	}
	std::weak_ptr<node_base> node = sequence_stack.top();
	if (!node.expired())
	{
		sequence_stack.pop();
	}
	return node;
}

int behavior_data::get_sequence_step(std::string_view name)
{
	std::string name_text = std::string(name);
	if (run_sequence_step_map.count(name_text) == 0)
	{
		run_sequence_step_map.insert(std::make_pair(name_text, 0));
	}

	return run_sequence_step_map.at(name_text);
}

void behavior_data::set_sequence_step(std::string_view name, int step)
{
	run_sequence_step_map.at(std::string(name)) = step;
}

void behavior_data::initialize()
{
	run_sequence_step_map.clear();
	while (sequence_stack.size() > 0)
	{
		sequence_stack.pop();
	}
}