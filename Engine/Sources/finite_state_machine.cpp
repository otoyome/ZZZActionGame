#include "finite_state_machine.h"
#include "misc.h"
#include "system_utility.h"

function_finite_state_node::function_finite_state_node(std::string_view name, std::function<void()> on_enter, std::function<void(float)> on_update, std::function<void()> on_exit)
: finite_state_node_base(name)
, on_enter(on_enter)
, on_update(on_update)
, on_exit(on_exit)
{
}

function_finite_state_node::~function_finite_state_node()
{
}

void function_finite_state_node::enter()
{
	if (on_enter)
		on_enter();
}

void function_finite_state_node::update(float elapsed_time)
{ 
	if (on_update)
		on_update(elapsed_time);
}

void function_finite_state_node::exit()
{
	if (on_exit)
		on_exit();
}

void function_finite_state_node::set_enter(std::function<void()> on_enter)
{
	this->on_enter = on_enter;
}

void function_finite_state_node::set_exit(std::function<void()> on_exit)
{
	this->on_exit = on_exit;
}

void function_finite_state_node::set_update(std::function<void(float)> on_update)
{
	this->on_update = on_update;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void hierarchy_finite_state_node::set_state_machine(std::shared_ptr<finite_state_machine> state_machine)
{
	this->state_machine = state_machine;
}

hierarchy_finite_state_node::hierarchy_finite_state_node(std::string_view name)
: finite_state_node_base(name)
, states()
, parent_node()
, current_node()
{
}

hierarchy_finite_state_node::~hierarchy_finite_state_node()
{
	on_finalize();
}

void hierarchy_finite_state_node::on_initialize()
{
	on_initialize_core();
}

void hierarchy_finite_state_node::on_finalize()
{
	state_machine.reset();
	parent_node.reset();
	current_node.reset();

	for  (auto state : states)
	{
		state.second->on_finalize();
	}
	states.clear();

	on_finalize_core();
}

void hierarchy_finite_state_node::enter()
{
}

void hierarchy_finite_state_node::update(float elapsed_time)
{ 
	if (current_node)
		current_node->update(elapsed_time);
}

void hierarchy_finite_state_node::exit()
{
}

bool hierarchy_finite_state_node::change_state(std::string_view key)
{
	auto it_next = states.find(std::string(key));
	if (it_next == states.end())
	{
		DEBUG_LOG("not found state : %d\n", key);
		return false;
	}

	it_next->second->enter();
	if (current_node)
		current_node->exit();

#if defined(_DEBUG) || defined(USE_IMGUI)
	DEBUG_LOG("change_state:%s > %s\n",
		current_node ? current_node->get_name().data() : "none",
		it_next != states.end() ? it_next->second->get_name().data() : "none");
#endif // defined(_DEBUG) || defined(USE_IMGUI)

	current_node = it_next->second;
	return true;
}

bool hierarchy_finite_state_node::register_state(std::shared_ptr<hierarchy_finite_state_node> state)
{
	std::string key_text = std::string(state->get_name());
	if (states.find(key_text) != states.end())
		return false;

	state->parent_node = shared_from_this();
	state->on_initialize();
	states.emplace(std::make_pair(key_text, state));
	return true;
}

bool hierarchy_finite_state_node::unregister_state(std::string_view key)
{
	std::string key_text = std::string(key);
	auto it = states.find(key_text);
	if (it != states.end())
	{
		if (current_node == it->second)
			current_node.reset();
		it->second->on_finalize();
		states.erase(it);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
finite_state_machine::finite_state_machine()
: states()
, current_node()
{
}

finite_state_machine::~finite_state_machine()
{
	for  (auto state : states)
	{
		state.second->on_finalize();
	}
	states.clear();
}

void finite_state_machine::update(float elapsed_time)
{
	if (current_node)
	{
		current_node->update(elapsed_time);
	}
}

bool finite_state_machine::change_state(std::string_view key)
{
	auto it_next = states.find(std::string(key));
	if (it_next == states.end())
	{
		DEBUG_LOG("not found state : %d\n", key);
		return false;
	}

	it_next->second->enter();

#if defined(_DEBUG) || defined(USE_IMGUI)
	DEBUG_LOG("change_state:%s > %s\n",
		current_node ? current_node->get_name().data() : "none",
		it_next != states.end() ? it_next->second->get_name().data() : "none");
#endif // defined(_DEBUG) || defined(USE_IMGUI)

	current_node = it_next->second;
	return true;
}

bool finite_state_machine::register_state(std::shared_ptr<finite_state_node_base> state)
{
	std::string key_text = std::string(state->get_name());
	if (states.find(key_text) != states.end())
		return false;

	state->on_initialize();
	states.emplace(std::make_pair(key_text, state));
	return true;
}

bool finite_state_machine::unregister_state(std::string_view key)
{
	std::string key_text = std::string(key);
	auto it = states.find(key_text);
	if (it != states.end())
	{
		if (current_node == it->second)
			current_node.reset();
		it->second->on_finalize();
		states.erase(it);
		return true;
	}

	return false;
}

void finite_state_machine::clear()
{
	for  (auto state : states)
	{
		state.second->on_finalize();
	}
	current_node.reset();
	states.clear();
}
