#include "object/character/player/player_camera_controller_object.h"

#include "object/cinematics/cinematics_object.h"

namespace
{
	//	ヨー回転速度
	constexpr float rotation_speed = DirectX::XMConvertToRadians(90.0f);
	//	±ピッチ制限角度
	constexpr DirectX::XMFLOAT2 pitch_limit_angle = { DirectX::XMConvertToRadians(-10.0f), DirectX::XMConvertToRadians(45.0f) };
	//	開始距離
	constexpr float default_distance = system_units::convert_m_to_unit(5.0f);
	//	角度によるフォーカス位置のオフセット
	constexpr float focus_y_min_offset = -system_units::convert_m_to_unit(0.3f);
	constexpr float focus_y_max_offset = +system_units::convert_m_to_unit(0.3f);
	//	フォーカス追従制限
	constexpr float focus_limit_range_max = system_units::convert_m_to_unit(2.0f); // 基本フォーカス
	constexpr float hi_dash_focus_limit_range_max = system_units::convert_m_to_unit(4.0f); // ハイダッシュ中
	constexpr float attack_focus_limit_range_max = system_units::convert_m_to_unit(15.0f); // 攻撃中
	//	パリィ情報
	constexpr float parry_state_time = system_units::convert_fps_to_sec(60.0f);
	constexpr DirectX::XMFLOAT2 parry_first_focus_distance_time_range = { system_units::convert_fps_to_sec(0.0f), system_units::convert_fps_to_sec(15.0f) };
	constexpr DirectX::XMFLOAT2 parry_second_focus_distance_time_range = { parry_first_focus_distance_time_range.y, system_units::convert_fps_to_sec(30.0f) };
	constexpr float update_parry_focus_time = system_units::convert_fps_to_sec(0.5f);
	//	よろけ情報
	constexpr float break_effect_time = 4.0f;
	constexpr common_animation::float_animation break_effect_scaling_distance[] =
	{
		{0.00f, math::ease::type::out_elastic, 1.0f},
		{1.0f, math::ease::type::lerp, 0.55f},
	};
	constexpr common_animation::float_animation break_effect_update_focus_time[] =
	{
		{0.00f, math::ease::type::lerp, 1.0f},
		{1.0f, math::ease::type::lerp, 0.0001f},
	};

	constexpr std::string_view OnAttackHitCameraShakeId = "CAMERA_ATTACK_HIT_SHAKE_CALLBACK";
	constexpr std::string_view OnGuardHitCameraShakeId = "CAMERA_GUARD_HIT_SHAKE_CALLBACK";
	constexpr std::string_view OnDamagedCameraShakeId = "CAMERA_DAMAGED_SHAKE_CALLBACK";

	constexpr std::string_view OnCameraTransformId = "CAMERA_TRANSFORM_CALLBACK";

	constexpr std::string_view OnParryCameraId = "PARRY_CAMERA_CALLBACK";
	constexpr std::string_view OnStaggerCameraEnterId = "STAGGER_CAMERA_ENTER_CALLBACK";
	constexpr std::string_view OnStaggerCameraExitId = "STAGGER_CAMERA_EXIT_CALLBACK";

	constexpr std::string_view OnControllObjectDead = "CONTROLL_OBJECT_DEAD";
}

const math::rotator& player_camera_controller_object::get_rotation() const
{
	return camera->get_rotation();
}

void player_camera_controller_object::set_rotation(const math::rotator& rotator)
{
	camera->set_rotation(rotator);
}

const DirectX::XMFLOAT3& player_camera_controller_object::get_position() const
{
	return camera->get_position();
}

void player_camera_controller_object::set_position(const DirectX::XMFLOAT3& position)
{
	camera->set_position(position);
}

void player_camera_controller_object::set_input_purmit_flag(bool flag)
{
	input_purmit_flag = flag;
}

player_camera_controller_object::player_camera_controller_object(std::string_view name, std::shared_ptr<player_controller_object> player_controller, std::shared_ptr<camera_object> camera)
: game_object(name)
, player_controller(player_controller)
, camera(camera)

, dead_state(dead_states::none)
, state(states::free)
, sub_state(0)
, work_float(0.0f)
, input_purmit_flag(true)
, clear_performance_transition_flag(false)

, minimum_distance(system_units::convert_m_to_unit(2.0f))
, current_distance(default_distance)
, maximum_distance(system_units::convert_m_to_unit(10.0f))
, physique_distance_scale(1.0f)
, game_distance_scale(1.0f)
, game_distance_scale_inner(1.0f)
, game_distance_scale_rate(0.0f)

, disengage_direction(0, 0, 0)
, update_disengage_direction_rate(0.2f)

, focus_distance(default_distance)
, focus_limit_range(focus_limit_range_max)
, update_focus_rate_inner(0.05f)
{
	//	アニメーション登録
	register_animations();

	//	シェイク処理をキャラクターオブジェクトに設定
	player_controller->get_controll_object()->register_attack_hit_callback(OnAttackHitCameraShakeId, [this](const character_object::damage_datas& data) {on_shake(data); });
	player_controller->get_controll_object()->register_damage_callback(OnDamagedCameraShakeId, [this](const character_object::damage_datas& data) {on_shake(data); });

	//	姿勢変更命をキャラクターオブジェクトに設定
	player_controller->get_controll_object()->register_camera_transform(OnCameraTransformId, [this](const character_object::camera_transform_descs& desc) {on_camera_transform(desc);});

	//	キャラクターオブジェクトの死亡
	player_controller->get_controll_object()->register_change_state_callback(character_object::state_names[character_object::state_dead_complete], OnControllObjectDead, [this](character_object* character) {on_character_dead(character); });

	//	パリィカメラ対応
	player_controller->get_controll_object()->register_guarded_callback(OnGuardHitCameraShakeId, [this](const character_object::damage_datas& data) {on_shake(data); });
	player_controller->get_controll_object()->register_change_state_callback(character_object::state_names[character_object::state_parry], OnParryCameraId, [this](character_object* character) {on_parry_camera(character); });

	//	スタッガーカメラ対応
	player_controller->register_stagger_enter_callback(OnStaggerCameraEnterId, [this](const character_object::stagger_datas& data) {on_stagger_camera_enter(data); });
	player_controller->register_stagger_exit_callback(OnStaggerCameraExitId, [this]() {on_stagger_camera_exit(); });
}

player_camera_controller_object::~player_camera_controller_object()
{
	dead_process();
}

void player_camera_controller_object::fixed_update(float elapsed_time)
{
	//	死亡破棄処理
	if (dead_state != dead_states::none)
	{
		if (dead_state == dead_states::call)
			dead_process();
		return;
	}

	//	ちょっと面倒なのでここで対応
	switch (state)
	{
		case states::entry:
		{
			fixed_update_entry(elapsed_time);
			break;
		}
		case states::clear_performance:
		{
			fixed_update_clear_performance(elapsed_time);
			break;
		}
		default:
		{
			fixed_update_default(elapsed_time);
			break;
		}
	}
}

void player_camera_controller_object::fixed_update_default(float elapsed_time)
{
	DirectX::XMVECTOR Up{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };

	DirectX::XMFLOAT3 new_focus = player_controller->get_controll_object()->get_focus_point();
	float update_focus_rate = update_focus_rate_inner;

	//	特殊カメラ演出中の処理
	switch (state)
	{
		case states::parry:
		{
			work_float -= elapsed_time;
			if (work_float <= 0.0f)
			{
				state = states::free;
				break;
			}
			update_focus_rate = update_parry_focus_time;
			float rate = math::smoothstep(parry_first_focus_distance_time_range.x, parry_first_focus_distance_time_range.y, parry_state_time - work_float);
			rate += math::smoothstep(parry_second_focus_distance_time_range.x, parry_second_focus_distance_time_range.y, parry_state_time - work_float);

			//	距離調整
			game_distance_scale_inner = math::lerp(0.5f, 1.0f, math::saturate(rate - 1.0f));
			break;
		}
		case states::stagger:
		{
			work_float -= elapsed_time;
			if (work_float <= 0.0f)
			{
				game_distance_scale_inner = 1.0f;
				state = states::free;
				break;
			}
			float time = break_effect_time - work_float;

			//	距離調整
			std::function<void(float)> scale_func = [&](float value) {game_distance_scale_inner = value;};
			common_animation::compute_animation(time, break_effect_scaling_distance, ARRAYSIZE(break_effect_scaling_distance), scale_func);

			//	フォーカス調整
			std::function<void(float)> focus_func = [&](float value) {update_focus_rate = value;};
			common_animation::compute_animation(time, break_effect_update_focus_time, ARRAYSIZE(break_effect_update_focus_time), focus_func);
			break;
		}
	}

	//	物体との距離に応じて調整
	{
		//	ロックオンターゲットの押し合い判定とサイズ差からカメラのスケーリング値を算出
		auto taget_object = player_controller->get_controll_object()->get_lockon_taget_object().lock();
		if (taget_object)
		{
			auto& self_push_box = player_controller->get_controll_object()->get_push_box();
			auto& target_push_box = taget_object->get_push_box();
			float scale_x = target_push_box->get_local_scale().x / self_push_box->get_local_scale().x;
			float scale_y = target_push_box->get_local_scale().y / self_push_box->get_local_scale().y;
			float scale_z = target_push_box->get_local_scale().z / self_push_box->get_local_scale().z;
			float scale = std::max(scale_x, std::max(scale_y, scale_z)) - physique_distance_scale;
			scale *= update_focus_rate;
			physique_distance_scale = std::max(1.0f, physique_distance_scale + scale);
		}

		//	ゲーム側からの指定スケール
		if (game_distance_scale_inner != game_distance_scale)
		{
			float rate = (update_focus_rate != game_distance_scale_rate) ? update_focus_rate : game_distance_scale_rate;
			game_distance_scale += (game_distance_scale_inner - game_distance_scale) * rate;
			game_distance_scale_inner = 1.0f;
			game_distance_scale_rate = update_focus_rate;
		}

		//	攻撃によるスケール
		float attack_scale = 1.0f;
		if (attack_hit_distance_scale_time > 0.0f)
		{
			float rate = math::ease::out_cubic(math::saturate(attack_hit_distance_scale_time));
			attack_scale = math::lerp(1.0f, attack_hit_distance_scale, rate);
			attack_hit_distance_scale_time -= elapsed_time;
		}

		//	フォーカス距離算出
		float scale = physique_distance_scale * game_distance_scale * attack_scale;
		focus_distance = math::clamp(current_distance * scale, minimum_distance, maximum_distance);
	}

	//	フォーカス範囲の制限更新
	{
		float new_focus_range = focus_limit_range_max;

		//	ハイダッシュ中は距離制限を緩める
		if (player_controller->get_controll_object()->check_same_state(character_object::state_names[character_object::state_hi_dash]))
		{
			new_focus_range = hi_dash_focus_limit_range_max;
		}
		else if (player_controller->get_controll_object()->check_same_state(character_object::state_names[character_object::state_attack]))
		{
			new_focus_range = attack_focus_limit_range_max;
		}

		focus_limit_range += (new_focus_range - focus_limit_range) * update_focus_rate;
	}

	//	フォーカス更新
	{
		float focus_rate = update_focus_rate;
		{
			//	一時座標算出
			DirectX::XMFLOAT3 temp_focus;
			temp_focus.x = focus.x + (new_focus.x - focus.x) * update_focus_rate;
			temp_focus.y = focus.y + (new_focus.y - focus.y) * update_focus_rate;
			temp_focus.z = focus.z + (new_focus.z - focus.z) * update_focus_rate;

			//	距離制限
			DirectX::XMFLOAT3 focus_vector;
			focus_vector.x = new_focus.x - temp_focus.x;
			focus_vector.y = new_focus.y - temp_focus.y;
			focus_vector.z = new_focus.z - temp_focus.z;
			float length_sq = focus_vector.x * focus_vector.x + focus_vector.y * focus_vector.y + focus_vector.z * focus_vector.z;

			focus_rate = math::lerp(focus_rate, 1.0f, math::saturate(length_sq / (focus_limit_range * focus_limit_range)));
		}
		focus.x += (new_focus.x - focus.x) * focus_rate;
		focus.y += (new_focus.y - focus.y) * focus_rate;
		focus.z += (new_focus.z - focus.z) * focus_rate;
	}

	//	旋回処理
	{
		math::rotator camera_rotation = get_rotation();
		DirectX::XMVECTOR Rot1, Rot2;
		Rot1 = DirectX::XMLoadFloat4(&camera_rotation.convert_quaternion());
		Rot2 = DirectX::XMLoadFloat4(&disengage_direction.convert_quaternion());
		DirectX::XMFLOAT4 rotation;
		DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionSlerp(Rot1, Rot2, update_disengage_direction_rate));
		camera_rotation = rotation;
		camera_rotation.z = 0.0f;

		camera_rotation.x = math::clamp(camera_rotation.x, pitch_limit_angle.x, pitch_limit_angle.y);
		camera_rotation.y = math::normalize_angle(camera_rotation.y);
		set_rotation(camera_rotation);
	}

	//	カメラの姿勢の更新
	{
		DirectX::XMFLOAT3 camera_position, camera_front;
		math::rotator camera_rotation = get_rotation();
		camera_front = camera_rotation.get_front();

		camera_position.x = focus.x - camera_front.x * focus_distance;
		camera_position.y = focus.y - camera_front.y * focus_distance;
		camera_position.z = focus.z - camera_front.z * focus_distance;
		float min_y_offset = math::linearstep(0.0f, pitch_limit_angle.x, camera_rotation.x);
		float max_y_offset = math::linearstep(0.0f, pitch_limit_angle.y, camera_rotation.x);
		camera_position.y += min_y_offset * focus_y_min_offset + max_y_offset * focus_y_max_offset;
		//	物体と当たり判定
		{
			object_manager::ray_cast_descs desc;
			desc.filter = collision_filter_manager::instance()->search_filter("camera");
			desc.start = focus;
			desc.end = camera_position;

			//	ヒット処理
			collision::intersect_data intersect_data;
			collision_object_ptr hit_collision_object;
			if (object_manager::instance()->ray_cast(desc, intersect_data, hit_collision_object))
			{
				float length = intersect_data[0].length;
				DirectX::XMFLOAT3 vector;
				vector.x = camera_position.x - focus.x;
				vector.y = camera_position.y - focus.y;
				vector.z = camera_position.z - focus.z;
				DirectX::XMStoreFloat3(&vector, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&vector)));
				camera_position.x = focus.x + vector.x * length;
				camera_position.y = focus.y + vector.y * length;
				camera_position.z = focus.z + vector.z * length;
			}
		}
		set_rotation(camera_rotation);
		set_position(camera_position);
	}
}

void player_camera_controller_object::update(float elapsed_time)
{
	//	死亡破棄処理
	if (dead_state != dead_states::none)
	{
		if (dead_state == dead_states::call)
			dead_process();
		return;
	}

	//	ちょっと面倒なのでこれで対応
	if (!input_purmit_flag)
		return;

	//	旋回加速処理
	DirectX::XMFLOAT2 move;
	move.x = static_cast<float>(mouse::instance()->get_move_cursol().x);
	move.y = static_cast<float>(mouse::instance()->get_move_cursol().y);
	float move_length = (move.x * move.x + move.y * move.y);
	if (move_length > 0.001f * 0.001f)
	{
		disengage_direction.x += DirectX::XMConvertToRadians(move.y) * 0.1f;
		disengage_direction.y += DirectX::XMConvertToRadians(move.x) * 0.1f;

		disengage_direction.x = math::clamp(disengage_direction.x, pitch_limit_angle.x, pitch_limit_angle.y);
		disengage_direction.y = math::normalize_angle(disengage_direction.y);
	}
}

#if defined(USE_IMGUI)
#define CHECK_BOX(FLAG) \
{\
	bool is_##FLAG = is_flag(flags::##FLAG);\
	ImGui::Checkbox(#FLAG, &is_##FLAG);\
	set_flag(flags::##FLAG, is_##FLAG);\
}

void player_camera_controller_object::imgui_render_core()
{
	ImGui::Text("state = %s", NAMEOF_ENUM(state).data());
	ImGui::SliderFloat("minimum_distance", &minimum_distance, 0.01f, 100.0f);
	ImGui::SliderFloat("maximum_distance", &maximum_distance, 0.01f, 100.0f);
	ImGui::SliderFloat("focus_distance", &focus_distance, minimum_distance, maximum_distance);
	ImGui::SliderFloat("update_focus_rate", &update_focus_rate_inner, 0.01f, 1.0f);
}
#undef CHECK_BOX
#endif	//	defined(USE_IMGUI)

void player_camera_controller_object::change_state(states state)
{
	this->state = state;
	sub_state = 0;
	work_float = 0;
	clear_performance_transition_flag = false;
}

void player_camera_controller_object::register_character_object(std::shared_ptr<character_object> character)
{
	//	シェイク処理をキャラクターオブジェクトに設定
	character->register_attack_hit_callback(OnAttackHitCameraShakeId, [this](const character_object::damage_datas& data) {on_shake(data); });
	character->register_damage_callback(OnDamagedCameraShakeId, [this](const character_object::damage_datas& data) {on_shake(data); });

	//	パリィカメラ対応
	character->register_guarded_callback(OnGuardHitCameraShakeId, [this](const character_object::damage_datas& data) {on_shake(data); });
}

void player_camera_controller_object::on_shake(const character_object::damage_datas& damage_data)
{
	camera_object::shake_descs desc;
	desc.time = damage_data.desc.camera_shake_time;
	desc.power = damage_data.desc.camera_shake_power;
	camera->shake(desc);

	attack_hit_distance_scale = math::clamp(attack_hit_distance_scale + 0.1f, 1.0f, 1.15f);
	attack_hit_distance_scale_time = system_units::convert_fps_to_sec(600.0f);
}

void player_camera_controller_object::on_camera_transform(const character_object::camera_transform_descs& desc)
{
	if (desc.is_flag(character_object::camera_transform_descs::flags::set_focus_direction))
	{
		DirectX::XMVECTOR Vector = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&desc.focus_direction));
		disengage_direction.set_parameter_from_direction(system_utility::convert_xmfloat3_from_vector(Vector));
		disengage_direction.x = math::clamp(disengage_direction.x, pitch_limit_angle.x, pitch_limit_angle.y);
		disengage_direction.y = math::normalize_angle(disengage_direction.y);
	}
	if (desc.is_flag(character_object::camera_transform_descs::flags::set_focus_distance_factor))
	{
		game_distance_scale_inner = desc.focus_distance_factor;
		game_distance_scale_rate = desc.focus_distance_rate;
	}
}

void player_camera_controller_object::on_character_dead(character_object* character)
{
	//	コールバック破棄
	if (player_controller->get_controll_object() &&
		player_controller->get_controll_object().get() == character &&
		dead_state == dead_states::none)
	{
		dead_state = dead_states::call;
	}
}

void player_camera_controller_object::on_parry_camera(character_object* character)
{
	state = states::parry;
	work_float = parry_state_time;

	//	座標算出
	//	@remainder	この辺はパリィカメラポイントをキャラクター側に持たせておく
	{
		auto self_object = player_controller->get_controll_object();
		auto taget_object = player_controller->get_controll_object()->get_lockon_taget_object().lock();
		DirectX::XMVECTOR SelfPosition = DirectX::XMLoadFloat3(&self_object->get_focus_point());
		DirectX::XMVECTOR TargetPosition = DirectX::XMLoadFloat3(&taget_object->get_focus_point());
		DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&get_position());
		DirectX::XMVECTOR Vector = DirectX::XMVectorSubtract(TargetPosition, SelfPosition);

		//	左側か右側か判定
		DirectX::XMVECTOR VectorXZ = DirectX::XMVectorSetY(Vector, DirectX::XMVectorGetZ(Vector));
		DirectX::XMVECTOR CameraVector = DirectX::XMVectorSubtract(CameraPosition, SelfPosition);
		DirectX::XMVECTOR CameraVectorXZ = DirectX::XMVectorSetY(CameraVector, DirectX::XMVectorGetZ(CameraVector));
		DirectX::XMVECTOR Sin = DirectX::XMVector2Cross(VectorXZ, CameraVectorXZ);
		float left = DirectX::XMVectorGetY(Sin) < 0.0f ? -1.0f : +1.0f;

		//	Y差分は0にしておく
		Vector = DirectX::XMVectorSetY(Vector, 0.0f);

		//	正規化
		Vector = DirectX::XMVector3Normalize(Vector);

		//	右方向ベクトル算出
		DirectX::XMVECTOR Up{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };
		DirectX::XMVECTOR Right = DirectX::XMVector3Cross(Up, Vector);

		//	一定距離離す
		Vector = DirectX::XMVectorScale(Vector, system_units::convert_m_to_unit(0.5f));
		Vector = DirectX::XMVectorAdd(Vector, DirectX::XMVectorScale(Right, system_units::convert_m_to_unit(left * 0.25f)));
		Vector = DirectX::XMVectorAdd(Vector, DirectX::XMVectorScale(Up, system_units::convert_m_to_unit(0.1f)));
		Vector = DirectX::XMVector3Normalize(Vector);

		//	回転量算出
		disengage_direction.set_parameter_from_direction(system_utility::convert_xmfloat3_from_vector(Vector));
		disengage_direction.x = math::clamp(disengage_direction.x, pitch_limit_angle.x, pitch_limit_angle.y);
		disengage_direction.y = math::normalize_angle(disengage_direction.y);
	}
}

void player_camera_controller_object::on_stagger_camera_enter(const character_object::stagger_datas& data)
{
	state = states::stagger;
	work_float = break_effect_time;
	//data.desc.slow_time_delay_time;
}

void player_camera_controller_object::on_stagger_camera_exit()
{
	state = states::stagger;
	work_float = 0.0f;
	//data.desc.slow_time_delay_time;
}

void player_camera_controller_object::dead_process()
{
	if (dead_state != dead_states::call)
		return;

	player_controller->get_controll_object()->unregister_change_state_callback(character_object::state_names[character_object::state_dead_complete], OnControllObjectDead);
	player_controller->get_controll_object()->unregister_attack_hit_callback(OnAttackHitCameraShakeId);
	player_controller->get_controll_object()->unregister_damage_callback(OnDamagedCameraShakeId);
	player_controller->get_controll_object()->unregister_camera_transform(OnCameraTransformId);
	player_controller->get_controll_object()->unregister_change_state_callback(character_object::state_names[character_object::state_dead_complete], OnControllObjectDead);
	player_controller->get_controll_object()->unregister_guarded_callback(OnGuardHitCameraShakeId);
	player_controller->get_controll_object()->unregister_change_state_callback(character_object::state_names[character_object::state_parry], OnParryCameraId);
	player_controller->unregister_stagger_enter_callback(OnStaggerCameraEnterId);
	player_controller->unregister_stagger_exit_callback(OnStaggerCameraExitId);

	object_manager::instance()->unregister_object(shared_from_this());
	player_controller.reset();
	dead_state = dead_states::end;
}
