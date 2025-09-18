#pragma	once

#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <DirectXMath.h>
#include <wrl.h>
#include <algorithm>

enum	class	vmd_error_code
{
	succeeded,
	unparsed,
	not_suppot_version,
	not_suppot,
};

struct	vmd_bone_frame
{
	std::string			name;				//	ボーン名
	unsigned int		frame;				//	フレーム
	DirectX::XMFLOAT3	position;			//	位置
	DirectX::XMFLOAT4	rotation;			//	回転(クォータニオン)
	bool				is_linear[4];		//	線形補間フラグ
	unsigned char		interpolate[64];	//	補間曲線テーブル
};

struct	vmd_morph_frame
{
	std::string		name;	//	モーフ名
	unsigned int	frame;	//	フレーム
	float			weight;	//	影響度
};

struct	vmd_camera_frame
{
	unsigned int		frame;				//	フレーム
	float				distance;			//	距離
	DirectX::XMFLOAT3	position;			//	注視点
	DirectX::XMFLOAT3	rotation;			//	回転(オイラー角)
	unsigned char		interpolate[24];	//	補間曲線(未使用)
											//	補間曲線は4点のベジェ曲線(0,0),(x1,y1),(x2,y2),(127,127)で表している。
											//	各軸のパラメータを
											//	X軸の補間パラメータ		( X_x1, X_y1 ), ( X_x2, X_y2 )
											//	Y軸の補間パラメータ		( Y_x1, Y_y1 ), ( Y_x2, Y_y2 )
											//	Z軸の補間パラメータ		( Z_x1, Z_y1 ), ( Z_x2, Z_y2 )
											//	回転の補間パラメータ	( R_x1, R_y1 ), ( R_x2, R_y2 )
											//	距離の補間パラメータ	( L_x1, L_y1 ), ( L_x2, L_y2 )
											//	視野角の補間パラメータ	( V_x1, V_y1 ), ( V_x2, V_y2 )
											//	とした時、補間パラメータは以下の通り.
											//	X_x1 X_x2 X_y1 X_y2
											//	Y_x1 Y_x2 Y_y1 Y_y2
											//	Z_x1 Z_x2 Z_y1 Z_y2
											//	R_x1 R_x2 R_y1 R_y2
											//	L_x1 L_x2 L_y1 L_y2
											//	V_x1 V_x2 V_y1 V_y2
	int					view_angle;			//	視野角(Deg)
	unsigned char		is_parth;			//	正射影か透視投影か
};

struct	vmd_light_frame
{
	unsigned int		m_uFrame;	//	フレーム
	DirectX::XMFLOAT3	m_vPos;		//	目標点
	DirectX::XMFLOAT3	m_vColor;	//	目標色( 0 ～ 1.0f)
};

struct	vmd_self_shadow_frame
{
	unsigned int	frame;		//	フレーム
	unsigned char	type;		//	シャドウタイプ( 0 : OFF, 1 : Mode1, 2 : Mode2 )
	float			distance;	//	シャドウ距離
};

struct	vmd_info_ik
{
	std::string		name;		//	ボーン名
	unsigned char	is_switch;	//	IKのON/OFF( 0 : OFF, 1 : ON )
};

struct	vmd_show_model_and_ik_frame
{
	unsigned int				frame;		//	フレーム
	unsigned char				show;		//	モデル表示フラグ( 0 : OFF, 1 : ON )
	std::vector<vmd_info_ik>	info_ik;	//	IK情報
};
