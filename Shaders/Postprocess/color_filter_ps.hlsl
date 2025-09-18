#include "fullscreen_quad.hlsli"
#include "shading_functions.hlsli"
#include "scene_constant_buffer.hlsli"
#include "filter_functions.hlsli"
#include "common_gbuffer_loader.hlsli"
#include "common.hlsli"

//	色調補正情報
struct color_grading_data
{
    float intensity; // 色調補正適応量
    float3 hsv_weight; // HSV色空間での調整係数
    float3 color_multiplyer; // 乗算値
    float dummy;
};

//	ラジアルブラー情報
struct radial_blur_datas
{
    float radius;
    int sampling_count;
    float2 center;

    float mask_radius;
    float3 dummy;
};

//	色の分割処理情報
struct separate_color_datas
{
    float intensity; // 色調補正適応量
    float3 color_min; // 閾値以下の場合の色
    float threshold; // 色閾値
    float3 color_max; // 閾値以上の場合の色
};

//	文字描画処理
struct overwrite_text_datas
{
    float intensity; // 色調補正適応量
    float texcoord_scale; // 文字拡大値
    float2 texcoord_offset; // 文字移動
    float4 color; // 閾値以下の場合の色
};

//	集中線処理
struct concentrated_line_work_datas
{
    float intensity; // 色調補正適応量
    float use_animation; // アニメーション使用フラグ
    float animation_speed; // アニメーション速度
    float pattern_seed; // パターンシード値

    float noise_scale; // ノイズ拡大値
    float3 color; // 集中線色

    float2 edge; // エッジ
    float2 dummy;
};

cbuffer COLOR_FILTER_CONSTANT_BUFFER : register(b11)
{
    color_grading_data color_grading;
    radial_blur_datas radial_blur;
    separate_color_datas separate_color;
    overwrite_text_datas overwrite_text;
    concentrated_line_work_datas concentrated_line_work;
};

Texture2D<float4> text_texture : register(t11);

//引数に渡された対象座標の角度を0~1の範囲に圧縮して返す。
//反時計周りに徐々に値が大きくなる。
float2 calculate_uv_angle(float2 uv)
{
    float2 fixed_uv = uv * 2 - 1;
    float angle = atan2(fixed_uv.y, fixed_uv.x);
    return angle / (PI * 2);
}

float calculate_radial_line(float angle)
{
    float2 line_seed = floor(float2(angle, angle) * concentrated_line_work.noise_scale);
    float line_value = random(line_seed);
    return lerp(sin(line_value * concentrated_line_work.pattern_seed),
                sin(line_value * time_data.x * concentrated_line_work.animation_speed),
                concentrated_line_work.use_animation);
}

float calculate_center_circle(float2 uv)
{
    float2 fixed_uv = uv * 2 - 1;
    return length(fixed_uv);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float4 main(VS_OUT pin) : SV_TARGET
{
    //  GBufferテクスチャから情報をデコードする
    PSGBufferTextures gbuffer_textures = get_gbuffer_textures();
    GBufferData data = get_gbuffer_data(gbuffer_textures, pin.texcoord);

    float2 scene_map_size = get_gbuffer_map_size();
    float4 color = float4(data.base_color, 1);
    
    //  ラジアルブラー
    if (radial_blur.radius > 0.0f && radial_blur.sampling_count > 1)
    {
        float4 result_color = color;
        
        float2 blur_vector = (radial_blur.center - pin.texcoord);

        //if (blur_adjust)
        {
            //  sin関数は-1～+1の値を算出するので、
            //  ブラーベクトルから角度を求めて、
            //  sin 関数を用いて振幅値を求める。
            float pixel_angle = atan2(blur_vector.y, blur_vector.x);
            float wave_length = lerp(50, 100, random(pixel_angle + time_data.y));
            float amplitude = lerp(20, 50, random(pixel_angle + time_data.x));
            float value = sin(pixel_angle * wave_length + amplitude);
            blur_vector *= abs(value);
        }

        float radius = radial_blur.radius;  //  lerp(radial_blur.radius * 0.5f, radial_blur.radius, random(time_data.y));
        blur_vector *= (radius.xx / scene_map_size.xy) / radial_blur.sampling_count;
        float rand_factor = random(pin.texcoord) * 0.25f + 0.75f;
        float total_alpha = 1.0f;
        for (int index = 1; index < radial_blur.sampling_count; ++index)
        {
            float alpha = (float) (radial_blur.sampling_count - index) / radial_blur.sampling_count;
            result_color += gbuffer_textures.base_color.Sample(sampler_states, pin.texcoord + blur_vector * index) * alpha;
            total_alpha += alpha;
        }

        //  指定の範囲内は適応量を変える
        float mask_value = 1.0f;
        if (radial_blur.mask_radius > 0.0f)
        {
            float mask_radius = radial_blur.mask_radius / min(scene_map_size.x, scene_map_size.y);
            mask_value = saturate(length(pin.texcoord - radial_blur.center) / mask_radius);
        }

        color = lerp(color, result_color / total_alpha, mask_value);
    }

    //  色調補正
    if (color_grading.intensity > 0.0f)
    {
        if (!IsGBufferFlags(data, g_buffer_stencil_character_flag))
        {
            color.rgb = convert_rgb_to_hsv(color.rgb);
            color.rgb *= color_grading.intensity * color_grading.hsv_weight;
            color.rgb = convert_hsv_to_rgb(color.rgb);
            color.rgb *= color_grading.color_multiplyer;
        }
    }
    
    //  輝度値から色を分割して表現
    //  ※そもそもマテリアル側のフラグを見て処理した方がより厳密な色分割が出来て良いと思う('ω')
    if (separate_color.intensity > 0.0f)
    {
        float luma = convert_rgb_to_luminance(color.rgb);
        float3 mix_color = lerp(separate_color.color_min, separate_color.color_max, step(separate_color.threshold, luma));
        color.rgb = lerp(color.rgb, mix_color, separate_color.intensity);
    }
    
    //  文字書き込み処理
    if (overwrite_text.intensity > 0.0f)
    {
        float2 texcoord = (pin.texcoord - 0.5f) * overwrite_text.texcoord_scale + 0.5f;
        texcoord += overwrite_text.texcoord_offset;
        texcoord.x = saturate(texcoord.x);
        texcoord.y = saturate(texcoord.y);

        float alpha = text_texture.Sample(sampler_states, texcoord).r * overwrite_text.color.a;
        float character = !IsGBufferFlags(data, g_buffer_stencil_character_flag);

        color.rgb = lerp(color.rgb, overwrite_text.color.rgb, saturate(alpha * character));
    }

    //  集中線
    if (concentrated_line_work.intensity > 0.0f)
    {
        float angle = calculate_uv_angle(pin.texcoord);
        float line_value = calculate_radial_line(angle);
        float circle = calculate_center_circle(pin.texcoord);
        float result_line = saturate(line_value * circle);
        float smooth_alpha = smoothstep(concentrated_line_work.edge.x, concentrated_line_work.edge.y, result_line);

        color.rgb = lerp(color.rgb, concentrated_line_work.color, smooth_alpha);
    }
    
    return color;
}

