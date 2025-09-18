#include "pmx_model.hlsli"

VS_OUT main(VS_IN vin)
{
    VS_OUT vout = (VS_OUT) 0;
    float3 position = 0, normal = 0;
    for (int index = 0; index < 4; ++index)
    {
        position += mul(float4(vin.position, 1), bone_transforms[vin.joints[index]]).xyz * vin.weights[index];
        normal += mul(float4(vin.normal, 0), bone_transforms[vin.joints[index]]).xyz * vin.weights[index];
    }
    vout.w_position = mul(float4(position, 1), world).xyz;
    vout.position = mul(float4(vout.w_position, 1), view_projection_transform);
    vout.w_normal = normalize(mul(float4(normal, 0), world)).xyz;
    vout.texcoord = vin.texcoord;

    float4 previous_position = mul(float4(vin.position, 1), mul(previous_world, previous_view_projection_transform));
    vout.velocity = calculate_uvspace_velocity(vout.position, previous_position);

    //  PMX‚ÍÚüî•ñ‚ª‘¶Ý‚µ‚È‚¢‚Ì‚ÅA’´‚â‚è‚Ã‚ç‚¢
    float3 tangent = float3(0, 0, 1);   //  ‚Æ‚è‚ ‚¦‚¸
    vout.w_tangent = normalize(mul(float4(tangent, 1), world));
    vout.w_tangent.w = -1;

    vout.texcoord = vin.texcoord;
    vout.outline = false;
	
    return vout;
}
