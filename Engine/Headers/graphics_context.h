#pragma once

#include "light.h"
#include "texture.h"
#include "buffer_view.h"
#include "collision/capsule.h"

#include "post_process_volume.h"

#include "common_pipeline_states.h"

//	G-Buffer識別子
enum class GBufferId
{
	BaseColor = 0,
	Emissive,
	Normal_Depth,
	Parameters,
	Velocity,
	UniqueData,

	//	追加分
	ShadeColor,

	Max,
	Start = BaseColor,
};

static constexpr std::string_view SceneColorName = "Scene Color";
static constexpr std::string_view SceneDepthStencilName = "Scene DepthStencil";
static constexpr std::string_view ShadowmapName = "Shadowmap";
static constexpr std::string_view GBufferNames[] =
{
	"BaseColor",
	"Emissive",
	"Normal_Depth",
	"Parameters",
	"Velocity",
	"UniqueData",

	"ShadeColor",
};

//	材質
class material;
struct material_datas
{
	const material* material;
};

//	メッシュ描画情報コンテナ
struct mesh_container
{
	//	カプセルシャドウ情報
	struct capsule_shadow_datas
	{
		collision::bounding_capsule capsule;
	};
	std::vector<buffer_datas> buffer_datas;
	std::vector<material_datas> material_datas;
	std::vector<capsule_shadow_datas> capsule_shadow_datas;
	//	描画情報
	struct draw_datas
	{
		std::size_t buffer_index;
		std::size_t material_index;
	};
	std::vector<draw_datas> draw_datas;
	std::vector<std::size_t> color_datas;
	std::vector<std::size_t> shadow_datas;
	void clear()
	{
		draw_datas.clear();
		shadow_datas.clear();
		color_datas.clear();
	}
};

//	デカール情報
struct decal_datas
{
	DirectX::XMFLOAT3 scale;
	math::rotator rotation;
	DirectX::XMFLOAT3 position;
	material_datas material;
};
struct decal_container
{
	std::vector<decal_datas> decal_datas;
	void clear()
	{
		decal_datas.clear();
	}
};

//	スプライト情報コンテナ
class sprite;
struct sprite_datas
{
	int sort_key;
	int sort_order;
	const sprite* sprite;
};
struct sprite_container
{
	std::vector<sprite_datas> sprite_datas;
};

#if defined(_DEBUG)
//	デバッグ描画情報コンテナ
struct debug_draw_primitive_proxy_container
{
	buffer_datas line_buffer_datas;
};
#endif // defined(_DEBUG)

//	描画コンテキスト
struct graphics_context
{
	ID3D11DeviceContext* device_context;
	common_pipeline_states::scene_constant_buffer scene_constant;
	light_container light_container;
	post_process_volume_container post_process_volume_container;
	decal_container decal_container;
	mesh_container mesh_container;

	sprite_container sprite_container;

#if defined(_DEBUG)
	debug_draw_primitive_proxy_container debug_draw_container;
#endif // defined(_DEBUG)

	//	IBL関係
	texture_ptr diffuse_ibl_texture;
	texture_ptr specluar_ibl_texture;
	texture_ptr lut_ggx_texture;

	//	シーンテクスチャ
	texture_ptr scene_texture;

	//	GBuffer関係
	texture_ptr g_buffer_textures[static_cast<UINT>(GBufferId::Max)];

	//	遅延ライティング処理用情報
	common_pipeline_states::light_constant_buffer light_constant;
	common_pipeline_states::shadow_map_constant_buffer shadow_map_constant;
	texture_ptr shadow_map_texture;

	//	適応ポストプロセス情報
	post_process_datas post_process_data;

public:
	//	定数バッファの設定
	void set_constant_buffer(int start_slot, int num, ID3D11Buffer* const* constant_buffers) const;

	//	シェーダーリソースビューの設定
	void set_shader_resource(int start_slot, int num, ID3D11ShaderResourceView* const* shader_resources) const;

	//	サンプラーステートの設定
	void set_sampler(int start_slot, int num, ID3D11SamplerState* const* sampler_state) const;

	//	設定していたシェーダーを全て解除する
	void clear_shader_slots() const;

	//	設定されている頂点バッファを外す
	void clear_vertex_buffer() const;

	//	設定されているインデックスバッファを外す
	void clear_index_buffer() const;

	//	描画ターゲットのクリア
	void clear_render_target_slots() const;

	//	CBVのクリア設定
	void clear_constant_buffers(int start_slot = 0, int num = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT) const;

	//	設定していたSRVを全て解除する
	void clear_shader_resorce_views(int start_slot = 0, int num = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) const;

	//	設定していたUAVを全て解除する
	//	FEATURELEVELが11.1ならnumはD3D11_1_UAV_SLOT_COUNTになる
	void clear_unordered_access_views(int start_slot = 0, int num = 8) const;

	//	設定していたSamplerを全て解除する
	void clear_sampler(int start_slot = 0, int num = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) const;
};
