#include "sky_box.hlsli"

VS_OUT main(uint vin : SV_VERTEXID)
{
    VS_OUT vout;
    vout.position.xy = float2(vin % 2, vin / 2) * float2(2.0f, -2.0f) - float2(1.0f, -1.0f);
    vout.position.z = 1;
    vout.position.w = 1;

    //  NDC座標からワールド座標へ変換
    vout.world_position = mul(vout.position, inverse_view_projection_transform);
    vout.world_position /= vout.world_position.w;

    //  速度ベクトル計算用座標を算出
    vout.current_clip_position = vout.position;
    vout.previous_clip_position = mul(vout.world_position, previous_view_projection_transform);
    vout.previous_clip_position /= vout.previous_clip_position.w;
    return vout;
}
