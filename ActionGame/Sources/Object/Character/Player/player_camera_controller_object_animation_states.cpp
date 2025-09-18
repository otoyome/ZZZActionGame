#include "object/character/player/player_camera_controller_object.h"

#include "object/cinematics/cinematics_object.h"

//	入場演出情報
namespace entry_performance
{
	constexpr std::string_view animation_name = "EntryPerformance";
}

//	クリア演出情報
namespace clear_performance
{
	constexpr std::string_view common_animation_names[] =
	{
		"ClearPerformanceCommonStart",
		"ClearPerformanceCut2",
		"ClearPerformanceCut3",
		"ClearPerformanceCommonEnd",
	};

	//	揺らぎ
	struct distortion_animation_data
	{
		float time;
		float power;
		float speed;
		float value;
	};
	constexpr std::string_view distortion_animation_identifier = NAMEOF_TYPE(distortion_animation_data);

	//	強めの揺らぎ
	struct sin_distortion_animation_data
	{
		float time;
		float height_angular_frequency;
		float height_amplitude;
		int height_postraize_count;
		float value;
	};
	constexpr std::string_view sin_distortion_animation_identifier = NAMEOF_TYPE(sin_distortion_animation_data);

	//	ボロノイノイズによる揺らぎ
	struct distortion_voronoi_animation_data
	{
		float time;
		int square_count;
		float value;
	};
	constexpr std::string_view distortion_voronoi_animation_identifier = NAMEOF_TYPE(distortion_voronoi_animation_data);

	//	キャラクターとエフェクトのアニメーション時間スケール
	struct character_and_effect_time_scale_animation_data
	{
		float time;
		float value;
	};
	constexpr std::string_view character_and_effect_time_scale_animation_identifier = NAMEOF_TYPE(character_and_effect_time_scale_animation_data);

	//	色調補正
	struct color_grading_animation_data
	{
		float time;
		postprocess::color_grading_data data;
	};
	constexpr std::string_view color_grading_animation_identifier = NAMEOF_TYPE(color_grading_animation_data);

	//	上書き文字のサイズ補正
	struct overwrite_text_scale_animation_data
	{
		float time;
		float value;
	};
	constexpr std::string_view overwrite_text_scale_animation_identifier = NAMEOF_TYPE(overwrite_text_scale_animation_data);

	//	文字列色の適応
	struct texts_wave_posterize_animation_data
	{
		float time;

		DirectX::XMFLOAT4 color;

		float posterize_step;
		float posterize_animation_rate;
		float posterize_amount;
	};
	constexpr std::string_view texts_wave_posterize_animation_identifier = NAMEOF_TYPE(texts_wave_posterize_animation_data);

	//	汎用開始アニメーション
	namespace common_start_cut_animations
	{
		constexpr animation::camera_spherical camera[] =
		{
			{
				0.0f,
				system_units::convert_m_to_unit(4.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 0, 0}),
				{0.0f, 0.0f, 0.0f},
			},
			{
				0.25f,
				system_units::convert_m_to_unit(2.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 0, 0}),
				{0.0f, DirectX::XM_PIDIV2 / 4, 0.0f},
			},
			{
				0.30f,
				system_units::convert_m_to_unit(2.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 0, 0}),
				{0.0f, DirectX::XM_PIDIV2 / 2, 0.0f},
			},
			{
				1.50f,
				system_units::convert_m_to_unit(2.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 0, 0}),
				{0.0f, DirectX::XM_PIDIV2 / 2, 0.0f},
			},
		};

		constexpr distortion_animation_data distortion[] =
		{
			{0.0f, 600, 1.0f, 1.0f},
			{0.2f, 300, 1.0f, 1.0f},
			{0.5f, 60, 1.0f, 0.0f},

			{0.699f, 300, 1.0f, 0.0f},
			{0.7f, 300, 1.0f, 1.0f},
			{0.75f, 280, 1.0f, 0.0f},

			{1.199f, 300, 1.0f, 0.0f},
			{1.2f, 300, 1.0f, 0.25f},
			{1.25f, 300, 1.0f, 0.0f},

			{1.299f, 300, 1.0f, 0.0f},
			{1.3f, 300, 1.0f, 0.15f},
			{1.35f, 300, 1.0f, 0.0f},

			{1.5f, 300, 1.0f, 0.0f},
		};

		sin_distortion_animation_data sin_distortion[] =
		{
			{  0.0f, 60.0f, 0060.0f / 1280.0f, 0, 1.0f},
			{  0.3f, 60.0f, 0060.0f / 1280.0f, 0, 0.0f},
		};

		sin_distortion_animation_data sin_distortion_big[] =
		{
			{  0.0f, 1.0f, 0180.0f / 1280.0f, 0, 1.0f},
			{  0.3f, 1.0f, 0180.0f / 1280.0f, 0, 0.0f},
		};

		distortion_voronoi_animation_data distortion_voronoi[] =
		{
			{0.0f, 50, 2.0f},
			{0.5f, 50, 0.0f},
		};

		character_and_effect_time_scale_animation_data character_and_effect_time_scale[] =
		{
			{0.0f, 1.0f},
			{0.5f, 0.0f},
		};

		color_grading_animation_data color_grading[] =
		{
			{0.0f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{0.1f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{0.2f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{0.3f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{0.35f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{0.4f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{0.45f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{0.475f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{0.5f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{0.7f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,1,1}, 0, {1, 0.376f, 0}, 0}}},

			{1.4749f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,1,1}, 0, {1, 0.376f, 0}, 0}}},
			{1.475f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{1.5f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,1,1}, 0, {1, 0.376f, 0}, 0}}},
		};

		overwrite_text_scale_animation_data overwrite_text_scale[] =
		{
			{0.0f, 1.0f / 0.5f},
			{0.1f, 1.0f / 1.0f},
		};

		texts_wave_posterize_animation_data texts_wave_posterize[] =
		{
			{0.000f, {0.25f, 0.25f, 0.25f, 0.05f}, 3, +0.0f, 1.0f},
			{0.500f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +0.0f, 1.0f},
			{1.500f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
		};
	}

	//	カット2
	namespace cut2_animations
	{
		constexpr animation::camera_spherical camera[] =
		{
			{
				0.0f,
				system_units::convert_m_to_unit(5.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 1.5f, 0}),
				{0.0f, -DirectX::XM_PIDIV2, 0.0f},
			},
			{
				1.50f,
				system_units::convert_m_to_unit(5.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 1.5f, 0}),
				{0.0f, -DirectX::XM_PIDIV2, 0.0f},
			},
		};

		constexpr distortion_animation_data distortion[] =
		{
			{0.00f, 300, 1.0f, 0.0f},
			{0.10f, 300, 1.0f, 0.15f},
			{0.15f, 300, 1.0f, 0.0f},

			{0.69f, 300, 1.0f, 0.0f},
			{0.70f, 300, 1.0f, 0.15f},
			{0.75f, 300, 1.0f, 0.0f},

			{0.79f, 300, 1.0f, 0.0f},
			{0.80f, 300, 1.0f, 0.25f},
			{0.85f, 300, 1.0f, 0.0f},

			{1.19f, 300, 1.0f, 0.0f},
			{1.2f, 300, 1.0f, 0.15f},
			{1.25f, 300, 1.0f, 0.0f},

			{1.39f, 300, 1.0f, 0.0f},
			{1.4f, 300, 1.0f, 0.25f},
			{1.45f, 300, 1.0f, 0.0f},

			{1.5f, 300, 1.0f, 0.0f},
		};

		color_grading_animation_data color_grading[] =
		{
			{1.349f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,1,1}, 0, {1, 0.376f, 0}, 0}}},
			{1.345f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{1.5f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,1,1}, 0, {1, 0.376f, 0}, 0}}},
		};

		texts_wave_posterize_animation_data texts_wave_posterize[] =
		{
			{0.00f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{0.29f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{0.30f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 0.0f},
			{0.34f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 0.0f},
			{0.35f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},

			{1.29f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{1.30f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 0.0f},
			{1.34f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 0.0f},
			{1.35f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 1.0f},
		};
	}

	//	カット3
	namespace cut3_animations
	{
		constexpr animation::camera_spherical camera[] =
		{
			{
				0.0f,
				system_units::convert_m_to_unit(8.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 1.5f, 0}),
				{-0.3f, 0.5f, 0.0f},
			},
			{
				1.5f,
				system_units::convert_m_to_unit(8.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 1.5f, 0}),
				{-0.3f, 0.5f + 0.05f, 0.0f},
			},
		};

		constexpr distortion_animation_data distortion[] =
		{
			{0.0f, 300, 1.0f, 0.0f},
			{0.2f, 300, 1.0f, 0.15f},
			{0.25f, 300, 1.0f, 0.0f},

			{0.29f, 300, 1.0f, 0.0f},
			{0.3f, 300, 1.0f, 0.25f},
			{0.35f, 300, 1.0f, 0.0f},

			{0.69f, 300, 1.0f, 0.0f},
			{0.7f, 300, 1.0f, 0.15f},
			{0.75f, 300, 1.0f, 0.0f},

			{0.89f, 300, 1.0f, 0.0f},
			{0.9f, 300, 1.0f, 0.3f},
			{0.95f, 300, 1.0f, 0.0f},

			{1.19f, 300, 1.0f, 0.0f},
			{1.2f, 300, 1.0f, 0.4f},
			{1.25f, 300, 1.0f, 0.0f},

			{1.29f, 300, 1.0f, 0.0f},
			{1.3f, 300, 1.0f, 0.25f},
			{1.35f, 300, 1.0f, 0.0f},

			{1.5f, 300, 1.0f, 0.0f},
		};

		color_grading_animation_data color_grading[] =
		{
			{1.349f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,1,1}, 0, {1, 0.376f, 0}, 0}}},
			{1.345f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
			{1.5f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,1,1}, 0, {1, 0.376f, 0}, 0}}},
		};

		texts_wave_posterize_animation_data texts_wave_posterize[] =
		{
			{0.00f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{0.99f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{1.00f, {0.02f, 0.02f, 0.02f, 0.05f}, 5, +1.0f, 0.0f},
			{1.04f, {0.02f, 0.02f, 0.02f, 0.05f}, 5, +1.0f, 0.0f},
			{1.05f, {0.02f, 0.02f, 0.02f, 0.05f}, 5, +1.0f, 1.0f},

			{1.19f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{1.20f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 0.0f},
			{1.24f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 0.0f},
			{1.25f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 1.0f},
		};
	}

	//	汎用終了カット
	namespace common_end_cut_animations
	{
		constexpr animation::camera_spherical camera[] =
		{
			{
				0.0f,
				system_units::convert_m_to_unit(20.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 0, 0}),
				{0.0f, DirectX::XM_PIDIV2, 0.0f},
			},
			{
				2.0f,
				system_units::convert_m_to_unit(18.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 0, 0}),
				{0.0f, DirectX::XM_PIDIV2, 0.0f},
			},
			{
				3.0f,
				system_units::convert_m_to_unit(0.0f), 0.0f, true,
				system_units::convert_m_to_unit({0, 0, 0}),
				{0.0f, DirectX::XM_PIDIV2, 0.0f},
			},
		};

		constexpr distortion_animation_data distortion[] =
		{
			{0.0f, 300, 1.0f, 0.0f},
			{0.49f, 300, 1.0f, 0.0f},
			{0.5f, 1200, 1.0f, 0.5f},
			{0.55f, 300, 1.0f, 0.0f},

			{0.59f, 300, 1.0f, 0.0f},
			{0.6f, 2000, 1.0f, 0.5f},
			{0.65f, 300, 1.0f, 0.0f},

			{0.79f, 300, 1.0f, 0.0f},
			{0.8f, 5300, 1.0f, 0.5f},
			{1.05f, 1280, 1.0f, 0.0f},

			{1.09f, 12800, 1.0f, 0.0f},
			{1.1f, 5300, 1.0f, 0.5f},
			{10.25f, 128000, 1.0f, 1.0f},
		};

		color_grading_animation_data color_grading[] =
		{
			{0.0f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,1,1}, 0, {1, 0.376f, 0}, 0}}},
			{1.0f, {1.0f, 10.0f, {0,0}, {{1,0,1}, 0, {1,1,1}, 0}, {{1,0,1}, 0, {1,1,1}, 0}}},
		};

		overwrite_text_scale_animation_data overwrite_text_scale[] =
		{
			{0.0f, 1.0f / 1.0f},
			{2.0f, 1.0f / 1.5f},
			{3.0f, 1.0f / 12.0f},
		};

		texts_wave_posterize_animation_data texts_wave_posterize[] =
		{
			{0.00f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{0.99f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{1.00f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 0.0f},
			{1.04f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 0.0f},
			{1.05f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},

			{1.10f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 0.0f},
			{1.14f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 0.0f},
			{1.15f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{1.19f - 0.5f, {0.02f, 0.02f, 0.02f, 0.05f}, 5, +1.0f, 1.0f},

			{1.20f - 0.5f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 0.0f},
			{1.24f - 0.5f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 0.0f},
			{1.25f - 0.5f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 1.0f},
			{1.29f - 0.5f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 1.0f},

			{1.30f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 0.0f},
			{1.34f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 0.0f},
			{1.35f - 0.5f, {0.25f, 0.25f, 0.25f, 0.05f}, 5, +1.0f, 1.0f},
			{1.39f - 0.5f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 1.0f},

			{1.40f - 0.5f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 0.0f},
			{1.44f - 0.5f, {0.02f, 0.02f, 0.02f, 0.75f}, 5, +1.0f, 0.0f},
			{1.45f - 0.5f, {0.05f, 0.05f, 0.05f, 0.75f}, 5, +1.0f, 0.0f},
		};
	}

	constexpr float event_transition_frame = 2.0f;
	constexpr std::string_view event_transition_id = "Transition Event";

	animation::camera_spherical calculate_camera_animation(const animation::camera_spherical* animations, size_t size, float time)
	{
		animation::camera_spherical camera;
		camera.time = time;

		float frame = time;
		size_t index = 0;
		for (; index < size - 1; ++index)
		{
			auto& animation_1 = animations[index + 0];
			auto& animation_2 = animations[index + 1];
			float value = (frame - animation_1.time) / (animation_2.time - animation_1.time);
			if (0.0f <= value && value < 1.0f)
			{
				camera.distance = math::lerp(animation_1.distance, animation_2.distance, value);
				camera.focus = math::lerp(animation_1.focus, animation_2.focus, value);

				camera.is_parth = math::lerp(animation_1.is_parth, animation_2.is_parth, value);
				camera.rotation = math::lerp(animation_1.rotation, animation_2.rotation, value);

				camera.view_angle = DirectX::XMConvertToRadians(static_cast<float>(math::lerp(animation_1.view_angle, animation_2.view_angle, value)));
				return camera;
			}
		}

		camera.distance = animations[index].distance;
		camera.focus = animations[index].focus;

		camera.is_parth = animations[index].is_parth;
		camera.rotation = animations[index].rotation;

		camera.view_angle = DirectX::XMConvertToRadians(static_cast<float>(animations[index].view_angle));
		return camera;
	};

	animation::variable calculate_variable_animation(const void* row_animations, size_t size, std::string_view identifier, float time)
	{
		animation::variable variable;
		variable.time = time;
		size_t index = 0;
		if (identifier == distortion_animation_identifier)
		{
			variable.buffer.resize(sizeof(distortion_animation_data));

			distortion_animation_data* animation_data = reinterpret_cast<distortion_animation_data*>(variable.buffer.data());
			const distortion_animation_data* animations = reinterpret_cast<const distortion_animation_data*>(row_animations);
			for (; index < size - 1; ++index)
			{
				auto& animation_1 = animations[index + 0];
				auto& animation_2 = animations[index + 1];
				float value = (time - animation_1.time) / (animation_2.time - animation_1.time);
				if (0.0f <= value && value < 1.0f)
				{
					animation_data->power = math::lerp(animation_1.power, animation_2.power, value);
					animation_data->speed = math::lerp(animation_1.speed, animation_2.speed, value);
					animation_data->value = math::lerp(animation_1.value, animation_2.value, value);
					return variable;
				}
			}
			animation_data->power = animations[index].power;
			animation_data->speed = animations[index].speed;
			animation_data->value = animations[index].value;
		}
		if (identifier == std::string(sin_distortion_animation_identifier) + "0")
		{
			variable.buffer.resize(sizeof(sin_distortion_animation_data));

			sin_distortion_animation_data* animation_data = reinterpret_cast<sin_distortion_animation_data*>(variable.buffer.data());
			const sin_distortion_animation_data* animations = reinterpret_cast<const sin_distortion_animation_data*>(row_animations);
			for (; index < size - 1; ++index)
			{
				auto& animation_1 = animations[index + 0];
				auto& animation_2 = animations[index + 1];
				float value = (time - animation_1.time) / (animation_2.time - animation_1.time);
				if (0.0f <= value && value < 1.0f)
				{
					animation_data->height_amplitude = math::lerp(animation_1.height_amplitude, animation_2.height_amplitude, value);
					animation_data->height_angular_frequency = math::lerp(animation_1.height_angular_frequency, animation_2.height_angular_frequency, value);
					animation_data->height_postraize_count = math::lerp(animation_1.height_postraize_count, animation_2.height_postraize_count, value);
					animation_data->value = math::lerp(animation_1.value, animation_2.value, value);
					return variable;
				}
			}
			animation_data->height_amplitude = animations[index].height_amplitude;
			animation_data->height_angular_frequency = animations[index].height_angular_frequency;
			animation_data->height_postraize_count = animations[index].height_postraize_count;
			animation_data->value = animations[index].value;
		}
		if (identifier == std::string(sin_distortion_animation_identifier) + "1")
		{
			variable.buffer.resize(sizeof(sin_distortion_animation_data));

			sin_distortion_animation_data* animation_data = reinterpret_cast<sin_distortion_animation_data*>(variable.buffer.data());
			const sin_distortion_animation_data* animations = reinterpret_cast<const sin_distortion_animation_data*>(row_animations);
			for (; index < size - 1; ++index)
			{
				auto& animation_1 = animations[index + 0];
				auto& animation_2 = animations[index + 1];
				float value = (time - animation_1.time) / (animation_2.time - animation_1.time);
				if (0.0f <= value && value < 1.0f)
				{
					animation_data->height_amplitude = math::lerp(animation_1.height_amplitude, animation_2.height_amplitude, value);
					animation_data->height_angular_frequency = math::lerp(animation_1.height_angular_frequency, animation_2.height_angular_frequency, value);
					animation_data->height_postraize_count = math::lerp(animation_1.height_postraize_count, animation_2.height_postraize_count, value);
					animation_data->value = math::lerp(animation_1.value, animation_2.value, value);
					return variable;
				}
			}
			animation_data->height_amplitude = animations[index].height_amplitude;
			animation_data->height_angular_frequency = animations[index].height_angular_frequency;
			animation_data->height_postraize_count = animations[index].height_postraize_count;
			animation_data->value = animations[index].value;
		}
		if (identifier == distortion_voronoi_animation_identifier)
		{
			variable.buffer.resize(sizeof(distortion_voronoi_animation_data));

			distortion_voronoi_animation_data* animation_data = reinterpret_cast<distortion_voronoi_animation_data*>(variable.buffer.data());
			const distortion_voronoi_animation_data* animations = reinterpret_cast<const distortion_voronoi_animation_data*>(row_animations);
			for (; index < size - 1; ++index)
			{
				auto& animation_1 = animations[index + 0];
				auto& animation_2 = animations[index + 1];
				float value = (time - animation_1.time) / (animation_2.time - animation_1.time);
				if (0.0f <= value && value < 1.0f)
				{
					animation_data->square_count = math::lerp(animation_1.square_count, animation_2.square_count, value);
					animation_data->value = math::lerp(animation_1.value, animation_2.value, value);
					return variable;
				}
			}
			animation_data->square_count = animations[index].square_count;
			animation_data->value = animations[index].value;
		}
		if (identifier == character_and_effect_time_scale_animation_identifier)
		{
			variable.buffer.resize(sizeof(character_and_effect_time_scale_animation_data));

			character_and_effect_time_scale_animation_data* animation_data = reinterpret_cast<character_and_effect_time_scale_animation_data*>(variable.buffer.data());
			const character_and_effect_time_scale_animation_data* animations = reinterpret_cast<const character_and_effect_time_scale_animation_data*>(row_animations);
			for (; index < size - 1; ++index)
			{
				auto& animation_1 = animations[index + 0];
				auto& animation_2 = animations[index + 1];
				float value = (time - animation_1.time) / (animation_2.time - animation_1.time);
				if (0.0f <= value && value < 1.0f)
				{
					animation_data->value = math::lerp(animation_1.value, animation_2.value, value);
					return variable;
				}
			}
			animation_data->value = animations[index].value;
		}
		if (identifier == color_grading_animation_identifier)
		{
			variable.buffer.resize(sizeof(color_grading_animation_data));

			color_grading_animation_data* animation_data = reinterpret_cast<color_grading_animation_data*>(variable.buffer.data());
			const color_grading_animation_data* animations = reinterpret_cast<const color_grading_animation_data*>(row_animations);
			for (; index < size - 1; ++index)
			{
				auto& animation_1 = animations[index + 0];
				auto& animation_2 = animations[index + 1];
				float value = (time - animation_1.time) / (animation_2.time - animation_1.time);
				if (0.0f <= value && value < 1.0f)
				{
					animation_data->data.intensity = math::lerp(animation_1.data.intensity, animation_2.data.intensity, value);
					animation_data->data.stage_depth_scale = math::lerp(animation_1.data.stage_depth_scale, animation_2.data.stage_depth_scale, value);
					animation_data->data.character_data.hsv_weight = math::lerp(animation_1.data.character_data.hsv_weight, animation_2.data.character_data.hsv_weight, value);
					animation_data->data.character_data.color_multiplyer = math::lerp(animation_1.data.character_data.color_multiplyer, animation_2.data.character_data.color_multiplyer, value);
					animation_data->data.stage_data.hsv_weight = math::lerp(animation_1.data.stage_data.hsv_weight, animation_2.data.stage_data.hsv_weight, value);
					animation_data->data.stage_data.color_multiplyer = math::lerp(animation_1.data.stage_data.color_multiplyer, animation_2.data.stage_data.color_multiplyer, value);
					return variable;
				}
			}
			animation_data->data.intensity = animations[index].data.intensity;
			animation_data->data.stage_depth_scale = animations[index].data.stage_depth_scale;
			animation_data->data.character_data.hsv_weight = animations[index].data.character_data.hsv_weight;
			animation_data->data.character_data.color_multiplyer = animations[index].data.character_data.color_multiplyer;
			animation_data->data.stage_data.hsv_weight = animations[index].data.stage_data.hsv_weight;
			animation_data->data.stage_data.color_multiplyer = animations[index].data.stage_data.color_multiplyer;
		}
		if (identifier == overwrite_text_scale_animation_identifier)
		{
			variable.buffer.resize(sizeof(overwrite_text_scale_animation_data));

			overwrite_text_scale_animation_data* animation_data = reinterpret_cast<overwrite_text_scale_animation_data*>(variable.buffer.data());
			const overwrite_text_scale_animation_data* animations = reinterpret_cast<const overwrite_text_scale_animation_data*>(row_animations);
			for (; index < size - 1; ++index)
			{
				auto& animation_1 = animations[index + 0];
				auto& animation_2 = animations[index + 1];
				float value = (time - animation_1.time) / (animation_2.time - animation_1.time);
				if (0.0f <= value && value < 1.0f)
				{
					animation_data->value = math::lerp(animation_1.value, animation_2.value, value);
					return variable;
				}
			}
			animation_data->value = animations[index].value;
		}
		if (identifier == texts_wave_posterize_animation_identifier)
		{
			variable.buffer.resize(sizeof(texts_wave_posterize_animation_data));

			texts_wave_posterize_animation_data* animation_data = reinterpret_cast<texts_wave_posterize_animation_data*>(variable.buffer.data());
			const texts_wave_posterize_animation_data* animations = reinterpret_cast<const texts_wave_posterize_animation_data*>(row_animations);
			for (; index < size - 1; ++index)
			{
				auto& animation_1 = animations[index + 0];
				auto& animation_2 = animations[index + 1];
				float value = (time - animation_1.time) / (animation_2.time - animation_1.time);
				if (0.0f <= value && value < 1.0f)
				{
					animation_data->color = math::lerp(animation_1.color, animation_2.color, value);
					animation_data->posterize_step = math::lerp(animation_1.posterize_step, animation_2.posterize_step, value);
					animation_data->posterize_animation_rate = math::lerp(animation_1.posterize_animation_rate, animation_2.posterize_animation_rate, value);
					animation_data->posterize_amount = math::lerp(animation_1.posterize_amount, animation_2.posterize_amount, value);
					return variable;
				}
			}
			animation_data->color = animations[index].color;
			animation_data->posterize_step = animations[index].posterize_step;
			animation_data->posterize_animation_rate = animations[index].posterize_animation_rate;
			animation_data->posterize_amount = animations[index].posterize_amount;
		}

		return variable;
	}

	void call_variable_animation(std::shared_ptr<cinematics_object> cinematics, float rate, std::string_view identifier, const animation::variable& data1, const animation::variable& data2)
	{
		if (identifier == distortion_animation_identifier)
		{
			auto& animation_1 = *reinterpret_cast<const distortion_animation_data*>(data1.buffer.data());
			auto& animation_2 = *reinterpret_cast<const distortion_animation_data*>(data1.buffer.data());

			post_process_datas data = cinematics->get_external_post_process_data();
			data.distortion_data.power = math::lerp(animation_1.power, animation_2.power, rate);
			data.distortion_data.speed = math::lerp(animation_1.speed, animation_2.speed, rate);
			data.distortion_data.amount = math::lerp(animation_1.value, animation_2.value, rate);
			cinematics->set_external_post_process_data(data);
		}
		if (identifier == std::string(sin_distortion_animation_identifier) + "0")
		{
			auto& animation_1 = *reinterpret_cast<const sin_distortion_animation_data*>(data1.buffer.data());
			auto& animation_2 = *reinterpret_cast<const sin_distortion_animation_data*>(data1.buffer.data());

			post_process_datas data = cinematics->get_external_post_process_data();
			data.sin_distortion_datas[0].height_amplitude = math::lerp(animation_1.height_amplitude, animation_2.height_amplitude, rate);
			data.sin_distortion_datas[0].height_angular_frequency = math::lerp(animation_1.height_angular_frequency, animation_2.height_angular_frequency, rate);
			data.sin_distortion_datas[0].height_postraize_count = math::lerp(animation_1.height_postraize_count, animation_2.height_postraize_count, rate);
			data.sin_distortion_datas[0].amount = math::lerp(animation_1.value, animation_2.value, rate);
			cinematics->set_external_post_process_data(data);
		}
		if (identifier == std::string(sin_distortion_animation_identifier) + "1")
		{
			auto& animation_1 = *reinterpret_cast<const sin_distortion_animation_data*>(data1.buffer.data());
			auto& animation_2 = *reinterpret_cast<const sin_distortion_animation_data*>(data1.buffer.data());

			post_process_datas data = cinematics->get_external_post_process_data();
			data.sin_distortion_datas[1].height_amplitude = math::lerp(animation_1.height_amplitude, animation_2.height_amplitude, rate);
			data.sin_distortion_datas[1].height_angular_frequency = math::lerp(animation_1.height_angular_frequency, animation_2.height_angular_frequency, rate);
			data.sin_distortion_datas[1].height_postraize_count = math::lerp(animation_1.height_postraize_count, animation_2.height_postraize_count, rate);
			data.sin_distortion_datas[1].amount = math::lerp(animation_1.value, animation_2.value, rate);
			cinematics->set_external_post_process_data(data);
		}
		if (identifier == distortion_voronoi_animation_identifier)
		{
			auto& animation_1 = *reinterpret_cast<const distortion_voronoi_animation_data*>(data1.buffer.data());
			auto& animation_2 = *reinterpret_cast<const distortion_voronoi_animation_data*>(data1.buffer.data());

			post_process_datas data = cinematics->get_external_post_process_data();
			data.voronoi_distortion_data.square_count = math::lerp(animation_1.square_count, animation_2.square_count, rate);
			data.voronoi_distortion_data.amount = math::lerp(animation_1.value, animation_2.value, rate);
			cinematics->set_external_post_process_data(data);
		}
		if (identifier == character_and_effect_time_scale_animation_identifier)
		{
			auto& animation_1 = *reinterpret_cast<const character_and_effect_time_scale_animation_data*>(data1.buffer.data());
			auto& animation_2 = *reinterpret_cast<const character_and_effect_time_scale_animation_data*>(data1.buffer.data());

			cinematics->set_external_time_scale(math::lerp(animation_1.value, animation_2.value, rate));
		}
		if (identifier == color_grading_animation_identifier)
		{
			auto& animation_1 = *reinterpret_cast<const color_grading_animation_data*>(data1.buffer.data());
			auto& animation_2 = *reinterpret_cast<const color_grading_animation_data*>(data1.buffer.data());

			post_process_datas data = cinematics->get_external_post_process_data();
			data.color_grading.intensity = math::lerp(animation_1.data.intensity, animation_2.data.intensity, rate);
			data.color_grading.stage_depth_scale = math::lerp(animation_1.data.stage_depth_scale, animation_2.data.stage_depth_scale, rate);
			data.color_grading.character_data.hsv_weight = math::lerp(animation_1.data.character_data.hsv_weight, animation_2.data.character_data.hsv_weight, rate);
			data.color_grading.character_data.color_multiplyer = math::lerp(animation_1.data.character_data.color_multiplyer, animation_2.data.character_data.color_multiplyer, rate);
			data.color_grading.stage_data.hsv_weight = math::lerp(animation_1.data.stage_data.hsv_weight, animation_2.data.stage_data.hsv_weight, rate);
			data.color_grading.stage_data.color_multiplyer = math::lerp(animation_1.data.stage_data.color_multiplyer, animation_2.data.stage_data.color_multiplyer, rate);
			cinematics->set_external_post_process_data(data);
		}
		if (identifier == overwrite_text_scale_animation_identifier)
		{
			auto& animation_1 = *reinterpret_cast<const overwrite_text_scale_animation_data*>(data1.buffer.data());
			auto& animation_2 = *reinterpret_cast<const overwrite_text_scale_animation_data*>(data1.buffer.data());

			post_process_datas data = cinematics->get_external_post_process_data();
			data.overwrite_text.texcoord_scale = math::lerp(animation_1.value, animation_2.value, rate);
			cinematics->set_external_post_process_data(data);
		}
		if (identifier == texts_wave_posterize_animation_identifier)
		{
			auto& animation_1 = *reinterpret_cast<const texts_wave_posterize_animation_data*>(data1.buffer.data());
			auto& animation_2 = *reinterpret_cast<const texts_wave_posterize_animation_data*>(data1.buffer.data());

			post_process_datas data = cinematics->get_external_post_process_data();
			data.overwrite_text.posterize_data.step = math::lerp(animation_1.posterize_step, animation_2.posterize_step, rate);
			data.overwrite_text.posterize_data.animation_rate = math::lerp(animation_1.posterize_animation_rate, animation_2.posterize_animation_rate, rate);
			data.overwrite_text.posterize_data.amount = math::lerp(animation_1.posterize_amount, animation_2.posterize_amount, rate);
			data.overwrite_text.color = math::lerp(animation_1.color, animation_2.color, rate);
			cinematics->set_external_post_process_data(data);
		}
	}
}

void player_camera_controller_object::register_animations()
{
	{
		static constexpr std::string_view animation_name = "Assets/Resources/camera_animations/common_game_camera.ema";
		std::filesystem::path path(animation_name);

#if defined(_DEBUG)
		if (!std::filesystem::exists(path))
		{
			//	無いので変換
			path.replace_extension("vmd");
			{
				std::string directory = path.parent_path().string();

				//	本体生成
				std::shared_ptr<animation_resource> resource = std::make_shared<animation_resource>();

				//	入場演出カメラを登録
				{
					static constexpr std::string_view animation_name = "Assets/Resources/camera_animations/common_entry_camera.vmd";
					std::filesystem::path path(animation_name);

					std::ifstream	read_file;
					read_file.open(path, std::ios::in | std::ios::binary);
					_ASSERT_EXPR_A(read_file, L"Failed to load file");

					std::string	file_buffer;
					file_buffer = std::string(std::istreambuf_iterator<char>(read_file), std::istreambuf_iterator<char>());

					vmd_loader loader;
					bool succeeded = loader.load(file_buffer.data(), static_cast<unsigned int>(file_buffer.size()));
					_ASSERT_EXPR_A(succeeded, L"Failed to load file");
					if (succeeded)
					{
						if (resource->convert_from_vmd(entry_performance::animation_name, loader))
						{
							resource->get_animations_for_debug()[0].root_motion_xz = true;
						}
					}
				}

				//	ついでに各種汎用クリアカメラ演出を登録しておく
				float sampling_rate = system_units::convert_fps_to_sec(1.0f);

				//	カメラ
				const animation::camera_spherical* camera_animations[] =
				{
					clear_performance::common_start_cut_animations::camera,
					clear_performance::cut2_animations::camera,
					clear_performance::cut3_animations::camera,
					clear_performance::common_end_cut_animations::camera,
				};
				const size_t camera_animation_sizes[] =
				{
					ARRAYSIZE(clear_performance::common_start_cut_animations::camera),
					ARRAYSIZE(clear_performance::cut2_animations::camera),
					ARRAYSIZE(clear_performance::cut3_animations::camera),
					ARRAYSIZE(clear_performance::common_end_cut_animations::camera),
				};

				//	揺らぎ
				const clear_performance::distortion_animation_data* distortion_animations[] =
				{
					clear_performance::common_start_cut_animations::distortion,
					clear_performance::cut2_animations::distortion,
					clear_performance::cut3_animations::distortion,
					clear_performance::common_end_cut_animations::distortion,
				};
				const size_t distortion_animation_sizes[] =
				{
					ARRAYSIZE(clear_performance::common_start_cut_animations::distortion),
					ARRAYSIZE(clear_performance::cut2_animations::distortion),
					ARRAYSIZE(clear_performance::cut3_animations::distortion),
					ARRAYSIZE(clear_performance::common_end_cut_animations::distortion),
				};

				//	sin波揺らぎ
				const clear_performance::sin_distortion_animation_data* sin_distortion_0_animations[] =
				{
					clear_performance::common_start_cut_animations::sin_distortion,
					nullptr,
					nullptr,
					nullptr,
				};
				const size_t sin_distortion_0_animation_sizes[] =
				{
					ARRAYSIZE(clear_performance::common_start_cut_animations::sin_distortion),
					0,
					0,
					0,
				};
				const clear_performance::sin_distortion_animation_data* sin_distortion_1_animations[] =
				{
					clear_performance::common_start_cut_animations::sin_distortion_big,
					nullptr,
					nullptr,
					nullptr,
				};
				const size_t sin_distortion_1_animation_sizes[] =
				{
					ARRAYSIZE(clear_performance::common_start_cut_animations::sin_distortion_big),
					0,
					0,
					0,
				};

				//	ボロノイノイズによる揺らぎ
				const clear_performance::distortion_voronoi_animation_data* voronoi_distortion_animations[] =
				{
					clear_performance::common_start_cut_animations::distortion_voronoi,
					nullptr,
					nullptr,
					nullptr,
				};
				const size_t voronoi_distortion_animation_sizes[] =
				{
					ARRAYSIZE(clear_performance::common_start_cut_animations::distortion_voronoi),
					0,
					0,
					0,
				};

				//	時間スケール
				const clear_performance::character_and_effect_time_scale_animation_data* character_and_effect_time_scale_animations[] =
				{
					clear_performance::common_start_cut_animations::character_and_effect_time_scale,
					nullptr,
					nullptr,
					nullptr,
				};
				const size_t character_and_effect_time_scale_animation_sizes[] =
				{
					ARRAYSIZE(clear_performance::common_start_cut_animations::character_and_effect_time_scale),
					0,
					0,
					0,
				};

				//	色調補正
				const clear_performance::color_grading_animation_data* color_grading_animations[] =
				{
					clear_performance::common_start_cut_animations::color_grading,
					clear_performance::cut2_animations::color_grading,
					clear_performance::cut3_animations::color_grading,
					clear_performance::common_end_cut_animations::color_grading,
				};
				const size_t color_grading_animation_sizes[] =
				{
					ARRAYSIZE(clear_performance::common_start_cut_animations::color_grading),
					ARRAYSIZE(clear_performance::cut2_animations::color_grading),
					ARRAYSIZE(clear_performance::cut3_animations::color_grading),
					ARRAYSIZE(clear_performance::common_end_cut_animations::color_grading),
				};

				//	上書き文字のサイズ補正
				const clear_performance::overwrite_text_scale_animation_data* overwrite_text_scale_animations[] =
				{
					clear_performance::common_start_cut_animations::overwrite_text_scale,
					nullptr,
					nullptr,
					clear_performance::common_end_cut_animations::overwrite_text_scale,
				};
				const size_t overwrite_text_scale_animation_sizes[] =
				{
					ARRAYSIZE(clear_performance::common_start_cut_animations::overwrite_text_scale),
					0,
					0,
					ARRAYSIZE(clear_performance::common_end_cut_animations::overwrite_text_scale),
				};

				//	上書き文字の色補正
				const clear_performance::texts_wave_posterize_animation_data* texts_wave_posterize_animations[] =
				{
					clear_performance::common_start_cut_animations::texts_wave_posterize,
					clear_performance::cut2_animations::texts_wave_posterize,
					clear_performance::cut3_animations::texts_wave_posterize,
					clear_performance::common_end_cut_animations::texts_wave_posterize,
				};
				const size_t texts_wave_posterize_animation_sizes[] =
				{
					ARRAYSIZE(clear_performance::common_start_cut_animations::texts_wave_posterize),
					ARRAYSIZE(clear_performance::cut2_animations::texts_wave_posterize),
					ARRAYSIZE(clear_performance::cut3_animations::texts_wave_posterize),
					ARRAYSIZE(clear_performance::common_end_cut_animations::texts_wave_posterize),
				};

				//	イベントリスト
				const animation::event event_common_end_cut_animations[] =
				{
					{
						clear_performance::event_transition_frame,
						animation::event::variable_type::None,
						std::string(clear_performance::event_transition_id),
						"",
					},
				};

				const animation::event* event_animations[] =
				{
					nullptr,
					nullptr,
					nullptr,
					event_common_end_cut_animations,
				};
				const size_t event_animation_sizes[] =
				{
					0,
					0,
					0,
					ARRAYSIZE(event_common_end_cut_animations),
				};

				const bool root_rotation_animation[] =
				{
					true,
					false,
					false,
					false,
				};
				for (size_t index = 0; index < ARRAYSIZE(clear_performance::common_animation_names); index++)
				{
					animation::data data;
					data.name = std::string(clear_performance::common_animation_names[index]);
					data.root_motion_xz = true;
					data.root_motion_y = true;
					data.root_rotation = root_rotation_animation[index];

					//	終端フレームはカメラモーションの情報を参照
					data.end_time = camera_animations[index][camera_animation_sizes[index] - 1].time;

					//	アニメーション情報の設定
					float sampling_time = 0;
					for (sampling_time = 0; sampling_time <= data.end_time; sampling_time += sampling_rate)
					{
						if (camera_animations[index])
						{
							data.camera_sphericals.emplace_back(clear_performance::calculate_camera_animation(camera_animations[index],
																 camera_animation_sizes[index],
																 sampling_time));
						}

						if (distortion_animations[index])
						{
							std::string key = std::string(clear_performance::distortion_animation_identifier);
							auto it = data.variables.find(key);
							if (it == data.variables.end())
							{
								data.variables.insert_or_assign(key, std::vector<animation::variable>());
								it = data.variables.find(key);
							}
							it->second.emplace_back(clear_performance::calculate_variable_animation(distortion_animations[index],
													 distortion_animation_sizes[index],
													 key,
													 sampling_time));
						}
						if (sin_distortion_0_animations[index])
						{
							std::string key = std::string(clear_performance::sin_distortion_animation_identifier) + "0";
							auto it = data.variables.find(key);
							if (it == data.variables.end())
							{
								data.variables.insert_or_assign(key, std::vector<animation::variable>());
								it = data.variables.find(key);
							}
							it->second.emplace_back(clear_performance::calculate_variable_animation(sin_distortion_0_animations[index],
													 sin_distortion_0_animation_sizes[index],
													 key,
													 sampling_time));
						}
						if (sin_distortion_1_animations[index])
						{
							std::string key = std::string(clear_performance::sin_distortion_animation_identifier) + "1";
							auto it = data.variables.find(key);
							if (it == data.variables.end())
							{
								data.variables.insert_or_assign(key, std::vector<animation::variable>());
								it = data.variables.find(key);
							}
							it->second.emplace_back(clear_performance::calculate_variable_animation(sin_distortion_1_animations[index],
													 sin_distortion_1_animation_sizes[index],
													 key,
													 sampling_time));
						}
						if (voronoi_distortion_animations[index])
						{
							std::string key = std::string(clear_performance::distortion_voronoi_animation_identifier);
							auto it = data.variables.find(key);
							if (it == data.variables.end())
							{
								data.variables.insert_or_assign(key, std::vector<animation::variable>());
								it = data.variables.find(key);
							}
							it->second.emplace_back(clear_performance::calculate_variable_animation(voronoi_distortion_animations[index],
													 voronoi_distortion_animation_sizes[index],
													 key,
													 sampling_time));
						}
						if (character_and_effect_time_scale_animations[index])
						{
							std::string key = std::string(clear_performance::character_and_effect_time_scale_animation_identifier);
							auto it = data.variables.find(key);
							if (it == data.variables.end())
							{
								data.variables.insert_or_assign(key, std::vector<animation::variable>());
								it = data.variables.find(key);
							}
							it->second.emplace_back(clear_performance::calculate_variable_animation(character_and_effect_time_scale_animations[index],
								character_and_effect_time_scale_animation_sizes[index],
								key,
								sampling_time));
						}
						if (color_grading_animations[index])
						{
							std::string key = std::string(clear_performance::color_grading_animation_identifier);
							auto it = data.variables.find(key);
							if (it == data.variables.end())
							{
								data.variables.insert_or_assign(key, std::vector<animation::variable>());
								it = data.variables.find(key);
							}
							it->second.emplace_back(clear_performance::calculate_variable_animation(color_grading_animations[index],
								color_grading_animation_sizes[index],
								key,
								sampling_time));
						}
						if (overwrite_text_scale_animations[index])
						{
							std::string key = std::string(clear_performance::overwrite_text_scale_animation_identifier);
							auto it = data.variables.find(key);
							if (it == data.variables.end())
							{
								data.variables.insert_or_assign(key, std::vector<animation::variable>());
								it = data.variables.find(key);
							}
							it->second.emplace_back(clear_performance::calculate_variable_animation(overwrite_text_scale_animations[index],
								overwrite_text_scale_animation_sizes[index],
								key,
								sampling_time));
						}
						if (texts_wave_posterize_animations[index])
						{
							std::string key = std::string(clear_performance::texts_wave_posterize_animation_identifier);
							auto it = data.variables.find(key);
							if (it == data.variables.end())
							{
								data.variables.insert_or_assign(key, std::vector<animation::variable>());
								it = data.variables.find(key);
							}
							it->second.emplace_back(clear_performance::calculate_variable_animation(texts_wave_posterize_animations[index],
								texts_wave_posterize_animation_sizes[index],
								key,
								sampling_time));
						}
					}

					//	イベント登録
					for (int event_index = 0; event_index < event_animation_sizes[index]; ++event_index)
					{
						data.events.push_back(event_animations[index][event_index]);
					}

					resource->add_animation_data(data);
				}

				resource->serialize(animation_name);
			}
			path.replace_extension("ema");
		}
#endif	//	defined(_DEBUG)

		animations.emplace_back(std::make_shared<animation_resource>());
		animations.back()->deserialize(path.string());
	}
}

bool player_camera_controller_object::is_camera_transition_wait()
{
	if (state == states::clear_performance)
	{
		//	クリア演出中は遷移フラグがたてば次に進める
		return clear_performance_transition_flag;
	}

	return (state == states::entry);
}

void player_camera_controller_object::fixed_update_entry(float elapsed_time)
{
	fixed_update_default(elapsed_time);

	float apply_animation_rate_value = 0.0f;
	enum sub_states
	{
		play_anim,
		wait_anim
	};
	switch (sub_state)
	{
		case sub_states::play_anim:
		{
			play_animation(entry_performance::animation_name, false);
			++sub_state;
			break;
		}
		case sub_states::wait_anim:
		{
			apply_animation_rate_value = math::smoothstep(140.0f / 160.0f, 160.0f / 160.0f, calculate_animation_rate());
			if (is_playing_animation())
				break;

			state = states::free;
			sub_state = 0;
			break;
		}
	}
	auto rotation = get_rotation();
	auto position = get_position();
	update_animation(elapsed_time);

	set_rotation(math::slerp(get_rotation(), rotation, apply_animation_rate_value));
	set_position(math::lerp(get_position(), position, apply_animation_rate_value));
}

void player_camera_controller_object::fixed_update_clear_performance(float elapsed_time)
{
	fixed_update_default(elapsed_time);

	auto self_object = player_controller->get_controll_object();
	this->focus = self_object->get_focus_point();

	enum sub_states
	{
		play_anim,
		wait_anim
	};
	float apply_animation_rate_value = 0.0f;
	switch (sub_state % 2)
	{
	case sub_states::play_anim:
	{
		//	最初はプレイヤーのアニメーションに合わせた位置へ移動させるのが望ましい
		fixed_update_default(elapsed_time);
		if (play_animation(clear_performance::common_animation_names[sub_state / 2], false))
		{
			++sub_state;
		}
		if (sub_state / 2 > 0)
		{
			apply_animation_rate_value = 0.0f;
		}
		break;
	}
	case sub_states::wait_anim:
	{
		apply_animation_rate_value = sub_state / 2 == 0 ? math::smoothstep(0.7f, 1.0f, 1.0f - calculate_animation_rate()) : 0.0f;
		if (is_playing_animation())
			break;

		++sub_state;
		break;
	}
	}
	auto rotation = get_rotation();
	auto position = get_position();
	update_animation(elapsed_time);

	set_rotation(math::slerp(get_rotation(), rotation, apply_animation_rate_value));
	set_position(math::lerp(get_position(), position, apply_animation_rate_value));
}

bool player_camera_controller_object::play_animation(std::string_view animation_name, bool loop)
{
	character_animation_index = 0;
	size_t index = animations[character_animation_index]->get_index_from_name(animation_name);
	if (index == animation_resource::invalid_id)
		return false;

	current_animation_index = index;
	animation_time = 0;
	animation_loop = loop;
	return true;
}

void player_camera_controller_object::update_animation(float elapsed_time)
{
	auto& animation_datas = animations[character_animation_index]->get_animation();
	if (!animation_datas.empty())
	{
		auto& animation_data = animation_datas[current_animation_index];
		float previous_time = animation_time;
		animation_time += elapsed_time;
		float current_time = animation_time;
		if (current_time > animation_data.end_time)
		{
			current_time = animation_loop ? fmodf(current_time, animation_data.end_time) : animation_data.end_time;
		}

		animation::camera_spherical current_animation;
		if (animation_data.compute_animation(current_animation, current_time))
		{
			animation::camera_spherical begin_animation, previous_animation;
			animation_data.compute_animation(begin_animation, 0.0f);
			animation_data.compute_animation(previous_animation, previous_time);
			math::rotator rotation = get_rotation();
			if (animation_data.root_rotation)
			{
				//	ローカル回転を算出
				DirectX::XMVECTOR LocalRotation, WorkRotation[2];
#if 0
				if (animation_loop && (previous_time > current_time))
				{
					animation::camera_spherical end_camera_animation;
					animation_data.compute_animation(end_camera_animation, animation_data.end_time);
					WorkRotation[0] = DirectX::XMLoadFloat4(&end_camera_animation.rotation);
					WorkRotation[1] = DirectX::XMLoadFloat4(&previous_animation.rotation);
					WorkRotation[1] = DirectX::XMQuaternionInverse(WorkRotation[1]);
					LocalRotation = DirectX::XMQuaternionMultiply(WorkRotation[0], WorkRotation[1]);
					WorkRotation[0] = DirectX::XMLoadFloat4(&current_animation.rotation);
					WorkRotation[1] = DirectX::XMLoadFloat4(&begin_animation.rotation);
					WorkRotation[1] = DirectX::XMQuaternionInverse(WorkRotation[1]);
					WorkRotation[1] = DirectX::XMQuaternionMultiply(WorkRotation[0], WorkRotation[1]);
					LocalRotation = DirectX::XMQuaternionMultiply(LocalRotation, WorkRotation[1]);
				}
				else
#endif
				{
					WorkRotation[0] = DirectX::XMQuaternionRotationRollPitchYaw(current_animation.rotation.x, current_animation.rotation.y, current_animation.rotation.z);
					WorkRotation[1] = DirectX::XMQuaternionRotationRollPitchYaw(previous_animation.rotation.x, previous_animation.rotation.y, previous_animation.rotation.z);
					WorkRotation[1] = DirectX::XMQuaternionInverse(WorkRotation[1]);
					LocalRotation = DirectX::XMQuaternionMultiply(WorkRotation[0], WorkRotation[1]);
				}

				//	ワールド回転更新
				DirectX::XMFLOAT4 local_rotation;
				DirectX::XMStoreFloat4(&local_rotation, LocalRotation);
				DirectX::XMFLOAT3 local_rotator = math::convert_quaternion_to_euler(local_rotation);
				rotation.x = math::normalize_angle(rotation.x + local_rotator.x);
				rotation.y = math::normalize_angle(rotation.y + local_rotator.y);
				rotation.z = math::normalize_angle(rotation.z + local_rotator.z);
			}
			else
			{
				rotation = current_animation.rotation;
			}

			DirectX::XMFLOAT3 vector = rotation.get_front();
			DirectX::XMFLOAT3 focus = current_animation.focus;
			if (animation_data.root_motion_xz)
			{
				focus.x += this->focus.x;
				focus.z += this->focus.z;
			}
			if (animation_data.root_motion_y)
			{
				focus.y += this->focus.y;
			}
			focus.x -= vector.x * current_animation.distance;
			focus.y -= vector.y * current_animation.distance;
			focus.z -= vector.z * current_animation.distance;
			set_rotation(rotation);
			set_position(focus);
		}

		//	イベント処理
		auto event_func = [&](const animation::event& event)
		{
			if (event.function_name == clear_performance::event_transition_id)
			{
				clear_performance_transition_flag = true;
			}
		};
		animation_data.call_events({ previous_time, current_time }, event_func);

		//	その他処理を呼び出す
		auto cinematics = object_manager::instance()->search_object<cinematics_object>();
		auto variable_func = [&](float rate, std::string_view identifier, const animation::variable& data1, const animation::variable& data2)
		{
			if (cinematics)
				clear_performance::call_variable_animation(cinematics, rate, identifier, data1, data2);
		};

		animation_data.compute_animation(current_time, variable_func);
	}
}

bool player_camera_controller_object::is_playing_animation()
{
	if (animation_loop)
		return false;
	auto& animation_datas = animations[character_animation_index]->get_animation();
	if (!animation_datas.empty())
	{
		return animation_time < animation_datas[current_animation_index].end_time;
	}
	return false;
}

float player_camera_controller_object::calculate_animation_rate()
{
	auto& animation_datas = animations[character_animation_index]->get_animation();
	if (!animation_datas.empty())
	{
		return animation_time / animation_datas[current_animation_index].end_time;
	}
	return 0;
}
