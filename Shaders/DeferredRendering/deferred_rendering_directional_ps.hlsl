#include "deferred_rendering.hlsli"

//  シャドウマップ用テクスチャ
Texture2DArray<float> shadow_maps : register(t10);
SamplerState shadow_sampler_state : register(s10);

float4 main(VS_OUT pin) : SV_TARGET
{
    //  GBufferテクスチャから情報をデコードする
    GBufferData data = get_gbuffer_data(pin.texcoord);

    float3 total_diffuse = (float3) 0, total_specular = (float3) 0;
    if (data.shading_model != shading_model_unlit)
    {
        directional_lights directional_light = convert_directional_lights();
        float3 L = normalize(directional_light.direction.xyz);

        float shading_attenuation = 1;
        float4 shadowing_attenuation = 1.0f;
        
        //  影処理
        if (shadow_mask & (1 << data.shadow_category))
        {
            if (light_attribute & light_flag_use_shadow)
            {
                shadow_map_sample_data shadow_data;
                shadow_data.tex = shadow_maps;
                shadow_data.samp = shadow_sampler_state;
                if (light_attribute & light_flag_debug_csm_area_flag)
                    return float4(debug_shadowing_process(data.w_position, view_transform).xyz, 1);
                else if (light_attribute & light_flag_debug_visualize_ailiasing_error_flag)
                    return float4(debug_shadowing_process_visualize_ailiasing(shadow_data, data.w_position, view_transform).xyz, 1);
                else
                    shadowing_attenuation = shadowing_process(shadow_data, data.w_position, view_transform);
            }
        }

        LightingData lighting_data;
        lighting_data.shading_attenuation = shading_attenuation;
        lighting_data.shadowing_attenuation = shadowing_attenuation;
        lighting_data.direction = L;
        lighting_data.color = directional_light.color.rgb * directional_light.color.a;
        lighting_data.camera_position = camera_position.xyz;
        DirectLighting result = integrate_bxdf(data, lighting_data);
        total_diffuse += result.diffuse;
        total_specular += result.specular;
    }
    return float4(total_diffuse + total_specular, 1);
}
