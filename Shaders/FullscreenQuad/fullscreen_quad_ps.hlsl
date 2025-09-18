#include "fullscreen_quad.hlsli"


Texture2D<float4> color_map : register(t0);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define POINT_CLAMP 3
#define LINEAR_CLAMP 4
#define ANISOTROPIC_CLAMP 5
#define LINEAR_BORDER_BLACK 6
#define LINEAR_BORDER_WHITE 7
SamplerState sampler_states[8] : register(s0);


float4 main(PS_IN pin) : SV_TARGET
{
    return color_map.Sample(sampler_states[POINT], pin.texcoord);
}
