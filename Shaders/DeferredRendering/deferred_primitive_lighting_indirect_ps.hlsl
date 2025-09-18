#include "deferred_primitive_lighting.hlsli"

//  IBL用テクスチャ
TextureCube diffuse_iem : register(t33);
TextureCube specular_pmrem : register(t34);
Texture2D lut_ggx : register(t35);
SamplerState ibl_sampler_state : register(s1);

//  プローブ情報
cbuffer PROBE_CONSTANT_BUFFER : register(b2)
{
    float4 probe_position;
    float3 probe_area;
    float probe_blend_rate;
};

float4 main(VS_OUT pin) : SV_TARGET
{
    //  テクスチャ座標を算出
    float2 texcoord = pin.position.xy / float2(1280, 720);

    //  GBufferテクスチャから情報をデコードする
    GBufferData data = get_gbuffer_data(texcoord);

    DirectLighting lighting_result = (DirectLighting) 0;
    if ((uint) (data.shading_model + 0.5f) != shading_model_unlit)
    {
	    //	IBL処理
        if (!IsGBufferFlags(data, g_buffer_stencil_indirect_lighting_off))
        {
            float3 albedo = data.base_color;
            float3 emissive_color = data.emissive_color;
            float3 N = normalize(data.w_normal);
            float3 V = normalize(data.w_position.xyz - camera_position.xyz);

	        //	入射光のうち拡散反射になる割合
            float3 diffuse_reflectance = lerp(albedo.rgb, 0.0f, data.metalness);

	        //	垂直反射時のフレネル反射率(非金属でも最低4%は鏡面反射する)
            float3 F0 = lerp(0.0f, albedo.rgb, data.metalness);

            lighting_result.diffuse += DiffuseIBL(N, V, data.roughness, diffuse_reflectance, F0, diffuse_iem, ibl_sampler_state);
            lighting_result.specular += SpecularIBL(N, V, data.roughness, F0, lut_ggx, specular_pmrem, ibl_sampler_state);

	        //	自己遮蔽
            OcclusionResult occlusion = integrate_occlusion(data, lighting_result);
            lighting_result.diffuse = occlusion.diffuse;
            lighting_result.specular = occlusion.specular;
        }
    }

    return float4(lighting_result.diffuse + lighting_result.specular, saturate(probe_blend_rate));
}
