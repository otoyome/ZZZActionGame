#include "fullscreen_quad.hlsli"
#include "shading_functions.hlsli"
#include "scene_constant_buffer.hlsli"
#include "filter_functions.hlsli"
#include "common_gbuffer_loader.hlsli"
#include "common.hlsli"

//	色調補正情報
struct color_grading_part_data
{
    float3 hsv_weight; // HSV色空間での調整係数
    float dummy;
    float3 color_multiplyer; // 乗算値
    float dummy2;
};

struct color_grading_data
{
    float intensity; // 色調補正適応量
    float stage_depth_scale; // ステージ用の深度による適応にスケール
    float2 dummy;

    //  キャラクター用
    color_grading_part_data character_data;
    
    //  ステージ用
    color_grading_part_data stage_data;
    
    //  何なら空用もありかもしれない
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
struct overwrite_text_posterize_color
{
    float step;
    float animation_rate;
    float amount;
    float dummy;
};
struct overwrite_text_datas
{
    float intensity; // 色調補正適応量
    float texcoord_scale; // 文字拡大値
    float2 texcoord_offset; // 文字移動
    float4 color; // 閾値以下の場合の色
    overwrite_text_posterize_color posterize_data;
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

//  ゆらぎ情報
struct distortion_datas
{
    float power;
    float speed;
    float amount;
    float dummy;
};

//  sin波情報
struct sin_distortion_data
{
    float height_amplitude;
    float height_angular_frequency;
    int height_postraize_count;
    float amount;
};

//  sin波情報
struct voronoi_distortion_datas
{
    int square_count;
    float amount;
	float2 dummy;
};

cbuffer COLOR_FILTER_CONSTANT_BUFFER : register(b11)
{
    color_grading_data color_grading;
    radial_blur_datas radial_blur;
    separate_color_datas separate_color;
    overwrite_text_datas overwrite_text;
    concentrated_line_work_datas concentrated_line_work;

    distortion_datas distortion_data;
    sin_distortion_data sin_distortion_datas[2];

    voronoi_distortion_datas voronoi_distortion_data;
};

Texture2D<float4> text_texture : register(t11);

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
// https://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/
#define FXAA_REDUCE_MIN (1.0f / 128.0f)
#define FXAA_REDUCE_MUL (1.0f / 8.0f)
#define FXAA_SPAN_MAX 8.0f
#define FXAA_SUBPIX_SHIFT (1.0f / 4.0f)
float3 fxaa(Texture2D color_texture, SamplerState state, float2 texcoord, float2 pixel_size)
{
    //  周辺ピクセルを参照
    float2 texcoord2 = texcoord.xy - (pixel_size * (0.5 + FXAA_SUBPIX_SHIFT));
    float3 sampled_nw = color_texture.Sample(state, texcoord2.xy).xyz;
    float3 sampled_ne = color_texture.Sample(state, texcoord2.xy, float2(1, 0)).xyz;
    float3 sampled_sw = color_texture.Sample(state, texcoord2.xy, float2(0, 1)).xyz;
    float3 sampled_se = color_texture.Sample(state, texcoord2.xy, float2(1, 1)).xyz;
    float3 sampled_m = color_texture.Sample(state, texcoord.xy).xyz;

    //  それぞれの輝度を算出
    float luma_nw = convert_rgb_to_luminance(sampled_nw);
    float luma_ne = convert_rgb_to_luminance(sampled_ne);
    float luma_sw = convert_rgb_to_luminance(sampled_sw);
    float luma_se = convert_rgb_to_luminance(sampled_se);
    float luma_m = convert_rgb_to_luminance(sampled_m);

    //  輝度差の最小最大を取得
    float luma_min = min(luma_m, min(min(luma_nw, luma_ne), min(luma_sw, luma_se)));
    float luma_max = max(luma_m, max(max(luma_nw, luma_ne), max(luma_sw, luma_se)));

    //  輝度差から向きベクトルを算出
    float2 dir;
    dir.x = -((luma_nw + luma_ne) - (luma_sw + luma_se));
    dir.y = +((luma_nw + luma_sw) - (luma_ne + luma_se));

    //  複数ピクセルを横断するようにエッジを伸ばす
    float dir_reduce = max((luma_nw + luma_ne + luma_sw + luma_se) * (0.25f * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcp_dir_min = 1.0f / (min(abs(dir.x), abs(dir.y)) + dir_reduce);
    dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcp_dir_min)) * pixel_size;

    //  向きベクトルを元に複数位置の色をブレンドして返す
    float3 rgb_a = (1.0f / 2.0f)
                 * (color_texture.Sample(state, texcoord.xy + dir * (1.0f / 3.0f - 0.5f)).xyz +
                    color_texture.Sample(state, texcoord.xy + dir * (2.0f / 3.0f - 0.5f)).xyz);
    float3 rgb_b = rgb_a * (1.0f / 2.0f)
                 + (1.0f / 4.0f) * (color_texture.Sample(state, texcoord.xy + dir * (0.0f / 3.0f - 0.5f)).xyz +
		                            color_texture.Sample(state, texcoord.xy + dir * (3.0f / 3.0f - 0.5f)).xyz);
    float luma_b = convert_rgb_to_luminance(rgb_b);
    return ((luma_b < luma_min) || (luma_b > luma_max)) ? rgb_a : rgb_b;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float noise_random(float2 seeds)
{
    return -1.0 + 2.0 * block_noise(seeds);
}

float2 voronoi(float2 texcoord, float elapsed_time, int square_count = 50)
{
    float2 st = texcoord;
    st *= square_count;


    float2 ist = floor(st); //整数
    float2 fst = frac(st); //少数の右側

    float distance = 5;
    float2 p_min;

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 neighbor = float2(x, y);
            float2 p = 0.5 + 0.5 * sin(elapsed_time + 6.2831 * random(ist + neighbor));

            float2 diff = neighbor + p - fst;

            if (distance > length(diff))
            {
                distance = length(diff);
                //最も近い白点も更新
                p_min = p;
            }
        }
    }
    
    return p_min;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float easeInOutQuad(float x)
{
    return x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
}

float easing(float x)
{
    return easeInOutQuad(x);
}

float4 calcurate_text_color(float text_mask, float2 texcoord)
{
    uint2 viewport_size = viewport_rect.zw - viewport_rect.xy;
    float aspect_ratio = viewport_size.x / viewport_size.y;

    float2 texcoord_vector = texcoord - float2(0.5f, 0.5f);
    texcoord_vector.x *= aspect_ratio;

    //  文字の輪郭線係数
    float outline_factor;
    {
        float2 texture_size;
        text_texture.GetDimensions(texture_size.x, texture_size.y);
        float2 pixel_size = 2.0f / texture_size;
        float l = text_texture.Sample(sampler_states, texcoord + float2(-pixel_size.x, 0.0f)).r;
        float r = text_texture.Sample(sampler_states, texcoord + float2(+pixel_size.x, 0.0f)).r;
        float u = text_texture.Sample(sampler_states, texcoord + float2(0.0f, -pixel_size.y)).r;
        float d = text_texture.Sample(sampler_states, texcoord + float2(0.0f, +pixel_size.y)).r;
        outline_factor = 1 - smoothstep(0, 1, abs(l + r + u + d - 2) * 0.5f);
    }

    //  文字色
    float4 color_factor = 0;
    {
        //  波紋処理
        float texcoord_length = length(texcoord_vector);
        float value = texcoord_length;
        value += lerp(1.0f, -1.0f, easing(smoothstep(0.0f, 1.0f, overwrite_text.posterize_data.animation_rate)));
        color_factor = posterize(value * 0.5f + 0.5f, 4);

        //  色のポスタライズ処理
        static const int step_max = 5;
        int step_count = clamp((int) overwrite_text.posterize_data.step, 1, step_max);
        int color_index = (int) (posterize(smoothstep(1.0f, -1.0f, value), step_count) * step_count);

        //  カラーパレットリスト
        float x_split_factor = (texcoord.x * 2.0f - 1.0f) * -1.5f; //  横方向で色を変化
        float4 color_list[step_max] =
        {
            //  半透明グレー
            float4(float3(0.25f, 0.25f, 0.25f), 0.25f),
            //  ←紫→赤
            float4(float3(1.0f, 0.0f, saturate(x_split_factor)), 1.0f),
            //  ←深青→青
            float4(float3(0.0f, 0.0f, 0.8f - 0.2f * x_split_factor), 1.0f),
            //  白
            float4(1.0f, 1.0f, 1.0f, 1.0f),
            //  ←赤橙色→橙色
            float4(float3(1.0f, saturate(0.3f - 0.15f * x_split_factor), 0.0f), 1.0f),
        };
        color_factor = color_list[color_index];

        float rate = color_factor.a;
        //  
        color_factor = lerp(overwrite_text.color, color_factor, overwrite_text.posterize_data.amount);

        //  輪郭線は色に入れ替える
        float4 c = float4(1, 1, 1, overwrite_text.posterize_data.amount);
        color_factor = lerp(color_factor, c, saturate(outline_factor - color_factor.a));
    }

    //  文字マスクファクター
    return color_factor * text_mask;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float4 main(VS_OUT pin) : SV_TARGET
{
    //  ディストーション
    {
        float elapsed_time = time_data.x;

        //  ボロノイ調整
        if (voronoi_distortion_data.amount > 0)
        {
            pin.texcoord = lerp(pin.texcoord, voronoi(pin.texcoord, elapsed_time, voronoi_distortion_data.square_count), voronoi_distortion_data.amount * 0.02f);
        }

        //  通常
        if (distortion_data.amount > 0)
        {
            //  揺れ
            float noise = block_noise((pin.texcoord.y + elapsed_time) * 1.5f);
            noise += random(pin.texcoord.y) * distortion_data.power;
            float2 random_value = noise_random(float2(pin.texcoord.y, elapsed_time * distortion_data.speed));
            float adapt_value = random_value * sin(sin(distortion_data.power) * 0.5f) * sin(-sin(noise) * 0.2f);
            adapt_value *= distortion_data.amount;
        
            pin.texcoord.x += adapt_value;
        }
        //  sin波
        for (int index = 0; index < 2; ++index)
        {
            //  揺れ位置調整
            float adapt_value = sin(PI * (pin.texcoord.y * 2.0f - 1.0f) * sin_distortion_datas[index].height_angular_frequency);
            if (sin_distortion_datas[index].height_postraize_count > 0)
            {
                //  揺れのポスタライズ処理
                adapt_value = sign(adapt_value)
                            * posterize(abs(adapt_value), sin_distortion_datas[index].height_postraize_count);
            }

            //  強度
            adapt_value *= sin_distortion_datas[index].height_amplitude;
            adapt_value *= sin_distortion_datas[index].amount;

            pin.texcoord.x += adapt_value;
        }
    }
    
    //  GBufferテクスチャから情報をデコードする
    PSGBufferTextures gbuffer_textures = get_gbuffer_textures();
    GBufferData data = get_gbuffer_data(gbuffer_textures, pin.texcoord);

    float2 scene_map_size = get_gbuffer_map_size();
    float4 color = float4(data.base_color, 1);
    color.rgb = fxaa(gbuffer_textures.base_color, gbuffer_textures.state, pin.texcoord, 1.0f / get_gbuffer_map_size());
    
    //  ラジアルブラー
    if (radial_blur.radius > 0.0f && radial_blur.sampling_count > 1)
    {
        float2 blur_vector = (radial_blur.center - pin.texcoord);

        {
            float pixel_angle = atan2(blur_vector.y, blur_vector.x);
            float wave_length = lerp(50, 100, random(pixel_angle + time_data.y));
            float amplitude = lerp(20, 50, random(pixel_angle + time_data.x));
            float value = sin(pixel_angle * wave_length + amplitude);
            blur_vector *= abs(value);
        }

        float radius = radial_blur.radius;
        blur_vector *= (radius.xx / scene_map_size.xy) / radial_blur.sampling_count;
        float rand_factor = random(pin.texcoord) * 0.25f + 0.75f;

        float4 result_color = color;
        float total_alpha = 1.0f;
        for (int index = 1; index < radial_blur.sampling_count; ++index)
        {
            float alpha = (float) (radial_blur.sampling_count - index) / radial_blur.sampling_count;
            result_color += gbuffer_textures.base_color.Sample(sampler_states, pin.texcoord + blur_vector * index) * alpha;
            total_alpha += alpha;
        }
        result_color.rgb /= total_alpha;
        result_color.a = 1;

        //  指定の範囲内は適応量を変える
        float mask_value = 1.0f;
        if (radial_blur.mask_radius > 0.0f)
        {
            float mask_radius = radial_blur.mask_radius / min(scene_map_size.x, scene_map_size.y);
            mask_value = saturate(length(pin.texcoord - radial_blur.center) / mask_radius);
        }
        color = lerp(color, result_color, mask_value);
    }

    //  色調補正
    if (color_grading.intensity > 0.0f)
    {
        //  キャラクターは白黒
        if (IsGBufferFlags(data, g_buffer_stencil_character_flag))
        {
            color.rgb = convert_rgb_to_hsv(color.rgb);
            color.rgb *= lerp(float3(1, 1, 1), color_grading.character_data.hsv_weight, color_grading.intensity);
            color.rgb = convert_hsv_to_rgb(color.rgb);
            color.rgb *= lerp(float3(1, 1, 1), color_grading.character_data.color_multiplyer, color_grading.intensity);
        }
        else
        {
            //float3 max_color = float3(1, 0.376f, 0);
            float3 adjust_color = convert_rgb_to_hsv(color.rgb);
            adjust_color *= lerp(float3(1, 1, 1), color_grading.stage_data.hsv_weight, color_grading.intensity);;
            adjust_color = convert_hsv_to_rgb(adjust_color);
            adjust_color *= lerp(float3(1, 1, 1), color_grading.stage_data.color_multiplyer, color_grading.intensity);
            //  距離に応じて補正
            color.rgb = lerp(color.rgb, adjust_color, saturate(data.linear_depth * color_grading.stage_depth_scale));
        }
    }
    
    //  輝度値から色を分割して表現
    //  ※そもそもマテリアル側にフラグを持たせて処理した方がより厳密な色分割が出来てデザイナーさん的には良いと思う('ω')
    if (separate_color.intensity > 0.0f)
    {
        float luma = convert_rgb_to_luminance(color.rgb);
        float3 mix_color = lerp(separate_color.color_min, separate_color.color_max, step(separate_color.threshold, luma));
        color.rgb = lerp(color.rgb, mix_color, separate_color.intensity);
    }
    
    //  文字書き込み処理
    if (overwrite_text.intensity > 0.0f && !IsGBufferFlags(data, g_buffer_stencil_character_flag))
    {
        float2 texcoord = (pin.texcoord - 0.5f) * overwrite_text.texcoord_scale + 0.5f;
        texcoord += overwrite_text.texcoord_offset;
        texcoord.x = saturate(texcoord.x);
        texcoord.y = saturate(texcoord.y);

        //  idの設定の仕方はかなり適当なのでこれで
        float mask = text_texture.Sample(sampler_states, texcoord).r;
        float4 text_base_color = calcurate_text_color(mask, texcoord);
        color.rgb = lerp(color.rgb, text_base_color.rgb, text_base_color.a);
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

