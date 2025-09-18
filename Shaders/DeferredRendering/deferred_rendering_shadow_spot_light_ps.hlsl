#include "deferred_rendering.hlsli"

//  シャドウマップ用テクスチャ
Texture2DArray<float> shadow_maps : register(t10);
SamplerState shadow_sampler_state : register(s10);

float4 main(VS_OUT pin) : SV_TARGET
{
    //  GBufferテクスチャから情報をデコードする
    GBufferData data = get_gbuffer_data(pin.texcoord);

    float3 multi_value = (float3) 1;

    bool same_shadow_category = shadow_mask & (1 << data.shadow_category);
    if ((data.shading_model != shading_model_unlit) && !same_shadow_category)
    {
        spot_lights spot_light = convert_spot_lights();
        float3 L = data.w_position.xyz - spot_light.position.xyz;
        float len = length(L);
        L /= len;
        if (len < spot_light.range && dot(-L, data.w_normal.xyz) >= 0.0f)
        {
            //  シャドウィング
            shadow_map_sample_data shadow_data;
            shadow_data.tex = shadow_maps;
            shadow_data.samp = shadow_sampler_state;
            float4 shadow_color = shadowing_process(shadow_data, data.w_position, view_transform);
            multi_value = shadow_color.rgb;

            //  コンタクトシャドウ
            if (shadow_color.z < shadow_map_max &&
                contact_shadow > 0.0f &&
                IsGBufferFlags(data, g_buffer_stencil_contact_shadow_on))
            {
                contact_shadow_data shadow_data;
                shadow_data.normal_depth_map = gbuffer_normal_depth;
                shadow_data.samp_state = sampler_states;
                shadow_data.world_position = data.w_position;
                shadow_data.world_normal = data.w_normal;
                shadow_data.march_direction = -L;
                shadow_data.march_length = contact_shadow;
                float value = integrate_contact_shadow(data, shadow_data, 32, false);
                multi_value = lerp(shadow_attenuation, float3(1.0f, 1.0f, 1.0f), value);
            }
        }
    }

    return float4(multi_value, 1);
}
