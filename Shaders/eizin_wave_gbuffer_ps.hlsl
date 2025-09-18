#include "pmx_model.hlsli"
#include "gbuffer.hlsli"
#include "scene_constant_buffer.hlsli"

#define DIFFUSE 0
#define SPHERE 1
#define RAMP 2
Texture2D<float4> textures[3] : register(t0);

SamplerState samplerstate : register(s0);

PSGBufferOut main(VS_OUT pin, bool is_front_face : SV_IsFrontFace)
{
    //  基本色算出
    float4 base_color = diffuse_color;
    if (texture_index.x > -1)
    {
        float time_scale = 0.000025f;
        float rand_texcoordY = random(pin.texcoord + options.z * time_scale) * 0.0001f;
        float2 texcoord = frac(pin.texcoord + frac(float2(0.0f, rand_texcoordY)));
        float4 sampled = textures[DIFFUSE].Sample(samplerstate, texcoord);

        sampled.rgb = pow(sampled.rgb, GammaFactor);
        base_color *= sampled;
    }

    //  波として利用
    if (texture_index.y > -1)
    {
        float rand_texcoord = random(pin.texcoord + 0.0f) * 0.000001f;
        float time_scale = 0.000025f;
        float2 texcoord1 = frac(pin.texcoord + float2(rand_texcoord * 1, 0.0f) * 128.0f + float2(0.0f, rand_texcoord + options.z * time_scale) * 256.0f);
        float2 texcoord2 = frac(pin.texcoord + float2(rand_texcoord * 2, 0.3f) * 64.0f + float2(0.0f, rand_texcoord + options.z * time_scale) * 512.0f);
        float2 texcoord3 = frac(pin.texcoord + float2(rand_texcoord * 3, 0.6f) * 32.0f + float2(0.0f, rand_texcoord + options.z * time_scale) * 768.0f);
        float2 texcoord4 = frac(pin.texcoord + float2(rand_texcoord * 4, 0.9f) * 16.0f + float2(0.0f, rand_texcoord + options.z * time_scale) * 1024.0f);
        float2 texcoord5 = frac(pin.texcoord + float2(rand_texcoord * 5, 1.2f) * 8.0f + float2(0.0f, rand_texcoord + options.z * time_scale) * 1280.0f);

        float4 sampled = textures[SPHERE].Sample(samplerstate, texcoord1);
        sampled += textures[SPHERE].Sample(samplerstate, texcoord2);
        sampled += textures[SPHERE].Sample(samplerstate, texcoord3);
        sampled += textures[SPHERE].Sample(samplerstate, texcoord4);
        sampled += textures[SPHERE].Sample(samplerstate, texcoord5);
        sampled = sampled / 5.0f;

        float4 c = posterize(1 - sampled.a, 4);
        float alpha = step(c.a, 0.5f);
        base_color.rgb += alpha;
        //base_color.a *= alpha;
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
    GBufferData data = (GBufferData)0;
    data.base_color = base_color.rgb;
    data.w_normal = normalize(pin.w_normal.xyz) * (is_front_face ? +1 : -1);
    data.w_position = pin.w_position.xyz;
    data.velocity = pin.velocity;
//  data.depth はEncode時はいらない
    data.shading_model = shading_model_eizin;
    data.emissive_color = 0;    //  現時点では自己発光は使わないが、後々使うことになると思うので開けておく
    data.roughness = specular_color.x;
    data.metalness = specular_color.y;
    data.occlusion_factor = specular_color.z / 255.0f;
    data.occlusion_strength = specular_color.w;
    data.shade_color = shade_color;
    data.stencil = material_attribute;
    
    //  シャドウマスク値の設定
    data.shadow_mask = shadow_mask;

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

    //  輪郭線の場合は下記のようにしておく
    if (pin.outline)
    {
        data.base_color.rgb = edge_color.rgb;
        data.roughness = 1.0f;
        data.metalness = 0;
        data.shade_color.rgb = edge_color.rgb;
    }
    return EncodeGBuffer(data, view_projection_transform, z_buffer_parameteres);
}

