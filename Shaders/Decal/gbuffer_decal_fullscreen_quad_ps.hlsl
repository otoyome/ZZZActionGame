#include "gbuffer_decal_fullscreen_quad.hlsli"

Texture2D<float4> base_texture : register(t10);
Texture2D<float4> normal_depth_texture : register(t11);
Texture2D<uint4> unique_data : register(t12);
SamplerState sampler_states : register(s10);

//  ピクセルシェーダーへの出力用構造体
struct PSOut
{
    float4 base_color : SV_TARGET0;
    float4 normal_depth : SV_TARGET1;
};

PSOut main(VS_OUT pin)
{
    float2 gbuffer_texcoord = pin.texcoord.xy;

    //  キャラクターフラグが立っていたら描きこまない
    GBufferData gbuffer_data = DecodeGBufferStencilOnly(unique_data, gbuffer_texcoord);
    clip(IsGBufferFlags(gbuffer_data, g_buffer_stencil_character_flag) ? -1.0f : 1.0f);

    //  ワールド座標生成
    float4 normal_depth = normal_depth_texture.Sample(sampler_states, gbuffer_texcoord);
    float3 normal = normalize(normal_depth.rgb);
    float depth = convert_linear_depth_to_projection_depth(normal_depth.a, z_buffer_parameteres);
    float4 w_position = float4(gbuffer_texcoord.xy * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), depth, 1.0f);
    w_position = mul(w_position, inverse_view_projection_transform);
    w_position = w_position / w_position.w;

    //  法線と向きから適応させるかどうかを決める
    clip(dot(normalize(decal_direction.xyz), normal) >= normal_angle ? -1 : 1);

    //  キューブ基準の空間の座標に変換してデカールのUV座標に変換
    float4 cube_texture_position = mul(w_position, decal_inverse_transform);
    cube_texture_position /= cube_texture_position.w;
    cube_texture_position.xy = cube_texture_position.xy * float2(+0.5f, -0.5f) + float2(0.5f, 0.5f);

    float4 color = base_texture.Sample(sampler_states, cube_texture_position.xy);

    //  アルファマスク
    float dither = calculate_alpha_dithering(pin.position.xy);
    clip(color.a <= dither ? -1.0f : 1.0f);

    //  GBufferDataに出力情報をまとめる
    PSOut pout = (PSOut) 0;
    pout.base_color.rgb = color.rgb;
    pout.base_color.a = shading_model_toon_lit;
    pout.normal_depth.xyz = normalize(normal.xyz);
    pout.normal_depth.a = normal_depth.a;
    return pout;
}
