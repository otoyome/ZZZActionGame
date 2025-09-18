/********************************************************************************!

	@file	Triangle.cpp

*********************************************************************************/
#include	"Triangle.h"

using	namespace	EME;
using	namespace	Collision;

CTriangle::CTriangle()
{
}

CTriangle::CTriangle( const Math::CVector3& _rPos1, const Math::CVector3& _rPos2, const Math::CVector3& _rPos3 )
{
	Init( _rPos1, _rPos2, _rPos3 );
}

void	CTriangle::Init( const Math::CVector3& _rPos1, const Math::CVector3& _rPos2, const Math::CVector3& _rPos3 )
{
	m_vPos[0]	= _rPos1;
	m_vPos[1]	= _rPos2;
	m_vPos[2]	= _rPos3;
	m_vNormal	= Math::Vector3::Cross( m_vPos[2] - m_vPos[0], m_vPos[1] - m_vPos[0] );
	m_vNormal.Normalize();
}

void	CTriangle::Init( const Math::CVector3& _rPos1, const Math::CVector3& _rPos2, const Math::CVector3& _rPos3, const Math::CVector3& _rNormal )
{
	m_vPos[0]	= _rPos1;
	m_vPos[1]	= _rPos2;
	m_vPos[2]	= _rPos3;
	m_vNormal	= _rNormal.CreateNormalize();
}

F32		CTriangle::Dist( const CLine& _rLine, Math::CVector3& _rInsert )
{
	if( _rLine.Mode() == CLine::enMode_Line )
	{
		DebugAssert( !"0" );
		return	F32_MAX;
	}
	return	SegmentDist( _rLine, _rInsert );
}

Bool	CTriangle::IntersectionPoint( const Math::CVector3& _rStart, const Math::CVector3& _rDir, Math::CVector3& _rInsert, F32& _rDist )
{
	//	表裏判定
	F32	fDot	= Math::Vector3::Dot( _rDir, m_vNormal );
	if( fDot >= 0.0f )
		return	false;

	//	交点距離算出
	F32	fDot2	= Math::Vector3::Dot( m_vPos[0] - _rStart, m_vNormal );
	F32	fDelta	= ( fDot2 == 0.0f ) ? 0 : ( fDot2 / fDot );
	if( fDelta < 0.0f )
		return	false;

	Math::CVector3	vWork	= _rDir * fDelta + _rStart, vWork2;
	//	内点判定
	vWork2	= Math::Vector3::Cross( m_vPos[1] - m_vPos[0], m_vPos[0] - vWork );
	if( Math::Vector3::Dot( vWork2, m_vNormal ) < 0.0f )
		return	false;
	vWork2	= Math::Vector3::Cross( m_vPos[2] - m_vPos[1], m_vPos[1] - vWork );
	if( Math::Vector3::Dot( vWork2, m_vNormal ) < 0.0f )
		return	false;
	vWork2	= Math::Vector3::Cross( m_vPos[0] - m_vPos[2], m_vPos[2] - vWork );
	if( Math::Vector3::Dot( vWork2, m_vNormal ) < 0.0f )
		return	false;
	_rInsert	= vWork;
	_rDist		= fDelta;
	return	true;
}

F32		CTriangle::SegmentDist( const CLine& _rLine, Math::CVector3& _rInsert )
{
	Math::CVector3	vHorizon1, vHorizon2, vTmp;
	F32	fHDelta1, fHDelta2;
	F32	fDist, fTmp;

	//	線分PQと線分ABの最短距離
	CLine	cAB( CLine::enMode_Segment, m_vPos[0], m_vPos[1] );
	fDist	= _rLine.Dist( vHorizon1, fHDelta1, vHorizon2, fHDelta2, cAB );

	//	線分PQと線分BCの最短距離
	CLine	cBC( CLine::enMode_Segment, m_vPos[1], m_vPos[2] );
	fTmp	= _rLine.Dist( vHorizon1, fHDelta1, vHorizon2, fHDelta2, cBC );
	if( fTmp < fDist )
	{
		fDist		= fTmp;
	}

	//	線分PQと線分CAの最短距離
	CLine	cCA( CLine::enMode_Segment, m_vPos[2], m_vPos[0] );
	fTmp	= _rLine.Dist( vHorizon1, fHDelta1, vHorizon2, fHDelta2, cCA );
	if( fTmp < fDist )
	{
		fDist		= fTmp;
	}

	//	点Pと三角形ABCを含む平面への距離
	if( IntersectionPoint( _rLine[0], -m_vNormal, vTmp, fTmp ) )
	{
		if( fTmp < fDist )
		{
			if( fTmp <= Math::FloatEpsilon )
			{
				_rInsert	= vTmp;
			}
			fDist		= fTmp;
		}
	}
	//	点Qと三角形ABCを含む平面への距離
	if( IntersectionPoint( _rLine[1], -m_vNormal, vTmp, fTmp ) )
	{
		if( fTmp < fDist )
		{
			if( fTmp <= Math::FloatEpsilon )
			{
				_rInsert	= vTmp;
			}
			fDist		= fTmp;
		}
	}
	return	fDist;
}
