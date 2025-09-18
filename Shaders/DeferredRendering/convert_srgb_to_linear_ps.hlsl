#include "fullscreen_quad.hlsli"
#include "shading_functions.hlsli"

Texture2D color_map : register(t0);
SamplerState sampler_states : register(s0);

float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = color_map.Sample(sampler_states, pin.texcoord);
    color.rgb = pow(color.rgb, GammaFactor);
    return color;
}

