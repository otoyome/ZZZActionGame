#pragma once

#include "effect_resource.h"

class effect_instance
{
public:
	bool is_exist() const;

	DirectX::XMFLOAT3 get_scale() const;
	void set_scale(const DirectX::XMFLOAT3& scale);

	math::rotator get_rotation() const;
	void set_rotation(const math::rotator& rotation);

	DirectX::XMFLOAT3 get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);

	float get_animation_speed() const;
	void set_animation_speed(float animation_speed);

public:
	effect_instance();
	effect_instance(Effekseer::Handle handle);
	~effect_instance();

	void stop();

private:
	::Effekseer::Handle handle;
};

using effect_instance_ptr = std::shared_ptr<effect_instance>;
