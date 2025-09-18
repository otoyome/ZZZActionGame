#include "deferred_rendering.hlsli"

//  シャドウマップ用テクスチャ
Texture2D shadow_map : register(t10);
SamplerState shadow_sampler_state : register(s10);

float4 main(VS_OUT pin) : SV_TARGET
{
    //  GBufferテクスチャから情報をデコードする
    GBufferData data = get_gbuffer_data(pin.texcoord);

    float3 total_diffuse = (float3) 0, total_specular = (float3) 0;
    if (data.shading_model != shading_model_unlit)
    {
        point_lights point_light = convert_point_lights();
        float3 L = data.w_position.xyz - point_light.position.xyz;
        float len = length(L);
        if (len < point_light.range)
        {
            L /= len;
            
            float attenuateLength = saturate(1.0f - len / point_light.range);
            float shading_attenuation = attenuateLength * attenuateLength;
            
            float shadowing_attenuation = 1;
            
            LightingData lighting_data;
            lighting_data.shading_attenuation = shading_attenuation;
            lighting_data.shadowing_attenuation = shadowing_attenuation;
            lighting_data.direction = L;
            lighting_data.color = point_light.color.rgb * point_light.color.a;
            lighting_data.camera_position = camera_position.xyz;
            DirectLighting result = integrate_bxdf(data, lighting_data);
            total_diffuse += result.diffuse;
            total_specular += result.specular;
        }
    }

    return float4(total_diffuse + total_specular, 1);
}
