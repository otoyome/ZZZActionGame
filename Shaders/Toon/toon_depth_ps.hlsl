#include "toon_rendering.hlsli"

#define TEXTURESAMPLE(TEXTURE_ID, TEXCOORD_ID)  TextureSample(TEXTURE_ID, TEXCOORD_ID, pin.texcoord[TEXCOORD_ID])

void main(VS_OUT pin)
{
    //  ベースカラー
    float4 base_color = material_color[BaseMap];
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
}

