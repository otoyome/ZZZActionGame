#include "fullscreen_quad.hlsli"

VS_OUT main(uint vin : SV_VERTEXID)
{
    VS_OUT vout;
    vout.texcoord.x = vin % 2;
    vout.texcoord.y = vin / 2;
    vout.position.xy = vout.texcoord * float2(2.0f, -2.0f) - float2(1.0f, -1.0f);
    vout.position.z = 0;
    vout.position.w = 1;
    return vout;
}
