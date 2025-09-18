#include	"vmd_loader.h"

#include	<iostream>
#include	<locale> 
#include	<codecvt> 
#include	<cstdio>

namespace
{
	std::string	ConvertUtf8(const char* string, size_t string_length)
	{
		return	std::string(string);
	}

	std::string	ConvertUtf8(std::string_view string)
	{
		return	std::string(string);
	}
};

vmd_loader::vmd_loader()
: error_code(vmd_error_code::unparsed)
, version(0)

, data_buffer(nullptr)
, data_buffer_size(0)
, incriment(0)

, end_frame()
, model_name()
, bone_frame_list()
, morph_frame_list()
, camera_frame_list()
//, light_frame_list()
//, self_shadow_frame_list()
//, show_ik_frame_list()
{
}

vmd_loader::~vmd_loader()
{
}

bool	vmd_loader::load(void* buffer, unsigned int buffer_size)
{
	error_code = vmd_error_code::unparsed;
	incriment = 0;
	data_buffer = static_cast<char*>(buffer);
	data_buffer_size = buffer_size;

	//	マジックナンバーチェック
	char* magic_number = read_buffer<char>(30);
	if (strncmp(magic_number, "Vocaloid Motion Data file", 25) == 0)
	{
		//	モーションバージョン 1 は非公開情報なので読み込めない
		set_error_code(vmd_error_code::not_suppot_version);
		return	false;
	}
	if (strncmp(magic_number, "Vocaloid Motion Data", 20))
	{
		set_error_code(vmd_error_code::not_suppot);
		return	false;
	}
	version = atoi(magic_number + 20);

	//	どのモデルで作成したか調査用名前
	model_name = ConvertUtf8(read_buffer<char>(20));

	unsigned int	end_frame	= 0;

	//	ボーン
	if (incriment < data_buffer_size)
	{
		bone_frame_list.resize(*read_buffer<unsigned int>(sizeof(unsigned int)));
		for (auto& bone_frame : bone_frame_list)
		{
			bone_frame.name = ConvertUtf8(read_buffer<char>(sizeof(char) * 15));
			bone_frame.frame = *read_buffer<int>();
			bone_frame.position = *read_buffer<DirectX::XMFLOAT3>();
			bone_frame.rotation = *read_buffer<DirectX::XMFLOAT4>();
			//	補間曲線
			//	補間曲線は4点のベジェ曲線(0,0),(x1,y1),(x2,y2),(127,127)で表している。
			//	各軸のパラメータを
			//	X軸の補間パラメータ　( X_x1, X_y1 ), ( X_x2, X_y2 )
			//	Y軸の補間パラメータ　( Y_x1, Y_y1 ), ( Y_x2, Y_y2 )
			//	Z軸の補間パラメータ　( Z_x1, Z_y1 ), ( Z_x2, Z_y2 )
			//	回転の補間パラメータ ( R_x1, R_y1 ), ( R_x2, R_y2 )
			//	とした時、補間パラメータは以下の通り。
			//	X_x1, Y_x1, Z_x1, R_x1, X_y1, Y_y1, Z_y1, R_y1, X_x2, Y_x2, Z_x2, R_x2, X_y2, Y_y2, Z_y2, R_y2,
			//	Y_x1, Z_x1, R_x1, X_y1, Y_y1, Z_y1, R_y1, X_x2, Y_x2, Z_x2, R_x2, X_y2, Y_y2, Z_y2, R_y2,   01,
			//	Z_x1, R_x1, X_y1, Y_y1, Z_y1, R_y1, X_x2, Y_x2, Z_x2, R_x2, X_y2, Y_y2, Z_y2, R_y2,   01,   00,
			//	R_x1, X_y1, Y_y1, Z_y1, R_y1, X_x2, Y_x2, Z_x2, R_x2, X_y2, Y_y2, Z_y2, R_y2,   01,   00,   00
			for (unsigned int jj = 0; jj < ARRAYSIZE(bone_frame.interpolate); jj++)
			{
				bone_frame.interpolate[jj] = *read_buffer<unsigned char>();
			}

			if (end_frame < bone_frame.frame)
				end_frame = bone_frame.frame;
		}
		std::sort(bone_frame_list.begin()
			, bone_frame_list.end()
			, [](const vmd_bone_frame& left, const vmd_bone_frame& right) { return	left.frame < right.frame; }
		);
	}

	//	モーフ
	if (incriment < data_buffer_size)
	{
		morph_frame_list.resize(*read_buffer<unsigned int>());
		for (auto& morph_data : morph_frame_list)
		{
			morph_data.name = ConvertUtf8(read_buffer<char>(sizeof(char) * 15));
			morph_data.frame = *read_buffer<int>();
			morph_data.weight = *read_buffer<float>();
			if (end_frame < morph_data.frame)
				end_frame = morph_data.frame;
		}
		std::sort(morph_frame_list.begin()
			, morph_frame_list.end()
			, [](const vmd_morph_frame& left, const vmd_morph_frame& right) { return	left.frame < right.frame; }
		);
	}

	//	カメラ
	if (incriment < data_buffer_size)
	{
		camera_frame_list.resize(*read_buffer<unsigned int>());
		for (auto& camera_frame : camera_frame_list)
		{
			camera_frame.frame = *read_buffer<int>();
			camera_frame.distance = *read_buffer<float>();
			camera_frame.position = *read_buffer<DirectX::XMFLOAT3>();
			camera_frame.rotation = *read_buffer<DirectX::XMFLOAT3>();
			for (unsigned int jj = 0; jj < ARRAYSIZE(camera_frame.interpolate); jj++)
			{
				camera_frame.interpolate[jj] = *read_buffer<unsigned char>();
			}
			camera_frame.view_angle = *read_buffer<int>();
			camera_frame.is_parth = *read_buffer<unsigned char>();
			if (end_frame < camera_frame.frame)
				end_frame = camera_frame.frame;
		}
		std::sort(camera_frame_list.begin()
			, camera_frame_list.end()
			, [](const vmd_camera_frame& left, const vmd_camera_frame& right) { return	left.frame < right.frame; }
		);
	}
#if 0
	//	ライト
	if( m_uIncriment < m_uDataBufferSize )
	{
		m_pLightFrameList.resize( *ReadBuffer<U32>( sizeof( U32 ) ) );
		for( U32 ii = 0; ii < m_pLightFrameList.size(); ii++ )
		{
			StLightFrame*	pLightFrame	= &m_pLightFrameList[ii];
			pLightFrame->m_uFrame	= *ReadBuffer<S32>( sizeof( S32 ) );
		//	pLightFrame->m_vPos		= *ReadBuffer<Math::Float3>( sizeof( F32 ) * 3 );
			pLightFrame->m_vPos		= Math::Vector3( *ReadBuffer<Math::Float3>( sizeof( F32 ) * 3 ) );
			pLightFrame->m_vColor	= *ReadBuffer<Math::Float3>( sizeof( F32 ) * 3 );
			if( uEndFrame < pLightFrame->m_uFrame )
			{
				//	終端フレームの更新
				uEndFrame	= pLightFrame->m_uFrame;
			}
		}
		std::sort( m_pLightFrameList.begin()
				 , m_pLightFrameList.end()
				 , []( const StLightFrame& rLFrame, const StLightFrame& rRFrame ){ return	rLFrame.m_uFrame < rRFrame.m_uFrame; }
		);
		DebugMessage( "VMD : Light : %d\n", m_pLightFrameList.size() );
	}

	//	セルフシャドウ
	if( m_uIncriment < m_uDataBufferSize )
	{
		m_pSelfShadowFrameList.resize( *ReadBuffer<U32>( sizeof( U32 ) ) );
		for( U32 ii = 0; ii < m_pSelfShadowFrameList.size(); ii++ )
		{
			StSelfShadowFrame*	pSelfShadowFrame	= &m_pSelfShadowFrameList[ii];
			pSelfShadowFrame->m_uFrame	= *ReadBuffer<S32>( sizeof( S32 ) );
			pSelfShadowFrame->m_uType	= *ReadBuffer<U8>( sizeof( U8 ) );
		//	pSelfShadowFrame->m_fDist	= *ReadBuffer<F32>( sizeof( F32 ) );
			pSelfShadowFrame->m_fDist	= *ReadBuffer<F32>( sizeof( F32 ) );
			if( uEndFrame < pSelfShadowFrame->m_uFrame )
			{
				//	終端フレームの更新
				uEndFrame	= pSelfShadowFrame->m_uFrame;
			}
		}
		std::sort( m_pSelfShadowFrameList.begin()
				 , m_pSelfShadowFrameList.end()
				 , []( const StSelfShadowFrame& rLFrame, const StSelfShadowFrame& rRFrame ){ return	rLFrame.m_uFrame < rRFrame.m_uFrame; }
		);
		DebugMessage( "VMD : SelfShadow : %d\n", m_pSelfShadowFrameList.size() );
	}

	//	表示/IK
	if( m_uIncriment < m_uDataBufferSize )
	{
		m_pShowIKFrameList.resize( *ReadBuffer<U32>( sizeof( U32 ) ) );
		for( U32 ii = 0; ii < m_pShowIKFrameList.size(); ii++ )
		{
			StShowModelAndIKFrame*	pShowIKFrame	= &m_pShowIKFrameList[ii];
			pShowIKFrame->m_uFrame	= *ReadBuffer<S32>( sizeof( S32 ) );
			pShowIKFrame->m_bShow	= *ReadBuffer<U8>( sizeof( U8 ) );
			pShowIKFrame->m_pInfoIK.resize( *ReadBuffer<U32>( sizeof( S32 ) ) );
			for( U32 jj = 0; jj < pShowIKFrame->m_pInfoIK.size(); jj++ )
			{
				StInfoIK*	pInfoIK	= &pShowIKFrame->m_pInfoIK[jj];
				pInfoIK->m_strName	= ReadBuffer<char>( sizeof( char ) * 20 );
				pInfoIK->m_bSwitch	= *ReadBuffer<U8>( sizeof( U8 ) );
			}
			if( uEndFrame < pShowIKFrame->m_uFrame )
			{
				//	終端フレームの更新
				uEndFrame	= pShowIKFrame->m_uFrame;
			}
		}
		std::sort( m_pShowIKFrameList.begin()
				 , m_pShowIKFrameList.end()
				 , []( const StShowModelAndIKFrame& rLFrame, const StShowModelAndIKFrame& rRFrame ){ return	rLFrame.m_uFrame < rRFrame.m_uFrame; }
		);
		DebugMessage( "VMD : ShowIK : %d\n", m_pShowIKFrameList.size() );
	}
#endif
	this->end_frame = end_frame;

	set_error_code(vmd_error_code::succeeded );
	return	true;
}

void	vmd_loader::set_error_code(vmd_error_code error_code)
{
	this->error_code = error_code;
}
