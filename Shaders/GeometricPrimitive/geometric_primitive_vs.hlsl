#include "geometric_primitive.hlsli"

VS_OUT main(float4 position : POSITION, float2 texcoord : TEXCOORD)
{
    VS_OUT vout;
    vout.position = mul(position, mul(world, view_projection_transform));
    vout.texcoord = texcoord;
    return vout;
}
