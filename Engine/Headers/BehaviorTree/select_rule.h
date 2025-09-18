#pragma	once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <type_traits>

enum class behavior_tree_select_rule
{
	none,				// 無い末端ノード用
	priority,			// 優先順位
	sequence,			// シーケンス
	sequential_looping,	// シーケンシャルルーピング
	random,				// ランダム
};
