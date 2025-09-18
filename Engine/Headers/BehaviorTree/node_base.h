#pragma	once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <type_traits>
#include "select_rule.h"

#include "action_interface.h"

class judgment_interface;
class behavior_data;

// ノード
class node_base : public std::enable_shared_from_this<node_base>
{
public:
	std::string_view get_name() const { return name; }

	int get_priority() const { return priority; }

	bool has_action() const { return action_ptr != nullptr ? true : false; }

public:
	node_base(std::string_view name, std::weak_ptr<node_base> parent, int priority, behavior_tree_select_rule selectRule, std::shared_ptr<judgment_interface> judgment, std::shared_ptr<action_interface> action);
	virtual ~node_base();

	void add_child(std::shared_ptr<node_base> child);

	void all_clear_children();

	bool judgment();

	std::shared_ptr<node_base> select_priority(std::vector<std::shared_ptr<node_base>>& list);

	std::shared_ptr<node_base> select_random(std::vector<std::shared_ptr<node_base>>& list);

	std::shared_ptr<node_base> select_sequence(std::vector<std::shared_ptr<node_base>>& list, std::weak_ptr<behavior_data> data_ptr);

	std::shared_ptr<node_base> search_node(std::string_view search_name);

	std::shared_ptr<node_base> inference(std::weak_ptr<behavior_data> data);

	action_interface::state run(float elapsedTime);

protected:
	std::string								name;			// 名前
	behavior_tree_select_rule				select_rule;	// 選択ルール
	std::shared_ptr<judgment_interface>		judgment_ptr;	// 判定クラス
	std::shared_ptr<action_interface>		action_ptr;		// 実行クラス
	unsigned int							priority;		// 優先順位
	std::weak_ptr<node_base>				parent;			// 親ノード
	std::vector<std::shared_ptr<node_base>>	children;		// 子ノード
};