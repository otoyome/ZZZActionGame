#ifndef __FILTER_FUNCTIONS_HLSLI__
#define __FILTER_FUNCTIONS_HLSLI__

#include "common.hlsli"

//--------------------------------------------
//	RGB色空間の数値からHSV色空間の数値への変換関数
//--------------------------------------------
//rgb:RGB色空間の数値
float3 convert_rgb_to_hsv( float3 rgb )
{
    float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = (rgb.g < rgb.b) ? float4(rgb.bg, K.wz) : float4(rgb.gb, K.xy);
    float4 q = (rgb.r < p.x) ? float4(p.xyw, rgb.r) : float4(rgb.r, p.yzx);
    float d = q.x - min(q.w, q.y);
    float eps = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0 * d + eps)), d / (q.x + eps), q.x);
}

//--------------------------------------------
//	HSV色空間の数値からRGB色空間の数値への変換関数
//--------------------------------------------
//hsv:HSV色空間の数値
float3	convert_hsv_to_rgb( float3 hsv )
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(hsv.xxx + K.xyz) * 6.0 - K.www);
    return hsv.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), clamp(hsv.y, 0.0, 1.0));
}

//--------------------------------------------
//	RGB色から輝度への変換関数
//--------------------------------------------
float convert_rgb_to_luminance(float3 rgb)
{
    static const float3 luminanceValue = float3(0.299f, 0.587f, 0.114f);
    return dot(luminanceValue, rgb);
}

//--------------------------------------------
//	クワハラフィルター
//	https://qiita.com/Cartelet/items/5c1c012c132be3aa9608
//--------------------------------------------
float4 sample_quadrant(Texture2D color_texture, SamplerState samp_state, float2 color_texture_size, float2 uv, int x1, int x2, int y1, int y2, float n)
{
    float luminance_sum = 0.0f;
    float luminance_sum2 = 0.0f;
    float3 col_sum = float3(0, 0, 0);
    
     [unroll]
    for (int x = x1; x <= x2; ++x)
    {
        for (int y = y1; y <= y2; ++y)
        {
            float3 sample = color_texture.Sample(samp_state, uv + float2(x, y) / color_texture_size.xy).rgb;
            float l = dot(sample, float3(0.299f, 0.587f, 0.114f));
            luminance_sum += l;
            luminance_sum2 += l * l;
            col_sum += saturate(sample);
        }
    }

    float mean = luminance_sum / n;
    float std = abs(luminance_sum2 / n - mean * mean);

    return float4(col_sum / n, std);
}
float4 kuwahara_filter(Texture2D color_texture, SamplerState samp_state, float2 color_texture_size, float2 uv, float radius)
{
    uint seed = 0;
    float Range = (radius * 2) + 1;
    int minKernelSize = floor(Range);
    int maxKernelSize = ceil(Range);
    float t = frac(Range);
    
    float windowSize = 2.0f * maxKernelSize + 1;
    int quadSize = ceil(windowSize / 2);
    int numSamples = quadSize * quadSize;

    float4 q1 = sample_quadrant(color_texture, samp_state, color_texture_size, uv, -minKernelSize, 0, -minKernelSize, 0, numSamples);
    float4 q2 = sample_quadrant(color_texture, samp_state, color_texture_size, uv, 0, minKernelSize, -minKernelSize, 0, numSamples);
    float4 q3 = sample_quadrant(color_texture, samp_state, color_texture_size, uv, 0, minKernelSize, 0, minKernelSize, numSamples);
    float4 q4 = sample_quadrant(color_texture, samp_state, color_texture_size, uv, -minKernelSize, 0, 0, minKernelSize, numSamples);


    float minstd = min(q1.a, min(q2.a, min(q3.a, q4.a)));
    int4 q = float4(q1.a, q2.a, q3.a, q4.a) == minstd;
    
    float4 result1 = 0;
    if (dot(q, 1) > 1)
        result1 = saturate(float4((q1.rgb + q2.rgb + q3.rgb + q4.rgb) / 4.0f, 1.0f));
    else
        result1 = saturate(float4(q1.rgb * q.x + q2.rgb * q.y + q3.rgb * q.z + q4.rgb * q.w, 1.0f));

    windowSize = 2.0f * maxKernelSize + 1;
    quadSize = int(ceil(windowSize / 2.0f));
    numSamples = quadSize * quadSize;

    q1 = sample_quadrant(color_texture, samp_state, color_texture_size, uv, -minKernelSize, 0, -minKernelSize, 0, numSamples);
    q2 = sample_quadrant(color_texture, samp_state, color_texture_size, uv, 0, minKernelSize, -minKernelSize, 0, numSamples);
    q3 = sample_quadrant(color_texture, samp_state, color_texture_size, uv, 0, minKernelSize, 0, minKernelSize, numSamples);
    q4 = sample_quadrant(color_texture, samp_state, color_texture_size, uv, -minKernelSize, 0, 0, minKernelSize, numSamples);
    float4 result2 = 0;
    if (dot(q, 1) > 1)
        result2 = saturate(float4((q1.rgb + q2.rgb + q3.rgb + q4.rgb) / 4.0f, 1.0f));
    else
        result2 = saturate(float4(q1.rgb * q.x + q2.rgb * q.y + q3.rgb * q.z + q4.rgb * q.w, 1.0f));

    return lerp(result1, result2, t);
}

#endif	//	__FILTER_FUNCTIONS_HLSLI__
