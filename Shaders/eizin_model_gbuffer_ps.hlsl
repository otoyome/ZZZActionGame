#include "pmx_model.hlsli"
#include "gbuffer.hlsli"
#include "scene_constant_buffer.hlsli"

#define DIFFUSE 0
#define SPHERE 1
#define RAMP 2
#define SHADOW_THRESHOLD 3
Texture2D<float4> textures[4] : register(t0);

SamplerState samplerstate : register(s0);

PSGBufferOut main(VS_OUT pin, bool is_front_face : SV_IsFrontFace)
{
    //  基本色算出
    float4 base_color = diffuse_color;
    if (texture_index.x > -1)
    {
        float4 sampled = textures[DIFFUSE].Sample(samplerstate, pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GammaFactor);
        base_color *= sampled;
    }
    clip(base_color.a - alpha_clip_threshold);
    clip(base_color.a - calculate_alpha_dithering(pin.position.xy) + alpha_dither_factor);
    
    //  ベースカラーをトップ色/シェードカラーをボトム色として算出
    //  本来はテクスチャでマッピングしておくべきだがMMDのテクスチャ的にはこれでやっておく。
    float3 shade_color = base_color.rgb;
    if (texture_index.z > -1)
    {
        shade_color *= textures[RAMP].Sample(samplerstate, float2(0.5f, 0.99f)).rgb;
    }

    //  GBufferDataに出力情報をまとめる
    GBufferData data = (GBufferData) 0;
    data.base_color = base_color.rgb;
    data.w_normal = normalize(pin.w_normal.xyz) * (is_front_face ? +1 : -1);
    data.w_position = pin.w_position.xyz;
    data.velocity = pin.velocity;
//  data.depth はEncode時はいらない
    data.shading_model = shading_model_eizin;
    data.emissive_color = 0; //  現時点では自己発光は使わないが、後々使うことになると思うので開けておく
    data.roughness = specular_color.x;
    data.metalness = specular_color.y;
    data.occlusion_factor = specular_color.z / 255.0f;
    data.occlusion_strength = specular_color.w;
    data.shade_color = shade_color;

    //  G-Bufferに対してマスク情報を送る
    data.stencil = material_attribute;

    //  コンタクトシャドウの閾値
    if (texture_index.w > -1)
    {
        //  陰の閾値
        float2 texcoord = pin.texcoord + shadow_threshold_texcoord_offset;
        data.shadow_threshold.x = textures[SHADOW_THRESHOLD].Sample(samplerstate, texcoord).r;
        data.shadow_threshold.y = textures[SHADOW_THRESHOLD].Sample(samplerstate, float2(1.0f - texcoord.x, texcoord.y)).r;

        //  メッシュの方向を指定
        data.mesh_direction.xy = normalize(mesh_direction.xy);
        data.mesh_direction.zw = normalize(mesh_direction.zw);
    }
    
    //  シャドウマスク値の設定
    data.shadow_mask = shadow_mask;

    //  輪郭線の場合は下記のようにしておく
    if (pin.outline)
    {
        data.base_color.rgb = edge_color.rgb;
        data.roughness = 1.0f;
        data.metalness = 0;
        data.shade_color.rgb = edge_color.rgb;
        data.unique_parameter = 0;
    }
    else
    {
        //  カラーラインの情報
        {
            float2 pack_data;
            pack_data.x = color_diff_line_factor;
            pack_data.x *= saturate(smoothstep(outline_adjust_parameter.y, outline_adjust_parameter.x, pin.position.w));
            pack_data.y = color_diff_line_threshold;
            data.unique_parameter.x = PackFloat2ToFloat(pack_data);
            //  色補正情報
            pack_data.x = color_line_adjust_saturate;
            pack_data.y = color_line_adjust_brightness;
            data.unique_parameter.y = PackFloat2ToFloat(pack_data);
        }

        //  法線/深度によるライン情報/ポスタライズ情報
        {
            float2 pack_data;
            pack_data.x = depth_normal_outline_factor;
            pack_data.x *= saturate(smoothstep(outline_adjust_parameter.w, outline_adjust_parameter.z, pin.position.w));
            pack_data.y = color_posterize_factor / 255.0f;
            data.unique_parameter.z = PackFloat2ToFloat(pack_data);
        }
    }
    return EncodeGBuffer(data, view_projection_transform, z_buffer_parameteres);
}

