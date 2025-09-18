#include "sky_box.hlsli"

#include "deferred_light_data.hlsli"

TextureCube specular_pmrem : register(t0);
SamplerState samplerstate : register(s0);

PSGBufferOut main(VS_OUT pin)
{
    //  ライトプローブ
    light_probes light_probe = convert_light_probes();

    //	視線ベクトル
    float3 E = normalize(pin.world_position.xyz - camera_position.xyz);

	//	mipmap使うと崩れるのでmipmapなしになるようにしておく
    float3 base_color = specular_pmrem.SampleLevel(samplerstate, E, 0);
    base_color *= light_probe.color.rgb * light_probe.color.a;

    //  GBufferDataに出力情報をまとめる
    GBufferData data = (GBufferData)0;
    data.shading_model = shading_model_unlit;
    data.emissive_color = base_color.rgb;
    data.w_position = pin.world_position.xyz;
    data.velocity = calculate_uvspace_velocity(pin.current_clip_position, pin.previous_clip_position);
    SetGBufferFlags(data, g_buffer_stencil_filter_off, true);
    return EncodeGBuffer(data, view_projection_transform, z_buffer_parameteres);
}
