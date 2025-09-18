#include "fullscreen_quad.hlsli"
#include "shading_functions.hlsli"
#include "scene_constant_buffer.hlsli"
#include "filter_functions.hlsli"
#include "common_gbuffer_loader.hlsli"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  法線と深度による輪郭線
float calculate_normal_depth_outline(in GBufferData center, in GBufferData data[4], float threshold = 0.5f)
{
    //  周辺に
    float4 bitmask;
    bitmask.x = IsGBufferFlags(data[0], g_buffer_stencil_normal_depth_outline_off);
    bitmask.y = IsGBufferFlags(data[1], g_buffer_stencil_normal_depth_outline_off);
    bitmask.z = IsGBufferFlags(data[2], g_buffer_stencil_normal_depth_outline_off);
    bitmask.w = IsGBufferFlags(data[3], g_buffer_stencil_normal_depth_outline_off);
    float4 linear_depth;
    linear_depth.x = data[0].linear_depth;
    linear_depth.y = data[1].linear_depth;
    linear_depth.z = data[2].linear_depth;
    linear_depth.w = data[3].linear_depth;
    float4 depth = lerp(linear_depth, center.linear_depth.xxxx, bitmask);

    float3 normal[4];
    normal[0] = lerp(data[0].w_normal, center.w_normal, bitmask.x);
    normal[1] = lerp(data[1].w_normal, center.w_normal, bitmask.y);
    normal[2] = lerp(data[2].w_normal, center.w_normal, bitmask.z);
    normal[3] = lerp(data[3].w_normal, center.w_normal, bitmask.w);
    
    float diff_depth00_11 = depth.x - depth.y;
    float diff_depth10_01 = depth.z - depth.w;
    float3 diff_norm00_11 = normal[0] - normal[1];
    float3 diff_norm10_01 = normal[2] - normal[3];

    float depth_outline_value = diff_depth00_11 * diff_depth00_11 + diff_depth10_01 * diff_depth10_01;
    float3 normal_outline_values = diff_norm00_11 * diff_norm00_11 + diff_norm10_01 * diff_norm10_01;
    float normal_outline_value = (normal_outline_values.x + normal_outline_values.y + normal_outline_values.z) / 3;
    float outline_value = (depth_outline_value + normal_outline_value) / 2;
    return (outline_value - threshold > 0) ? 0 : 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  深度による輪郭線
float calculate_depth_outline(in GBufferData data[4], float threshold = 0.5f)
{
    float diff_depth00_11 = data[0].linear_depth - data[1].linear_depth;
    float diff_depth10_01 = data[2].linear_depth - data[3].linear_depth;

    float outline_value = diff_depth00_11 * diff_depth00_11 + diff_depth10_01 * diff_depth10_01;
    return (outline_value - threshold > 0) ? 0 : 1;
}
//  法線による輪郭線
float calculate_normal_outline(in GBufferData data[4], float threshold = 0.5f)
{
    float3 diff_norm00_11 = data[0].w_normal - data[1].w_normal;
    float3 diff_norm10_01 = data[2].w_normal - data[3].w_normal;

    float3 normal_outline_values = diff_norm00_11 * diff_norm00_11 + diff_norm10_01 * diff_norm10_01;
    float outline_value = (normal_outline_values.x + normal_outline_values.y + normal_outline_values.z) / 3;
    return (outline_value - threshold > 0) ? 0 : 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  色による輪郭線
float calculate_color_outline(in GBufferData data[4], float threshold = 0.5f)
{
#if 01
    //  輝度で輪郭線を抽出
    float diff_luma00_11 = convert_rgb_to_luminance(data[0].base_color) - convert_rgb_to_luminance(data[1].base_color);
    float diff_luma10_01 = convert_rgb_to_luminance(data[2].base_color) - convert_rgb_to_luminance(data[3].base_color);
#else
    //  彩度で輪郭線を抽出
    float diff_luma00_11 = convert_rgb_to_hsv(data[0].base_color).g - convert_rgb_to_hsv(data[1].base_color).g;
    float diff_luma10_01 = convert_rgb_to_hsv(data[2].base_color).g - convert_rgb_to_hsv(data[3].base_color).g;
#endif
    float depth_outline_value = diff_luma00_11 * diff_luma00_11 + diff_luma10_01 * diff_luma10_01;
    return ((depth_outline_value - threshold > 0) ? 0 : 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float4 main(VS_OUT pin) : SV_TARGET
{
    //  GBufferテクスチャから情報をデコードする
    PSGBufferTextures gbuffer_textures = get_gbuffer_textures();
    GBufferData data = get_gbuffer_data(gbuffer_textures, pin.texcoord);

    float2 scene_map_size = get_gbuffer_map_size();

    float4 color = float4(data.base_color, 1);
    float4 old_color = color;

    if (!IsGBufferFlags(data, g_buffer_stencil_filter_off))
    {
        float2 offsets = 1.0f;
        GBufferData sub_data[4];
        sub_data[0] = DecodeGBuffer(gbuffer_textures, pin.texcoord + float2(-offsets.x, -offsets.y) / scene_map_size, inverse_view_projection_transform, z_buffer_parameteres);
        sub_data[1] = DecodeGBuffer(gbuffer_textures, pin.texcoord + float2(+offsets.x, +offsets.y) / scene_map_size, inverse_view_projection_transform, z_buffer_parameteres);
        sub_data[2] = DecodeGBuffer(gbuffer_textures, pin.texcoord + float2(+offsets.x, -offsets.y) / scene_map_size, inverse_view_projection_transform, z_buffer_parameteres);
        sub_data[3] = DecodeGBuffer(gbuffer_textures, pin.texcoord + float2(-offsets.x, +offsets.y) / scene_map_size, inverse_view_projection_transform, z_buffer_parameteres);

        float outline_threshold, posterize_step;
        {
            float2 packed_data;
            //  法線・深度によるライン情報/ポスタライズ情報
            packed_data = UnpackFloatToFloat2(data.unique_parameter.z);
            outline_threshold = packed_data.x;
            posterize_step = packed_data.y * 255.0f;
        }

        //  計算で使用するためリマップ
        sub_data[0].base_color.rgb = posterize(sub_data[0].base_color.rgb, posterize_step);
        sub_data[1].base_color.rgb = posterize(sub_data[1].base_color.rgb, posterize_step);
        sub_data[2].base_color.rgb = posterize(sub_data[2].base_color.rgb, posterize_step);
        sub_data[3].base_color.rgb = posterize(sub_data[3].base_color.rgb, posterize_step);

        //  色をポスタライズ
        if (!IsGBufferFlags(data, g_buffer_stencil_color_posterize_off))
        {
            color.rgb = posterize(color.rgb, posterize_step);
        }

        //  法線と深度で輪郭線
        if (!IsGBufferFlags(data, g_buffer_stencil_normal_depth_outline_off))
        {
            color.rgb *= calculate_normal_depth_outline(data, sub_data, outline_threshold);
        }

        //  ノイズによるムラ
        if (!IsGBufferFlags(data, g_buffer_stencil_noise_off))
        {
            //  ノイズ係数算出
            float noise_factor = block_noise(pin.texcoord * 350) > 0.3f ? 1.0f : 0.5f;

            float lumminance = saturate(convert_rgb_to_luminance(color.rgb) * 2.0f);

            float3 noise_color = convert_rgb_to_hsv(color.rgb);
            noise_color.g *= noise_factor;
            noise_color = convert_hsv_to_rgb(noise_color);

            noise_factor = saturate(noise_factor) * data.occlusion_strength;
            color.rgb = lerp(color.rgb, noise_color, noise_factor);
        }

        //  色差分による輪郭線
        if (!IsGBufferFlags(data, g_buffer_stencil_color_outline_off))
        {
            float2 packed_data;

            packed_data = UnpackFloatToFloat2(data.unique_parameter.x);
            float color_diff_line_factor = packed_data.x;
            float color_diff_line_threshold = packed_data.y;

            packed_data = UnpackFloatToFloat2(data.unique_parameter.y);
            float color_line_adjust_saturate = packed_data.x;
            float color_line_adjust_brightness = packed_data.y;

            float colorize_outline_factor = calculate_color_outline(sub_data, color_diff_line_threshold);
            float3 colorize_outline_color = color.rgb * color.rgb;

            colorize_outline_color = convert_rgb_to_hsv(colorize_outline_color);
            colorize_outline_color.g *= color_line_adjust_saturate;
            colorize_outline_color.b *= color_line_adjust_brightness;
            colorize_outline_color = convert_hsv_to_rgb(colorize_outline_color);

            colorize_outline_factor = smoothstep(colorize_outline_factor, 1.0f, color_diff_line_factor);
            color.rgb = lerp(color.rgb, colorize_outline_color, colorize_outline_factor);
        }

        color = saturate(color);
    }
    return color;
}

