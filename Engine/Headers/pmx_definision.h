#pragma	once

#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <DirectXMath.h>
#include <wrl.h>
#include <algorithm>

enum class pmx_error_code
{
	succeeded,
	unparsed,
	not_suppot,
};

enum class pmx_skining_type
{
	brdf1	= 0,
	brdf2	= 1,
	brdf4	= 2,
	sdef	= 3,	//	未サポート
	qdef	= 4,	//	未サポート
};

enum class pmx_morph_type
{
	group			=  0,
	vertex			=  1,
	bone			=  2,
	uv				=  3,
	additional_uv1	=  4,
	additional_uv2	=  5,
	additional_uv3	=  6,
	additional_uv4	=  7,
	matrial			=  8,
	flip			=  9,
	implus			= 10,
};

enum class pmx_morph_category
{
	reserved_category	= 0,
	eyebrow				= 1,
	eye					= 2,
	mouth				= 3,
	other				= 4,
};

enum class pmx_rigidbody_type
{
	kinematic	= 0,				//	Bone追従
	dynamic,						//	物理演算
	dynamic_apply_rotation_only,	//	物理演算(回転のみ適応)
};

enum class pmx_rigidbody_shape_type
{
	sphere	= 0,	//	球体形状
	box,			//	箱形状
	capsule,		//	カプセル形状
};

enum class pmx_joint_type
{
	enJointType_Generic6DofSpring	= 0,
	enJointType_Generic6Dof			= 1,
	enJointType_Point2Point			= 2,
	enJointType_ConeTwist			= 3,
	enJointType_Slider				= 4,
	enJointType_Hinge				= 5,
};

enum class pmx_bone_bitflag
{
	has_connection_bone			= 0x0001,	//	接続先(0:座標オフセットで指定 1:ボーンで指定)
	is_rotation					= 0x0002,	//	回転可能
	is_translation				= 0x0004,	//	移動可能
	is_dispplay					= 0x0008,	//	表示フラグ
	is_control					= 0x0010,	//	操作可
	is_ik						= 0x0020,	//	IKボーン
	is_append_local				= 0x0080,	//	ローカル付与 | 付与対象 0:ユーザー変形値／IKリンク／多重付与 1:親のローカル変形量
	is_append_rotation			= 0x0100,	//	(モーション最終合成時)回転付与
	is_append_translation		= 0x0200,	//	(モーション最終合成時)移動付与
	is_ik_axis_lock				= 0x0400,	//	(IK用)軸固定
	local_axis					= 0x0800,	//	ローカル軸
	is_post_physics				= 0x1000,	//	物理後変形
	is_external_parent_deform	= 0x2000,	//	外部親変形
};

enum class pmx_material_draw_bitflag
{
	cull_both			= 0x01,	//	両面描画フラグ
	use_shadow			= 0x02,	//	影使用フラグ
	use_self_shadow_map	= 0x04,	//	セルフシャドウにシャドウマップを使うかどうか
	use_self_shadow		= 0x08,	//	セルフシャドウをするかどうか
	use_edge_line		= 0x10,	//	輪郭線フラグ
};

//	バージョンインデックス
static	constexpr	const	unsigned int	pmx_version1_0		= 100;
static	constexpr	const	unsigned int	pmx_version2_0		= 200;
static	constexpr	const	unsigned int	pmx_version2_1		= 210;
//	無効Id
static	constexpr	const	unsigned int	pmx_invalid_target_id	= 0xFFFFFFFF;
static	constexpr	const	unsigned int	pmx_invalid_texture_id	= 0xFFFFFFFF;

struct	pmx_header
{
	char	magic_number[4];	//	マジックナンバー
	float	version;			//	バージョン情報
};

struct	pmx_index_data
{
	unsigned char	encoding;				//	エンコード方式( 0:UTF16 1:UTF8 )
	unsigned char	uv;						//	追加UV数( 0～4 )
	unsigned char	vertex_index_size;		//	頂点インデックスサイズ( 1,2,4 のいずれか )
	unsigned char	texture_index_size;		//	テクスチャインデックスサイズ( 1,2,4 のいずれか )
	unsigned char	material_index_size;	//	マテリアルインデックスサイズ( 1,2,4 のいずれか )
	unsigned char	bone_index_size;		//	ボーンインデックスサイズ( 1,2,4 のいずれか )
	unsigned char	morph_index_size;		//	モーフインデックスサイズ( 1,2,4 のいずれか )
	unsigned char	rigidbody_index_size;	//	剛体インデックスサイズ( 1,2,4 のいずれか )
};

struct	pmx_vertex
{
	DirectX::XMFLOAT3	position;			//	座標
	DirectX::XMFLOAT3	normal;				//	法線
	DirectX::XMFLOAT2	texcoord;			//	UV座標
	DirectX::XMFLOAT4	additional_uv[4];	//	追加UV
	pmx_skining_type	skining_type;		//	スキニングタイプ
	DirectX::XMUINT4	bone_indices;			//	ボーンインデックス
	DirectX::XMFLOAT4	bone_weights;		//	ボーン影響度
	DirectX::XMFLOAT3	sdef[3];			//	Sdef用座標( 0 : c, 1 : r0, 2 : r1 )
	float				edge_value;			//	エッジ倍率
};

struct	pmx_material
{
	std::string			name;					//	材質名
	DirectX::XMFLOAT4	diffuse;				//	ディフューズカラー
	DirectX::XMFLOAT3	specular;				//	スペキュラカラー
	float				specularlity;			//	光沢度
	DirectX::XMFLOAT3	ambient;				//	アンビエントカラー
	unsigned char		draw_flag;				//	マテリアル描画フラグ
	DirectX::XMFLOAT4	edge_color;				//	エッジ色
	float				edge_size;				//	エッジサイズ
	unsigned int		deffuse_texture_index;	//	ディフューズテクスチャインデックス
	unsigned int		sphere_texture_index;	//	環境マップインデックス
	unsigned char		sphere_mode;			//	環境マップ演算モード
	unsigned char		toon_flag;				//	トゥーンフラグ
	unsigned int		toon_texture_index;		//	トゥーンテクスチャインデックス
												//	m_uToonFlag	: 0			= toontextureIndex
												//	m_uToonFlag	: 1 ～ 9	= (toon01 ~ toon10.bmp)
	unsigned int		index_count;			//	インデックス数
};

struct	pmx_ik_link
{
	unsigned int		link_target;	//	リンクボーンインデックス
	unsigned char		angle_lock;		//	角度制限
	DirectX::XMFLOAT3	angle_max;		//	最大制限角度
	DirectX::XMFLOAT3	angle_min;		//	最小制限角度
};

struct	pmx_bone
{
	std::string					name;						//	ボーン名
	DirectX::XMFLOAT3			position;					//	座標
	unsigned int				parent_index;				//	親ボーンインデックス
	unsigned int				level;						//	レベル(ボーンの更新階層)
	unsigned short				bone_flag;					//	ボーンフラグ(16bit) 各bit 0:OFF 1:ON
	DirectX::XMFLOAT3			offset;						//	(pmx_bone_bitflag::has_connection_bone == 0)の場合座標オフセット
	unsigned int				target_index;				//	(pmx_bone_bitflag::has_connection_bone == 1)の場合接続先ボーンインデックス
	//	(pmx_bone_bitflag::is_append_rotation == 1)	回転付与
	//	(pmx_bone_bitflag::is_append_translation == 1)	移動付与
	unsigned int				append_parent_index;		//	付与親ボーンインデックス
	float						append_weight;				//	付与率
	//	(pmx_bone_bitflag::is_ik_axis_lock == 1)	軸固定
	DirectX::XMFLOAT3			lock_axis_orientation;		//	固定軸の方向
	//	(pmx_bone_bitflag::local_axis == 1)	ボーンのローカル操作時の軸
	DirectX::XMFLOAT3			local_axis_x_orientation;	//	ボーンのローカル操作時の軸のX軸方向
	DirectX::XMFLOAT3			local_axis_z_orientation;	//	ボーンのローカル操作時の軸のZ軸方向
	//	(pmx_bone_bitflag::is_external_parent_deform == 1)	外部親変形
	unsigned int				key;						//	外部変形のkey値
	//	(pmx_bone_bitflag::is_ik == 1)	IK
	unsigned int				ik_target_bone_index;		//	IKターゲットボーン
	unsigned int				ik_loop;					//	IKループ回数
	float						ik_loop_angle_limit;		//	IKループ計算時の角度制限(ラジアン角)
	std::vector<pmx_ik_link>	ik_link_list;				//	IKリンクリスト
};

struct	pmx_morph_bone_offset
{
	unsigned int		bone_index;
	DirectX::XMFLOAT3	translation;
	DirectX::XMFLOAT4	rotation;
};

struct	pmx_morph_material_offset
{
	unsigned int		material_index;
	unsigned char		offset_operation;
	DirectX::XMFLOAT4	diffuse;
	DirectX::XMFLOAT3	specular;
	float				specularlity;
	DirectX::XMFLOAT3	ambient;
	DirectX::XMFLOAT4	edge_color;
	float				edge_size;
	DirectX::XMFLOAT4	textureARGB;
	DirectX::XMFLOAT4	shpereARGB;
	DirectX::XMFLOAT4	toonARGB;
};

struct	pmx_morph_group_offset
{
	unsigned int	morph_index;
	float			morph_weight;
};

struct	pmx_morph_flip_offset
{
	unsigned int	morph_index;
	float			morph_value;
};

struct	pmx_morph_implus_offset
{
	unsigned int		rigidbody_index;
	unsigned char		is_local;
	DirectX::XMFLOAT3	velocity;
	DirectX::XMFLOAT3	angular_torque;
};

struct	pmx_morph
{
	std::string									name;					//	モーフ名
	pmx_morph_type								morph_type;				//	カテゴリ
	pmx_morph_category							morph_category;			//	タイプ
																		//	以下、継承するのもアレだったので全部保持する形に
	std::unordered_map<unsigned int, DirectX::XMFLOAT3>	vertex_offset;	//	頂点オフセットリスト
	std::unordered_map<unsigned int, DirectX::XMFLOAT4>	uv_offset[5];	//	UVオフセットリスト
	std::vector<pmx_morph_bone_offset>			bone_offset;			//	ボーンオフセットリスト
	std::vector<pmx_morph_material_offset>		material_offset;		//	マテリアルオフセットリスト
	std::vector<pmx_morph_group_offset>			group_offset;			//	グループオフセットリスト
	std::vector<pmx_morph_flip_offset>			flip_offset;			//	フリップオフセットリスト
	std::vector<pmx_morph_implus_offset>		implus_offset;			//	インパルスオフセットリスト
};
