#include "post_process_volume.h"
#include "texture.h"

post_process_volume::post_process_volume()
: system_flags(0)
, post_process_data()

, position(0.0f, 0.0f, 0.0f)
, inner_range(50.0f, 50.0f, 50.0f)
, outer_range(100.0f, 100.0f, 100.0f)
, color(1, 1, 1, 1)
{
	set_flag(flags::active, true);
}

post_process_volume::~post_process_volume()
{
}

DirectX::XMFLOAT3 post_process_volume::get_position() const
{
	return position;
}

void post_process_volume::set_position(const DirectX::XMFLOAT3& position)
{
	this->position = position;
}

DirectX::XMFLOAT3 post_process_volume::get_inner_range() const
{
	return inner_range;
}

void post_process_volume::set_inner_range(const DirectX::XMFLOAT3& inner_range)
{
	this->inner_range = inner_range;
}

DirectX::XMFLOAT3 post_process_volume::get_outer_range() const
{
	return outer_range;
}

void post_process_volume::set_outer_range(const DirectX::XMFLOAT3& outer_range)
{
	this->outer_range = outer_range;
}

const DirectX::XMFLOAT4& post_process_volume::get_color() const
{
	return color;
}

void post_process_volume::set_color(const DirectX::XMFLOAT4& color)
{
	this->color = color;
}

post_process_datas post_process_volume::get_post_process_data() const
{
	return post_process_data;
}

void post_process_volume::set_post_process_data(post_process_datas& post_process_data)
{
	this->post_process_data = post_process_data;
}

////////////////////////////////////////////////////////////////////////////////////////////////
post_process_volume_manager::post_process_volume_manager()
{
}

post_process_volume_manager::~post_process_volume_manager()
{
}

bool post_process_volume_manager::generate_post_process_volume(post_process_volume_ptr& out_post_process_volume)
{
	out_post_process_volume = std::make_shared<post_process_volume>();
	post_process_volumes.emplace_back(out_post_process_volume);
	return true;
}

void post_process_volume_manager::get_container(post_process_volume_container& container)
{
	//	ƒ{ƒŠƒ…[ƒ€
	{
		container.post_process_volumes.clear();
		auto it = post_process_volumes.begin();
		for (; it != post_process_volumes.end();)
		{
			if (it->expired())
			{
				it = post_process_volumes.erase(it);
				continue;
			}
			container.post_process_volumes.emplace_back(it->lock());
			++it;
		}
	}
}
