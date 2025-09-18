#ifndef __CSM_CONSTANT_BUFFER_HLSLI__
#define __CSM_CONSTANT_BUFFER_HLSLI__

static const uint shadow_map_max = 6;
static const uint shadow_map_constant_bias = 0.0001f;

//  シャドウマップ情報
cbuffer SHADOW_MAP_CONSTANT_BUFFER : register(b4)
{
    uint shadow_map_count;
    float3 shadow_attenuation;
    float4 shadow_bias[shadow_map_max];
    float4 shadow_split_distance[shadow_map_max];
    row_major float4x4 light_view_projection_transform[shadow_map_max];
	uint shadow_mask;
	float3 shadow_dummy;
};

//	シャドウサンプル用構造体
struct shadow_map_sample_data
{
	Texture2DArray<float> tex;
//	SamplerComparisonState samp;
	SamplerState samp;
};

//	シャドウサンプル
float3 shadowmap_fetch(in shadow_map_sample_data shadow_map, int index, float3 texcoord)
{
    float3 texcoordUVW = float3(texcoord.xy, index);
    float depth = shadow_map.tex.Sample(shadow_map.samp, texcoordUVW).r;
    float factor = step(texcoord.z, depth + shadow_map_constant_bias);
    return lerp(shadow_attenuation, float3(1.0f, 1.0f, 1.0f), factor);
}

float3 shadowmap_fetch_pcf(in shadow_map_sample_data shadow_map, int index, float3 texcoord, float radius = 3.0f)
{
    float3 tex_size;
    shadow_map.tex.GetDimensions(tex_size.x, tex_size.y, tex_size.z);

    //  PCFノイズ
    static const float2 noise[16] =
    {
        { 0.772417068, -0.423055708 },
        { -0.714291692, 0.509235382 },
        { -0.780199528, -0.183640003 },
        { 0.472295880, 0.162610054 },
        { -0.442540765, -0.263137937 },
        { -0.960392714, -0.472289205 },
        { -0.347602427, 0.830676079 },
        { -0.573735476, -0.472531378 },
        { -0.629811406, -0.642059684 },
        { -0.352298439, -0.320050240 },
        { 0.599728346, 0.426071405 },
        { -0.173549354, 0.653944254 },
        { -0.287484705, -0.0727353096 },
        { 0.324079156, 0.534158945 },
        { 0.968484163, -0.229418755 },
        { -0.377048135, 0.338529825 },
    };
    static const uint noise_size = 16;

    float3 attenuation = (float3) 0.0f;
    [unroll] for (uint pcf_index = 0; pcf_index < noise_size; ++pcf_index)
    {
        float3 texcoordUVW = float3(texcoord.xy + radius.xx * (noise[pcf_index] / tex_size.xy), index);
        float depth = shadow_map.tex.Sample(shadow_map.samp, texcoordUVW);
        float value = step(texcoord.z, depth + shadow_map_constant_bias);
        attenuation += value.xxx;
    }
    return lerp(shadow_attenuation, float3(1.0f, 1.0f, 1.0f), attenuation / noise_size);
}

//	シャドウサンプル
float4 shadowing_process(in shadow_map_sample_data shadow_map, float3 world_position, float4x4 view_matrix)
{
	float4 view_position = mul(float4(world_position, 1.0f), view_matrix);
	float view_depth = view_position.z / view_position.w;

    float3 light_attenuation = float3(1.0f, 1.0f, 1.0f);
    uint sm_index = shadow_map_max;
    for (uint index = 0; index < shadow_map_max; ++index)
	{
        if (index >= shadow_map_count)
            break;

		//	深度クリップ
        if (!(shadow_split_distance[index].x <= view_depth && view_depth <= shadow_split_distance[index].w))
            continue;

		//	シャドウ座標
        float4 shadow_position = mul(float4(world_position, 1.0f), light_view_projection_transform[index]);
		shadow_position /= shadow_position.w;

		//	境界クリップ
		float3 texcoord = float3(shadow_position.xy * float2(0.5f, -0.5f) + (float2)0.5f, shadow_position.z);
		if (!(all(texcoord >= (float3)0.0f) * all(texcoord <= (float3)1.0f)))
			continue;

        //light_attenuation = shadowmap_fetch(shadow_map, index, texcoord);
        light_attenuation = shadowmap_fetch_pcf(shadow_map, index, texcoord, shadow_bias[index].w);
        sm_index = index;
		break;
	}

	return float4(light_attenuation, sm_index);
}

//	デバッグシャドウサンプル
float4 debug_shadowing_process(float3 world_position, float4x4 view_matrix)
{
    float4 view_position = mul(float4(world_position, 1.0f), view_matrix);
    float view_depth = view_position.z / view_position.w;

    static const float3 debug_color[] =
    {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1),
        float3(1, 1, 0),
        float3(1, 0, 1),
        float3(0, 1, 1),
        float3(1, 1, 1),
    };

    for (uint index = 0; index < shadow_map_max; ++index)
    {
        if (index >= shadow_map_count)
            break;

		//	深度クリップ
        if (!(shadow_split_distance[index].x <= view_depth && view_depth <= shadow_split_distance[index].w))
            continue;

		//	シャドウ座標
        float4 shadow_position = mul(float4(world_position, 1.0f), light_view_projection_transform[index]);
        shadow_position /= shadow_position.w;

		//	境界クリップ
        float3 texcoord = float3(shadow_position.xy * float2(0.5f, -0.5f) + (float2) 0.5f, shadow_position.z);
        if (!(all(texcoord >= (float3) 0.0f) * all(texcoord <= (float3) 1.0f)))
            continue;

        return float4(debug_color[index], index);
    }

    return float4(0.0f, 0.0f, 0.0f, shadow_map_max);
}

//	シャドウサンプル
float3 shadow_map_visualize_aliasing(float2 texcoord, float2 shadow_map_size)
{
    static const float3 values[] =
    {
        float3(0.0f, 1.0f / 7.75f, 1.0f / 7.75f),
        float3(1.0f / 7.75f, 1.0f / 3.25f, 1.0f / 3.25f - 1.0f / 7.75f),
        float3(1.0f / 3.25f, 1.0f, 1.0f - 1.0f / 3.25f),
        float3(1.0f, 3.25f, 2.25f),
        float3(3.25f, 7.75f, 4.5f),
        float3(7.75f, 10.0f, 7.75f),
    };

    static const float3 color[] =
    {
        float3(0.2f, 0.0f, 0.0f),
        float3(1.0f, 0.2f, 0.0f),
        float3(1.0f, 1.0f, 0.0f),
        float3(0.0f, 1.0f, 0.0f),
        float3(0.3f, 0.8f, 0.0f),
        float3(0.0f, 0.0f, 1.0f),
        float3(0.0f, 0.0f, 0.2f),
    };

    float2 ds = shadow_map_size.x * ddx(texcoord);
    float2 dt = shadow_map_size.y * ddy(texcoord);
    float error = max(length(ds + dt), length(ds - dt));
    float3 result = 1.0f;
    for (uint index = 0; index < 6; ++index)
    {
        if (error >= values[index].x && error < values[index].y)
        {
            result = lerp(color[index], color[index + 1], (error - values[index].x) / values[index].z);
            break;
        }
        else
        {
            result = color[6];
        }
    }

    return result;
}

//	シャドウサンプル
float3 debug_shadowing_process_visualize_ailiasing(in shadow_map_sample_data shadow_map, float3 world_position, float4x4 view_matrix)
{
    float4 view_position = mul(float4(world_position, 1.0f), view_matrix);
    float view_depth = view_position.z / view_position.w;

    for (uint index = 0; index < shadow_map_max; ++index)
    {
        if (index >= shadow_map_count)
            break;

		//	深度クリップ
        if (!(shadow_split_distance[index].x <= view_depth && view_depth <= shadow_split_distance[index].w))
            continue;

		//	シャドウ座標
        float4 shadow_position = mul(float4(world_position, 1.0f), light_view_projection_transform[index]);
        shadow_position /= shadow_position.w;

		//	境界クリップ
        float3 texcoord = float3(shadow_position.xy * float2(0.5f, -0.5f) + (float2) 0.5f, shadow_position.z);
        if (!(all(texcoord >= (float3) 0.0f) * all(texcoord <= (float3) 1.0f)))
            continue;

        float3 tex_size;
        shadow_map.tex.GetDimensions(tex_size.x, tex_size.y, tex_size.z);
        return shadow_map_visualize_aliasing(shadow_position.xy, tex_size.xy);
    }

    return float4(0.0f, 0.0f, 0.0f, shadow_map_max);
}
#endif  //  __CSM_CONSTANT_BUFFER_HLSLI__
