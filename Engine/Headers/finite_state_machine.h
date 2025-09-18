#pragma	once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <type_traits>

class finite_state_node_base
{
public:
	finite_state_node_base(std::string_view name) : name(name) {}
	virtual ~finite_state_node_base() {}
	virtual void on_initialize() {}
	virtual void on_finalize() {}

	virtual void enter() {}
	virtual void update(float elapsed_time) = 0;
	virtual void exit() {}

	std::string_view get_name() const {return name;}
	void set_name(std::string_view name) {this->name = name;}

private:
	std::string name;
};

class function_finite_state_node final : public finite_state_node_base
{
public:
	function_finite_state_node(std::string_view name, std::function<void()> on_enter, std::function<void(float)> on_update, std::function<void()> on_exit);
	~function_finite_state_node() override;

	void enter() override;
	void update(float elapsed_time) override;
	void exit() override;

	void set_enter(std::function<void()> on_enter);
	void set_exit(std::function<void()> on_exit);
	void set_update(std::function<void(float)> on_update);

private:
	std::function<void()> on_enter;
	std::function<void()> on_exit;
	std::function<void(float)> on_update;
};

class hierarchy_finite_state_node : public finite_state_node_base, public std::enable_shared_from_this<hierarchy_finite_state_node>
{
public:
	inline std::shared_ptr<hierarchy_finite_state_node> get_parent_node() const {return parent_node;}
	inline std::shared_ptr<hierarchy_finite_state_node> get_current_state() const {return current_node;}
	inline const std::unordered_map<std::string, std::shared_ptr<hierarchy_finite_state_node>>& get_states() const {return states;}
	void set_state_machine(std::shared_ptr<class finite_state_machine> state_machine);

public:
	hierarchy_finite_state_node(std::string_view name);
	~hierarchy_finite_state_node() override;
	void on_initialize() override final;
	void on_finalize() override final;

	virtual void on_initialize_core() {}
	virtual void on_finalize_core() {}
	void enter() override;
	void update(float elapsed_time) override;
	void exit() override;

	bool change_state(std::string_view key);
	template<typename Type, typename ... Arguments>
	bool register_state(const Arguments ... arguments)
	{
		std::shared_ptr<Type> row_state = std::make_shared<Type>(arguments...);
		return register_state(std::static_pointer_cast<hierarchy_finite_state_node>(row_state));
	}
	bool unregister_state(std::string_view key);

private:
	bool register_state(std::shared_ptr<hierarchy_finite_state_node> state);

private:
	std::unordered_map<std::string, std::shared_ptr<hierarchy_finite_state_node>> states;
	std::shared_ptr<hierarchy_finite_state_node> parent_node;
	std::shared_ptr<hierarchy_finite_state_node> current_node;
	std::shared_ptr<class finite_state_machine> state_machine;
};

class finite_state_machine : public std::enable_shared_from_this<finite_state_machine>
{
public:
	inline std::shared_ptr<finite_state_node_base> get_current_state() const {return current_node;}
	inline const std::unordered_map<std::string, std::shared_ptr<finite_state_node_base>>& get_states() const {return states;}

public:
	finite_state_machine();
	virtual ~finite_state_machine();

	void update(float elapsed_time);

	bool change_state(std::string_view key);
	template<typename Type, typename ... Arguments>
	bool register_state(const Arguments ... arguments)
	{
		std::shared_ptr<Type> row_state = std::make_shared<Type>(arguments...);
		return register_state(std::static_pointer_cast<finite_state_node_base>(row_state));
	}
	bool unregister_state(std::string_view key);

	void clear();

private:
	bool register_state(std::shared_ptr<finite_state_node_base> state);

private:
	std::unordered_map<std::string, std::shared_ptr<finite_state_node_base>> states;
	std::shared_ptr<finite_state_node_base> current_node;
};
