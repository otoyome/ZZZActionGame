#pragma once

#include "graphics_context.h"

class material;

//	描画用パイプラインステート
//	☆ﾃｷﾄｰ('ω')ｰﾄｷﾃ☆
class pipeline_state
{
public:
	enum class domain
	{
		surface,
		postprocess,

		system,
	};

public:
	pipeline_state(){}
	virtual ~pipeline_state() {}

	virtual	void begin(const graphics_context& context) = 0;
	virtual	void set_parameters(const graphics_context& context, const material& material) = 0;
	virtual	void draw_indexed(const graphics_context& context, const buffer_datas& buffer) { assert(!"not compatible draw_indexed"); }
	virtual	void draw(const graphics_context& context, const buffer_datas& buffer) { assert(!"not compatible draw"); }
	virtual	void draw(const graphics_context& context) { assert(!"not compatible draw"); }
	virtual	void end(const graphics_context& context) = 0;
};

using pipeline_state_ptr = std::shared_ptr<pipeline_state>;

class pipeline_state_manager
{
	SINGLETON(pipeline_state_manager)

	using pipeline_state_map = std::unordered_map<std::string, std::shared_ptr<pipeline_state>>;

private:
	pipeline_state_manager(){}
	~pipeline_state_manager(){}

public:
	template<class type>
	bool	register_pipeline_state(std::string_view key, pipeline_state_ptr* pipeline_state_ptr = nullptr)
	{
		if (auto ps = pipeline_state_manager::search(key))
		{
			if (pipeline_state_ptr)
				*pipeline_state_ptr = std::static_pointer_cast<type>(ps);
			return true;
		}

		auto obj = std::make_shared<type>();
		if (register_pipeline_state(key, std::static_pointer_cast<pipeline_state>(obj)))
		{
			if (pipeline_state_ptr)
				*pipeline_state_ptr = obj;
			return true;
		}
		return false;
	}
	pipeline_state_ptr	search(std::string_view key);

private:
	bool	register_pipeline_state(std::string_view key, pipeline_state_ptr& pipeline_state);

private:
	pipeline_state_map	ps_map;
};
