#include "deferred_rendering.hlsli"

//  IBL用テクスチャ
TextureCube diffuse_iem : register(t33);
TextureCube specular_pmrem : register(t34);
Texture2D lut_ggx : register(t35);
SamplerState ibl_sampler_state : register(s1);

float4 main(VS_OUT pin) : SV_TARGET
{
    //  GBufferテクスチャから情報をデコードする
    GBufferData data = get_gbuffer_data(pin.texcoord);

    float3 total_diffuse = (float3) 0, total_specular = (float3) 0;
    if (data.shading_model != shading_model_unlit)
    {
	    //	IBL処理
        if (!IsGBufferFlags(data, g_buffer_stencil_indirect_lighting_off))
        {
            float3 albedo = data.base_color;
            float3 N = normalize(data.w_normal);
            float3 V = normalize(data.w_position.xyz - camera_position.xyz);

	        //	入射光のうち拡散反射になる割合
            float3 diffuse_reflectance = lerp(albedo.rgb, 0.0f, data.metalness);

	        //	垂直反射時のフレネル反射率
            float3 F0 = lerp(0.0f, albedo.rgb, data.metalness);

            //  ライトプローブ
            light_probes light_probe = convert_light_probes();
            float3 color_adjust = light_probe.color.rgb * light_probe.color.a;

            switch (data.shading_model)
            {
                case shading_model_default_lit:
                {
                    total_diffuse += DiffuseIBL(N, V, data.roughness, diffuse_reflectance, F0, diffuse_iem, ibl_sampler_state) * color_adjust;
                    total_specular += SpecularIBL(N, V, data.roughness, F0, lut_ggx, specular_pmrem, ibl_sampler_state) * color_adjust;
                    break;
                }
                case shading_model_toon_lit:
                {
                    total_diffuse += DiffuseIBL(N, V, data.roughness, diffuse_reflectance, F0, diffuse_iem, ibl_sampler_state) * color_adjust;
                    //  スペキュラが大分邪魔になるので破棄
                    //total_specular += SpecularIBL(N, V, data.roughness, F0, lut_ggx, specular_pmrem, ibl_sampler_state) * color_adjust;
                    break;
                }
            }

    	    //	自己遮蔽
            total_diffuse = lerp(total_diffuse, total_diffuse * data.occlusion_factor, data.occlusion_strength);
            total_specular = lerp(total_specular, total_specular * data.occlusion_factor, data.occlusion_strength);
        }
    }

    return float4(total_diffuse + total_specular, 1);
}
