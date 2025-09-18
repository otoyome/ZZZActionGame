#pragma	once

#include <string>
#include <unordered_map>
#include <memory>
#include <stack>

class node_base;

class behavior_data
{
public:
	behavior_data() { initialize(); }

	void push_sequence_node(std::weak_ptr<node_base> node) { sequence_stack.push(node); }
	std::weak_ptr<node_base> pop_sequence_node();
	int get_sequence_step(std::string_view name);
	void set_sequence_step(std::string_view name, int step);
	void initialize();

private:
	std::stack<std::weak_ptr<node_base>> sequence_stack;		//	実行する中間ノードをスタック
	std::unordered_map<std::string, int> run_sequence_step_map;	//	実行中の中間ノードのステップを記録
};
