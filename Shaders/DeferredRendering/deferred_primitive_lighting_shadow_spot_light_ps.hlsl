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

    float3 multi_value = (float3) 1;

#if 0
    //  シャドウマスクされていたら除外
    if (data.shading_model != shading_model_unlit && (shadow_mask & (1 << data.shadow_category)))
    {
        spot_lights spot_light = convert_spot_lights();
        float3 L = data.w_position.xyz - spot_light.position.xyz;
        float len = length(L);
        if (len < spot_light.range)
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
            float factor = shadow_texcoord.z - depth < shadow_bias;
            multi_value = lerp(shadow_attenuation, 1, factor);
        }
    }
#endif
    return float4(multi_value, 1);
}
