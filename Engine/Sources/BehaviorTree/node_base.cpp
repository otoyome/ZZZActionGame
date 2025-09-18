#include "BehaviorTree/node_base.h"
#include "misc.h"
#include "system_utility.h"

#include "BehaviorTree/judgment_interface.h"
#include "BehaviorTree/behavior_data.h"

node_base::node_base(std::string_view name, std::weak_ptr<node_base> parent, int priority, behavior_tree_select_rule select_rule, std::shared_ptr<judgment_interface> judgment, std::shared_ptr<action_interface> action)
: name(std::string(name))
, parent(parent)
, priority(priority)
, select_rule(select_rule)
, judgment_ptr(judgment)
, action_ptr(action)
, children()
{
}

node_base::~node_base()
{
}


void node_base::add_child(std::shared_ptr<node_base> child)
{
	children.push_back(child);
}

void node_base::all_clear_children()
{
	for (auto& child : children)
	{
		child->all_clear_children();
	}

	children.clear();
}

bool node_base::judgment()
{
	if (judgment_ptr)
	{
		return judgment_ptr->judgment();
	}
	return true;
}

action_interface::state node_base::run(float elapsedTime)
{
	if (action_ptr)
	{
		return action_ptr->run(elapsedTime);
	}

	return action_interface::state::failed;
}

// ノード検索
std::shared_ptr<node_base> node_base::search_node(std::string_view search_name)
{
	if (name == search_name)
	{
		return shared_from_this();
	}

	for (auto itr = children.begin(); itr != children.end(); itr++)
	{
		std::shared_ptr<node_base> ret = (*itr)->search_node(search_name);

		if (ret)
			return ret;
	}

	return nullptr;
}

// ノード推論
std::shared_ptr<node_base> node_base::inference(std::weak_ptr<behavior_data> data)
{
	std::vector<std::shared_ptr<node_base>> list;
	std::shared_ptr<node_base> result;

	for (size_t index = 0; index < children.size(); ++index)
	{
		if (children.at(index)->judgment_ptr)
		{
			if (children.at(index)->judgment_ptr->judgment())
				list.emplace_back(children.at(index));
		}
		else
		{
			//	判定クラスがなければ追加
			list.emplace_back(children.at(index));
		}
	}

	// 選択ルールでノード決め
	switch (select_rule)
	{
		// 優先順位
	case behavior_tree_select_rule::priority:
		result = select_priority(list);
		break;
		// ランダム
	case behavior_tree_select_rule::random:
		result = select_random(list);
		break;
		// シーケンス
	case behavior_tree_select_rule::sequence:
	case behavior_tree_select_rule::sequential_looping:
		result = select_sequence(list, data);
		break;
	}

	if (result)
	{
		if (result->has_action())
		{
			//	行動があれば終了
			return result;
		}
		else
		{
			//	推論開始
			result = result->inference(data);
		}
	}

	return result;
}

// 優先順位でノード選択
std::shared_ptr<node_base> node_base::select_priority(std::vector<std::shared_ptr<node_base>>& list)
{
	std::shared_ptr<node_base> select_node;
	int priority = INT_MAX;

	// 一番優先順位が高いノードを探してselectNodeに格納
	for (auto itr = list.begin(); itr != list.end(); itr++)
	{
		if (priority > (*itr)->get_priority())
		{
			select_node = (*itr);
			priority = (*itr)->get_priority();
		}
	}

	return select_node;
}


// ランダムでノード選択
std::shared_ptr<node_base> node_base::select_random(std::vector<std::shared_ptr<node_base>>& list)
{
	int selectNo = rand() % list.size();
	return list.at(selectNo);
}

std::shared_ptr<node_base> node_base::select_sequence(std::vector<std::shared_ptr<node_base>>& list, std::weak_ptr<behavior_data> data_ptr)
{
	auto data = data_ptr.lock();
	if (data == nullptr)
		return nullptr;

	int step = 0;

	// 指定されている中間ノードのシーケンスがどこまで実行されたか取得する
	step = data->get_sequence_step(name);

	// 中間ノードに登録されているノード数以上の場合、
	if (step >= children.size())
	{
		// ルールによって処理を切り替える
		if (select_rule == behavior_tree_select_rule::sequential_looping)
		{
			step = 0;
		}
		else
		{
			return nullptr;
		}

	}
	for (auto itr = list.begin(); itr != list.end(); itr++)
	{
		// 子ノードが実行可能リストに含まれているか
		if (children.at(step)->get_name() == (*itr)->get_name())
		{
			// 現在の実行ノードの保存、次に実行するステップの保存を行った後、
			// 現在のステップ番号のノードをリターンしなさい
			// ①スタックにはdata->PushSequenceNode関数を使用する。保存するデータは実行中の中間ノード。
			// ②また、次に実行する中間ノードとステップ数を保存する
			// 　保存にはdata->SetSequenceStep関数を使用。
			// 　保存データは中間ノードの名前と次のステップ数です(step + 1)
			// ③ステップ番号目の子ノードを実行ノードとしてリターンする
			data->push_sequence_node(weak_from_this());
			data->set_sequence_step(name, step + 1);
			return children.at(step);
		}
	}
	return nullptr;
}
