#ifndef __SPRITE_HLSLI__
#define __SPRITE_HLSLI__

#include "scene_constant_buffer.hlsli"

struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 transform_matrix;
};

#endif // __SPRITE_HLSLI__
