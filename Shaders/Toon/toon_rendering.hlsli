#ifndef __TOON_RENDERING_HLSLI__
#define __TOON_RENDERING_HLSLI__

#define HAS_TANGENT
#define VERTEX_COLOR_MAX    2
#define TEXCOORD_MAX        3
#define BONE_MAX            8
#include	"model.hlsli"

static const uint BaseMap = 0;
static const uint IlmMap = 1;
static const uint SssMap = 2;
static const uint EmissiveMap = 3;
static const uint DecalMap = 4;
static const uint TextMap = 5;
static const uint DisolveMap = 6;
static const uint FaceThresholdMap = 7;
static const uint TextureMax = 8;

static const uint BaseTexcoord = 0;
static const uint IlmTexcoord = 0;
static const uint SssTexcoord = 0;
static const uint EmissiveTexcoord = 0;
static const uint DecalTexcoord = 1;
static const uint TextTexcoord = 2;
static const uint DisolveTexcoord = 0;
static const uint FaceThresholdTexcoord = 0;

Texture2D<float4> textures[TextureMax] : register(t0);
SamplerState sampler_states[TextureMax] : register(s0);

cbuffer MATERIAL_CONSTANT_BUFFER : register(b5)
{
    int4    texture_index[TextureMax / 4];  // テクスチャ番号
    float4  material_color[TextureMax];     // マテリアルカラー

    uint    texture_srgb_flags;             // テクスチャがSRGB色空間で生成されたものかを判断
    uint    material_attribute;             // マテリアル側のフラグ類
	uint    game_material_attribute;        // 未使用
    float   alpha_clip_threshold;           // 透明/不透明閾値

    //  材質パラメーター
    float4  material_parameters;            // r : 粗さ, g : 金属質, b : 陰影諧調数, a : ノイズ係数

    //  背面法による輪郭線情報
    float   outline_z_offset_max;           //  Zオフセット距離
    float   outline_depth_factor;           //  膨張係数
    float2  outline_depth_range;            //  膨張範囲
    
    float   outline_size;                   //  輪郭線の基本サイズ
	uint    shadow_category;	            //	シャドウカテゴリー
    float2  shadow_threshold_uv_offset;	    //	影閾値マップのオフセット

    float4  mesh_direction;	                //	メッシュの方向(xy : 右ベクトル, zw : 左ベクトル)

    //  ポストエフェクトの色差によるライン算出係数
    float   color_diff_line_factor;         // アウトライン係数
    float   color_diff_line_threshold;      // 検知係数
    float   color_line_adjust_saturate;     //  ライン彩度調整係数
    float   color_line_adjust_brightness;   //  ライン明度調整係数

    float   color_posterize_factor;         //  色のポスタライズ数
    float   depth_normal_outline_factor;    //  ポストエフェクトの法線/深度によるアウトライン算出係数
    uint2   material_constant_dummy;
    float4  outline_adjust_parameter;       //  輪郭線調整距離パラメーター
};

//  テクスチャ使用可否
bool HasTexture(uint texture_id)
{
    return texture_index[texture_id / 4][texture_id % 4] > -1;
}

//  テクスチャサンプル
float4 TextureSample(uint texture_id, uint texcoord_id, float2 texcoord)
{
    float4 sampled = textures[texture_id].Sample(sampler_states[texcoord_id], texcoord);
    if (texture_srgb_flags & (1 << texture_id))
    {
        sampled.rgb = degamma(sampled.rgb);
    }

    return  sampled;
}

#endif  //  __TOON_RENDERING_HLSLI__
