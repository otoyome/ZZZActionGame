#pragma once

#include "object/ui/ui_object.h"
#include "object/ui/ui_image.h"

//	顔ウィンドウ用オブジェクト
class face_image_controller_object : public game_object
{
	GAME_OBJECT_DECRLEAR(face_image_controller_object, game_object);

public:
	struct shake_descs
	{
		float time = 0.0f;
		float power = 0.0f;
	};

	struct object_descs
	{
		std::string name;
		std::shared_ptr<ui_image> cover;
		std::shared_ptr<ui_image> base;
		std::shared_ptr<ui_image> main;
	};

public:
	texture_ptr get_image() const;
	DirectX::XMFLOAT4 get_uv_rect() const;
	void set_image(texture_ptr image, DirectX::XMFLOAT4 uv_rect);

public:
	face_image_controller_object(const object_descs& object_desc);
	~face_image_controller_object() override;

	void post_update(float elapsed_time) override;

	void call_shake(const shake_descs& shake_desc);

	void set_alpha(float alpha);

private:
	shake_descs shake_desc;

	std::shared_ptr<ui_image> uis[3];
	DirectX::XMFLOAT4 origin_rects[3];
	DirectX::XMFLOAT4 origin_colors[3];
	DirectX::XMFLOAT2 shake_offset = {0, 0};
};
