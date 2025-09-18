#include "toon_rendering.hlsli"

[maxvertexcount(6)]
void main(triangle VS_OUT input[3], inout TriangleStream<VS_OUT> output)
{
    int i, j;

	//	•\–Ê“o˜^
    [unroll] for (i = 0; i < 3; i++)
    {
        VS_OUT element = (VS_OUT) 0;
        element.position = input[i].position;
        element.w_position = input[i].w_position;
        element.w_normal = input[i].w_normal;
        element.w_normal.w = 0.0f;
#if defined(HAS_TANGENT)
        element.w_tangent = input[i].w_tangent;
#endif  //  defined(HAS_TANGENT)

#if VERTEX_COLOR_MAX > 0
        [unroll] for (j = 0; j < VertexColorMax; ++j)
        {
            element.color[j] = input[i].color[j];
        }
#endif  //  VERTEX_COLOR_MAX > 0

#if TEXCOORD_MAX > 0
        [unroll] for (j = 0; j < TexcoordMax; ++j)
        {
            element.texcoord[j] = input[i].texcoord[j];
        }
#endif  //  TEXCOORD_MAX > 0
        element.velocity = input[i].velocity;
        output.Append(element);
    }
    output.RestartStrip();

    //  ƒJƒƒ‰‚©‚ç‚Ì‹——£‚É‚æ‚éŒW”ŽZo
    float3 vp[3] =
    {
        mul(float4(input[0].w_position.xyz, 1), view_transform).xyz,
        mul(float4(input[1].w_position.xyz, 1), view_transform).xyz,
        mul(float4(input[2].w_position.xyz, 1), view_transform).xyz
    };
    float3 vn[3] =
    {
        normalize(mul(float4(input[0].w_normal.xyz, 0), view_transform).xyz),
        normalize(mul(float4(input[1].w_normal.xyz, 0), view_transform).xyz),
        normalize(mul(float4(input[2].w_normal.xyz, 0), view_transform).xyz)
    };
    float face_depth_factor = saturate(smoothstep(outline_depth_range.x, outline_depth_range.y, ((vp[0] + vp[1] + vp[2]) / 3.0f).z));

    //  —ÖŠsüˆ—
    [unroll] for (i = 2; i >= 0; i--)
    {
        VS_OUT element = (VS_OUT) 0;

        //  –{—ˆ‚Í’¸“_ƒJƒ‰[‚É“o˜^‚µ‚½î•ñ‚ðŒ³‚É‚µ‚Ä’¸“_‚ð–c‚ç‚Ü‚·ˆ—‚ð’²®‚µ‚½‚è‚µ‚Ü‚·B
        //  R : ‰A‰e’²®‚ÅŽg—p‚µ‚½‚è‚·‚é‚Ì‚Å‚±‚±‚Å‚ÍŽg—p‚µ‚È‚¢
        //  G : ƒJƒƒ‰‚©‚ç‚Ì‹——£‚É‰ž‚¶‚Ä—ÖŠsü‚ð‚Ç‚Ì’ö“x–c’£‚³‚¹‚é‚©‚ÌŒW”
        //  B : —ÖŠsü‚ÌZƒIƒtƒZƒbƒg’l
        //  A : —ÖŠsü‚Ì‘¾‚³ŒW”B0.5‚ª•W€‚ÅC1‚É‚·‚é‚ÆMAX‚Ì‘¾‚³C0‚É‚·‚é‚Æ—ÖŠsü‚È‚µ‚É‚È‚é
        float4 vin_color = float4(0.0f, 0.0f, 0.2f, 0.5f);
        float3 outline_parameter = vin_color.yzw;

        float3 v_position = vp[i];
        float3 v_normal = vn[i];

        //  ƒJƒƒ‰‚©‚ç‚Ì‹——£‚É‚æ‚éŒW”ŽZo
        float depth_factor = lerp(0.0f, outline_depth_factor * face_depth_factor, outline_parameter.x);

        //  ZƒIƒtƒZƒbƒg
        v_position.xyz += normalize(v_position.xyz) * outline_parameter.y * outline_z_offset_max;

        //  —ÖŠsü‚Ì–c’£ˆ—
        float vertex_outline_size = outline_size;
        vertex_outline_size *= clamp(outline_parameter.z * 2.0f, 0.0f, 2.0f);
        vertex_outline_size += depth_factor;
        v_position.xyz += v_normal * vertex_outline_size;

        //  
        element.position = mul(float4(v_position.xyz, 1), projection_transform);
        element.w_position = input[i].w_position;
        element.w_normal.xyz = input[i].w_normal.xyz;
        element.w_normal.w = 1.0f;
#if defined(HAS_TANGENT)
        element.w_tangent = input[i].w_tangent;
#endif  //  defined(HAS_TANGENT)

#if VERTEX_COLOR_MAX > 0
        [unroll] for (j = 0; j < VertexColorMax; ++j)
        {
            element.color[j] = input[i].color[j];
        }
#endif  //  VERTEX_COLOR_MAX > 0

#if TEXCOORD_MAX > 0
        [unroll] for (j = 0; j < TexcoordMax; ++j)
        {
            element.texcoord[j] = input[i].texcoord[j];
        }
#endif  //  TEXCOORD_MAX > 0
        element.velocity = input[i].velocity;
        output.Append(element);
    }

    output.RestartStrip();
}
