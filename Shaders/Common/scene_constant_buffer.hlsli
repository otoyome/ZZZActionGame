#ifndef __SCENE_CONSTANT_BUFFER_H__
#define __SCENE_CONSTANT_BUFFER_H__

cbuffer SCENE_CONSTANT_BUFFER : register(b0)
{
	float4 time_data;	//	x : 現在時間, y : 前時間, zw : 空き
    float4 z_buffer_parameteres; //	非線形深度から線形深度へ変換するためのパラメーター
    float4 camera_position;
    float4 camera_direction;
    float4 camera_clip_distance;
    float4 viewport_rect;   //  xy : 左上, zw : 右下
    row_major float4x4 view_transform;
    row_major float4x4 projection_transform;
    row_major float4x4 view_projection_transform;
    row_major float4x4 inverse_view_transform;
    row_major float4x4 inverse_projection_transform;
    row_major float4x4 inverse_view_projection_transform;

    row_major float4x4 previous_view_projection_transform;
};

#endif  //  __SCENE_CONSTANT_BUFFER_H__
