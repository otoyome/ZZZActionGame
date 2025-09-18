#ifndef __SDF_SHADOW_THRESHOLD_MASK__
#define __SDF_SHADOW_THRESHOLD_MASK__

#include "gbuffer.hlsli"
#include "shading_functions.hlsli"

//  sdfシャドウマスク算出
float sdf_shadow_threshold_mask(in GBufferData gbuffer_data, in float3 light_direction)
{
    float2 right = normalize(gbuffer_data.mesh_direction.xy);
    float2 front = normalize(gbuffer_data.mesh_direction.zw);
    float2 light = normalize(light_direction.xz);

    float attenuation = dot(front, light) * 0.5f + 0.5f;
    float illumination = dot(right, light) < 0.0f ? gbuffer_data.shadow_threshold.x : gbuffer_data.shadow_threshold.y;

    return step(illumination, attenuation);
}

#endif  //  __SDF_SHADOW_THRESHOLD_MASK__
