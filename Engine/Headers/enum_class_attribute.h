#pragma	once

#define	ENUM_ATTR_BITFLAG( TYPE )											\
static	constexpr	TYPE	operator |( const TYPE& _rLeft, const TYPE& _rRight )	\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<TYPE>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	TYPE	operator &( const TYPE& _rLeft, const TYPE& _rRight )	\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<TYPE>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	TYPE	operator ^( const TYPE& _rLeft, const TYPE& _rRight )	\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<TYPE>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	TYPE	operator ~( const TYPE& _rValue )						\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<TYPE>( ~static_cast<UTYPE>( _rValue ) );								\
}																					\
																					\
static	inline		TYPE&	operator|=( TYPE& _rLeft, const TYPE& _rRight )			\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<TYPE>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	inline		TYPE&	operator&=( TYPE& _rLeft, const TYPE& _rRight )			\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<TYPE>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	inline		TYPE&	operator^=( TYPE& _rLeft, const TYPE& _rRight )			\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<TYPE>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned long long	operator |( const unsigned long long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned long long>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned long long	operator &( const unsigned long long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned long long>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned long long	operator ^( const unsigned long long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned long long>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned long long	operator|=( unsigned long long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned long long>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned long long	operator&=( unsigned long long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned long long>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned long long	operator^=( unsigned long long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned long long>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	long long	operator |( const long long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<long long>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	long long	operator &( const long long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<long long>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	long long	operator ^( const long long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<long long>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	long long	operator|=( long long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<long long>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	long long	operator&=( long long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<long long>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	long long	operator^=( long long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<long long>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned long	operator |( const unsigned long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned long>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned long	operator &( const unsigned long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned long>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned long	operator ^( const unsigned long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned long>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned long	operator|=( unsigned long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned long>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned long	operator&=( unsigned long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned long>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned long	operator^=( unsigned long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned long>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	long	operator |( const long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<long>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	long	operator &( const long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<long>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	long	operator ^( const long& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<long>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	long	operator|=( long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<long>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	long	operator&=( long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<long>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	long	operator^=( long& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<long>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned short	operator |( const unsigned short& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned short>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned short	operator &( const unsigned short& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned short>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned short	operator ^( const unsigned short& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned short>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned short	operator|=( unsigned short& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned short>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned short	operator&=( unsigned short& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned short>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned short	operator^=( unsigned short& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned short>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	short	operator |( const short& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<short>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	short	operator &( const short& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<short>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	short	operator ^( const short& _rLeft, const TYPE& _rRight )		\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<short>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	short	operator|=( short& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<short>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	short	operator&=( short& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<short>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	short	operator^=( short& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<short>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );	\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned char	operator |( const unsigned char& _rLeft, const TYPE& _rRight )			\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned char>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned char	operator &( const unsigned char& _rLeft, const TYPE& _rRight )			\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned char>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned char	operator ^( const unsigned char& _rLeft, const TYPE& _rRight )			\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<unsigned char>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	unsigned char	operator|=( unsigned char& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned char>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned char	operator&=( unsigned char& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned char>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	unsigned char	operator^=( unsigned char& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<unsigned char>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	char	operator |( const char& _rLeft, const TYPE& _rRight )			\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<char>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	char	operator &( const char& _rLeft, const TYPE& _rRight )			\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<char>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	char	operator ^( const char& _rLeft, const TYPE& _rRight )			\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	return	static_cast<char>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
}																					\
																					\
static	constexpr	char	operator|=( char& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<char>( static_cast<UTYPE>( _rLeft ) | static_cast<UTYPE>( _rRight ) );		\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	char	operator&=( char& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<char>( static_cast<UTYPE>( _rLeft ) & static_cast<UTYPE>( _rRight ) );		\
	return	_rLeft;																	\
}																					\
																					\
static	constexpr	char	operator^=( char& _rLeft, const TYPE& _rRight )				\
{																					\
	using	UTYPE	= typename std::underlying_type<TYPE>::type;					\
	_rLeft	= static_cast<char>( static_cast<UTYPE>( _rLeft ) ^ static_cast<UTYPE>( _rRight ) );		\
	return	_rLeft;																	\
}																					\

