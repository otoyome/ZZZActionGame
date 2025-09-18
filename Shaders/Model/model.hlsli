//  下記の定数を定義してインクルードする事
//  #define HAS_TANGENT         : 接線を所持しているかどうか
//  #define VERTEX_COLOR_MAX    : 頂点カラー数
//  #define TEXCOORD_MAX        : UV座標数
//  #define BONE_MAX            : ボーンの最大数
#ifndef __MODEL_HLSLI__
#define __MODEL_HLSLI__

#include	"lights.hlsli"
#include    "common.hlsli"

#include	"scene_constant_buffer.hlsli"

#include    "gbuffer.hlsli"

static const uint VertexColorMax = VERTEX_COLOR_MAX;
static const uint TexcoordMax = TEXCOORD_MAX;
static const uint AlignmentBoneMax = alignment( BONE_MAX, 4 ) / 4;
static const uint BoneMax = BONE_MAX;

struct VS_DEPTH_IN
{
    float3 position : POSITION;
#if VERTEX_COLOR_MAX > 0
    float4 color[VertexColorMax] : COLOR;
#endif  //  VERTEX_COLOR_MAX > 0
#if TEXCOORD_MAX > 0
    float2 texcoord[TexcoordMax] : TEXCOORD;
#endif  //  TEXCOORD_MAX > 0
#if BONE_MAX > 0
    uint4 joints[AlignmentBoneMax] : JOINTS;
    float4 weights[AlignmentBoneMax] : WEIGHTS;
#endif  //  BONE_MAX > 0
};

struct VS_DEPTH_OUT
{
    float4 position : SV_POSITION;
#if VERTEX_COLOR_MAX > 0
    float4 color[VertexColorMax] : COLOR;
#endif  //  VERTEX_COLOR_MAX > 0
#if TEXCOORD_MAX > 0
    float2 texcoord[TexcoordMax] : TEXCOORD;
#endif  //  TEXCOORD_MAX > 0
};

struct VS_SHADOW_MAP_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
#if VERTEX_COLOR_MAX > 0
    float4 color[VertexColorMax] : COLOR;
#endif  //  VERTEX_COLOR_MAX > 0
#if TEXCOORD_MAX > 0
    float2 texcoord[TexcoordMax] : TEXCOORD;
#endif  //  TEXCOORD_MAX > 0
#if BONE_MAX > 0
    uint4 joints[AlignmentBoneMax] : JOINTS;
    float4 weights[AlignmentBoneMax] : WEIGHTS;
#endif  //  BONE_MAX > 0
};

struct VS_SHADOW_MAP_OUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
#if VERTEX_COLOR_MAX > 0
    float4 color[VertexColorMax] : COLOR;
#endif  //  VERTEX_COLOR_MAX > 0
#if TEXCOORD_MAX > 0
    float2 texcoord[TexcoordMax] : TEXCOORD;
#endif  //  TEXCOORD_MAX > 0
};

struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
#if defined(HAS_TANGENT)
    float4 tangent : TANGENT;
#endif  //  defined(HAS_TANGENT)
#if VERTEX_COLOR_MAX > 0
    float4 color[VertexColorMax] : COLOR;
#endif  //  VERTEX_COLOR_MAX > 0
#if TEXCOORD_MAX > 0
    float2 texcoord[TexcoordMax] : TEXCOORD;
#endif  //  TEXCOORD_MAX > 0
#if BONE_MAX > 0
    uint4 joints[AlignmentBoneMax] : JOINTS;
    float4 weights[AlignmentBoneMax] : WEIGHTS;
#endif  //  BONE_MAX > 0
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 w_position : POSITION;   //  xyz : ワールド座標, w = 空き
    float4 w_normal : NORMAL; //  xyz : ワールド法線, w = 空き
#if defined(HAS_TANGENT)
    float4 w_tangent : TANGENT; //  xyz : ワールド接線, w = 従法線方向
#endif  //  defined(HAS_TANGENT)
#if VERTEX_COLOR_MAX > 0
    float4 color[VertexColorMax] : COLOR;
#endif  //  VERTEX_COLOR_MAX > 0
#if TEXCOORD_MAX > 0
    float2 texcoord[TexcoordMax] : TEXCOORD;
#endif  //  TEXCOORD_MAX > 0
    float2 velocity : VELOCITY;
};

#if BONE_MAX > 0
static const uint MeshBoneMax = 512;
cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 bone_transforms[MeshBoneMax];
};

cbuffer PREVIOUS_PRIMITIVE_CONSTANT_BUFFER : register(b2)
{
    row_major float4x4 previous_bone_transforms[MeshBoneMax];
};
#endif  //  BONE_MAX > 0

#endif  //  __MODEL_HLSLI__
