#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

float3 gamma(float3 x)
{
	//	return  pow(x, 1.0f / 2.2f)
	return  x < 0.0031308f ? 12.92f * x : 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
}

float3 degamma(float3 x)
{
	//	return  pow(x, 2.2)
	return  x < 0.04045f ? x / 12.92f : pow( (x + 0.055f) / 1.055f, 2.4f );
}

#define	alignment( s, a )		( ( s + ( a - 1 ) ) & ~( a - 1 ) )

float2 convert_ndcspaceposition_xy_to_uv(float2 ndc)
{
    return float2(ndc.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f));
}

float2 convert_uv_to_ndcspaceposition_xy(float2 uv)
{
    return float2(uv.xy * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f));
}

float calculate_translucent_view_depth(float view_depth, float distance = 10.0f, float adjust_distance_add = -5.0f)
{
    return saturate((view_depth + adjust_distance_add) / distance);
}

static const uint dither_pattern_16 = 0;
static const uint dither_pattern_64 = 1;
float calculate_alpha_dithering(float2 screen_position, uint dither_pattern_index = dither_pattern_64)
{
    float dither = 0.0f;
    uint sx = (int) screen_position.x;
    uint sy = (int) screen_position.y;
    uint x = 0, y = 0;
    switch(dither_pattern_index)
    {
        case dither_pattern_16:
        {
                static const float dither_pattern[16] =
            {
                 0.0f / 16.0f,  8.0f / 16.0f,  2.0f / 16.0f, 10.0f / 16.0f,
                12.0f / 16.0f,  4.0f / 16.0f, 14.0f / 16.0f,  6.0f / 16.0f,
                 3.0f / 16.0f, 11.0f / 16.0f,  1.0f / 16.0f,  9.0f / 16.0f,
                15.0f / 16.0f,  7.0f / 16.0f, 13.0f / 16.0f,  5.0f / 16.0f
            };
            x = sx % 4;
            y = sy % 4;
            dither = (float) dither_pattern[x + y * 4];
            break;
        }
        default:
        case dither_pattern_64:
        {
            static const float dither_pattern[64] =
            {
                 0.0f / 64.0f, 48.0f / 64.0f, 12.0f / 64.0f, 60.0f / 64.0f,  3.0f / 64.0f, 51.0f / 64.0f, 15.0f / 64.0f, 63.0f / 64.0f,
                32.0f / 64.0f, 16.0f / 64.0f, 44.0f / 64.0f, 28.0f / 64.0f, 35.0f / 64.0f, 19.0f / 64.0f, 47.0f / 64.0f, 31.0f / 64.0f,
                 8.0f / 64.0f, 56.0f / 64.0f,  4.0f / 64.0f, 52.0f / 64.0f, 11.0f / 64.0f, 59.0f / 64.0f,  7.0f / 64.0f, 55.0f / 64.0f,
                40.0f / 64.0f, 24.0f / 64.0f, 36.0f / 64.0f, 20.0f / 64.0f, 43.0f / 64.0f, 27.0f / 64.0f, 39.0f / 64.0f, 23.0f / 64.0f,
                 2.0f / 64.0f, 50.0f / 64.0f, 14.0f / 64.0f, 62.0f / 64.0f,  1.0f / 64.0f, 49.0f / 64.0f, 13.0f / 64.0f, 61.0f / 64.0f,
                34.0f / 64.0f, 18.0f / 64.0f, 46.0f / 64.0f, 30.0f / 64.0f, 33.0f / 64.0f, 17.0f / 64.0f, 45.0f / 64.0f, 29.0f / 64.0f,
                10.0f / 64.0f, 58.0f / 64.0f,  6.0f / 64.0f, 54.0f / 64.0f,  9.0f / 64.0f, 57.0f / 64.0f,  5.0f / 64.0f, 53.0f / 64.0f,
                42.0f / 64.0f, 26.0f / 64.0f, 38.0f / 64.0f, 22.0f / 64.0f, 41.0f / 64.0f, 25.0f / 64.0f, 37.0f / 64.0f, 21.0f / 64.0f
                };
            x = sx % 8;
            y = sy % 8;
            dither = (float) dither_pattern[x + y * 8];
            break;
        }
    }
    return dither;
}

float random(float2 seeds)
{
    return frac(sin(dot(seeds, float2(12.9898f, 78.233f))) * 43758.5453f);
}

float block_noise(float2 seeds)
{
    return random(floor(seeds));
}

#endif  //  __COMMON_HLSLI__
