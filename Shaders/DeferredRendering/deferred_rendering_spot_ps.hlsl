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
        spot_lights spot_light = convert_spot_lights();
        float3 L = data.w_position.xyz - spot_light.position.xyz;
        float len = length(L);
        if (len < spot_light.range)
        {
            float attenuateLength = saturate(1.0f - len / spot_light.range);
            float shading_attenuation = attenuateLength * attenuateLength;
            L /= len;
            float3 spotDirection = normalize(spot_light.direction.xyz);
            float innerCorn = cos(spot_light.innerCorn);
            float outerCorn = cos(spot_light.outerCorn);
            float angle = dot(spotDirection, L);
            float area = innerCorn - outerCorn;
            shading_attenuation *= saturate(1.0f - (innerCorn - angle) / area);

            float3 shadowing_attenuation = 1;
            if (shadow_mask & (1 << data.shadow_category))
            {
                if (light_attribute & light_flag_use_shadow)
                {
                    shadow_map_sample_data shadow_data;
                    shadow_data.tex = shadow_maps;
                    shadow_data.samp = shadow_sampler_state;
                    shadowing_attenuation = shadowing_process(shadow_data, data.w_position, view_transform);
                }
            }
            
            LightingData lighting_data;
            lighting_data.shading_attenuation = shading_attenuation;
            lighting_data.shadowing_attenuation = shadowing_attenuation;
            lighting_data.direction = L;
            lighting_data.color = spot_light.color.rgb * spot_light.color.a;
            lighting_data.camera_position = camera_position.xyz;
            DirectLighting result = integrate_bxdf(data, lighting_data);
            total_diffuse += result.diffuse;
            total_specular += result.specular;
        }
    }
    return float4(total_diffuse + total_specular, 1);
}
