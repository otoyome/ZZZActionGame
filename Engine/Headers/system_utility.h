#pragma	once

#define	NOMINMAX
#include <tchar.h>
#include <sstream>
#include <algorithm>
#include <string>

#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>

#include <d3d11.h>
#include <wrl.h>
#include <cassert>

#include "enum_class_attribute.h"
#include "system_units.h"

#include "cereal/cereal.hpp"
#include "magic_enum/include/magic_enum/magic_enum.hpp"

namespace	system_utility
{
	template<typename	LeftType, typename	RightType>
	inline	bool	equals( const std::weak_ptr<LeftType>& left, const std::weak_ptr<RightType>& right )
	{
		return	!left.owner_before( right ) && !right.owner_before( left );
	}

	template<typename	LeftType, typename	RightType>
	inline	bool	equals( const std::weak_ptr<LeftType>& left, const std::shared_ptr<RightType>& right )
	{
		return !left.owner_before( left ) && !right.owner_before( left );
	}

	template<typename	Type>
	struct	is_iterable
	{
	private:
		template<typename	UnderlingType>
		static	constexpr	auto	check_adl( UnderlingType&& _rrValue )
		-> decltype( begin( _rrValue ), end( _rrValue ), std::true_type() );
		static	constexpr	std::false_type	check_adl( ... );

		template<typename	UnderlingType>
		static	constexpr	auto	check_std( UnderlingType&& _rrValue )
		-> decltype( std::begin( _rrValue ), std::end( _rrValue ), std::true_type() );
		static	constexpr	std::false_type	check_std( ... );

		template<typename	UnderlingType>
		static	constexpr	auto	check_member( UnderlingType&& _rrValue )
		-> decltype( _rrValue.begin(), _rrValue.end(), std::true_type() );
		static	constexpr	std::false_type	check_member(...);

	public:
		static	constexpr	bool	value	= ( decltype( check_adl( std::declval<Type>() ) )::value ||
												decltype( check_std( std::declval<Type>() ) )::value ||
												decltype( check_member( std::declval<Type>() ) )::value );
	};

	template<bool Test, typename Ty = void>
	using	enable_if_type	= typename std::enable_if<Test, Ty>::type;

	//	unique_ptr用キャスト処理
	template<typename Derived, typename Type, typename enable_if_type<!std::is_same<Derived, Type>{}, std::nullptr_t> = nullptr>
	inline	std::unique_ptr<Derived>	static_pointer_cast( const std::unique_ptr<Type>& pointer )
	{
		return	std::unique_ptr<Derived>( static_cast<Derived*>( pointer.release() ) );
	}

	//	unique_ptr用同じ型同士のキャスト
	template<typename Derived, typename Type, typename enable_if_type<std::is_same<Derived, Type>{}, std::nullptr_t> = nullptr>
	inline	std::unique_ptr<Derived>	static_pointer_cast( const std::unique_ptr<Type>& pointer )
	{
		return	std::unique_ptr<Derived>( pointer.release() );
	}

	//	std::string用の書式変換関数
	template<typename ... Args>
	std::string	format( std::string_view _rFormat, Args ... _pArgs )
	{
		size_t	sLength	= std::snprintf( nullptr, 0, _rFormat.data(), _pArgs ... );
		std::vector<char> vBuf( sLength + 1 );
		std::snprintf( &vBuf[0], sLength + 1, _rFormat.data(), _pArgs ... );
		return	std::string( &vBuf[0], &vBuf[0] + sLength );
	}

	template<typename BitFlagType, typename FlagType>
	bool is_bitflag(const BitFlagType& bit_flags, FlagType flag)
	{
		return bit_flags & static_cast<BitFlagType>(flag);
	}

	inline int convert_shift_jis_to_utf16(const char* shift_jis, wchar_t* unicode, size_t unicode_buffer_size)
	{
		const size_t length = strlen(shift_jis) + 1;
		return	MultiByteToWideChar(CP_ACP, 0, shift_jis, static_cast<int>(length) + 1, unicode, static_cast<int>(unicode_buffer_size));
	}

	inline int convert_unicode_to_utf8(const wchar_t* unicode, char* utf8, size_t utf8_buffer_size)
	{
		const size_t length	= wcslen( unicode ) + 1;
		return	WideCharToMultiByte(CP_UTF8, 0, unicode, static_cast<int>(length) + 1, utf8, static_cast<int>(utf8_buffer_size), NULL, NULL);
	}

	inline int convert_shift_jis_to_utf8(const char* shift_jis, char* utf8, size_t utf8_buffer_size)
	{
		wchar_t	unicode[MAX_PATH];
		const size_t length = convert_shift_jis_to_utf16(shift_jis, unicode, MAX_PATH);
		if (length > 0)
		{
			return convert_unicode_to_utf8(unicode, utf8, utf8_buffer_size);
		}
		return 0;
	}
	inline std::string convert_shift_jis_to_utf8(const char* shift_jis)
	{
		wchar_t	unicode[MAX_PATH];
		const size_t length = convert_shift_jis_to_utf16(shift_jis, unicode, MAX_PATH);
		if (length > 0)
		{
			char utf8[MAX_PATH];
			if (convert_unicode_to_utf8(unicode, utf8, ARRAYSIZE(utf8)))
			{
				return utf8;
			}
		}
		return "";
	}

	template<typename BitFlagType, typename FlagType>
	void set_bitflag(BitFlagType& bit_flags, FlagType flag, bool value)
	{
		if(value)	bit_flags	|= static_cast<BitFlagType>(flag);
		else		bit_flags	&= ~static_cast<BitFlagType>(flag);
	}

	template<typename Type>
	void std_vector_swap_clear(std::vector<Type>& vector)
	{
		vector.clear();
		std::vector<Type> v;
		std::swap(vector, v);
	}

	inline DirectX::XMFLOAT3 convert_xmfloat3_from_vector(DirectX::XMVECTOR value)
	{
		DirectX::XMFLOAT3 vector;
		DirectX::XMStoreFloat3(&vector, value);
		return vector;
	}
	inline DirectX::XMFLOAT4 convert_xmfloat4_from_vector(DirectX::XMVECTOR value)
	{
		DirectX::XMFLOAT4 vector;
		DirectX::XMStoreFloat4(&vector, value);
		return vector;
	}
	inline DirectX::XMFLOAT4X4 convert_xmfloat4x4_from_matrix(DirectX::XMMATRIX value)
	{
		DirectX::XMFLOAT4X4 mat;
		DirectX::XMStoreFloat4x4(&mat, value);
		return mat;
	}

	inline std::string to_display_time_string(float time)
	{
		int minits = static_cast<int>(time / 60);
		int second = static_cast<int>(fmodf(time, 60));
		int micro = static_cast<int>((time - minits * 60 - second) * 100);
		if (minits >= 100)
		{
			minits = 99;
			second = 59;
			micro = 99;
		}

		std::string t;
		if (minits < 10)
			t += "0";
		t += std::to_string(minits);
		t += ":";
		if (second < 10)
			t += "0";
		t += std::to_string(second);
		t += ":";
		if (micro < 10)
			t += "0";
		t += std::to_string(micro);
		return t;
	}
}	//	namespace system_utility

// シリアライズ
namespace DirectX
{
	template<class Archive>
	void serialize(Archive& archive, XMINT2& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMINT3& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMINT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMUINT2& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMUINT3& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMUINT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT2& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT3& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT4X4& m)
	{
		archive(
			cereal::make_nvp("_11", m._11), cereal::make_nvp("_12", m._12), cereal::make_nvp("_13", m._13), cereal::make_nvp("_14", m._14),
			cereal::make_nvp("_21", m._21), cereal::make_nvp("_22", m._22), cereal::make_nvp("_23", m._23), cereal::make_nvp("_24", m._24),
			cereal::make_nvp("_31", m._31), cereal::make_nvp("_32", m._32), cereal::make_nvp("_33", m._33), cereal::make_nvp("_34", m._34),
			cereal::make_nvp("_41", m._41), cereal::make_nvp("_42", m._42), cereal::make_nvp("_43", m._43), cereal::make_nvp("_44", m._44)
		);
	}
}
