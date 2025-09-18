//  下の定義をインクルード前に設定しておくことでスレッドの数を設定
//  #define LENGTH

//  下の定義をインクルード前に設定しておくことで処理方向を設定
//  #define PASS_X
//  #define PASS_Y

Texture2D<float4> source_texture : register(t0);
RWTexture2D<float4> destination_texture : register(u0);

groupshared float4 temp_buffer[LENGTH];

[numthreads(LENGTH, 1, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
    const uint buffer_id = position.x;
    float2 texel_size;
#if defined(PASS_X)
    // 水平パス
    const uint2 texture_position = position.xy;
    source_texture.GetDimensions(texel_size.x, texel_size.y);
#endif
#if defined(PASS_Y)
    // 垂直パス
    const uint2 texture_position = position.yx;
    source_texture.GetDimensions(texel_size.y, texel_size.x);
#endif
    texel_size /= float2(LENGTH, LENGTH);

    //  テクスチャから色情報を1度だけ読み込んで共有メモリに保存
    float2 sample_position = float2(texture_position) * texel_size;
    temp_buffer[buffer_id] = source_texture.Load(int3(sample_position, 0));

    //  計算済みのバッファを確認していくことで計算量を削減する
    [unroll]
    for (int i = 0; i < log2(LENGTH); ++i)
    {
        //  計算待機
        GroupMemoryBarrierWithGroupSync();

        //  2べき乗飛ばしで再帰的に加算していく事で計算回数を削減する
        const uint delta = (0x1 << i);
        const float4 buffer = (buffer_id >= delta) ? (temp_buffer[buffer_id - delta]) : (0.0f);

        //  バッファ取得/操作するので待機
        GroupMemoryBarrierWithGroupSync();
        temp_buffer[buffer_id] += buffer;
    }

    destination_texture[texture_position] = max(0.0f, temp_buffer[buffer_id]);
}
