#include "toon_rendering.hlsli"
#include "gbuffer.hlsli"

#define TEXTURESAMPLE(TEXTURE_ID, TEXCOORD_ID)  TextureSample(TEXTURE_ID, TEXCOORD_ID, pin.texcoord[TEXCOORD_ID])

PSGBufferOut main(VS_OUT pin, bool is_front_face : SV_IsFrontFace)
{
    GBufferData data = (GBufferData) 0;
    
    bool is_outline_pixel = pin.w_normal.w > 0.5f;
    
    float4 base_color = float4(0, 0, 0, 1);
    float4 ilm_color;
    float4 sss_color;
    float3 emisive_color;

    float roughness = 0.8f;
    float metalness = 0;
    float occlusion_factor = 0;
    float occlusion_strength = 0;
    float3 unique_parameter = 0;

    //  ベースカラー
    base_color = material_color[BaseMap];
    if (HasTexture(BaseMap))
    {
        base_color *= TEXTURESAMPLE(BaseMap, BaseTexcoord);
    }

    //  カメラからの距離に応じて透過する
    if (material_attribute & g_buffer_stencil_camera_distance_translucent)
    {
        base_color.a *= calculate_translucent_view_depth(pin.position.w);
    }

    //  アルファマスク
    if (base_color.a < 1.0f)
    {
        float dither = 1.0f;
        if (HasTexture(DisolveMap))
        {
            //  ディゾルブ
            dither *= TEXTURESAMPLE(DisolveMap, DisolveTexcoord).r;
        }
        else
        {
            //  ディザ抜き
            dither *= calculate_alpha_dithering(pin.position.xy);
        }
        clip((base_color.a - 0.003f) <= dither ? -1.0f : 1.0f);
    }

    if (is_outline_pixel)
    {
        base_color.rgb = float3(0, 0, 0);
        sss_color.rgb = float3(0, 0, 0);
        roughness = 1.0f;
        metalness = 0.0f;
        unique_parameter = 0;
    }
    else
    {
        //  照明制御
        //  R : スペキュラハイライトの強度
        //  G : 陰になりやすさ
        //      0 : 陰無し、 1 : 焼き込み影
        //  B : ハイライトの入りやすさ
        //      0 : ハイライト無し、 1 : 焼き込みハイライト
        ilm_color = material_color[IlmMap];
        if (HasTexture(IlmMap))
        {
            ilm_color *= TEXTURESAMPLE(IlmMap, IlmTexcoord);
        }

        //  SSSカラー
        //  RGB : 陰色
        //  A : 空き
        sss_color = material_color[SssMap];
        if (HasTexture(SssMap))
        {
            sss_color *= TEXTURESAMPLE(SssMap, SssTexcoord);
        }
        else
        {
            sss_color.rgb *= base_color.rgb;    //  無い場合は別途色を編集
        }

        //  自己発光
        emisive_color = material_color[EmissiveMap];
        if (HasTexture(EmissiveMap))
        {
            emisive_color *= TEXTURESAMPLE(EmissiveMap, EmissiveTexcoord);
        }

        //  パラメーター引き渡し
        roughness = material_parameters.x;
        metalness = material_parameters.y;
        occlusion_factor = material_parameters.z / 255.0f;
        occlusion_strength = material_parameters.w;
        
        //  カラーラインの情報
        {
            float2 pack_data;
            pack_data.x = color_diff_line_factor;
            pack_data.x *= saturate(smoothstep(outline_adjust_parameter.y, outline_adjust_parameter.x, pin.position.w));
            pack_data.y = color_diff_line_threshold;
            unique_parameter.x = PackFloat2ToFloat(pack_data);

            //  色補正情報
            pack_data.x = color_line_adjust_saturate;
            pack_data.y = color_line_adjust_brightness;
            unique_parameter.y = PackFloat2ToFloat(pack_data);
        }

        //  法線/深度によるライン情報/ポスタライズ情報
        {
            float2 pack_data;
            pack_data.x = depth_normal_outline_factor;
            pack_data.x *= saturate(smoothstep(outline_adjust_parameter.w, outline_adjust_parameter.z, pin.position.w));
            pack_data.y = color_posterize_factor / 255.0f;
            unique_parameter.z = PackFloat2ToFloat(pack_data);
        }
    }

    //  GBufferDataに出力情報をまとめる
    data.base_color = base_color.rgb;
    data.shading_model = is_outline_pixel ? shading_model_unlit : shading_model_toon_lit;
    data.emissive_color = emisive_color;
    data.w_normal = normalize(pin.w_normal.xyz) * (is_front_face ? +1 : -1);
    data.w_position = pin.w_position.xyz;
    data.velocity = pin.velocity;
    data.roughness = roughness;
    data.metalness = metalness;
    data.occlusion_factor = occlusion_factor;
    data.occlusion_strength = occlusion_strength;
    data.stencil = material_attribute;
    data.unique_parameter = unique_parameter;

    data.shade_color = sss_color.rgb;
    data.shadow_category = shadow_category;
    return EncodeGBuffer(data, view_projection_transform, z_buffer_parameteres);
}

