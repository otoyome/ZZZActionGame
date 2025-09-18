#include "object/cinematics/cinematics_object.h"
#include "graphics/effect_manager.h"

#include "object/ui/hud/hud_controller_object.h"

#include "object/game_constant.h"

namespace
{
	constexpr std::string_view BreakTexturePath = "Assets/Resources/texture/Breaked.png";

	constexpr std::string_view TimeSlowId = "CINEMATIC_MANAGER_TIME_SLOW_ID";
	constexpr std::string_view ParryEffectId = "CINEMATIC_MANAGER_PARRY_EFFECT_ID";
	constexpr std::string_view StaggerEffectId = "CINEMATIC_MANAGER_STAGGER_EFFECT_ID";

	constexpr std::string_view DeadStartId = "CINEMATIC_MANAGER_DEAD_START";
	constexpr std::string_view DeadEndId = "CINEMATIC_MANAGER_DEAD_END";

	//	('ω')先生、timelineが欲しいです。
	constexpr common_animation::float_animation break_effect_scaling_texts[] =
	{
		{0.00f, math::ease::type::lerp, 1.0f / 8.0f},
		{0.1f, math::ease::type::in_back, 1.0f},
	};
	constexpr common_animation::float2_animation break_effect_display_text_offsets[] =
	{
		{0.75f, math::ease::type::in_back, {0.00f, 0.00f}},
		{1.50f, math::ease::type::in_back, {0.25f, 0.00f}},
	};
	constexpr common_animation::float4_animation break_effect_display_text_colors[] =
	{
		{0.00f, math::ease::type::out_elastic, {0.25f, 0.25f, 1.0f, 1.0f}},
		{0.05f, math::ease::type::out_elastic, {0.25f, 1.0f, 1.0f, 1.0f}},
		{0.10f, math::ease::type::out_elastic, {1.0f, 0.25f, 1.0f, 1.0f}},
		{0.15f, math::ease::type::out_elastic, {0.25f, 0.25f, 1.0f, 1.0f}},
		{0.20f, math::ease::type::out_elastic, {0.25f, 1.0f, 1.0f, 1.0f}},
		{0.25f, math::ease::type::out_elastic, {1.0f, 0.25f, 1.0f, 1.0f}},
		{0.30f, math::ease::type::out_elastic, {0.25f, 0.25f, 1.0f, 1.0f}},
		{1.00f, math::ease::type::in_back, {0.0f, 0.0f, 0.0f, 0.5f}},
		{1.50f, math::ease::type::in_back, {0.0f, 0.0f, 0.0f, 0.0f}},
	};
}

cinematics_object::cinematics_object(std::string_view name, std::shared_ptr<camera_object> camera)
: game_object(name)
, state(states::scene)
, sub_state(0)
, time_control_data()
, camera(camera)
, parry_data()
, break_data()
, previous_time_scale(1.0f)
{
	post_process_volume_manager::instance()->generate_post_process_volume(post_process_volume);

	//	時間遅延
	{
		time_slow_post_process_origin_data.color_grading.intensity = 1.0f;
		time_slow_post_process_origin_data.color_grading.stage_depth_scale = 1000.0f;
		time_slow_post_process_origin_data.color_grading.stage_data.hsv_weight.y = 0.25f;
		time_slow_post_process_origin_data.color_grading.stage_data.hsv_weight.z = 0.7f;
		time_slow_post_process_origin_data.color_grading.stage_data.color_multiplyer = { 0.9f, 1.0f, 1.5f };	//	若干青色を優勢に
		time_slow_post_process_origin_data.radial_blur.radius = 50.0f;
		time_slow_post_process_origin_data.radial_blur.sampling_count = 10;
		time_slow_post_process_origin_data.radial_blur.mask_radius = 300.0f;
	}

	//	おパリィ('ω')
	{
		parry_post_process_origin_data.color_grading.intensity = 0.0f;
		parry_post_process_origin_data.radial_blur.radius = 1000.0f;
		parry_post_process_origin_data.radial_blur.sampling_count = 10;
		parry_post_process_origin_data.radial_blur.mask_radius = 500.0f;
		parry_post_process_origin_data.separate_color.intensity = 1.0f;
		parry_post_process_origin_data.separate_color.color_max = {1.0f, 1.0f, 1.0f};
		parry_post_process_origin_data.separate_color.color_min = {0.0f, 0.0f, 0.0f};
		parry_post_process_origin_data.separate_color.threshold = 0.15f;
		//parry_post_process_origin_data.distortion_data.power = 600;
		//parry_post_process_origin_data.distortion_data.speed = 1.0f;
		//parry_post_process_origin_data.distortion_data.amount = 1.0f;
		//parry_post_process_origin_data.sin_distortion_datas[0].height_angular_frequency = 60.0f;
		//parry_post_process_origin_data.sin_distortion_datas[0].height_amplitude = 0020.0f / 1280.0f;
		//parry_post_process_origin_data.sin_distortion_datas[0].height_postraize_count = 0;
		//parry_post_process_origin_data.sin_distortion_datas[0].amount = 0.2f;
		//parry_post_process_origin_data.sin_distortion_datas[1].height_angular_frequency = 1.0f;
		//parry_post_process_origin_data.sin_distortion_datas[1].height_amplitude = 0010.0f / 1280.0f;
		//parry_post_process_origin_data.sin_distortion_datas[1].height_postraize_count = 0;
		//parry_post_process_origin_data.sin_distortion_datas[1].amount = 0.2f;
	}

	//	ブレイク
	{
		break_post_process_origin_data.color_grading.intensity = 1.0f;
		break_post_process_origin_data.color_grading.stage_depth_scale = 1000.0f;
		break_post_process_origin_data.color_grading.stage_data.hsv_weight.y = 0.25f;
		break_post_process_origin_data.color_grading.stage_data.hsv_weight.z = 0.7f;
		break_post_process_origin_data.color_grading.stage_data.color_multiplyer = { 0.9f, 1.0f, 1.5f };	//	若干青色を優勢に
		break_post_process_origin_data.radial_blur.radius = 200.0f;
		break_post_process_origin_data.radial_blur.sampling_count = 10;
		break_post_process_origin_data.radial_blur.mask_radius = 300.0f;
		break_post_process_origin_data.separate_color.intensity = 0.0f;
		break_post_process_origin_data.overwrite_text.intensity = 1.0f;
		break_post_process_origin_data.overwrite_text.texcoord_scale = 1.0f;
		break_post_process_origin_data.overwrite_text.color = {0.5f, 0.5f, 0.5f, 1.0f};
	}

	texture_manager::instance()->load_texture_from_file(breaked_texture, BreakTexturePath);
}

cinematics_object::~cinematics_object()
{
}

void cinematics_object::fixed_update(float elapsed_time)
{
	switch (state)
	{
		case states::scene:
		{
			update_scene(elapsed_time);
			break;
		}
		case states::external:
		{
			update_external(elapsed_time);
			break;
		}
	}
}

void cinematics_object::pre_render()
{
	//	ポストプロセス
	post_process_datas post_process_data;

	switch (state)
	{
		case states::scene:
		{
			//	時間遅延
			if (time_control_data.time > 0.0f)
			{
				post_process_data = time_slow_post_process_origin_data;

				float factor = math::saturate(time_control_data.time * 4);
				post_process_data.color_grading.character_data.hsv_weight.y = math::lerp(1.0f, post_process_data.color_grading.character_data.hsv_weight.y, factor);
				post_process_data.color_grading.stage_data.hsv_weight.y = math::lerp(1.0f, post_process_data.color_grading.stage_data.hsv_weight.y, factor);
				post_process_data.radial_blur.radius = math::lerp(1.0f, post_process_data.radial_blur.radius, factor);
			}

			//	おパリィ演出
			if (parry_data.time > 0.0f)
			{
				post_process_data = parry_post_process_origin_data;

				float factor = math::saturate(parry_data.time);
				post_process_data.radial_blur.radius = math::lerp(1.0f, post_process_data.radial_blur.radius, factor);

				post_process_data.separate_color.intensity = parry_data.color_separate_time > system_units::convert_fps_to_sec(0.5f);
				post_process_data.distortion_data.amount = math::lerp(1.0f, post_process_data.distortion_data.amount, factor);
			}

			//	ブレイク演出
			if (break_data.time > 0.0f)
			{
				float time = game_constants::break_effect_time - break_data.time;
				post_process_data = break_post_process_origin_data;
				//	ラジアルブラー
				{
					float factor = math::saturate(break_data.time * 4);
					post_process_data.radial_blur.radius = math::lerp(1.0f, post_process_data.radial_blur.radius, factor);
				}

				//	文字サイズ調整
				std::function<void(float)> texcoord_scale_func = [&](float value)
				{
					post_process_data.overwrite_text.texcoord_scale = value;
				};
				common_animation::compute_animation(time, break_effect_scaling_texts, ARRAYSIZE(break_effect_scaling_texts), texcoord_scale_func);

				//	文字移動調整
				std::function<void(DirectX::XMFLOAT2)> texcoord_offset_func = [&](DirectX::XMFLOAT2 value)
				{
					post_process_data.overwrite_text.texcoord_offset.x = value.x;
					post_process_data.overwrite_text.texcoord_offset.y = value.y;
				};
				common_animation::compute_animation(time, break_effect_display_text_offsets, ARRAYSIZE(break_effect_display_text_offsets), texcoord_offset_func);

				//	文字の透過値調整
				std::function<void(DirectX::XMFLOAT4)> text_color_func = [&](DirectX::XMFLOAT4 value)
				{
					post_process_data.overwrite_text.color.x *= value.x;
					post_process_data.overwrite_text.color.y *= value.y;
					post_process_data.overwrite_text.color.z *= value.z;
					post_process_data.overwrite_text.color.w *= value.w;
				};
				common_animation::compute_animation(time, break_effect_display_text_colors, ARRAYSIZE(break_effect_display_text_colors), text_color_func);

				post_process_data.overwrite_text.intensity = 1.0f;
				post_process_data.overwrite_text_texture = breaked_texture;
			}

			break;
		}
		case states::external:
		{
			post_process_data = external_post_process_data;
			break;
		}
	}
	//	適応
	post_process_volume->set_post_process_data(post_process_data);
}

#if defined(USE_IMGUI)
void cinematics_object::imgui_render_core()
{
	auto func = [&](post_process_datas& data)
	{
		ImGui::SliderFloat("color_grading : intensity", &data.color_grading.intensity, 0.0f, 1.0f);
		ImGui::SliderFloat("color_grading : stage_depth_scale", &data.color_grading.stage_depth_scale, 0.0f, 1000.0f);
		ImGui::Separator();
		ImGui::SliderFloat3("color_grading : character_data : hsv_weight", &data.color_grading.character_data.hsv_weight.x, -2.0f, 2.0f);
		ImGui::SliderFloat3("color_grading : character_data : color_multiplyer", &data.color_grading.character_data.color_multiplyer.x, 0.0f, 2.0f);
		ImGui::SliderFloat3("color_grading : stage_data : hsv_weight", &data.color_grading.stage_data.hsv_weight.x, -2.0f, 2.0f);
		ImGui::SliderFloat3("color_grading : stage_data : color_multiplyer", &data.color_grading.stage_data.color_multiplyer.x, 0.0f, 2.0f);
		ImGui::Separator();
		ImGui::SliderInt("radial_blur : intensity", &data.radial_blur.sampling_count, 0, 50);
		ImGui::SliderFloat("radial_blur : radius", &data.radial_blur.radius, 0.0f, 1000.0f);
		ImGui::SliderFloat("radial_blur : mask_radius", &data.radial_blur.mask_radius, 0.0f, 1000.0f);
		ImGui::Separator();
		ImGui::SliderFloat("separate_color : intensity", &data.separate_color.intensity, 0.0f, 1.0f);
		ImGui::SliderFloat3("separate_color : color_min", &data.separate_color.color_min.x, 0.0f, 1.0f);
		ImGui::SliderFloat3("separate_color : color_max", &data.separate_color.color_max.x, 0.0f, 1.0f);

		//	その他は後で
	};
	int ididid = 0x446832;
	if (ImGui::TreeNode("time slow"))
	{
		ImGui::PushID(++ididid);
		func(time_slow_post_process_origin_data);

		if (ImGui::Button("test"))
		{
			character_object::time_controll_datas data;
			data.time = 5.0f;
			data.scale = 0.2f;
			on_slow(data);
		}
		ImGui::TreePop();
		ImGui::PopID();
	}
	if (ImGui::TreeNode("parry"))
	{
		ImGui::PushID(++ididid);
		func(parry_post_process_origin_data);

		if (ImGui::Button("test"))
		{
			character_object::damage_datas data;
			data.desc.hit_stop_time = 5.0f;
			on_parry(data);
		}
		ImGui::TreePop();
		ImGui::PopID();
	}
	if (ImGui::TreeNode("break"))
	{
		ImGui::PushID(++ididid);
		func(break_post_process_origin_data);

		if (ImGui::Button("test"))
		{
			character_object::stagger_datas data;
			on_break_enter(data);
		}
		ImGui::PopID();
		ImGui::TreePop();
	}
}
#endif	//	defined(USE_IMGUI)

void cinematics_object::update_scene(float elapsed_time)
{
	float current_time_scale = 1.0f;

	//	時間遅延
	if (time_control_data.time > 0.0f)
	{
		//	オブジェクトの動作を遅くする
		time_control_data.time = std::max(time_control_data.time - elapsed_time, 0.0f);

		current_time_scale = math::lerp(1.0f, time_control_data.scale, math::saturate(time_control_data.time));
	}

	//	おパリィ演出
	if (parry_data.time > 0.0f)
	{
		parry_data.time = std::max(parry_data.time - elapsed_time, 0.0f);
		parry_data.color_separate_time = std::max(parry_data.color_separate_time - elapsed_time, 0.0f);
	}

	//	ブレイク演出
	if (break_data.time > 0.0f)
	{
		break_data.time = std::max(break_data.time - elapsed_time, 0.0f);
		break_data.slow_time_delay_time = std::max(break_data.slow_time_delay_time - elapsed_time, 0.0f);
		float factor = math::saturate(break_data.time);
		factor = math::lerp(factor, 0.0f, math::step(FLT_EPSILON, break_data.slow_time_delay_time));
		factor = math::lerp(1.0f, break_data.scale, factor);
		if (factor < current_time_scale)
			current_time_scale = factor;
	}

	//	時間スケール
	if (previous_time_scale != current_time_scale)
	{
		previous_time_scale = current_time_scale;

		auto objects = object_manager::instance()->search_game_objects<character_object>();
		for (auto& object : objects)
		{
			auto character = std::static_pointer_cast<character_object>(object);
			character->set_elapsed_time_scale(current_time_scale);
		}
		effect_manager::instance()->set_elapsed_time_scale(current_time_scale);
	}

	//	時間スケール除外オブジェクトだけは別扱い
	{
		auto it = exclude_objects.begin();
		while (it != exclude_objects.end())
		{
			it->time -= elapsed_time;
			if (it->time <= 0.0f)
			{
				it = exclude_objects.erase(it);
				continue;
			}

			auto object = it->object.lock();
			if (!object)
			{
				it = exclude_objects.erase(it);
				continue;
			}

			auto character = std::static_pointer_cast<character_object>(object);
			float time_scale = math::lerp(1.0f, it->time_scale, math::saturate(it->time));
			character->set_elapsed_time_scale(time_scale);
			++it;
		}
	}
}

void cinematics_object::update_external(float elapsed_time)
{
	auto objects = object_manager::instance()->search_game_objects<character_object>();
	for (auto& object : objects)
	{
		auto character = std::static_pointer_cast<character_object>(object);
		character->set_elapsed_time_scale(external_time_scale);
	}
	effect_manager::instance()->set_elapsed_time_scale(external_time_scale);
}

void cinematics_object::on_active()
{
	post_process_volume->set_flag(post_process_volume::flags::active, true);
}

void cinematics_object::on_deactive()
{
	post_process_volume->set_flag(post_process_volume::flags::active, false);
}

void cinematics_object::register_effects_function(std::shared_ptr<character_object> character)
{
	character->register_time_controll(TimeSlowId, [this](const character_object::time_controll_datas& desc){on_slow(desc);});
	character->register_guarded_callback(ParryEffectId, [this](const character_object::damage_datas& desc) {on_parry(desc); });
}

void cinematics_object::register_effects_function(std::shared_ptr<player_controller_object> controller)
{
	register_effects_function(controller->get_controll_object());

	controller->register_stagger_enter_callback(StaggerEffectId, [this](const character_object::stagger_datas& data) {on_break_enter(data); });
	controller->register_stagger_exit_callback(StaggerEffectId, [this]() {on_break_exit(); });

	controller->register_dead_start_callback(DeadStartId, [this]()
		{
			external_setting_mode(true);
			auto data = get_external_post_process_data();
			data.color_grading.intensity = 1.0f;
			data.color_grading.stage_depth_scale = 100000.0f;
			data.color_grading.character_data.hsv_weight.y = 0;
			data.color_grading.stage_data.hsv_weight.y = 0;
			set_external_post_process_data(data);
		});

	controller->register_select_regenerate_callback(DeadEndId, [this]()
		{
			external_setting_mode(false);
		});
}

void cinematics_object::external_setting_mode(bool flag)
{
	state = flag ? states::external : states::scene;
	external_post_process_data = {};
	external_time_scale = 1.0f;
	previous_time_scale = -10000.0f;
}

bool cinematics_object::is_external_setting_mode() const
{
	return state == states::external;
}

const post_process_datas& cinematics_object::get_external_post_process_data() const
{
	return external_post_process_data;
}

void cinematics_object::set_external_post_process_data(const post_process_datas& data)
{
	 external_post_process_data = data;
}

void cinematics_object::set_external_time_scale(float scale)
{
	external_time_scale = scale;
}

void cinematics_object::on_slow(const character_object::time_controll_datas& data)
{
	time_control_data.time = data.time;
	time_control_data.scale = data.scale;

#if 01
	//	破棄するように変更
	exclude_objects.clear();
#else
	//	除外オブジェクトの情報を更新
	auto it = std::find_if(exclude_objects.begin(), exclude_objects.end(), [&](exclude_object_datas& data) {return data.object.lock() == data.object.lock(); });
	if (it != exclude_objects.end())
	{
		it->time = data.exclude_data.time;
		it->time_scale = data.scale;
		return;
	}
#endif

	exclude_object_datas exclude_object;
	exclude_object.object = data.exclude_data.object;
	exclude_object.time = data.exclude_data.time;
	exclude_object.time_scale = data.scale;
	exclude_objects.emplace_back(exclude_object);
}

void cinematics_object::on_parry(const character_object::damage_datas& data)
{
	parry_data.time = data.desc.hit_stop_time;
	parry_data.color_separate_time = system_units::convert_fps_to_sec(7.0f);
}

void cinematics_object::on_break_enter(const character_object::stagger_datas& data)
{
	break_data.time = game_constants::break_effect_time;
	break_data.scale = game_constants::break_effect_time_scale;
	break_data.slow_time_delay_time = data.desc.slow_time_delay_time;
}

void cinematics_object::on_break_exit()
{
	break_data.time = 0.5f;
	break_data.scale = game_constants::break_effect_time_scale;
}


