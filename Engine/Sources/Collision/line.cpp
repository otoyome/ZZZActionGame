#include	"collision/line.h"

using namespace collision;

line::line()
{
}

line::line(types type, const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& vector)
{
	initialize( type, start, vector );
}

void line::initialize(types type, const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& vector)
{
	this->type = type;
	point[0] = start;
	if (type == types::line)
	{
		DirectX::XMStoreFloat3(&point[1], DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&vector)));
	}
	else
	{
		point[1] = vector;
	}
}

float line::calculate_distance(DirectX::XMFLOAT3& horizon, float& delta, const DirectX::XMFLOAT3& point) const
{
	return (type == types::line) ? calculate_line_distance(horizon, delta, point)
								 : calculate_segment_distamce(horizon, delta, point);
}

float line::calculate_distance(DirectX::XMFLOAT3& horizon1, float& delta1, DirectX::XMFLOAT3& horizon2, float& delta2, const line& line) const
{
	return (type == types::line) ? calculate_line_distance(horizon1, delta1, horizon2, delta2, line)
								 : calculate_segment_distamce(horizon1, delta1, horizon2, delta2, line);
}

bool line::is_parallel(const line& line) const
{
	DirectX::XMVECTOR PointA[2] = {DirectX::XMLoadFloat3(&point[0]), DirectX::XMLoadFloat3(&point[1])};
	DirectX::XMVECTOR PointB[2] = {DirectX::XMLoadFloat3(&line.point[0]), DirectX::XMLoadFloat3(&line.point[1])};
	DirectX::XMVECTOR LineA = DirectX::XMVectorSubtract(PointA[1], PointA[0]);
	DirectX::XMVECTOR LineB = DirectX::XMVectorSubtract(PointB[1], PointB[0]);
	DirectX::XMVECTOR Cross = DirectX::XMVector3Cross(LineA, LineB);
	return	DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Cross)) < FLT_EPSILON;
}

float line::get_length() const
{
	DirectX::XMVECTOR PointA[2] = {DirectX::XMLoadFloat3(&point[0]), DirectX::XMLoadFloat3(&point[1])};
	DirectX::XMVECTOR LineA = DirectX::XMVectorSubtract(PointA[1], PointA[0]);

	return (type == types::line) ? FLT_MAX : DirectX::XMVectorGetX(DirectX::XMVector3Length(LineA));
}

line::types line::get_type() const
{
	return	type;
}

DirectX::XMFLOAT3 line::get_point(float t) const
{
	DirectX::XMVECTOR PointA[2] = {DirectX::XMLoadFloat3(&point[0]), DirectX::XMLoadFloat3(&point[1])};
	DirectX::XMVECTOR LineA = DirectX::XMVectorSubtract(PointA[1], PointA[0]);

	DirectX::XMVECTOR Position;
	if (type == types::line)
	{
		Position = DirectX::XMVectorMultiplyAdd(PointA[1], DirectX::XMVectorSet(t, t, t, 0), PointA[0]);
	}
	else
	{
		Position = DirectX::XMVectorLerp(PointA[0], PointA[1], t);
	}
	DirectX::XMFLOAT3 position;
	DirectX::XMStoreFloat3(&position, Position);
	return position;
}

float line::calculate_line_distance(DirectX::XMFLOAT3& horizon, float& delta, const DirectX::XMFLOAT3& point) const
{
	DirectX::XMVECTOR PointA[2] = {DirectX::XMLoadFloat3(&this->point[0]), DirectX::XMLoadFloat3(&this->point[1])};
	DirectX::XMVECTOR PointB = DirectX::XMLoadFloat3(&point);
	float square_length = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(PointA[1]));
	delta	= 0.0f;
	if (square_length > 0.0f)
	{
		delta = DirectX::XMVectorGetX(DirectX::XMVector3Dot(PointA[1], DirectX::XMVectorSubtract(PointB, PointA[0])));
		delta /= square_length;
	}

	DirectX::XMVECTOR Horizon;
	Horizon = DirectX::XMVectorMultiplyAdd(PointA[1], DirectX::XMVectorSet(delta, delta, delta, 0), PointA[0]);
	DirectX::XMStoreFloat3(&horizon, Horizon);

	return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(Horizon, PointB)));
}

float line::calculate_segment_distamce(DirectX::XMFLOAT3& horizon, float& delta, const DirectX::XMFLOAT3& point) const
{
	DirectX::XMVECTOR PointA[2] = {DirectX::XMLoadFloat3(&this->point[0]), DirectX::XMLoadFloat3(&this->point[1])};
	DirectX::XMVECTOR Point = DirectX::XMLoadFloat3(&point);
	DirectX::XMVECTOR LineA = DirectX::XMVectorSubtract(PointA[1], PointA[0]);
	DirectX::XMFLOAT3 lineA;
	DirectX::XMStoreFloat3(&lineA, LineA);
	//	垂線の長さ、垂線の足の座標及びtを算出
	line line(types::line, this->point[0], lineA);
	float length = line.calculate_line_distance(horizon, delta, point);

	if (!math::is_sharp_angle(point, this->point[0], this->point[1]))
	{
		//	∠_rPoint, m_vPoint[0], m_vPoint[1] が鈍角なので、始点の外側と判定する
		horizon	= this->point[0];
		return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PointA[0], Point)));
	}
	else if( !math::is_sharp_angle(point, this->point[1], this->point[0]))
	{
		//	∠_rPoint, m_vPoint[1], m_vPoint[0] が鈍角なので、終点側の外側と判定する
		horizon	= this->point[1];
		return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PointA[1], Point)));
	}
	return length;
}

float line::calculate_line_distance(DirectX::XMFLOAT3& horizon1, float& delta1, DirectX::XMFLOAT3& horizon2, float& delta2, const line& line) const
{
	//	2直線が平行？
	if (is_parallel(line))
	{
		//	点P11と直線L2の最短距離の問題に帰着
		float length = line.calculate_line_distance(horizon2, delta2, point[0]);
		horizon1	= point[0];
		delta1	= 0.0f;	//delta2	= 0.0f;

		return	length;
	}

	//	2直線はねじれ関係
	DirectX::XMVECTOR PointA[2] = {DirectX::XMLoadFloat3(&point[0]), DirectX::XMLoadFloat3(&point[1])};
	DirectX::XMVECTOR PointB[2] = {DirectX::XMLoadFloat3(&line.point[0]), DirectX::XMLoadFloat3(&line.point[1])};
	float dotV1_V2 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(PointA[1], PointB[1]));
	float dotV1_V1 = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(PointA[1]));
	float dotV2_V2 = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(PointB[1]));
	DirectX::XMVECTOR VectorP21_P11 = DirectX::XMVectorSubtract(PointA[0], PointB[0]);

	float dotB2_P21P11 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(PointB[1], VectorP21_P11));
	float dotB1_P21P11 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(PointA[1], VectorP21_P11));
	delta1 = ( dotV1_V2 * dotB2_P21P11 - dotV2_V2 * dotB1_P21P11 ) / ( dotV1_V1 * dotV2_V2 - dotV1_V2 * dotV1_V2 );
	horizon1 = get_point(delta1);
	DirectX::XMVECTOR Horizon1 = DirectX::XMLoadFloat3(&horizon1);
	delta2 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(PointB[1], DirectX::XMVectorSubtract(Horizon1, PointB[0])));
	delta2 /= dotV2_V2;
	horizon2 = line.get_point(delta2);
	DirectX::XMVECTOR Horizon2 = DirectX::XMLoadFloat3(&horizon2);
	return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(Horizon2, Horizon1)));
}

float line::calculate_segment_distamce(DirectX::XMFLOAT3& horizon1, float& delta1, DirectX::XMFLOAT3& horizon2, float& delta2, const line& line) const
{
	DirectX::XMVECTOR PointA[2] = {DirectX::XMLoadFloat3(&point[0]), DirectX::XMLoadFloat3(&point[1])};
	DirectX::XMVECTOR PointB[2] = {DirectX::XMLoadFloat3(&line.point[0]), DirectX::XMLoadFloat3(&line.point[1])};
	DirectX::XMVECTOR LineA2_A1 = DirectX::XMVectorSubtract(PointA[1], PointA[0]);
	DirectX::XMVECTOR LineB2_B1 = DirectX::XMVectorSubtract(PointB[1], PointB[0]);
	DirectX::XMVECTOR LineA1_B1 = DirectX::XMVectorSubtract(PointA[0], PointB[0]);

	float square_length1 = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(LineA2_A1));
	float square_length2 = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(LineB2_B1));
	float dotLB2B1_LA1B1 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(LineB2_B1, LineA1_B1));

	//	S1が縮退している？
	if( square_length1 <= FLT_EPSILON )
	{
		//	S2も縮退？
		if( square_length2 < FLT_EPSILON )
		{
			//	点と点の距離の問題に帰着
			delta1 = 0.0f;
			delta2 = 0.0f;
			horizon1 = point[0];
			horizon2 = line.point[0];
			DirectX::XMVECTOR Horizon1 = DirectX::XMLoadFloat3(&horizon1);
			DirectX::XMVECTOR Horizon2 = DirectX::XMLoadFloat3(&horizon2);
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(Horizon2, Horizon1)));
		}
		else
		{
			//	S1の始点とS2の最短問題に帰着
			delta1 = 0.0f;
			delta2 = math::saturate(dotLB2B1_LA1B1 / square_length2);
		}
	}
	else
	{
		float dotLA2A1_LA1B1 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(LineA2_A1, LineA1_B1));
		if( square_length2 <= FLT_EPSILON )
		{
			//	S2が縮退している場合は
			//	S2の始点とS1の最短問題に帰着
			delta2 = 0.0f;
			delta1 = math::saturate(-dotLA2A1_LA1B1 / square_length1);
		}
		else
		{
			float dotLA2A1_LB2B1 = DirectX::XMVectorGetX(DirectX::XMVector3Dot(LineA2_A1, LineB2_B1));
			float deNormal = square_length1 * square_length2 - dotLA2A1_LB2B1 * dotLA2A1_LB2B1;
			if( deNormal != 0.0f )
			{
				delta1 = math::saturate((dotLA2A1_LB2B1 * dotLB2B1_LA1B1 - dotLA2A1_LA1B1 * square_length2) / deNormal);
			}
			else
			{
				delta1 = 0.0f;
			}

			delta2 = ( dotLA2A1_LB2B1 * delta1 + dotLB2B1_LA1B1 ) / square_length2;
			
			//	S2側が外の場合
			if( delta2 < 0.0f )
			{
				delta2 = 0.0f;
				delta1 = math::saturate(-dotLA2A1_LA1B1 / square_length1);
			}
			else	if( delta2 > 1.0f )
			{
				delta2 = 1.0f;
				delta1 = math::saturate((dotLA2A1_LB2B1 - dotLA2A1_LA1B1) / square_length1);
			}
		}
	}

	DirectX::XMVECTOR Horizon1 = DirectX::XMVectorMultiplyAdd(LineA2_A1, DirectX::XMVectorSet(delta1, delta1, delta1, delta1), PointA[0]);
	DirectX::XMVECTOR Horizon2 = DirectX::XMVectorMultiplyAdd(LineB2_B1, DirectX::XMVectorSet(delta2, delta2, delta2, delta2), PointB[0]);
	float length =  DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(Horizon2, Horizon1)));
	DirectX::XMStoreFloat3(&horizon1, Horizon1);
	DirectX::XMStoreFloat3(&horizon2, Horizon2);
	return length;
}

