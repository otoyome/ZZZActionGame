#pragma	once

#include <string>
#include <memory>
#include "misc.h"

#include "BehaviorTree/select_rule.h"

class action_interface;
class judgment_interface;
class node_base;
class behavior_data;

// ビヘイビアツリー
class behavior_tree
{
public:
	behavior_tree() : root(nullptr){}
	~behavior_tree();

	void add_node(std::string_view parent_name, std::string_view entry_name, int priority, behavior_tree_select_rule select_rule, std::shared_ptr<judgment_interface> judgment, std::shared_ptr<action_interface> action);

	std::shared_ptr<node_base> active_node_inference(std::weak_ptr<behavior_data> data);

	std::shared_ptr<node_base> sequence_back(std::weak_ptr<node_base> sequence_node, std::weak_ptr<behavior_data> data);

	std::shared_ptr<node_base> run(std::weak_ptr<node_base> action_node, std::weak_ptr<behavior_data> data, float elapsed_time);

private:
	void node_all_clear(std::shared_ptr<node_base>& node);

private:
	std::shared_ptr<node_base> root;
};
