#pragma once

namespace common_pipeline_states
{
	//	共通定数バッファインデックス
	static constexpr UINT SceneCBVIndex = 0;
	static constexpr UINT LightCBVIndex = 3;
	static constexpr UINT ShadowmapCBVIndex = 4;

	//	モデル用定数バッファインデックス
	static constexpr UINT PrimitiveCBVIndex = 1;
	static constexpr UINT PreviousPrimitiveCBVIndex = 2;

	//	シーン情報
	struct scene_constant_buffer
	{
		DirectX::XMFLOAT4 time_data;	//	x : 現在時間, y : 前時間, zw : 空き
		DirectX::XMFLOAT4 z_buffer_parameteres;
		DirectX::XMFLOAT4 camera_position;
		DirectX::XMFLOAT4 camera_direction;
		DirectX::XMFLOAT4 camera_clip_distance;
		DirectX::XMFLOAT4 viewport_rect;
		DirectX::XMFLOAT4X4 view_transform;
		DirectX::XMFLOAT4X4 projection_transform;
		DirectX::XMFLOAT4X4 view_projection_transform;
		DirectX::XMFLOAT4X4 inverse_view_transform;
		DirectX::XMFLOAT4X4 inverse_projection_transform;
		DirectX::XMFLOAT4X4 inverse_view_projection_transform;
		DirectX::XMFLOAT4X4 previous_view_projection_transform;
	};

	//	ライト定数バッファ構造体
	struct light_constant_buffer
	{
		light_data light_data;
		UINT light_attribute;
		//  コンタクトシャドウ
		float contact_shadow;
		DirectX::XMFLOAT2 light_data_dummy;
	};

	//	影カテゴリー
	enum class shadow_category
	{
		default_id = 0,
		mask_1,
		mask_2,
		mask_3,
		mask_4,
		mask_5,
		mask_6,
		mask_7,
		max = 8,	// 8ビットまでなので
	};
	//	シャドウマップ情報
	static constexpr UINT shadow_map_max = 6;
	struct shadow_map_constant_buffer
	{
		UINT shadow_map_count;
		DirectX::XMFLOAT3 shadow_attenuation;
		DirectX::XMFLOAT4 shadow_bias[shadow_map_max];
		DirectX::XMFLOAT4 shadow_split_distance[shadow_map_max];
		DirectX::XMFLOAT4X4 light_view_projection_transform[shadow_map_max];
		UINT shadow_mask;	//	8ビットまで
		DirectX::XMFLOAT3 shadow_dummy;
	};

	//  G-Bufferステンシル
	static constexpr UINT g_buffer_stencil_indirect_lighting_off = 1 << 0;  //  間接光の処理を適応させない
	static constexpr UINT g_buffer_stencil_camera_distance_translucent = 1 <<  1;  //  カメラからの距離に応じて透過する
	static constexpr UINT g_buffer_stencil_character_flag = 1 << 2;	//	キャラクター識別フラグ

	static constexpr UINT g_buffer_stencil_filter_off = 1 << 20;  //  フィルター効果の適応フラグ
	static constexpr UINT g_buffer_stencil_normal_depth_outline_off = 1 << 21;  //  深度/法線によるライン描画フラグ
	static constexpr UINT g_buffer_stencil_color_outline_off = 1 << 22;  //  色によるライン描画フラグ
	static constexpr UINT g_buffer_stencil_noise_off = 1 << 23;  //  英人特有の影部分に対してのノイズ描画フラグ
	static constexpr UINT g_buffer_stencil_color_posterize_off = 1 << 24;  //  色のポスタライズ適応フラグ(ただし、カラーラインの方はポスタライズを元にして検知しているので注意)
	static constexpr UINT g_buffer_stencil_light_direction_ignore_y = 1 << 25;  //  シェーディング時にライトのYを無視する
	static constexpr UINT g_buffer_stencil_contact_shadow_on = 1 << 26;  //  コンタクトシャドウ適応フラグ

}	//	namespace common_pipeline_states

