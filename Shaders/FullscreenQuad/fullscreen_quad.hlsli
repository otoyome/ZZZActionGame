#ifndef __FULLSCREEN_QUAD_HLSLI__
#define __FULLSCREEN_QUAD_HLSLI__

#include "filter_functions.hlsli"

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

#define PS_IN VS_OUT

#endif  //  __FULLSCREEN_QUAD_HLSLI__
