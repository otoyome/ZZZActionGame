#ifndef __MODEL_DEPTH_VS_HLSLI__
#define __MODEL_DEPTH_VS_HLSLI__

//#include "model.hlsli"

VS_DEPTH_OUT main(VS_DEPTH_IN vin)
{
    uint index;
    VS_DEPTH_OUT vout = (VS_DEPTH_OUT)0;
    
#if BONE_MAX > 0
    float3 position = 0;
    [unroll] for (index = 0; index < BoneMax; ++index)
    {
        uint bone_index = vin.joints[index / 4][index % 4];
        float bone_weight = vin.weights[index / 4][index % 4];
        position += mul(float4(vin.position, 1), bone_transforms[bone_index]).xyz * bone_weight;
    }
#else  //  BONE_MAX > 0
    float3 position = vin.position;
#endif  //  BONE_MAX > 0

    vout.position = mul(float4(position, 1), view_projection_transform);

#if VERTEX_COLOR_MAX > 0
    [unroll] for (index = 0; index < VertexColorMax; ++index)
    {
        vout.color[index] = vin.color[index];
    }
#endif  //  VERTEX_COLOR_MAX > 0

#if TEXCOORD_MAX > 0
    [unroll] for (index = 0; index < TexcoordMax; ++index)
    {
        vout.texcoord[index] = vin.texcoord[index];
    }
#endif  //  TEXCOORD_MAX > 0

    return vout;
}

#endif  //  __MODEL_DEPTH_VS_HLSLI__
