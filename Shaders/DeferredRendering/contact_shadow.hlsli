#ifndef __CONTACT_SHADOW_HLSLI__
#define __CONTACT_SHADOW_HLSLI__

#include "gbuffer.hlsli"
#include "common_gbuffer_loader.hlsli"
#include "shading_models.hlsli"

#include "scene_constant_buffer.hlsli"
#include "sdf_shadow_threshold_mask.hlsli"

//  コンタクトシャドウ
struct contact_shadow_data
{
    Texture2D<float4> normal_depth_map;
    SamplerState samp_state;
    float3 world_position;
    float3 world_normal;
    float3 march_direction;
    float march_length;
};

bool ___inner_calculate_contact_shadow_check___(contact_shadow_data data, float2 texcoord, float projection_depth)
{
    static const float2 depth_range = { 0.00001f, 0.0001f };
    
    float linear_depth = convert_projection_depth_to_linear_depth(projection_depth, z_buffer_parameteres);
    
    uint2 texture_size;
    data.normal_depth_map.GetDimensions(texture_size.x, texture_size.y);
    float gbuffer_depth = data.normal_depth_map.SampleLevel(data.samp_state, texcoord, 0).a;

    float diff = linear_depth - gbuffer_depth;
    return depth_range.x < diff && diff < depth_range.y;
}

float ___inner_calculate_contact_shadow___(in contact_shadow_data data, int steps, bool soft)
{
    float3 step = data.march_direction * data.march_length / steps;
    data.world_position += step;
    if (soft)
    {
        float attenuation = 0;
        float2 rand_uv = 0;
        int count = 0;
        [loop] for (int i = 0; i < steps; ++i)
        {
            data.world_position += step * ((random(rand_uv) * 0.95f + 0.05f));

            float4 ndc = mul(float4(data.world_position, 1.0f), view_projection_transform);
            ndc /= ndc.w;
            float2 uv = ndc.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
            if (saturate(uv.x) != uv.x || saturate(uv.y) != uv.y || saturate(ndc.z) != ndc.z)
                break;
            
            rand_uv = uv;
            attenuation += ___inner_calculate_contact_shadow_check___(data, uv.xy, ndc.z);
            count++;
        }
        return 1.0f - attenuation / count;
    }
    
    [loop] for (int i = 0; i < steps; ++i)
    {
        data.world_position += step;

        float4 ndc = mul(float4(data.world_position, 1.0f), view_projection_transform);
        ndc /= ndc.w;
        float2 uv = ndc.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
        if (saturate(uv.x) != uv.x || saturate(uv.y) != uv.y || saturate(ndc.z) != ndc.z)
            break;

        if (___inner_calculate_contact_shadow_check___(data, uv.xy, ndc.z))
            return 0.0f;
    }
    
    return 1.0f;
}

//  コンタクトシャドウ
float calculate_contact_shadow(in GBufferData gbuffer_data, in contact_shadow_data cs_data, int steps = 32, bool soft = true)
{
    return ___inner_calculate_contact_shadow___(cs_data, steps, soft);
}

//  コンタクトシャドウ
//  スクリーンスペースで細かい遮蔽チェックを行い影を算出する。
float integrate_contact_shadow(in GBufferData gbuffer_data, in contact_shadow_data cs_data, int steps = 32, bool soft = true)
{
    float value = 1.0f;
    switch (gbuffer_data.shading_model)
    {
        case shading_model_default_lit:
        case shading_model_toon_lit:
        {
            value = calculate_contact_shadow(gbuffer_data, cs_data, steps, soft);
            break;
        }
    }

    return value;
}
#endif  //  __CONTACT_SHADOW_HLSLI__
