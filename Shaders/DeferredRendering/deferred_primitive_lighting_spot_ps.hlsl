#include "deferred_primitive_lighting.hlsli"

//  シャドウマップ用テクスチャ
Texture2D shadow_map : register(t10);
SamplerState shadow_sampler_state : register(s10);

float4 main(VS_OUT pin) : SV_TARGET
{
    //  テクスチャ座標を算出
    float2 texcoord = pin.position.xy / float2(1280, 720);

    //  GBufferテクスチャから情報をデコードする
    GBufferData data = get_gbuffer_data(texcoord);

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

            float shadowing_attenuation = 1;
#if 0
            if (use_shadow)
            {
	            //  シャドウマップ用のパラメーター計算
                float3 shadow_texcoord;
	            {
		            // ライトから見たNDC座標を算出
                    float4 wvpPos = mul(float4(data.w_position.xyz, 1.0f), light_view_projection);
		            // NDC座標からUV座標を算出する
                    wvpPos /= wvpPos.w;
                    wvpPos.y = -wvpPos.y;
                    wvpPos.xy = 0.5f * wvpPos.xy + 0.5f;
                    shadow_texcoord = wvpPos.xyz;
                }

                float depth = shadow_map.Sample(shadow_sampler_state, shadow_texcoord.xy).r;
                if (shadow_texcoord.z - depth > shadow_bias)
                {
                    shadowing_attenuation *= shadow_attenuation;
                }
            }
#endif

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
