#ifndef __MODEL_CSM_GS_HLSLI__
#define __MODEL_CSM_GS_HLSLI__

#include "model_cascade_shadow_map.hlsli"

[maxvertexcount(3 * shadow_map_max)]
void main(triangle VS_SHADOW_MAP_OUT input[3], inout TriangleStream<GS_CSM_OUT> output)
{
    uint i, j, k;

    [unroll] for (k = 0; k < shadow_map_max; k++)
    {
        if (k >= shadow_map_count)
            break;

        float4x4 view_projection = light_view_projection_transform[k];
        float4 offset_parameter = shadow_bias[k];        
        [unroll] for (i = 0; i < 3; i++)
        {
            GS_CSM_OUT element = (GS_CSM_OUT) 0;
            element.render_target_array_index = k;
            element.vout.position = mul(float4(input[i].position.xyz, 1.0f), view_projection);
            //	[“x•â³
            float depth_bias = 0;
            {
                float NoL = abs(dot(float3(view_projection[0].z, view_projection[1].z, view_projection[2].z), input[i].normal));
                float slope = clamp(abs(NoL) > 0 ? sqrt(saturate(1.0f - NoL * NoL)) / NoL
                                                 : offset_parameter.z, 0.0f, offset_parameter.z );
                float slope_bias = offset_parameter.y * slope;
                depth_bias = offset_parameter.x + slope_bias;
            }
			element.vout.position.z += depth_bias;

#if VERTEX_COLOR_MAX > 0
            [unroll] for (j = 0; j < VertexColorMax; ++j)
            {
                element.vout.color[j] = input[i].color[j];
            }
#endif  //  VERTEX_COLOR_MAX > 0

#if TEXCOORD_MAX > 0
            [unroll] for (j = 0; j < TexcoordMax; ++j)
            {
                element.vout.texcoord[j] = input[i].texcoord[j];
            }
#endif  //  TEXCOORD_MAX > 0

            output.Append(element);
        }
        output.RestartStrip();
    }
}

#endif  //  __MODEL_CSM_GS_HLSLI__
