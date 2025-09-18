#include "object/character/character_object.h"

namespace
{
	constexpr float instant_dogge_slow_time = system_units::convert_fps_to_sec(180.0f);
	constexpr float instant_dogge_self_slow_time = system_units::convert_fps_to_sec(60.0f);

	//	防御用ダメージ情報
	static constexpr character_object::attack_descs guard_damage_desc =
	{
		{0},
		{500, true, 0.0f},
		{0},
		{0},
		character_object::action_level::level_1,
		system_units::convert_fps_to_sec(20.0f),
		system_units::convert_m_to_unit(20.0f),
		true,
		system_units::convert_fps_to_sec(18.0f),
		system_units::convert_m_to_unit(0.5f),
	};
	constexpr float stagger_recovery_default_time_max = system_units::convert_fps_to_sec(600.0f);
}

character_object::character_object(std::string_view name, std::shared_ptr<model_object> model, std::string_view collision_filter_name)
: game_object(name)
, model(model)
, flag_data(0)
, camera_distance_translucent_inner_flag(-1)
, elapsed_time_scale(1.0f)
, collision_filter_name(std::string(collision_filter_name))

, state_machine()

, health(1000)
, max_health(1000)
, trunk(1000)
, max_trunk(1000)
, special_power(0)
, max_special_power(1000)
, deluxe_power(0)
, max_deluxe_power(1000)
, hit_stop_time(0.0f)
, just_dodge_judgement_time(system_units::convert_fps_to_sec(5.0f))
, stagger_recovery_time(0)
, stagger_recovery_time_max(stagger_recovery_default_time_max)
, invisile_timer(0)
, basic_movement_dash_flag(false)
, damage_cancel_resist_level(action_level::level_1)

, push_box()
, attack_box()
, hit_box()
, on_attack_hits()
, on_stagger_hits()
, hit_objects()
, on_damageds()
, on_camera_transforms()
, on_time_controlls()
, on_change_states()

, move_vector(0.0f, 0.0f)
, turn_front_vector(0.0f, 0.0f)
, velocity(0.0f, 0.0f, 0.0f)
, accelaration(system_units::convert_m_to_unit(6.0f))
, breaking_deceleration(system_units::convert_m_to_unit(6.0f))
, max_move_speed(system_units::convert_m_to_unit(15.0f))
, rotation_speed(DirectX::XMConvertToRadians(1440.0f))
, roll_value(0.0f)

, gravity_scale(1.0f)
, slope_rate(1.0f)
, step_offset(system_units::convert_m_to_unit(10.0f))
, slope_limit(DirectX::XMConvertToRadians(30.0f))
, ground_friction(0.5f)
{
	set_flag(flags::movable, true);
	set_flag(flags::on_ground, true);
	set_flag(flags::camera_distance_translucent, true);

	//	当たり判定の登録
	push_box = std::make_shared<collision_object>();
	push_box->set_draw_normal_color({0.0f, 1.0f, 0.0f, 1.0f});
	push_box->set_collision_enter([this](collision_object::data data) {on_push_box_enter(data);});
	push_box->set_collision_stay([this](collision_object::data data) {on_push_box_stay(data);});
	object_manager::instance()->register_object(push_box);

	//	ルートモーション情報の受け取り
	model->set_apply_root_animation_transform(false);
	model->register_root_animation_translation_callback([this](const DirectX::XMFLOAT3& translation) {on_root_animation_translation(translation);});
	model->register_root_animation_rotation_callback([this](const DirectX::XMFLOAT4& rotation) {on_root_animation_rotation(rotation);});

	set_elapsed_time_scale(1.0f);

	//	基本的なステートの登録
	register_common_states();

	//	基本的なエフェクトの登録
	register_common_effects();

	//	起動時は待機ステート
	change_state(character_object::state_names[state_idle]);
}

character_object::~character_object()
{
	clear_hit_box();
	clear_attack_box();
	lockon_taget_object.reset();
	model->unregister_root_animation_translation_callback();
	model->unregister_root_animation_rotation_callback();
	object_manager::instance()->unregister_object(push_box);
	object_manager::instance()->unregister_object(model);
	model.reset();
}

#if defined(USE_IMGUI)
#define CHECK_BOX(FLAG) \
{\
	bool is_##FLAG = is_flag(flags::##FLAG);\
	ImGui::Checkbox(#FLAG, &is_##FLAG);\
	set_flag(flags::##FLAG, is_##FLAG);\
}

void character_object::imgui_render_core()
{
	float degrees;
	CHECK_BOX(movable);
	CHECK_BOX(on_ground);
	{
		std::string current_state_name = "none";
		if (state_machine.get_current_state())
			current_state_name = state_machine.get_current_state()->get_name();
		ImGui::Text("current_state : %s", current_state_name.data(), current_state_name.size());
	}
	ImGui::DragInt("health", &health);
	ImGui::DragInt("max_health", &max_health);
	ImGui::DragInt("trunk", &trunk);
	ImGui::DragInt("max_trunk", &max_trunk);
	ImGui::DragInt("special_power", &special_power);
	ImGui::DragInt("max_special_power", &max_special_power);
	ImGui::DragInt("deluxe_power", &deluxe_power);
	ImGui::DragInt("max_deluxe_power", &max_deluxe_power);
	ImGui::Separator();
	ImGui::DragFloat("hit_stop_time", &hit_stop_time);
	ImGui::DragFloat("just_dodge_judgement_time", &just_dodge_judgement_time);
	ImGui::DragFloat("stagger_recovery_time", &stagger_recovery_time);
	ImGui::DragFloat("invisile_timer", &invisile_timer);

	{
		auto scale = get_scale();
		auto rotator = get_rotation();
		auto position = get_position();
		ImGui::DragFloat3("scale", &scale.x, 0.01f);
		ImGui::DragFloat3("rotation", &rotator.x, 0.01f);
		ImGui::DragFloat3("position", &position.x, 0.01f);
		set_scale(scale);
		set_rotation(rotator);
		set_position(position);
	}

	ImGui::DragFloat3("velocity", &velocity.x, 0.01f);
	ImGui::DragFloat("accelaration", &accelaration, 0.01f);
	ImGui::DragFloat("breaking_deceleration", &breaking_deceleration, 0.01f);
	ImGui::DragFloat("max_move_speed", &max_move_speed, 0.01f);

	degrees = DirectX::XMConvertToDegrees(rotation_speed);
	ImGui::DragFloat("rotation_speed", &degrees, 0.01f);
	rotation_speed = DirectX::XMConvertToRadians(degrees);

	ImGui::DragFloat("gravity_scale", &gravity_scale, 0.01f);
	ImGui::DragFloat("step_offset", &step_offset, 0.01f);

	degrees = DirectX::XMConvertToDegrees(slope_limit);
	ImGui::DragFloat("slope_limit", &degrees, 0.01f);
	slope_limit = DirectX::XMConvertToRadians(degrees);

	if (push_box)
	{
		if (ImGui::TreeNode("push_box"))
		{
			push_box->imgui_render();

			ImGui::TreePop();
		}
	}
}
#undef CHECK_BOX
#endif	//	defined(USE_IMGUI)

std::shared_ptr<model_object> character_object::get_model_object() const
{
	return model;
}

void character_object::set_model_object(std::shared_ptr<model_object> model)
{
	this->model = model;
}

texture_ptr character_object::get_face_texture() const
{
	return face_texture;
}

void character_object::set_face_texture(texture_ptr face)
{
	face_texture = face;
}

DirectX::XMFLOAT4 character_object::get_face_uv() const
{
	return face_uv;
}

void character_object::set_face_uv(DirectX::XMFLOAT4 face_uv)
{
	this->face_uv = face_uv;
}

collision_object_ptr character_object::get_push_box() const
{
	return push_box;
}

const DirectX::XMFLOAT3& character_object::get_scale() const
{
	return model->get_scale();
}

void character_object::set_scale(const DirectX::XMFLOAT3& scale)
{
	model->set_scale(scale);
}

const math::rotator& character_object::get_rotation() const
{
	return model->get_rotation();
}

void character_object::set_rotation(const math::rotator& rotator)
{
	model->set_rotation(rotator);
	if (push_box)
		push_box->set_rotation(rotator);
}

float character_object::get_rotation_speed() const
{
	return rotation_speed;
}

void character_object::set_rotation_speed(float rotation_speed)
{
	this->rotation_speed = rotation_speed;
}

const DirectX::XMFLOAT3& character_object::get_position() const
{
	return model->get_position();
}

void character_object::set_position(const DirectX::XMFLOAT3& position)
{
	model->set_position(position);
	if (push_box)
		push_box->set_position(position);
}

const DirectX::XMFLOAT3& character_object::get_footstep_position() const
{
	return footstep_position;
}

std::weak_ptr<character_object> character_object::get_lockon_taget_object() const
{
	return lockon_taget_object;
}

void character_object::set_lockon_taget_object(std::weak_ptr<character_object> lockon_taget_object)
{
	this->lockon_taget_object = lockon_taget_object;
}

DirectX::XMFLOAT3 character_object::get_focus_point() const
{
	auto transform = push_box->get_bounding_object()->get_transform_float4x4();
	return {transform._41, transform._42, transform._43};
}

void character_object::add_move_vector(DirectX::XMFLOAT2 move_vector, float accelaration)
{
	this->move_vector = move_vector;
	this->accelaration = accelaration;
}

void character_object::set_turn_vector(DirectX::XMFLOAT2 direction)
{
	this->turn_front_vector = direction;
}

DirectX::XMFLOAT2 character_object::get_turn_vector() const
{
	return this->turn_front_vector;
}

DirectX::XMFLOAT3 character_object::get_velocity() const
{
	return velocity;
}

void character_object::set_velocity(DirectX::XMFLOAT3 velocity)
{
	this->velocity = velocity;
}

float character_object::get_gravity_scale() const
{
	return gravity_scale;
}

void character_object::set_gravity_scale(float gravity_scale)
{
	this->gravity_scale = gravity_scale;
}

float character_object::get_step_offset() const
{
	return step_offset;
}

void character_object::set_step_offset(float step_offset)
{
	this->step_offset = step_offset;
}

int character_object::get_health() const
{
	return health;
}

void character_object::set_health(int health)
{
	this->health = math::clamp(health, 0, max_health);
}

int character_object::get_max_health() const
{
	return max_health;
}

void character_object::set_max_health(int max_health)
{
	this->max_health = max_health;
	//	最大値が減ったら減らす
	if (this->health > this->max_health)
	{
		this->health = this->max_health;
	}
}

int character_object::get_trunk() const
{
	return trunk;
}

void character_object::set_trunk(int trunk)
{
	this->trunk = math::clamp(trunk, 0, max_trunk);
}

int character_object::get_max_trunk() const
{
	return max_trunk;
}

void character_object::set_max_trunk(int max_trunk)
{
	this->max_trunk = std::max(max_trunk, 0);
	//	最大値が減ったら減らす
	if (this->trunk > this->max_trunk)
	{
		this->trunk = this->max_trunk;
	}
}

int character_object::get_special_power() const
{
	return special_power;
}

void character_object::set_special_power(int power)
{
	this->special_power = math::clamp(power, 0, max_special_power);
}

int character_object::get_max_special_power() const
{
	return max_special_power;
}

void character_object::set_max_special_power(int power)
{
	this->max_special_power = std::max(power, 0);
	//	最大値が減ったら減らす
	if (this->special_power > this->max_special_power)
	{
		this->special_power = this->max_special_power;
	}
}

int character_object::get_use_special_power() const
{
	return 0;
}

int character_object::get_deluxe_power() const
{
	return deluxe_power;
}

void character_object::set_deluxe_power(int power)
{
	this->deluxe_power = math::clamp(power, 0, max_deluxe_power);
}

int character_object::get_max_deluxe_power() const
{
	return max_deluxe_power;
}

void character_object::set_max_deluxe_power(int power)
{
	this->max_deluxe_power = std::max(power, 0);
	//	最大値が減ったら減らす
	if (this->deluxe_power > this->max_deluxe_power)
	{
		this->deluxe_power = this->max_deluxe_power;
	}
}

float character_object::get_elapsed_time_scale() const
{
	return elapsed_time_scale;
}

void character_object::set_elapsed_time_scale(float elapsed_time_scale)
{
	this->elapsed_time_scale = elapsed_time_scale;
	if (get_model_object())
	{
		get_model_object()->set_animation_speed(elapsed_time_scale);
	}
}

float character_object::get_stagger_recovery_time() const
{
	return stagger_recovery_time;
}

void character_object::set_stagger_recovery_time(float stagger_recovery_time)
{
	this->stagger_recovery_time = stagger_recovery_time;
}

float character_object::get_stagger_recovery_time_max() const
{
	return stagger_recovery_time_max;
}

void character_object::set_stagger_recovery_time_max(float stagger_recovery_time_max)
{
	this->stagger_recovery_time_max = stagger_recovery_time_max;
}

bool character_object::get_basic_movement_dash() const
{
	return basic_movement_dash_flag;
}

void character_object::set_basic_movement_dash(bool basic_movement_dash_flag)
{
	this->basic_movement_dash_flag = basic_movement_dash_flag;
}

character_object::action_level character_object::get_damage_cancel_resist_level() const
{
	return damage_cancel_resist_level;
}

void character_object::set_damage_cancel_resist_level(character_object::action_level damage_cancel_resist_level)
{
	this->damage_cancel_resist_level = damage_cancel_resist_level;
}

void character_object::add_impulse(DirectX::XMFLOAT3 delta_velocity)
{
	//	単純に速度に加算する
	velocity.x += delta_velocity.x;
	velocity.y += delta_velocity.y;
	velocity.z += delta_velocity.z;
}

float character_object::get_max_move_speed() const
{
	return max_move_speed;
}

void character_object::set_max_move_speed(float max_move_speed)
{
	this->max_move_speed = max_move_speed;
}

float character_object::get_breaking_deceleration() const
{
	return breaking_deceleration;
}

void character_object::set_breaking_deceleration(float breaking_deceleration)
{
	this->breaking_deceleration = breaking_deceleration;
}

float character_object::get_invisile_timer() const
{
	return invisile_timer;
}

void character_object::set_invisile_timer(float invisile_timer)
{
	this->invisile_timer = invisile_timer;
}

void character_object::add_attack_box(const bounding_datas& bounding_data, const damage_datas& damage_data)
{
	generate_box_data(attack_box, bounding_data);
	auto& data = attack_box.back();

	collision_object::user_datas user_data;
	user_data.identifier = attack_data_identifier;
	user_data.buffer.resize(sizeof(damage_datas));
	memcpy_s(user_data.buffer.data(), user_data.buffer.size(), &damage_data, sizeof(damage_data));
	data.object->set_user_data(user_data);
	data.object->set_draw_normal_color({1.0f, 0.0f, 1.0f, 1.0f});

	data.object->set_collision_enter([this](collision_object::data data) {on_attack_box_enter(data);});
//	data.object->set_collision_stay([this](collision_object::data data) {on_attack_box_stay(data);});
//	data.object->set_collision_end([this](collision_object::data data) {on_attack_box_end(data);});

	object_manager::instance()->register_object(data.object);
}

void character_object::clear_attack_box()
{
	for (auto& box : attack_box)
	{
		object_manager::instance()->unregister_object(box.object);
	}
	system_utility::std_vector_swap_clear(hit_objects);
	system_utility::std_vector_swap_clear(attack_box);
}

void character_object::add_hit_box(const bounding_datas& bounding_data)
{
	generate_box_data(hit_box, bounding_data);
	auto& data = hit_box.back();

	collision_object::user_datas user_data;
	user_data.identifier = hit_box_data_identifier;
	user_data.buffer.resize(sizeof(damage_datas));
	hit_box_datas hit_box_data;
	hit_box_data.self_character = this;
	memcpy_s(user_data.buffer.data(), user_data.buffer.size(), &hit_box_data, sizeof(hit_box_data));
	data.object->set_user_data(user_data);
	data.object->set_draw_normal_color({0.0f, 0.0f, 1.0f, 1.0f});

//	data.object->set_collision_enter([this](collision_object::data data) {on_hit_box_enter(data);});
//	data.object->set_collision_stay([this](collision_object::data data) {on_hit_box_stay(data);});
//	data.object->set_collision_end([this](collision_object::data data) {on_hit_box_end(data);});

	object_manager::instance()->register_object(data.object);
}

void character_object::clear_hit_box()
{
	for (auto& box : hit_box)
	{
		object_manager::instance()->unregister_object(box.object);
	}
	system_utility::std_vector_swap_clear(hit_box);
}

bool character_object::register_attack_hit_callback(std::string_view identifier, call_back_damage call_back)
{
	std::string id = std::string(identifier);
	if (on_attack_hits.find(id) != on_attack_hits.end())
		return false;

	on_attack_hits.insert_or_assign(id, call_back);
	return true;
}

bool character_object::unregister_attack_hit_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_attack_hits.find(id);
	if (it != on_attack_hits.end())
		return false;

	on_attack_hits.erase(it);
	return true;
}

bool character_object::register_stagger_hit_callback(std::string_view identifier, call_back_stagger call_back)
{
	std::string id = std::string(identifier);
	if (on_stagger_hits.find(id) != on_stagger_hits.end())
		return false;

	on_stagger_hits.insert_or_assign(id, call_back);
	return true;
}

bool character_object::unregister_stagger_hit_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_stagger_hits.find(id);
	if (it != on_stagger_hits.end())
		return false;

	on_stagger_hits.erase(it);
	return true;
}

bool character_object::register_stagger_callback(std::string_view identifier, call_back_stagger call_back)
{
	std::string id = std::string(identifier);
	if (on_staggers.find(id) != on_staggers.end())
		return false;

	on_staggers.insert_or_assign(id, call_back);
	return true;
}

bool character_object::unregister_stagger_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_staggers.find(id);
	if (it != on_staggers.end())
		return false;

	on_staggers.erase(it);
	return true;
}

bool character_object::register_damage_callback(std::string_view identifier, call_back_damage call_back)
{
	std::string id = std::string(identifier);
	if (on_damageds.find(id) != on_damageds.end())
		return false;

	on_damageds.insert_or_assign(id, call_back);
	return true;
}

bool character_object::unregister_damage_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_damageds.find(id);
	if (it != on_damageds.end())
		return false;

	on_damageds.erase(it);
	return true;
}

bool character_object::register_dead_callback(std::string_view identifier, call_back_dead call_back)
{
	std::string id = std::string(identifier);
	if (on_deads.find(id) != on_deads.end())
		return false;

	on_deads.insert_or_assign(id, call_back);
	return true;
}

bool character_object::unregister_dead_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_deads.find(id);
	if (it != on_deads.end())
		return false;

	on_deads.erase(it);
	return true;
}

bool character_object::register_guard_hit_callback(std::string_view identifier, call_back_damage call_back)
{
	std::string id = std::string(identifier);
	if (on_guard_hits.find(id) != on_guard_hits.end())
		return false;

	on_guard_hits.insert_or_assign(id, call_back);
	return true;
}

bool character_object::unregister_guard_hit_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_guard_hits.find(id);
	if (it != on_guard_hits.end())
		return false;

	on_guard_hits.erase(it);
	return true;
}

bool character_object::register_guarded_callback(std::string_view identifier, call_back_damage call_back)
{
	std::string id = std::string(identifier);
	if (on_guardeds.find(id) != on_guardeds.end())
		return false;

	on_guardeds.insert_or_assign(id, call_back);
	return true;
}

bool character_object::unregister_guarded_callback(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_guardeds.find(id);
	if (it != on_guardeds.end())
		return false;

	on_guardeds.erase(it);
	return true;
}

bool character_object::register_camera_transform(std::string_view identifier, call_back_camera_transform call_back)
{
	std::string id = std::string(identifier);
	if (on_camera_transforms.find(id) != on_camera_transforms.end())
		return false;

	on_camera_transforms.insert_or_assign(id, call_back);
	return true;
}

bool character_object::unregister_camera_transform(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_camera_transforms.find(id);
	if (it != on_camera_transforms.end())
		return false;

	on_camera_transforms.erase(it);
	return true;
}

bool character_object::register_time_controll(std::string_view identifier, call_back_time_controll call_back)
{
	std::string id = std::string(identifier);
	if (on_time_controlls.find(id) != on_time_controlls.end())
		return false;

	on_time_controlls.insert_or_assign(id, call_back);
	return true;
}

bool character_object::unregister_time_controll(std::string_view identifier)
{
	std::string id = std::string(identifier);
	auto it = on_time_controlls.find(id);
	if (it != on_time_controlls.end())
		return false;

	on_time_controlls.erase(it);
	return true;
}

void character_object::generate_box_data(std::vector<bounding_inner_datas>& datas, const bounding_datas& bounding_data)
{
	datas.emplace_back();
	auto& data = datas.back();
	data.is_follow_self = bounding_data.desc.is_follow_self;

	data.object = std::make_shared<collision_object>();
	switch (bounding_data.desc.type)
	{
		case collision::bounding_object::types::box:
		{
			data.object->set_bounding_object(std::make_shared<collision::bounding_box>());
			break;
		}
		case collision::bounding_object::types::capsule:
		{
			data.object->set_bounding_object(std::make_shared<collision::bounding_capsule>());
			break;
		}
		case collision::bounding_object::types::sphere:
		{
			data.object->set_bounding_object(std::make_shared<collision::bounding_sphere>());
			break;
		}
	}

#if defined(USE_IMGUI)
	data.object->set_name(get_name());
#endif	//	defined(USE_IMGUI)
	data.object->set_filter(collision_filter_name);
	data.object->set_local_scale(bounding_data.desc.local_scale);
	data.object->set_local_rotation(bounding_data.desc.local_rotator);
	data.object->set_local_offset(bounding_data.desc.local_offset);
	data.object->set_rotation(bounding_data.rotator);
	data.object->set_position(bounding_data.position);
	data.object->set_flag(collision_object::flags::detect_only, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void character_object::on_attack_hit(const damage_datas& data)
{
	//	ヒット処理
	hit_stop_time = data.desc.hit_stop_time;

	//	コールバック呼び出し
	for (auto& func : on_attack_hits)
	{
		func.second(data);
	}
}

void character_object::on_damaged(const damage_datas& data)
{
	//	よろけ中は無視する
	{
		set_velocity({0, 0, 0});
		//	吹き飛ばし速度を与える
		DirectX::XMFLOAT3 impulse;
		impulse.x = data.stagger_direction.x * data.desc.blow_power;
		impulse.y = data.stagger_direction.y * data.desc.blow_power;
		impulse.z = data.stagger_direction.z * data.desc.blow_power;
		add_impulse(impulse);

		//	ダメージ側に向きを向ける
		if (data.desc.update_turn_vector && !is_flag(flags::stagger))
			set_turn_vector({ -data.stagger_direction.x, -data.stagger_direction.z });
	}

	//	ヒットストップ処理
	hit_stop_time = data.desc.hit_stop_time;

	//	揺れ処理
	get_model_object()->shake(data.desc.hit_stop_time, data.desc.camera_shake_power);

	//	コールバック呼び出し
	for (auto& func : on_damageds)
	{
		func.second(data);
	}
}

void character_object::on_guard_hit(const damage_datas& data)
{
	//	ヒット処理
	hit_stop_time = data.desc.hit_stop_time;

	//	コールバック呼び出し
	for (auto& func : on_guard_hits)
	{
		func.second(data);
	}
}

void character_object::on_guarded(const damage_datas& data)
{
	//	吹き飛ばし速度を与える
	set_velocity({0, 0, 0});
	DirectX::XMFLOAT3 impulse;
	impulse.x = data.stagger_direction.x * data.desc.blow_power;
	impulse.y = data.stagger_direction.y * data.desc.blow_power;
	impulse.z = data.stagger_direction.z * data.desc.blow_power;
	add_impulse(impulse);

	//	ダメージ側に向きを向ける
	if (data.desc.update_turn_vector)
		set_turn_vector({-data.stagger_direction.x, -data.stagger_direction.z});

	//	ヒットストップ処理
	hit_stop_time = data.desc.hit_stop_time;

	//	コールバック呼び出し
	for (auto& func : on_guardeds)
	{
		func.second(data);
	}
}

void character_object::on_hit_while_dodging(const damage_datas& data)
{
	float animation_time = get_model_object()->get_animation_time();

	//	無敵時間タイマー
	invisile_timer = instant_dogge_slow_time;

	//	極限回避演出呼び出し
	if (animation_time <= just_dodge_judgement_time)
	{
		//	コールバック呼び出し
		time_controll_datas data;
		data.scale = 0.2f;
		data.time = instant_dogge_slow_time;
		data.exclude_data.object = weak_from_this();
		data.exclude_data.time = instant_dogge_self_slow_time;
		for (auto& func : on_time_controlls)
		{
			func.second(data);
		}
	}
}

void character_object::on_deead()
{
	//	既に死亡済みなら呼び出さない
	if (is_flag(flags::dead))
		return;

	set_flag(flags::dead, true);

	//	コールバック呼び出し
	dead_descs desc;
	desc.self_character = this;
	for (auto& func : on_deads)
	{
		func.second(desc);
	}
}

void character_object::on_stagger_hit(const stagger_datas& data)
{
	//	コールバック呼び出し
	for (auto& func : on_stagger_hits)
	{
		func.second(data);
	}
}

void character_object::on_stagger(const stagger_datas& data)
{
	//	既によろけ中なら呼び出さない
	if (is_flag(flags::stagger))
		return;

	//	よろけ状態へ移行
	set_flag(flags::stagger, true);
	stagger_recovery_time = 0.0f;

	//	コールバック呼び出し
	for (auto& func : on_staggers)
	{
		func.second(data);
	}
}

void character_object::on_attack_box_enter(collision_object::data data)
{
	auto& from_user_data = data.from->get_user_data();
	auto& to_user_data = data.to->get_user_data();

	//	攻撃ボックス
	if (from_user_data.identifier == attack_data_identifier)
	{
		damage_datas* damage_data = from_user_data.get_data<damage_datas>();

		//	ヒットボックスである場合
		if (to_user_data.identifier == hit_box_data_identifier)
		{
			const hit_box_datas* hit_box_data = to_user_data.get_data<hit_box_datas>();
			//	相手が死亡していた場合は除外
			if (hit_box_data->self_character && !hit_box_data->self_character->is_flag(flags::dead))
			{
				auto it = std::find_if(hit_objects.begin(), hit_objects.end(),
					[&](character_object* p) {return p == hit_box_data->self_character;});
				if (it == hit_objects.end())
				{
					hit_objects.push_back(hit_box_data->self_character);
					//	相手側が無敵だったら処理しない
					if (!hit_box_data->self_character->is_flag(flags::invisible))
					{
						//	特殊値処理
						set_special_power(get_special_power() + damage_data->desc.special_desc.value);
						hit_box_data->self_character->set_special_power(hit_box_data->self_character->get_special_power() + damage_data->desc.special_desc.value);

						//	体幹的に許容できない場合はガード出来ない
						int work = hit_box_data->self_character->get_trunk() - guard_damage_desc.trunk_desc.value;
						if (hit_box_data->self_character->is_flag(flags::guard) && work > 0)
						{
							damage_datas guard_damage_data;
							guard_damage_data.desc = guard_damage_desc;
							guard_damage_data.stagger_direction = damage_data->stagger_direction;

							//	体幹ダメージ処理
							if (!is_flag(flags::trunk_invisible))
							{
								int work = get_trunk() - guard_damage_desc.trunk_desc.value;
								set_trunk(work);
							}

							//	防御処理
							hit_box_data->self_character->on_guarded(guard_damage_data);

							on_guard_hit(guard_damage_data);
						}
						else
						{
							//	体力ダメージ処理
							int work = hit_box_data->self_character->get_health() - damage_data->desc.health_desc.value;
							if (!damage_data->desc.health_desc.defat_attack && work <= 0)
							{
								work = 1;
							}

							hit_box_data->self_character->set_health(work);
							if (hit_box_data->self_character->get_health() <= 0)
							{
								hit_box_data->self_character->on_deead();
							}
							else
							{
								//	よろけ中は処理しない
								if (!hit_box_data->self_character->is_flag(flags::stagger))
								{
									//	体幹ダメージ処理
									if (!hit_box_data->self_character->is_flag(flags::trunk_invisible))
									{
										work = hit_box_data->self_character->get_trunk() - damage_data->desc.trunk_desc.value;
										hit_box_data->self_character->set_trunk(work);
									}

									//	よろけ発生判定
									if (damage_data->desc.trunk_desc.stagger_attack &&
										hit_box_data->self_character->get_trunk() <= 0)
									{
										stagger_datas data;
										data.desc = damage_data->desc.trunk_desc;
										data.character = this;

										on_stagger_hit(data);
										hit_box_data->self_character->on_stagger(data);
									}
								}

								//	ダメージコールバック
								hit_box_data->self_character->on_damaged(*damage_data);
							}

							on_attack_hit(*damage_data);
						}
					}
					else if (hit_box_data->self_character->get_current_state() == state_names[state_dodge])
					{
						//	回避判定中は回避中にヒットした時のコールバックを呼び出す
						hit_box_data->self_character->on_hit_while_dodging(*damage_data);
					}
				}
			}
		}
	}
}

void character_object::on_push_box_enter(collision_object::data data)
{
	//	座標系を再設定
	if (!data.from->is_flag(collision_object::flags::detect_only))
		set_position(push_box->get_position());
}

void character_object::on_push_box_stay(collision_object::data data)
{
	//	座標系を再設定
	if (!data.from->is_flag(collision_object::flags::detect_only))
		set_position(push_box->get_position());
}

void character_object::on_root_animation_translation(const DirectX::XMFLOAT3& translation)
{
	//	Y方向は直接設定する
	DirectX::XMFLOAT3 position = get_position();
	position.y += translation.y;

	position.x += translation.x;
	position.z += translation.z;

	set_position(position);
}

void character_object::on_root_animation_rotation(const DirectX::XMFLOAT4& rotation)
{
	math::rotator rotator = rotation;
	rotator.x = get_rotation().x + rotator.x;
	rotator.y = get_rotation().y + rotator.y;
	rotator.z = get_rotation().z + rotator.z;
	set_rotation(rotator);
}

void character_object::call_camera_transforms(const camera_transform_descs& desc)
{
	for (auto& callback : on_camera_transforms)
	{
		callback.second(desc);
	}
}