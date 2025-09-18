#include "object/character/character_object.h"

namespace
{
	//	ワールド重力
	static constexpr DirectX::XMFLOAT3 world_gravity = {0.0f, system_units::convert_m_to_unit(-9.8f), 0.0f};
}

void character_object::fixed_update(float elapsed_time)
{
	//	時間遅延関係は除外して時間経過
	hit_stop_time = std::max(hit_stop_time - elapsed_time, 0.0f);
	invisile_timer = std::max(invisile_timer - elapsed_time, 0.0f);

	//	経過時間更新
	elapsed_time *= elapsed_time_scale;

	//	よろけ時間更新
	if (is_flag(flags::stagger))
	{
		stagger_recovery_time += elapsed_time;

		trunk = math::lerp(0, max_trunk, stagger_recovery_time / stagger_recovery_time_max);
		if (stagger_recovery_time >= stagger_recovery_time_max)
		{
			set_flag(flags::stagger, false);
			trunk = max_trunk;
		}
	}

	//	ヒットストップ
	get_model_object()->set_animation_speed(hit_stop_time <= FLT_EPSILON ? elapsed_time_scale : 0.0f);
	if (hit_stop_time > 0.0f)
		return;

	//	傾き破棄
	set_rotation({get_rotation().x, get_rotation().y, 0});

	//	各種フラグを落としておく
	set_flag(character_object::flags::invisible, false);
	set_flag(character_object::flags::guard, false);

	//	更新ステート呼び出し
	state_machine.update(elapsed_time);

	//	無敵処理
	if (invisile_timer > 0.0f)
	{
		set_flag(character_object::flags::invisible, true);
	}

	//	速度更新
	update_rotation(elapsed_time);
	update_vertical_velocity(elapsed_time);
	update_horizontal_velocity(elapsed_time);
	update_horizontal_move(elapsed_time);
	update_vertical_move(elapsed_time);

	//	傾き適応
	{
		set_rotation({get_rotation().x, get_rotation().y, roll_value});
		roll_value *= 0.1f;
	}

	//	各種速度関係の情報はクリアしておく
	move_vector = {};
	accelaration = {};

	for (auto& box : attack_box)
	{
		if (!box.is_follow_self)
			continue;
		box.object->set_position(get_position());
		box.object->set_rotation(get_rotation());
	}
	for (auto& box : hit_box)
	{
		if (!box.is_follow_self)
			continue;
		box.object->set_position(get_position());
		box.object->set_rotation(get_rotation());
	}
}

void character_object::post_update(float elapsed_time)
{
	//	カメラからの透過処理を起動
	if (camera_distance_translucent_inner_flag == -1 ||
		is_flag(flags::camera_distance_translucent) != static_cast<bool>(camera_distance_translucent_inner_flag))
	{
		camera_distance_translucent_inner_flag = is_flag(flags::camera_distance_translucent);
		bool flag = is_flag(flags::camera_distance_translucent);
		for (auto& material : get_model_object()->get_model_instance()->get_materials())
		{
			material.set_flag(material::flags::camera_distance_translucent, flag);
		}
		on_change_camera_translucent(flag);
	}
}

std::string_view character_object::get_current_state() const
{
	if (state_machine.get_current_state())
		return state_machine.get_current_state()->get_name();
	return "";
}

bool character_object::check_same_state(std::string_view state_name) const
{
	return get_current_state() == state_name;
//	if (get_current_state() == character_object::state_names[character_object::state_damage])
}

bool character_object::change_state(std::string_view state_id)
{
	if (state_machine.change_state(state_id))
	{
		//	変更時呼び出し
		auto it_id = on_change_states.find(std::string(state_id));
		if (it_id != on_change_states.end())
		{
			for (auto& it : it_id->second)
			{
				it.second(this);
			}
		}
		return true;
	}
	return false;
}

bool character_object::register_change_state_callback(std::string_view state_id, std::string_view identifier, call_back_change_state callback)
{
	auto it = on_change_states.find(std::string(state_id));
	if (it == on_change_states.end())
	{
		on_change_states.insert_or_assign(std::string(state_id), call_back_change_state_inner_map());
		it = on_change_states.find(std::string(state_id));
	}

	auto id = std::string(identifier);
	auto it_inner = it->second.find(id);
	if (it_inner != it->second.end())
		return false;

	it->second.insert_or_assign(id, callback);
	return true;
}

bool character_object::unregister_change_state_callback(std::string_view state_id, std::string_view identifier)
{
	auto it = on_change_states.find(std::string(state_id));
	if (it == on_change_states.end())
		return false;

	auto id = std::string(identifier);

	auto it_inner = it->second.find(id);
	if (it_inner == it->second.end())
		return false;

	it->second.erase(it_inner);
	return true;
}

void character_object::register_common_states()
{
#define REGISTER_MACRO(STATE) \
{ \
	register_state<function_finite_state_node>("state_"###STATE, \
												[this](){enter_##STATE();}, \
												[this](float elapsed_time) {update_##STATE(elapsed_time);}, \
												[this](){exit_##STATE();}); \
}
	REGISTER_MACRO(entry);
	REGISTER_MACRO(idle);
	REGISTER_MACRO(walk);
	REGISTER_MACRO(dash);
	REGISTER_MACRO(hi_dash);
	REGISTER_MACRO(move_stop);
	REGISTER_MACRO(dodge);
	REGISTER_MACRO(dodge_stop);
	REGISTER_MACRO(dodge_attack);

	REGISTER_MACRO(damage);
	REGISTER_MACRO(dead);
	REGISTER_MACRO(dead_complete);
	REGISTER_MACRO(regenerate);

	REGISTER_MACRO(stagger);
	REGISTER_MACRO(stagger_complete);

	REGISTER_MACRO(break_attack);

	REGISTER_MACRO(attack);
	REGISTER_MACRO(special);
	REGISTER_MACRO(deluxe);

	REGISTER_MACRO(guard);
	REGISTER_MACRO(parry);
	REGISTER_MACRO(parry_attack);
#undef REGISTER_MACRO
}

void character_object::update_rotation(float elapsed_time)
{
	DirectX::XMFLOAT2 vector = {turn_front_vector.x, turn_front_vector.y};
	float length = (vector.x * vector.x + vector.y * vector.y);
	if (length < 0.001f * 0.001f)
		return;

	length = sqrtf(length);
	vector.x /= length;
	vector.y /= length;

	//	前ベクトル算出
	math::rotator rotator = get_rotation();
	DirectX::XMFLOAT2 front = DirectX::XMFLOAT2(sinf(rotator.y), cosf(rotator.y));

	//	回転量算出
	float dot = front.x * vector.x + front.y * vector.y;
	float cross = front.x * vector.y - front.y * vector.x;
	float rotation = acosf(math::clamp(dot, -1.0f, 1.0f));
	float speed = rotation_speed * elapsed_time;
	if (rotation > speed)
		rotation = speed;

	rotator.y += ( cross < 0.0f ) ? rotation : -rotation;
	set_rotation(rotator);
}

void character_object::update_vertical_velocity(float elapsed_time)
{
	//	今回の更新での重力加速度
	frame_gravity.x = world_gravity.x * gravity_scale * elapsed_time;
	frame_gravity.y = world_gravity.y * gravity_scale * elapsed_time;
	frame_gravity.z = world_gravity.z * gravity_scale * elapsed_time;
	velocity.x += frame_gravity.x;
	velocity.y += frame_gravity.y;
	velocity.z += frame_gravity.z;
}

void character_object::update_vertical_move(float elapsed_time)
{
	//	初期化
	ground_friction	= 1.0f;
	slope_rate = 1.0f;
	ground_normal = {};

	//	ステップオフセット
	float step_offset = 0.0f;
	if (is_flag(flags::on_ground))
		step_offset = -step_offset * 0.5f;

	set_flag(flags::on_ground, false);

	//	座標算出
	DirectX::XMFLOAT3 position, normal;
	float fricition = 0;
	if (calculate_position(DirectX::XMFLOAT3(0.0f, velocity.y * elapsed_time + step_offset, 0.0f), true, position, normal, &fricition))
	{
		if (velocity.y < 0)
		{
			set_flag(flags::on_ground, true);
			ground_normal = normal;
			footstep_position = position;
			slope_rate = normal.y / ( sqrtf(normal.x * normal.x + normal.z * normal.z) + normal.y);
			slope_rate = 1.0f - slope_rate;
			ground_friction	= fricition;
		}
		velocity.y = 0.0f;
	}
	set_position(position);
}

void character_object::update_horizontal_velocity(float elapsed_time)
{
	//	入力フラグ
	float move_length = move_vector.x * move_vector.x + move_vector.y * move_vector.y;
	bool input_move = move_length > 0.01f * 0.01f;

	//	加速処理
	if (input_move)
	{
		//	add_force
		float acceleration_force = accelaration * elapsed_time;
		float frame_acceleration = acceleration_force / get_push_box()->get_mass();

		float move_vector_x = move_vector.x;
		float move_vector_z = move_vector.y;
		move_length = sqrtf(move_length);
		move_vector_x /= move_length;
		move_vector_z /= move_length;

		velocity.x += move_vector_x * frame_acceleration;
		velocity.z += move_vector_z * frame_acceleration;
	}

	//	摩擦処理
	if (is_flag(flags::on_ground))
	{
		//	垂直抗力
		float normal_force = fabsf(frame_gravity.y) * get_push_box()->get_mass();

		//	合成摩擦係数
		float total_friction = (ground_friction + get_push_box()->get_physical_data().friction) * 0.5f;

		//	動摩擦力
		float kinetic_friction_force = normal_force * total_friction;

		//	減衰速度
		float kinetic_acceleration = kinetic_friction_force / get_push_box()->get_mass();

		//	速度減衰
		float speed = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
		kinetic_acceleration = std::max(speed - fabsf(kinetic_acceleration), 0.0f);
		if (speed < FLT_EPSILON)
		{
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}
		else
		{
			velocity.x *= kinetic_acceleration / speed;
			velocity.z *= kinetic_acceleration / speed;
		}
	}

	//	空気抵抗 fluid
	{
		//	未対応
	}

	//	最大速度を超える時に追加減速
	float length = velocity.x * velocity.x + velocity.z * velocity.z;
	if (length > max_move_speed * max_move_speed || !input_move)
	{
		float acceleration_force = breaking_deceleration * elapsed_time;
		float frame_acceleration = acceleration_force / get_push_box()->get_mass();
		float move_vector_x = velocity.x;
		float move_vector_z = velocity.z;
		move_length = velocity.x * velocity.x + velocity.z * velocity.z;
		if (move_length <= frame_acceleration * frame_acceleration)
		{
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}
		else
		{
			move_length = sqrtf(move_length);
			move_vector_x /= move_length;
			move_vector_z /= move_length;
			float new_length = move_length - frame_acceleration;
			move_length = input_move ? std::max(max_move_speed, new_length) : new_length;
			velocity.x = move_vector_x * move_length;
			velocity.z = move_vector_z * move_length;
		}
	}

	if (is_flag(flags::on_ground) && slope_rate > 0.0f)
	{
		velocity.y -= length * slope_rate;
	}
}

void character_object::update_horizontal_move(float elapsed_time)
{
	if (velocity.x * velocity.x + velocity.z * velocity.z <= 0.001f * 0.001f)
		return;

	DirectX::XMFLOAT3 new_position, normal;
	calculate_position(DirectX::XMFLOAT3(velocity.x * elapsed_time, 0.0f, velocity.z * elapsed_time), false, new_position, normal);
	set_position(new_position);
}

void character_object::adjustment_velocity(DirectX::XMFLOAT2 move, float adjust_move_velocity_rate, bool* flip_flag)
{
	if (move.x == 0 && move.y == 0)
		return;

	float vx = get_velocity().x, vy = get_velocity().z;
	float vl = vx * vx + vy * vy;
	if (vl > FLT_EPSILON)
	{
		float mx = move.x, my = move.y;
		float ml = sqrtf(mx * mx + my * my);

		mx /= ml;
		my /= ml;

		vl = sqrtf(vl);
		vx /= vl;
		vy /= vl;

		float sin = math::clamp(vx * my - vy * mx, -1.0f, 1.0f);
		float dot = math::clamp(mx * vx + my * vy, -1.0f, 1.0f);

		//	ターン処理
		if (flip_flag && dot < -0.5f)
		{
			*flip_flag = true;

			//	ターンさせる場合は今の移動方向に進めておく
			move.x = vx;
			move.y = vy;
		}
		else
		{
			//	入力を速度方向に若干変化させる
			{
				float angle = asinf(sin) * adjust_move_velocity_rate;
				float mmx = move.x * cosf(angle) - move.y * sinf(angle);
				float mmy = move.x * sinf(angle) + move.y * cosf(angle);
				move.x = vx;
				move.y = vy;
			}

			//	入力方向に合わせて速度ベクトルの修正
			{
				if ((fabsf(sin) < 0.01f) && (dot < -0.1f))
				{
					set_velocity({ mx * vl, get_velocity().y, my * vl });
				}
				else
				{
					float angle = asinf(sin) * adjust_move_velocity_rate;
					float vvx = get_velocity().x * cosf(angle) - get_velocity().z * sinf(angle);
					float vvy = get_velocity().x * sinf(angle) + get_velocity().z * cosf(angle);
					set_velocity({ vvx, get_velocity().y, vvy });
				}

				//	速度に合わせて傾ける
				roll_value += DirectX::XMConvertToRadians(15.0f * sin);
				roll_value = math::clamp(roll_value, -DirectX::XMConvertToRadians(10.0f), DirectX::XMConvertToRadians(10.0f));
			}
		}
	}
}

bool character_object::calculate_position(DirectX::XMFLOAT3 velocity, bool vertical, DirectX::XMFLOAT3& out_position, DirectX::XMFLOAT3& out_normal, float* out_friction)
{
	auto collision_object = get_push_box();
	auto bounding_object = std::static_pointer_cast<collision::bounding_capsule>(collision_object->get_bounding_object());

	//	オフセット位置を算出
	float height = bounding_object->get_radius() + bounding_object->get_length() * 0.5f;
	DirectX::XMVECTOR Offset = DirectX::XMVectorSet(0.0f, height + step_offset * is_flag(flags::on_ground), 0.0f, 0.0f);

	//	初期座標
//	out_position = collision_object->get_position();
	out_position = get_position();

	object_manager::ray_cast_descs desc;
	if (is_flag(flags::invisible))
	{
		desc.filter = collision_filter_manager::instance()->search_filter("invisible");
	}
	else
	{
		desc.filter = collision_filter_manager::instance()->search_filter(collision_filter_name);
	}
	desc.exclude_objects.emplace_back(get_push_box());

	bool hit_flag = false;
	DirectX::XMVECTOR Velocity = DirectX::XMLoadFloat3(&velocity);
	for (size_t index = 0; index < 3; ++index)
	{
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Velocity));
		if (distance < 0.001f * 0.001f)
			break;

		static constexpr float SkinStep = 0.01f;
		distance = sqrtf(distance) + SkinStep;
		if (vertical)
		{
			if (is_flag(flags::on_ground))
				distance += step_offset;
		}

		//	進行ベクトル補正
		DirectX::XMVECTOR NormalizedVelocity = DirectX::XMVector3Normalize(Velocity);
		DirectX::XMVECTOR MoveVector = DirectX::XMVectorScale(NormalizedVelocity, distance);
		DirectX::XMVECTOR From, To;
		From = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&out_position), Offset);
		To = DirectX::XMVectorAdd(From, MoveVector);
		if (vertical)	//	レイキャスト用対応
		{
			if (velocity.y < 0)
				To = DirectX::XMVectorSubtract(To, Offset);
			else
				To = DirectX::XMVectorAdd(To, Offset);
		}

		DirectX::XMStoreFloat3(&desc.start, From);
		DirectX::XMStoreFloat3(&desc.end, To);

		//	ヒット処理
		collision::intersect_data intersect_data;
		collision_object_ptr hit_collision_object;
		if (object_manager::instance()->ray_cast(desc, intersect_data, hit_collision_object))
		{
			hit_flag = true;
			float hit_distance = intersect_data[0].length - SkinStep;

			if (vertical)
			{
				if (is_flag(flags::on_ground))
					hit_distance -= step_offset;
			}
			else
			{
				int ii = 0;
				++ii;
			}
			DirectX::XMVECTOR RemainVelocity = DirectX::XMVectorScale(NormalizedVelocity, hit_distance);
			DirectX::XMVECTOR Normal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&intersect_data[0].normal));
			DirectX::XMVECTOR Position = DirectX::XMLoadFloat3(&out_position);
			Position = DirectX::XMVectorAdd(From, RemainVelocity);
			DirectX::XMStoreFloat3(&out_position, Position);
			DirectX::XMStoreFloat3(&out_normal, Normal);

			//	摩擦係数を物理マテリアルから取得してくる
			if (out_friction)
			{
				*out_friction = hit_collision_object->get_physical_data().friction;
			}

			if (vertical)
			{
				float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&out_normal), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
				float slope_angle = acosf(math::clamp(dot, -1.0f, 1.0f));
				if (slope_angle >= slope_limit)
					break;
			}

			MoveVector = DirectX::XMVectorSubtract(To, From);
			float remain_distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(MoveVector)) - hit_distance;
			MoveVector = DirectX::XMVectorScale(DirectX::XMVector3Normalize(MoveVector), std::min(remain_distance, 0.0f));

			//	壁ずりベクトル算出
			DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(MoveVector, Normal);
			Normal = DirectX::XMVectorScale(Normal, DirectX::XMVectorGetX(Dot) * 1.005f);
			Velocity = DirectX::XMVectorSubtract(MoveVector, Normal);
		}
		else
		{
			DirectX::XMStoreFloat3(&out_position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&out_position), Velocity));
			hit_flag = false;
			break;
		}
	}
	DirectX::XMStoreFloat3(&velocity, Velocity);
	return hit_flag;
}
