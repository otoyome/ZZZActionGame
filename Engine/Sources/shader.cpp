#include "shader.h"
#include "misc.h"

#include "render_data.h"

#if 0
#include	"dxcapi.h"

#pragma	comment( lib, "dxcompiler.lib" )

namespace
{
	std::wstring_view	rShaderStages[s_cast<U32>( Graphics::ShaderStage::Max )]	=
	{
		L"ms",	//!<	Mesh Shader
		L"as",	//!<	Amplification Shader
		L"vs",	//!<	Vertex Shader
		L"hs",	//!<	Hull Shader
		L"ds",	//!<	Domain Shader
		L"gs",	//!<	Geometry Shader
		L"ps",	//!<	Pixel Shader
		L"cs",	//!<	Compute Shader
	//	L"lib",	//!<	Shader Library
	};
	std::wstring_view	rShaderModels[s_cast<U32>( Graphics::ShaderModel::Max )]	=
	{
		L"5_0",
		L"6_0",
		L"6_1",
		L"6_2",
		L"6_3",
		L"6_4",
		L"6_5",
		L"6_6",
		L"6_7",
	};
	/*!
		@struct	CompileFlags
		@brief	コンパイルフラグ情報
	**/
	enum	class	CompileFlags
	{
		None = 0,
		DisableOptimize	= 1 << 00,
	};
	ENUM_ATTR_BITFLAG( CompileFlags );

	/*!
		@struct	Define
		@brief	マクロ情報
	**/
	struct	Define
	{
		std::string_view	m_rName;	//!<	マクロ名
		std::string_view	m_rValue;	//!<	値
	};

	/*!
		@struct	CompileInputFile
		@brief	シェーダーコンパイルファイル入力情報
	**/
	struct	CompileInputFile
	{
		CompileFlags					m_eFlags;		//!<	コンパイル設定
		std::string_view				m_rFilepath;	//!<	シェーダーファイルパス
		std::string_view				m_rEntryPoint;	//!<	エントリーポイント
		Graphics::ShaderStage			m_eStage;		//!<	シェーダーステージ
		Graphics::ShaderModel			m_eModel;		//!<	シェーダーモデル
		std::vector<std::string_view>	m_vIncludes;	//!<	インクルード情報
		std::vector<Define>				m_vDefines;		//!<	マクロ情報
	};

	/*!
		@struct	CompilerOutput
		@brief	シェーダーコンパイル出力情報
	**/
	struct	CompilerOutput
	{
		inline	bool	IsValid() const { return	m_hInternalState.get() != nullptr; }

		std::shared_ptr<void>		m_hInternalState;

		const	U8*					m_pShaderData	= nullptr;
		U64							m_uShaderSize	= 0;
		std::vector<U8>				m_uShaderHash;
		std::string					m_strErrorMessage;
		std::vector<std::string>	m_vDependencies;
	};

	/*!
		@class		CompileFromFile
		@brief		シェーダーコンパイル
		@param[in]	_rInput		: 入力情報
		@param[out]	_rOutput	: 出力情報
	**/
	bool	CompileFromFile( const CompileInputFile& _rInput, CompilerOutput& _rOutput )
	{
		HRESULT	hr;
		std::vector<std::wstring>	vConvertTexts;
		vConvertTexts.reserve( _rInput.m_vDefines.size() + _rInput.m_vIncludes.size() );

		//	ファイルパス変換
		std::wstring	strFilepath	= System::ToWString( _rInput.m_rFilepath );

		//	DXC API準備
		Microsoft::WRL::ComPtr<IDxcLibrary>	hLibrary;
		if( FAILED( DxcCreateInstance( CLSID_DxcLibrary, IID_PPV_ARGS( hLibrary.GetAddressOf() ) ) ) )
			return	false;

		//	DXC Compiler生成
		Microsoft::WRL::ComPtr<IDxcCompiler3>	hCompiler;
		if( FAILED( DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( hCompiler.GetAddressOf() ) ) ) )
			return	false;

		//	DXC Utils生成
		Microsoft::WRL::ComPtr<IDxcUtils>	hDXCUtils;
		if( FAILED( DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( hDXCUtils.GetAddressOf() ) ) ) )
			return	false;

		//	インクルードハンドラーを生成
		Microsoft::WRL::ComPtr<IDxcIncludeHandler>	hIncludeHandler;
	//	if( FAILED( hLibrary->CreateIncludeHandler( hIncludeHandler.GetAddressOf() ) ) )
		if( FAILED( hDXCUtils->CreateDefaultIncludeHandler( hIncludeHandler.GetAddressOf() ) ) )
			return	false;

		//	コンパイル時引数設定
		//	https://github.com/microsoft/DirectXShaderCompiler/wiki/Using-dxc.exe-and-dxcompiler.dll#dxcompiler-dll-interface
		std::vector<LPCWSTR>	vArguments;
		{
			if( System::HasFlag( _rInput.m_eFlags, CompileFlags::DisableOptimize ) )
			{
				//	最適化OFF
				vArguments.push_back( L"-Od" );
			}

			//	ターゲット設定
			vArguments.push_back( L"-T" );
			switch( _rInput.m_eStage )
			{
				case	ShaderStage::Vertex:
				{
					switch( _rInput.m_eModel )
					{
						default:						vArguments.push_back( L"vs_5_0" );	break;
						case	ShaderModel::SM_6_0:	vArguments.push_back( L"vs_6_0" );	break;
						case	ShaderModel::SM_6_1:	vArguments.push_back( L"vs_6_1" );	break;
						case	ShaderModel::SM_6_2:	vArguments.push_back( L"vs_6_2" );	break;
						case	ShaderModel::SM_6_3:	vArguments.push_back( L"vs_6_3" );	break;
						case	ShaderModel::SM_6_4:	vArguments.push_back( L"vs_6_4" );	break;
						case	ShaderModel::SM_6_5:	vArguments.push_back( L"vs_6_5" );	break;
						case	ShaderModel::SM_6_6:	vArguments.push_back( L"vs_6_6" );	break;
						case	ShaderModel::SM_6_7:	vArguments.push_back( L"vs_6_7" );	break;
					}
					break;
				}
				case	ShaderStage::Hull:
				{
					switch( _rInput.m_eModel )
					{
						default:						vArguments.push_back( L"hs_5_0" );	break;
						case	ShaderModel::SM_6_0:	vArguments.push_back( L"hs_6_0" );	break;
						case	ShaderModel::SM_6_1:	vArguments.push_back( L"hs_6_1" );	break;
						case	ShaderModel::SM_6_2:	vArguments.push_back( L"hs_6_2" );	break;
						case	ShaderModel::SM_6_3:	vArguments.push_back( L"hs_6_3" );	break;
						case	ShaderModel::SM_6_4:	vArguments.push_back( L"hs_6_4" );	break;
						case	ShaderModel::SM_6_5:	vArguments.push_back( L"hs_6_5" );	break;
						case	ShaderModel::SM_6_6:	vArguments.push_back( L"hs_6_6" );	break;
						case	ShaderModel::SM_6_7:	vArguments.push_back( L"hs_6_7" );	break;
					}
					break;
				}
				case	ShaderStage::Domain:
				{
					switch( _rInput.m_eModel )
					{
						default:						vArguments.push_back( L"ds_5_0" );	break;
						case	ShaderModel::SM_6_0:	vArguments.push_back( L"ds_6_0" );	break;
						case	ShaderModel::SM_6_1:	vArguments.push_back( L"ds_6_1" );	break;
						case	ShaderModel::SM_6_2:	vArguments.push_back( L"ds_6_2" );	break;
						case	ShaderModel::SM_6_3:	vArguments.push_back( L"ds_6_3" );	break;
						case	ShaderModel::SM_6_4:	vArguments.push_back( L"ds_6_4" );	break;
						case	ShaderModel::SM_6_5:	vArguments.push_back( L"ds_6_5" );	break;
						case	ShaderModel::SM_6_6:	vArguments.push_back( L"ds_6_6" );	break;
						case	ShaderModel::SM_6_7:	vArguments.push_back( L"ds_6_7" );	break;
					}
					break;
				}
				case	ShaderStage::Geometry:
				{
					switch( _rInput.m_eModel )
					{
						default:						vArguments.push_back( L"gs_5_0" );	break;
						case	ShaderModel::SM_6_0:	vArguments.push_back( L"gs_6_0" );	break;
						case	ShaderModel::SM_6_1:	vArguments.push_back( L"gs_6_1" );	break;
						case	ShaderModel::SM_6_2:	vArguments.push_back( L"gs_6_2" );	break;
						case	ShaderModel::SM_6_3:	vArguments.push_back( L"gs_6_3" );	break;
						case	ShaderModel::SM_6_4:	vArguments.push_back( L"gs_6_4" );	break;
						case	ShaderModel::SM_6_5:	vArguments.push_back( L"gs_6_5" );	break;
						case	ShaderModel::SM_6_6:	vArguments.push_back( L"gs_6_6" );	break;
						case	ShaderModel::SM_6_7:	vArguments.push_back( L"gs_6_7" );	break;
					}
					break;
				}
				case	ShaderStage::Pixel:
				{
					switch( _rInput.m_eModel )
					{
						default:						vArguments.push_back( L"ps_5_0" );	break;
						case	ShaderModel::SM_6_0:	vArguments.push_back( L"ps_6_0" );	break;
						case	ShaderModel::SM_6_1:	vArguments.push_back( L"ps_6_1" );	break;
						case	ShaderModel::SM_6_2:	vArguments.push_back( L"ps_6_2" );	break;
						case	ShaderModel::SM_6_3:	vArguments.push_back( L"ps_6_3" );	break;
						case	ShaderModel::SM_6_4:	vArguments.push_back( L"ps_6_4" );	break;
						case	ShaderModel::SM_6_5:	vArguments.push_back( L"ps_6_5" );	break;
						case	ShaderModel::SM_6_6:	vArguments.push_back( L"ps_6_6" );	break;
						case	ShaderModel::SM_6_7:	vArguments.push_back( L"ps_6_7" );	break;
					}
					break;
				}
				case	ShaderStage::Compute:
				{
					switch( _rInput.m_eModel )
					{
						default:						vArguments.push_back( L"cs_5_0" );	break;
						case	ShaderModel::SM_6_0:	vArguments.push_back( L"cs_6_0" );	break;
						case	ShaderModel::SM_6_1:	vArguments.push_back( L"cs_6_1" );	break;
						case	ShaderModel::SM_6_2:	vArguments.push_back( L"cs_6_2" );	break;
						case	ShaderModel::SM_6_3:	vArguments.push_back( L"cs_6_3" );	break;
						case	ShaderModel::SM_6_4:	vArguments.push_back( L"cs_6_4" );	break;
						case	ShaderModel::SM_6_5:	vArguments.push_back( L"cs_6_5" );	break;
						case	ShaderModel::SM_6_6:	vArguments.push_back( L"cs_6_6" );	break;
						case	ShaderModel::SM_6_7:	vArguments.push_back( L"cs_6_7" );	break;
					}
					break;
				}
				default:
				{
					DebugAssert( !"Not Compatible" );
					return	false;
				}
			}

			//	マクロ
			for( U64 ii = 0; ii < _rInput.m_vDefines.size(); ++ii )
			{
				vArguments.push_back( L"-D" );
				vConvertTexts.emplace_back();
				vConvertTexts.back()	+= System::ToWString( _rInput.m_vDefines[ii].m_rName );
				vConvertTexts.back()	+= L"=";
				vConvertTexts.back()	+= System::ToWString( _rInput.m_vDefines[ii].m_rValue );
				vArguments.push_back( vConvertTexts.back().data() );
			}

			//	インクルード
			for( U64 ii = 0; ii < _rInput.m_vIncludes.size(); ++ii )
			{
				vArguments.push_back( L"-I" );
				vConvertTexts.emplace_back( System::ToWString( _rInput.m_vIncludes[ii] ) );
				vArguments.push_back( vConvertTexts.back().data() );
			}

			//	行列優先指定
			vArguments.push_back( Graphics::MatrixRowMajor ? DXC_ARG_PACK_MATRIX_ROW_MAJOR : DXC_ARG_PACK_MATRIX_COLUMN_MAJOR );

			//	エントリーポイント
			vArguments.push_back( L"-E" );
			vConvertTexts.emplace_back( System::ToWString( _rInput.m_rEntryPoint ) );
			vArguments.push_back( vConvertTexts.back().data() );

			//	ファイルパス
			vArguments.push_back( strFilepath.data() );
		}

		//	ファイル読み込み
		Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource;
		if( FAILED( hLibrary->CreateBlobFromFile( strFilepath.data(), nullptr, pSource.GetAddressOf() ) ) )
			return	false;

		DxcBuffer	stSource;
		stSource.Ptr		= pSource->GetBufferPointer();
		stSource.Size		= pSource->GetBufferSize();
		stSource.Encoding	= DXC_CP_ACP;

		//	読み込んだシェーダーをコンパイル
		Microsoft::WRL::ComPtr<IDxcResult>	hResult;
		if( FAILED( hCompiler->Compile( &stSource,
										vArguments.data(),
										s_cast<U32>( vArguments.size() ),
										hIncludeHandler.Get(),
										IID_PPV_ARGS( hResult.GetAddressOf() ) ) ) )
		{
			return	false;
		}

		//	エラー情報取得
		Microsoft::WRL::ComPtr<IDxcBlobUtf8>	hErrors;
		hr	= hResult->GetOutput( DXC_OUT_ERRORS, IID_PPV_ARGS( hErrors.GetAddressOf() ), nullptr );
		DebugAssert( SUCCEEDED( hr ) );
		if( hErrors != nullptr && hErrors->GetStringLength() != 0 )
		{
			_rOutput.m_strErrorMessage	= hErrors->GetStringPointer();
		}

		//	ステータスを取得して失敗しているか判定
		if( FAILED( hResult->GetStatus( &hr ) ) )
			return	false;
		if( FAILED( hr ) )
			return	false;

		//	コンパイル結果を取得
		Microsoft::WRL::ComPtr<IDxcBlob>	hShader;
		if( FAILED( hResult->GetOutput( DXC_OUT_OBJECT, IID_PPV_ARGS( hShader.GetAddressOf() ), nullptr ) ) )
			return	false;

		//	コンパイル結果を返す
		_rOutput.m_vDependencies.emplace_back( _rInput.m_rFilepath );
		_rOutput.m_pShaderData		= s_cast<const U8*>( hShader->GetBufferPointer() );
		_rOutput.m_uShaderSize		= hShader->GetBufferSize();
		auto	hInternalState		= std::make_shared<Microsoft::WRL::ComPtr<IDxcBlob>>();
		*hInternalState				= hShader;
		_rOutput.m_hInternalState	= hInternalState;

		//	シェーダーのハッシュ情報を保存
		Microsoft::WRL::ComPtr<IDxcBlob>	hHash;
		hr	= hResult->GetOutput( DXC_OUT_SHADER_HASH, IID_PPV_ARGS( hHash.GetAddressOf() ), nullptr );
		DebugAssert( SUCCEEDED( hr ) );
		if( hHash != nullptr )
		{
			DxcShaderHash*	pHashBuf = s_cast<DxcShaderHash*>( hHash->GetBufferPointer() );
			for( S32 ii = 0; ii < arrayof( pHashBuf->HashDigest ); ++ii )
			{
				_rOutput.m_uShaderHash.push_back( pHashBuf->HashDigest[ii] );
			}
		}
		return	true;
	}

	/*!
		@class		GetReflection
		@brief		リフレクション取得
		@param[in]	_pBufferPointer	: バッファポインタ
		@param[in]	_uBufferSize	: バッファサイズ
		@param[out]	_rOutput		: 出力情報
	**/
	bool	GetReflection( const void* _pBufferPointer, U64 _uBufferSize, ShaderReflectionPointer& _rOutput )
	{
		//	DXC Utils生成
		Microsoft::WRL::ComPtr<IDxcUtils>	hDXCUtils;
		if( FAILED( DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( hDXCUtils.GetAddressOf() ) ) ) )
			return	false;

		//	リフレクションコンテナ取得
		Microsoft::WRL::ComPtr<IDxcContainerReflection>	hContainerReflection;
		if( FAILED( DxcCreateInstance( CLSID_DxcContainerReflection, IID_PPV_ARGS( hContainerReflection.GetAddressOf() ) ) ) )
			return	false;

		//	ファイルポインタからBlob生成
		Microsoft::WRL::ComPtr<IDxcBlobEncoding>	hShader;
		if( FAILED( hDXCUtils->CreateBlob( _pBufferPointer, s_cast<U32>( _uBufferSize ), CP_ACP, hShader.GetAddressOf() ) ) )
			return	false;

		//	DXILチャンクブロックのインデックスを取得
		UINT	uShaderIndex	= 0;
		if( FAILED( hContainerReflection->Load( hShader.Get() ) ) )
			return	false;

		if( FAILED( hContainerReflection->FindFirstPartKind( DXC_PART_DXIL, &uShaderIndex ) ) )
			return	false;

		//	シェーダーリフレクションインターフェース取得
		if( FAILED( hContainerReflection->GetPartReflection( uShaderIndex, IID_PPV_ARGS( _rOutput.GetAddressOf() ) ) ) )
			return	false;

		return	true;
	}
}
#endif

shader_base::shader_base(shader_type type, shader_load_type load_type, std::string_view file_path)
: type(type)
, load_type(load_type)
, file_path(file_path)
{
}

bool shader_base::load()
{
	bool flag = false;
	switch(load_type)
	{
		case shader_load_type::cso:
		{
			flag = load_from_cso(file_path);
			break;
		}
		case shader_load_type::file:
		{
			flag = load_from_cso(file_path);
			break;
		}
	}

	return flag;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
vertex_shader::vertex_shader(shader_load_type type, std::string_view file_path, std::vector<D3D11_INPUT_ELEMENT_DESC>& input_element_desc)
: shader_base(shader_type::vertex, type, file_path)
, input_element_desc(input_element_desc)
{
}

vertex_shader::~vertex_shader()
{
}

bool vertex_shader::load_from_cso(std::string_view file_path)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, file_path.data(), "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
	hr = render_data::instance()->get_device()->CreateVertexShader(cso_data.get(), cso_sz, nullptr, shader.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	if(FAILED(hr))
		return false;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	if (!input_element_desc.empty())
	{
		hr = render_data::instance()->get_device()->CreateInputLayout(input_element_desc.data(), static_cast<UINT>(input_element_desc.size()), cso_data.get(), cso_sz, input_layout.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		if (FAILED(hr))
			return false;
	}

	this->shader = shader;
	this->input_layout = input_layout;
	return true;
}

bool vertex_shader::load_from_file(std::string_view file_path)
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
hull_shader::hull_shader(shader_load_type type, std::string_view file_path)
: shader_base(shader_type::hull, type, file_path)
{
}

hull_shader::~hull_shader()
{
}

bool hull_shader::load_from_cso(std::string_view file_path)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, file_path.data(), "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11HullShader> shader;
	hr = render_data::instance()->get_device()->CreateHullShader(cso_data.get(), cso_sz, nullptr, shader.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	if(FAILED(hr))
		return false;

	this->shader = shader;
	return true;
}

bool hull_shader::load_from_file(std::string_view file_path)
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
domain_shader::domain_shader(shader_load_type type, std::string_view file_path)
: shader_base(shader_type::domain, type, file_path)
{
}

domain_shader::~domain_shader()
{
}

bool domain_shader::load_from_cso(std::string_view file_path)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, file_path.data(), "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11DomainShader> shader;
	hr = render_data::instance()->get_device()->CreateDomainShader(cso_data.get(), cso_sz, nullptr, shader.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	if(FAILED(hr))
		return false;

	this->shader = shader;
	return true;
}

bool domain_shader::load_from_file(std::string_view file_path)
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
geometry_shader::geometry_shader(shader_load_type type, std::string_view file_path)
: shader_base(shader_type::geometry, type, file_path)
{
}

geometry_shader::~geometry_shader()
{
}

bool geometry_shader::load_from_cso(std::string_view file_path)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, file_path.data(), "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> shader;
	hr = render_data::instance()->get_device()->CreateGeometryShader(cso_data.get(), cso_sz, nullptr, shader.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	if(FAILED(hr))
		return false;

	this->shader = shader;
	return true;
}

bool geometry_shader::load_from_file(std::string_view file_path)
{
	return false;
}

#if 0
HRESULT create_gs_with_streamout_from_cso(ID3D11Device* device, const char* cso_name, ID3D11GeometryShader** geometry_shader, D3D11_SO_DECLARATION_ENTRY* streamout_declaration_desc, UINT num_entry, UINT* buffer_strides, UINT num_strides, UINT rasterized_stream)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	unique_ptr<unsigned char[]> cso_data{ make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	hr = device->CreateGeometryShaderWithStreamOutput(cso_data.get(), cso_sz, streamout_declaration_desc, num_entry, buffer_strides, num_strides, rasterized_stream, nullptr, geometry_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	return hr;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
pixel_shader::pixel_shader(shader_load_type type, std::string_view file_path)
: shader_base(shader_type::pixel, type, file_path)
{
}

pixel_shader::~pixel_shader()
{
}

bool pixel_shader::load_from_cso(std::string_view file_path)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, file_path.data(), "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
	hr = render_data::instance()->get_device()->CreatePixelShader(cso_data.get(), cso_sz, nullptr, shader.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	if(FAILED(hr))
		return false;

	this->shader = shader;
	return true;
}

bool pixel_shader::load_from_file(std::string_view file_path)
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
compute_shader::compute_shader(shader_load_type type, std::string_view file_path)
: shader_base(shader_type::compute, type, file_path)
{
}

compute_shader::~compute_shader()
{
}

bool compute_shader::load_from_cso(std::string_view file_path)
{
	FILE* fp{ nullptr };
	fopen_s(&fp, file_path.data(), "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz{ ftell(fp) };
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> shader;
	hr = render_data::instance()->get_device()->CreateComputeShader(cso_data.get(), cso_sz, nullptr, shader.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	if(FAILED(hr))
		return false;

	this->shader = shader;
	return true;
}

bool compute_shader::load_from_file(std::string_view file_path)
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
bool	shader_manager::load_shader(shader_type type, shader_ptr& out_shader, load_shader_cso data)
{
	auto	key_name = std::string(data.file_path);
	auto	it = shaderes.find(key_name);
	if (it != shaderes.end())
	{
		if (auto ptr = it->second.lock())
		{
			out_shader = ptr;
			return	true;
		}
		shaderes.erase(it);
	}

	switch(type)
	{
		case shader_type::vertex:
		{
			out_shader	= std::make_shared<vertex_shader>(shader_load_type::cso, data.file_path, data.input_element_desc);
			break;
		}
		case shader_type::hull:
		{
			out_shader	= std::make_shared<hull_shader>(shader_load_type::cso, data.file_path);
			break;
		}
		case shader_type::domain:
		{
			out_shader	= std::make_shared<domain_shader>(shader_load_type::cso, data.file_path);
			break;
		}
		case shader_type::geometry:
		{
			out_shader	= std::make_shared<geometry_shader>(shader_load_type::cso, data.file_path);
			break;
		}
		case shader_type::pixel:
		{
			out_shader	= std::make_shared<pixel_shader>(shader_load_type::cso, data.file_path);
			break;
		}
		case shader_type::compute:
		{
			out_shader	= std::make_shared<compute_shader>(shader_load_type::cso, data.file_path);
			break;
		}
	}
	//	シェーダー読み込み処理
	if (out_shader && out_shader->load())
	{
		shaderes.insert(std::make_pair(key_name, std::weak_ptr<shader_base>(out_shader)));
		return true;
	}

	out_shader	= nullptr;
	return false;
}

shader_ptr	shader_manager::search(std::string_view name)
{
	auto key_name	= std::string(name);
	auto it	= shaderes.find(key_name);
	if (it != shaderes.end())
	{
		auto ptr = it->second.lock();
		if(!ptr)
		{
			shaderes.erase(it);
			return	nullptr;
		}
		return	ptr;
	}

	return	nullptr;
}

#undef	RETURN_SEARCH_FILE
