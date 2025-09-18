#include "deferred_light_data.hlsli"
#include "gbuffer.hlsli"
#include "shading_models.hlsli"

#include "common_gbuffer_loader.hlsli"

#include "scene_constant_buffer.hlsli"
#include "contact_shadow.hlsli"

struct VS_OUT
{
    float4 position : SV_POSITION;
};
