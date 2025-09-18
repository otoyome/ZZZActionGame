//  下記の定数を定義してインクルードする事
//  #define HAS_TANGENT         : 接線を所持しているかどうか
//  #define VERTEX_COLOR_MAX    : 頂点カラー数
//  #define TEXCOORD_MAX        : UV座標数
//  #define BONE_MAX            : ボーンの最大数
#ifndef __MODEL_CSM_HLSLI__
#define __MODEL_CSM_HLSLI__

#include	"model.hlsli"
#include    "shadow_map_constant_buffer.hlsli"

struct GS_CSM_OUT
{
    VS_DEPTH_OUT vout;
    uint render_target_array_index : SV_RENDERTARGETARRAYINDEX;
};

#endif  //  __MODEL_CSM_HLSLI__
