#ifndef __LIGHTS_HLSLI__
#define __LIGHTS_HLSLI__
// 平行光源
struct directional_lights
{
    float4 direction;
    float4 color;
};

// 点光源
struct point_lights
{
	float4	position;
	float4	color;
	float	range;
	float3	dummy;
};

// スポットライト
struct spot_lights
{
	float4	position;
	float4	direction;
	float4	color;
	float	range;
	float	innerCorn;
	float	outerCorn;
	float	dummy;
};

// ライトプローブ
struct light_probes
{
	float4 position;
	float4 color;
	float4 inner_area;
	float4 outer_area;
};

// 半球ライト
struct hemisphere_lights
{
    float4 sky_color;
    float4 ground_color;
    float weight;
    float3 dummy;
};

//  ライト種別ID
static const int light_kind_directional_light = 1; //  平行光源
static const int light_kind_point_light = 2; //  点光源
static const int light_kind_spot_light = 3; //  スポットライト

//  ライト系のマスク値
static const uint light_flag_active = 1 << 0;
static const uint light_flag_use_shadow = 1 << 1;
static const uint light_flag_use_contact_shadow = 1 << 2;

static const uint light_flag_debug_csm_area_flag = 1 << 20;
static const uint light_flag_debug_visualize_ailiasing_error_flag = 1 << 21;

//  纏めたライト情報
struct compund_lights
{
    //  共有の情報
    //  work_data[3].w は共有でライトの種別番号を入れておく

    //  平行光源の場合
    //  work_data[0] = xyz = direction, w = dummy
    //  work_data[1] = color
    //  work_data[2] = dummy
    //  work_data[3] = xyz = dummy, w = ライト種別

    //  点光源の場合
    //  work_data[0] = xyz = position, w = dummy
    //  work_data[1] = color
    //  work_data[2] = x = range, yzw = dummy
    //  work_data[3] = xyz = dummy, w = ライト種別

    //  スポットライトの場合
    //  work_data[0] = xyz = position, w = dummy
    //  work_data[1] = xyz = direction, w = dummy
    //  work_data[2] = color
    //  work_data[3] = x = range, y = inner cone, z = outer cone, w = ライト種別

	//	プローブの場合
	//  work_data[0] = xyz = position, w = dummy
	//  work_data[1] = color
	//  work_data[2] = xyz = inner area, w = dummy
	//  work_data[3] = xyz = outer area, w = ライト種別
    float4 work_data[4];
};

//  光源情報
cbuffer LIGHT_CONSTANT_BUFFER : register(b3)
{
    compund_lights compund_light_data;
    uint light_attribute;
	//  コンタクトシャドウ
    float contact_shadow;
    float2 light_data_dummy;
};

//  ライト種別の取得
int get_light_kinds()
{
    return (int) (compund_light_data.work_data[3].w + 0.5f);
}

directional_lights convert_directional_lights()
{
    directional_lights data;
    data.direction = compund_light_data.work_data[0];
    data.color = compund_light_data.work_data[1];
    return data;
}

point_lights convert_point_lights()
{
    point_lights data;
    data.position = compund_light_data.work_data[0];
    data.color = compund_light_data.work_data[1];
    data.range = compund_light_data.work_data[2].x;
    return data;
}

spot_lights convert_spot_lights()
{
    spot_lights data;
    data.position = compund_light_data.work_data[0];
    data.direction = compund_light_data.work_data[1];
    data.color = compund_light_data.work_data[2];
    data.range = compund_light_data.work_data[3].x;
    data.innerCorn = compund_light_data.work_data[3].y;
    data.outerCorn = compund_light_data.work_data[3].z;
    return data;
}

light_probes convert_light_probes()
{
    light_probes light_probe;
    light_probe.position = compund_light_data.work_data[0];
    light_probe.color = compund_light_data.work_data[1];
    light_probe.inner_area = compund_light_data.work_data[2];
    light_probe.outer_area = compund_light_data.work_data[3];
    return light_probe;
}

#endif	//	__LIGHTS_HLSLI__
