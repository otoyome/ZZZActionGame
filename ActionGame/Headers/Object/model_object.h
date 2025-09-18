#pragma once

#include "collision/collision.h"

#include "model_instance.h"

//	モデル表示用オブジェクト
class model_object : public game_object
{
	GAME_OBJECT_DECRLEAR(model_object, game_object)

public:
	using call_back_root_animation_translation = std::function<void(const DirectX::XMFLOAT3&)>;
	using call_back_root_animation_rotation = std::function<void(const DirectX::XMFLOAT4&)>;

public:
	model_instance_ptr get_model_instance();

	const DirectX::XMFLOAT3& get_scale() const;
	void set_scale(const DirectX::XMFLOAT3& scale);

	const math::rotator& get_rotation() const;
	void set_rotation(const math::rotator& rotator);

	const DirectX::XMFLOAT3& get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);

	bool get_apply_root_animation_transform() const;
	void set_apply_root_animation_transform(bool flag);

public:
	model_object(std::string_view name, std::string_view file_path);
	~model_object() override;

	void fixed_update(float elapsed_time) override;
	void pre_render() override;

	void update_transform();
	void set_node_pose(size_t node_index, const model_instance::node_animation& node);

	bool register_root_animation_translation_callback(call_back_root_animation_translation callback);
	bool unregister_root_animation_translation_callback();

	bool register_root_animation_rotation_callback(call_back_root_animation_rotation callback);
	bool unregister_root_animation_rotation_callback();

#if defined(USE_IMGUI)
	void imgui_render_core() override;
#endif	//	defined(USE_IMGUI)

public:
	void play_animation(size_t animation_index, bool loop, float blend_time = 0.2f);
	std::string_view get_playing_animation_name() const;
	size_t get_animation_index() const;
	size_t get_animation_index_from_name(std::string_view animation_name) const;
	float get_animation_end_time() const;
	float get_animation_time() const;
	float get_animation_speed() const;
	void set_animation_speed(float animation_speed);

	void shake(float time, float power);

private:
	void on_active() override;
	void on_deactive() override;

private:
	model_instance_ptr instance;
	bool dirty = true;
	DirectX::XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};
	math::rotator rotator = {0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 position = {0.0f, 0.0f, 0.0f};

	bool apply_root_animation_transform = true;
	call_back_root_animation_translation on_root_animation_translation;
	call_back_root_animation_rotation on_root_animation_rotation;

	size_t animation_index = 0;
	bool animation_loop = false;
	float animation_time = 0;
	float animation_speed = 1.0f;

	float animation_blend_time = 0.0f;
	std::vector<model_instance::node_animation> current_node_animations;
	std::vector<model_instance::node_animation> previous_node_animations;

	float shake_time = 0;
	float shake_time_max = 0;
	float shake_power = 0;
};
