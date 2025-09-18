#ifndef __GBUFFER_DECAL_CONSTANT_BUFFER_HLSLI__
#define __GBUFFER_DECAL_CONSTANT_BUFFER_HLSLI__

cbuffer DECAL_OBJECT_CONSTANT_BUFFER : register(b2)
{
    row_major float4x4 decal_inverse_transform;
    float4 decal_direction;
};

static const float normal_angle = 0.45f;

#endif // __GBUFFER_DECAL_CONSTANT_BUFFER_HLSLI__
