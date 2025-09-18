#include "sky_box.hlsli"

TextureCube specular_pmrem : register(t0);
SamplerState samplerstate : register(s0);

PSGBufferOut main(VS_OUT pin)
{
    //	視線ベクトル
    float3 E = normalize(pin.world_position.xyz - camera_position.xyz);

	//	mipmap使うと崩れるのでmipmapなしになるようにしておく
    float3 base_color = specular_pmrem.SampleLevel(samplerstate, E, 0).xyz * pin.color.xyz;

    //  GBufferDataに出力情報をまとめる
    GBufferData data = (GBufferData) 0;
    data.shading_model = shading_model_unlit; //  ライティングさせない
    data.emissive_color = base_color.rgb; //  空は自己発光扱い
    data.w_position = pin.world_position.xyz; //  一応ワールド座標を入れておく
    data.velocity = pin.velocity; //  速度ベクトルも入れておく
    SetGBufferFlags(data, g_buffer_stencil_filter_off, true);
    return EncodeGBuffer(data, view_projection_transform, z_buffer_parameteres);
}
