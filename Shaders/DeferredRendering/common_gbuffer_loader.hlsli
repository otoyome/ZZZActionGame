#ifndef __COMMON_GBUFFER_LOADER_HLSLI__
#define __COMMON_GBUFFER_LOADER_HLSLI__

#include "gbuffer.hlsli"
#include "scene_constant_buffer.hlsli"

Texture2D<float4> gbuffer_base_color : register(t0);
Texture2D<float4> gbuffer_emissive_color : register(t1);
Texture2D<float4> gbuffer_normal_depth : register(t2);
Texture2D<float4> gbuffer_parameter : register(t3);
Texture2D<float4> gbuffer_velocity : register(t4);
Texture2D<uint4> gbuffer_unique_data : register(t5);
Texture2D<float4> gbuffer_shade_color : register(t6);

SamplerState sampler_states : register(s0);

PSGBufferTextures get_gbuffer_textures()
{
    PSGBufferTextures gbuffer_textures;
    gbuffer_textures.base_color = gbuffer_base_color;
    gbuffer_textures.emissive_color = gbuffer_emissive_color;
    gbuffer_textures.normal_depth = gbuffer_normal_depth;
    gbuffer_textures.parameter = gbuffer_parameter;
    gbuffer_textures.velocity = gbuffer_velocity;
    gbuffer_textures.unique_data = gbuffer_unique_data;
    gbuffer_textures.shade_color = gbuffer_shade_color;
    gbuffer_textures.state = sampler_states;
    return gbuffer_textures;
}

GBufferData get_gbuffer_data(PSGBufferTextures gbuffer_textures, float2 texcoord)
{
    return DecodeGBuffer(gbuffer_textures, texcoord, inverse_view_projection_transform, z_buffer_parameteres);
}

GBufferData get_gbuffer_data(float2 texcoord)
{
    return get_gbuffer_data(get_gbuffer_textures(), texcoord);
}

//  GBufferÇ™ê›íËÇ≥ÇÍÇƒÇ¢ÇÈëOíÒ
float2 get_gbuffer_map_size()
{
    float2 scene_map_size;
    gbuffer_base_color.GetDimensions(scene_map_size.x, scene_map_size.y);
    return scene_map_size;
}



#endif  //  __COMMON_GBUFFER_LOADER_HLSLI__