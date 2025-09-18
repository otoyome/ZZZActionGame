#include "debug_draw_line.hlsli"

VS_OUT main(float4 position : POSITION, float4 color : COLOR)
{
    VS_OUT vout;
    vout.position = mul(position, view_projection_transform);
    vout.color = color;
    return vout;
}
