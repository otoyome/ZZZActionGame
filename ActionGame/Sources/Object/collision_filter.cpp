#include "Object/collision_filter.h"

std::string_view collision_filter::get_name() const
{
	return name;
}

void collision_filter::set_name(std::string_view name)
{
	this->name = name;
}

collision_category collision_filter::get_category() const
{
	return category;
}

void collision_filter::set_category(collision_category category)
{
	this->category = category;
}

int collision_filter::get_mask() const
{
	return mask;
}

bool collision_filter::is_flag(collision_category category) const
{
	return system_utility::is_bitflag(mask, 1 << static_cast<UINT>(category));
}

void collision_filter::set_flag(collision_category category, bool value)
{
	system_utility::set_bitflag(mask, 1 << static_cast<UINT>(category), value);
}

collision_filter::collision_filter()
: category(static_cast<collision_category>(0)), mask(0xFFFFFFFF)
{
}

collision_filter::~collision_filter()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
collision_filter_manager::collision_filter_manager()
{
	collision_filter_ptr filter = std::make_shared<collision_filter>();
	filter->set_name("default");
	register_filter(filter);
}

collision_filter_manager::~collision_filter_manager()
{
}

bool collision_filter_manager::register_filter(collision_filter_ptr& filter)
{
	std::string filter_name = std::string(filter->get_name());
	auto it = filters.find(filter_name);
	if (it != filters.end())
		return false;

	filters.insert_or_assign(filter_name, filter);
	return true;
}

bool collision_filter_manager::unregister_filter(collision_filter_ptr& filter)
{
	return unregister_filter(filter->get_name());
}

bool collision_filter_manager::unregister_filter(std::string_view name)
{
	std::string filter_name = std::string(name);
	auto it = filters.find(filter_name);
	if (it == filters.end())
		return false;

	filters.erase(it);
	return true;
}

collision_filter_ptr collision_filter_manager::search_filter(std::string_view name)
{
	std::string filter_name = std::string(name);
	auto it = filters.find(filter_name);
	if (it == filters.end())
	{
		it = filters.find("default");
	}

	return it->second;
}
