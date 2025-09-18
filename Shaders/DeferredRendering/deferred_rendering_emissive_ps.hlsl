#include "deferred_rendering.hlsli"

float4 main(VS_OUT pin) : SV_TARGET
{
    //	©ŒÈ”­ŒõF‰ÁZ
    return float4(get_gbuffer_data(pin.texcoord).emissive_color, 1);
}
