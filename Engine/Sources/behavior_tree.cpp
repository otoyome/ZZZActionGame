#include "behavior_tree.h"
#include "misc.h"
#include "system_utility.h"

#include "BehaviorTree/judgment_interface.h"
#include "BehaviorTree/action_interface.h"
#include "BehaviorTree/node_base.h"
#include "BehaviorTree/behavior_data.h"


// デストラクタ
behavior_tree::~behavior_tree()
{
	node_all_clear(root);
}

void behavior_tree::add_node(std::string_view parent_name, std::string_view entry_name, int priority, behavior_tree_select_rule select_rule, std::shared_ptr<judgment_interface> judgment, std::shared_ptr<action_interface> action)
{
	if (parent_name.empty())
	{
		if (!root)
			root = std::make_shared<node_base>(entry_name, std::weak_ptr<node_base>(), priority, select_rule, judgment, action);
	}
	else
	{
		std::shared_ptr<node_base> parent_node = root->search_node(parent_name);

		if (parent_node)
		{
			auto node = std::make_shared<node_base>(entry_name, parent_node, priority, select_rule, judgment, action);
			parent_node->add_child(node);
		}
	}
}

std::shared_ptr<node_base> behavior_tree::active_node_inference(std::weak_ptr<behavior_data> data_ptr)
{
	// データをリセットして開始
	auto data = data_ptr.lock();
	assert(data);
	data->initialize();
	return root->inference(data);
}

std::shared_ptr<node_base> behavior_tree::sequence_back(std::weak_ptr<node_base> sequence_node, std::weak_ptr<behavior_data> data)
{
	auto node = sequence_node.lock();
	return node->inference(data);
}

std::shared_ptr<node_base> behavior_tree::run(std::weak_ptr<node_base> action_node, std::weak_ptr<behavior_data> data_ptr, float elapsed_time)
{
	auto node = action_node.lock();
	auto data = data_ptr.lock();
	assert(node);
	assert(data);

	// ノード実行
	action_interface::state state = node->run(elapsed_time);

	// 正常終了
	if (state == action_interface::state::complete)
	{
		// シーケンスの途中かを判断
		auto sequence_node = data->pop_sequence_node();

		// 途中じゃないなら終了
		if (sequence_node.expired())
		{
			return nullptr;
		}
		else
		{
			// 途中ならそこから始める
			return sequence_back(sequence_node, data);
		}
	}
	else if (state == action_interface::state::failed)
	{
		// 失敗は終了
		return nullptr;
	}

	// 現状維持
	return node;
}

// 登録されたノードを全て削除する
void behavior_tree::node_all_clear(std::shared_ptr<node_base>& node)
{
	if (!node)
		return;

	node->all_clear_children();
	node.reset();
}

