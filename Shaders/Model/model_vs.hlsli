#ifndef __MODEL_VS_HLSLI__
#define __MODEL_VS_HLSLI__

//#include "model.hlsli"

VS_OUT main(VS_IN vin)
{
    uint index;
    VS_OUT vout = (VS_OUT) 0;
    
#if BONE_MAX > 0
    
    float3 current_position = 0, normal = 0, previous_position = 0;
#if defined(HAS_TANGENT)
    float3 tangent = 0;
#endif  //  defined(HAS_TANGENT)
    [unroll] for (index = 0; index < BoneMax; ++index)
    {
        uint bone_index = vin.joints[index / 4][index % 4];
        float bone_weight = vin.weights[index / 4][index % 4];
        current_position += mul(float4(vin.position, 1), bone_transforms[bone_index]).xyz * bone_weight;
        normal += mul(float4(vin.normal, 0), bone_transforms[bone_index]).xyz * bone_weight;
#if defined(HAS_TANGENT)
        tangent += mul(float4(vin.tangent.xyz, 0), bone_transforms[bone_index]).xyz * bone_weight;
#endif  //  defined(HAS_TANGENT)

        previous_position += mul(float4(vin.position, 1), previous_bone_transforms[bone_index]).xyz * bone_weight;
    }
    
#else  //  BONE_MAX > 0
    
    float3 current_position = vin.position.xyz;
    float3 normal = vin.normal.xyz;
    float3 previous_position = vin.position.xyz;
#if defined(HAS_TANGENT)
    float3 tangent = vin.tangent.xyz;
#endif  //  defined(HAS_TANGENT)
    
#endif  //  BONE_MAX > 0
    
    vout.w_position.xyz = current_position;
    vout.w_position.w = 1.0f;
    vout.w_normal.xyz = normalize(normal).xyz;
    vout.w_normal.w = 0.0f;
#if defined(HAS_TANGENT)
    vout.w_tangent.xyz = normalize(tangent).xyz;
    vout.w_tangent.w = vin.tangent.w;
#endif  //  defined(HAS_TANGENT)

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

    vout.position = mul(float4(vout.w_position.xyz, 1), view_projection_transform);
    float4 p_position = mul(float4(previous_position, 1), previous_view_projection_transform);
    vout.velocity = calculate_uvspace_velocity(vout.position, p_position);
    return vout;
}

#endif  //  __MODEL_VS_HLSLI__
