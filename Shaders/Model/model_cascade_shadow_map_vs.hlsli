#ifndef __MODEL_CSM_VS_HLSLI__
#define __MODEL_CSM_VS_HLSLI__

//#include "model.hlsli"

VS_SHADOW_MAP_OUT main(VS_SHADOW_MAP_IN vin)
{
    uint index;
    VS_SHADOW_MAP_OUT vout = (VS_SHADOW_MAP_OUT)0;
    
#if BONE_MAX > 0
    float3 position = 0, normal = 0;
    [unroll] for (index = 0; index < BoneMax; ++index)
    {
        uint bone_index = vin.joints[index / 4][index % 4];
        float bone_weight = vin.weights[index / 4][index % 4];
        position += mul(float4(vin.position, 1), bone_transforms[bone_index]).xyz * bone_weight;
        normal += mul(float4(vin.normal, 0), bone_transforms[bone_index]).xyz * bone_weight;
    }
#else  //  BONE_MAX > 0
    float3 position = vin.position;
    float3 normal = vin.normal;
#endif  //  BONE_MAX > 0

    vout.position = float4(position, 1.0f);
    vout.normal = normal;

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

#endif  //  __MODEL_CSM_VS_HLSLI__
