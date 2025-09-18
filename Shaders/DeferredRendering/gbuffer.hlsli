#ifndef __GBUFFER_HLSLI__
#define __GBUFFER_HLSLI__

//  シェーディングモデル定数
static const uint shading_model_unlit       = 0;    //  シェーディング計算：ライティング無し
static const uint shading_model_default_lit = 1;    //  シェーディング計算：基本計算
static const uint shading_model_toon_lit    = 2;    //  シェーディング計算：トゥーン計算

//  G-Bufferステンシル
static const uint g_buffer_stencil_indirect_lighting_off        = 1 <<  0;  //  間接光の処理を適応させない
static const uint g_buffer_stencil_camera_distance_translucent  = 1 <<  1;  //  カメラからの距離に応じて透過する
static const uint g_buffer_stencil_character_flag               = 1 <<  2;  //  キャラクター識別フラグ

static const uint g_buffer_stencil_filter_off                   = 1 << 20;  //  フィルター効果の適応フラグ
static const uint g_buffer_stencil_normal_depth_outline_off     = 1 << 21;  //  深度/法線によるライン描画フラグ
static const uint g_buffer_stencil_color_outline_off            = 1 << 22;  //  色によるライン描画フラグ
static const uint g_buffer_stencil_noise_off                    = 1 << 23;  //  英人特有の影部分に対してのノイズ描画フラグ
static const uint g_buffer_stencil_color_posterize_off          = 1 << 24;  //  色のポスタライズ適応フラグ(ただし、カラーラインの方はポスタライズを元にして検知しているので注意)
static const uint g_buffer_stencil_light_direction_ignore_y     = 1 << 25;  //  シェーディング時にライトのYを無視する
static const uint g_buffer_stencil_contact_shadow_on            = 1 << 26;  //  コンタクトシャドウ適応フラグ

//static const uint g_buffer_stencil_ = 1 << 31;

//  ピクセルシェーダーへの出力用構造体
struct PSGBufferOut
{
    float4 base_color : SV_TARGET0;
    float4 emissive_color : SV_TARGET1;
    float4 normal_depth : SV_TARGET2;
    float4 parameter : SV_TARGET3;
    float4 velocity : SV_TARGET4;
    uint4 unique_data : SV_TARGET5;

    //  G-Buffer追加分
    float4 shade_color : SV_TARGET6;
};

//  GBuffer情報構造体
struct GBufferData
{
    float3 base_color;          //  ベースカラー
    uint shading_model;         //  シェーディング方式
    float3 emissive_color;      //  自己発光色
    float3 w_normal;            //  ワールド法線
    float3 w_position;          //  ワールド座標
    float2 velocity;            //  速度
    float linear_depth;         //  線形深度(Decode時のみ)
    float roughness;            //  粗さ
    float metalness;            //  金属質
    float occlusion_factor;     //  遮蔽値
    float occlusion_strength;   //  遮蔽強度
    uint stencil;               //  ビットフラグ(4バイト=32フラグ)

    float3 unique_parameter;    //  シェーディングモデル毎に変わる変数(32ビット浮動小数点)

    //  G-Buffer追加分
    float3 shade_color;         //  トゥーン用：陰色
    uint shadow_category;       //  シャドウマップ用マスク値(1バイト=8フラグ)
    float2 shadow_threshold;    //  影の閾値(x : ←, y : →)
    float4 mesh_direction;      //  メッシュの向き情報 xy = 右方向, zw = 前方向
};

//  GBufferテクスチャ受け渡し用構造体
struct PSGBufferTextures
{
    SamplerState state;
    Texture2D<float4> base_color;
    Texture2D<float4> emissive_color;
    Texture2D<float4> normal_depth;
    Texture2D<float4> parameter;
    Texture2D<float4> velocity;
    Texture2D<uint4> unique_data;

    //  G-Buffer追加分
    Texture2D<float4> shade_color;
};

//  圧縮
uint PackFloat2ToUInt(float2 value)
{
    uint a16 = f32tof16(value.x);
    uint b16 = f32tof16(value.y);
    uint abPacked = (a16 << 16) | b16;
    return abPacked;
}

float PackFloat2ToFloat(float2 value)
{
    return asfloat(PackFloat2ToUInt(value));
}

//  解凍
float2 UnpackUIntToFloat2(uint packedValue)
{
    float2 result;
    result.x = f16tof32(packedValue >> 16);
    result.y = f16tof32(packedValue);
    return result;
}
float2 UnpackFloatToFloat2(float packedValue)
{
    return UnpackUIntToFloat2(asuint(packedValue));
}

//  UV空間上の速度計算用関数
float2 calculate_uvspace_velocity(float4 current_clip_position, float4 previous_clip_position)
{
    current_clip_position /= current_clip_position.w;
    previous_clip_position /= previous_clip_position.w;
    return (current_clip_position.xy - previous_clip_position.xy) * 0.5f;
}

//  非線形深度から線形深度へ変換
float convert_projection_depth_to_linear_depth(float depth, float4 z_buffer_parameteres)
{
    return 1.0f / (z_buffer_parameteres.x * depth + z_buffer_parameteres.y);
}

//  線形深度から非線形深度へ変換
float convert_linear_depth_to_projection_depth(float depth, float4 z_buffer_parameteres)
{
    return (1.0f - z_buffer_parameteres.y * depth) / (z_buffer_parameteres.x * depth);
    //return (1.0f / depth - z_buffer_parameteres.y) / z_buffer_parameteres.x;
}

//  GBufferDataに纏めた情報をピクセルシェーダーの出力用構造体に変換
PSGBufferOut EncodeGBuffer(in GBufferData data, matrix view_projection_matrix, float4 z_buffer_parameteres)
{
    PSGBufferOut ret = (PSGBufferOut) 0;
    ret.base_color.rgb = data.base_color;
    ret.base_color.a = data.shading_model / 32.0f;

    ret.emissive_color.rgb = data.emissive_color;

    ret.normal_depth.rgb = data.w_normal;
    float4 position = mul(float4(data.w_position, 1.0f), view_projection_matrix);
    ret.normal_depth.a = convert_projection_depth_to_linear_depth(position.z / position.w, z_buffer_parameteres);

    ret.parameter.r = data.occlusion_factor;
    ret.parameter.g = data.roughness;
    ret.parameter.b = data.metalness;
    ret.parameter.a = data.occlusion_strength;
    
    ret.velocity.r = PackFloat2ToFloat(data.velocity * 0.5f + 0.5f);
    ret.velocity.g = data.unique_parameter.x;
    ret.velocity.b = data.unique_parameter.y;
    ret.velocity.a = data.unique_parameter.z;
    
    ret.unique_data.x = data.stencil;

    //  追加分
    ret.shade_color.yzw = data.shade_color.rgb;

    ret.emissive_color.a = data.shadow_category / 255.0f;

    ret.unique_data.y = PackFloat2ToUInt(data.shadow_threshold.xy);
    ret.unique_data.z = PackFloat2ToUInt(data.mesh_direction.xy * 0.5f + 0.5f);
    ret.unique_data.w = PackFloat2ToUInt(data.mesh_direction.zw * 0.5f + 0.5f);

    return ret;
};

//  ステンシルのみデコード
GBufferData DecodeGBufferStencilOnly(Texture2D<uint4> unique_data, float2 uv)
{
    GBufferData ret = (GBufferData) 0;
    uint2 texture_size;
    unique_data.GetDimensions(texture_size.x, texture_size.y);
    uint4 unique = unique_data.Load(uint3((uint2) (uv * texture_size), 0));
    ret.stencil = unique.x;
    return ret;
}

//  ピクセルシェーダーの出力用構造体からGBufferData情報に変換
GBufferData DecodeGBuffer(PSGBufferTextures textures, float2 uv, matrix inverse_view_projection_transform, float4 z_buffer_parameteres)
{
    uint2 texture_size;
    textures.unique_data.GetDimensions(texture_size.x, texture_size.y);

    //  各テクスチャから情報を取得
    float4 base_color = textures.base_color.SampleLevel(textures.state, uv, 0);
    float4 emissive_color = textures.emissive_color.SampleLevel(textures.state, uv, 0);
    float4 normal_depth = textures.normal_depth.SampleLevel(textures.state, uv, 0);
    float4 parameter = textures.parameter.SampleLevel(textures.state, uv, 0);
    float4 velocity = textures.velocity.SampleLevel(textures.state, uv, 0);
    uint4 unique_data = textures.unique_data.Load(uint3((uint2) (uv * texture_size), 0));

    float4 shade_color = textures.shade_color.SampleLevel(textures.state, uv, 0);

    GBufferData ret = (GBufferData) 0;
    ret.base_color = base_color.rgb;
    ret.shading_model = (int) (base_color.a * 32.0f + 0.5f);
    ret.emissive_color = emissive_color.rgb;
    ret.w_normal = normal_depth.rgb;
    ret.linear_depth = normal_depth.a;
    ret.occlusion_factor = parameter.r;
    ret.roughness = parameter.g;
    ret.metalness = parameter.b;
    ret.occlusion_strength = parameter.a;
    ret.velocity = UnpackFloatToFloat2(velocity.r * 2.0f - 1.0f);

    ret.unique_parameter.x = velocity.g;
    ret.unique_parameter.y = velocity.b;
    ret.unique_parameter.z = velocity.a;
    
    ret.stencil = unique_data.x;

    float4 position = float4(uv.xy * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), convert_linear_depth_to_projection_depth(ret.linear_depth, z_buffer_parameteres), 1);
    position = mul(position, inverse_view_projection_transform);
    ret.w_position = position.xyz / position.w;

    //  追加分
    ret.shade_color.rgb = shade_color.yzw;

    ret.shadow_category = (uint) (emissive_color.a * 255.0f + 0.5f);

    ret.shadow_threshold = UnpackUIntToFloat2(unique_data.y);
    ret.mesh_direction.xy = UnpackUIntToFloat2(unique_data.z) * 2.0f - 1.0f;
    ret.mesh_direction.zw = UnpackUIntToFloat2(unique_data.w) * 2.0f - 1.0f;

    return ret;
}

//  GBufferステンシルのフラグが立っているか判定
bool IsGBufferFlags(GBufferData data, uint bit_flag)
{
    return data.stencil & bit_flag;
}

void SetGBufferFlags(inout GBufferData data, uint bit_flag, bool flag)
{
    data.stencil = flag ? (data.stencil | bit_flag) : (data.stencil & ~bit_flag);
}

#endif  //  __GBUFFER_HLSLI__
