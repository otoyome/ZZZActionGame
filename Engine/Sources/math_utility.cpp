#include "math_utility.h"

#include "system_utility.h"

DirectX::XMFLOAT2 math::bezier(DirectX::XMFLOAT2 s, DirectX::XMFLOAT2 c1, DirectX::XMFLOAT2 e, float value)
{
	DirectX::XMFLOAT2 p1, p2, v;
	v.x = c1.x - s.x;
	v.y = c1.y - s.y;
	p1.x = v.x * value + s.x;
	p1.y = v.y * value + s.y;
	v.x = e.x - c1.x;
	v.y = e.y - c1.y;
	p2.x = v.x * value + c1.x;
	p2.y = v.y * value + c1.y;
	v.x = p2.x - p1.x;
	v.y = p2.y - p1.y;
	p1.x = v.x * value + p1.x;
	p1.y = v.y * value + p1.y;
	return p1;
}

DirectX::XMFLOAT3 math::bezier(DirectX::XMFLOAT3 s, DirectX::XMFLOAT3 c1, DirectX::XMFLOAT3 e, float value)
{
	DirectX::XMFLOAT3 p1, p2, v;
	v.x = c1.x - s.x;
	v.y = c1.y - s.y;
	v.z = c1.z - s.z;
	p1.x = v.x * value + s.x;
	p1.y = v.y * value + s.y;
	p1.z = v.z * value + s.z;
	v.x = e.x - c1.x;
	v.y = e.y - c1.y;
	v.z = e.z - c1.z;
	p2.x = v.x * value + c1.x;
	p2.y = v.y * value + c1.y;
	p2.z = v.z * value + c1.z;
	v.x = p2.x - p1.x;
	v.y = p2.y - p1.y;
	v.z = p2.z - p1.z;
	p1.x = v.x * value + p1.x;
	p1.y = v.y * value + p1.y;
	p1.z = v.z * value + p1.z;
	return p1;
}

DirectX::XMFLOAT4 math::slerp(DirectX::XMFLOAT4 s, DirectX::XMFLOAT4 e, float t)
{
	DirectX::XMFLOAT4 rotation;
	DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&s), DirectX::XMLoadFloat4(&e), t));
	return rotation;
}

bool math::equal(DirectX::XMFLOAT2 a, DirectX::XMFLOAT2 b, float epsilon)
{
	return (fabsf(a.x - b.x) <= epsilon && fabsf(a.y - b.y) <= epsilon);
}
bool math::equal(DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b, float epsilon)
{
	return (fabsf(a.x - b.x) <= epsilon && fabsf(a.y - b.y) <= epsilon && fabsf(a.z - b.z) <= epsilon);
}
bool math::equal(DirectX::XMFLOAT4 a, DirectX::XMFLOAT4 b, float epsilon)
{
	return (fabsf(a.x - b.x) <= epsilon && fabsf(a.y - b.y) <= epsilon && fabsf(a.z - b.z) <= epsilon && fabsf(a.w - b.w) <= epsilon);
}

float math::normalize_angle(float radians)
{
//	return atan2f(sinf(radians), cosf(radians));
	float ret = radians;
	while (ret >= DirectX::XM_2PI)
	{
		ret -= DirectX::XM_2PI;
	}
	while (ret < 0)
	{
		ret += DirectX::XM_2PI;
	}
	return ret;
}

float math::rand_range(float min, float max)
{
	float value = max - min;
	float v = static_cast<float>(rand() % 100000) * 0.00001f;
	return v * value + min;
}

float math::ease::in_sine(float time)
{
	return sinf(1.5707963f * time);
}

float math::ease::out_sine(float time)
{
	return 1.0f + sinf(DirectX::XM_PI * (--time));
}

float math::ease::in_out_sine(float time)
{
	return 0.5f * (1.0f + sinf(DirectX::XM_PI * (time - 0.5f)));
}

float math::ease::in_quad(float time)
{
	return time * time;
}

float math::ease::out_quad(float time)
{ 
	return time * (2.0f - time);
}

float math::ease::in_out_quad(float time)
{
	return time < 0.5 ? 2.0f * time * time : time * (4.0f - 2.0f * time) - 1.0f;
}

float math::ease::in_cubic(float time)
{
	return time * time * time;
}

float math::ease::out_cubic(float time)
{
	return 1.0f + (--time) * time * time;
}

float math::ease::in_out_cubic(float time)
{
	return time < 0.5f ? 4.0f * time * time * time : 1.0f + (--time) * (2.0f * (--time)) * (2.0f * time);
}

float math::ease::in_quart(float time)
{
	time *= time;
	return time * time;
}

float math::ease::out_quart(float time)
{
	time = (--time) * time;
	return 1.0f - time * time;
}

float math::ease::in_out_quart(float time)
{
	if (time < 0.5f)
	{
		time *= time;
		return 8.0f * time * time;
	}
	time = (--time) * time;
	return 1.0f - 8.0f * time * time;
}

float math::ease::in_quint(float time)
{
	float time_pow = time * time;
	return time * time_pow * time_pow;
}

float math::ease::out_quint(float time)
{
	float time_pow = (--time) * time;
	return 1.0f + time * time_pow * time_pow;
}

float math::ease::in_out_quint(float time)
{
	float time_pow;
	if (time < 0.5f)
	{
		time_pow = time * time;
		return 16.0f * time * time_pow * time_pow;
	}

	time_pow = (--time) * time;
	return 1.0f + 16.0f * time * time_pow * time_pow;
}

float math::ease::in_expo(float time)
{
	return (powf(2.0f, 8.0f * time) - 1.0f) / 255.0f;
}

float math::ease::out_expo(float time)
{
	return 1.0f - powf(2.0f, -8.0f * time);
}

float math::ease::in_out_expo(float time)
{
	if (time < 0.5f)
	{
		return (powf(2.0f, 16.0f * time) - 1.0f) / 510.0f;
	}

	return 1.0f - 0.5f * powf(2.0f, -16.0f * (time - 0.5f));
}

float math::ease::in_circ(float time)
{
	return 1.0f - sqrtf(1.0f - time);
}

float math::ease::out_circ(float time)
{
	return sqrtf(time);
}

float math::ease::in_out_circ(float time)
{
	if (time < 0.5f)
	{
		return (1.0f - sqrtf(1.0f - 2.0f * time)) * 0.5f;
	}

	return (1.0f + sqrtf(2.0f * time - 1.0f)) * 0.5f;
}

float math::ease::in_back(float time)
{
	return time * time * (2.70158f * time - 1.70158f);
}

float math::ease::out_back(float time)
{
	return 1.0f + (--time) * time * (2.70158f * time + 1.70158f);
}

float math::ease::in_out_back(float time)
{
	if (time < 0.5f)
	{
		return time * time * (7.0f * time - 2.5f) * 2.0f;
	}

	return 1.0f + (--time) * time * 2.0f * (7.0f * time + 2.5f);
}

float math::ease::in_elastic(float time)
{
	float time_pow = time * time;
	return time_pow * time_pow * sinf(time * DirectX::XM_PI * 4.5f);
}

float math::ease::out_elastic(float time)
{
	float time_pow = (time - 1.0f) * (time - 1.0f);
	return 1.0f - time_pow * time_pow * cosf(time * DirectX::XM_PI * 4.5f);
}

float math::ease::in_out_elastic(float time)
{
	float time_pow;
	if (time < 0.45f)
	{
		time_pow = time * time;
		return 8.0f * time_pow * time_pow * sinf(time * DirectX::XM_PI * 9.0f);
	}
	else if (time < 0.55f)
	{
		return 0.5f + 0.75f * sinf(time * DirectX::XM_PI * 4.0f);
	}

	time_pow = (time - 1.0f) * (time - 1.0f);
	return 1.0f - 8.0f * time_pow * time_pow * sinf(time * DirectX::XM_PI * 9.0f);
}

float math::ease::in_bounce(float time)
{
	return powf( 2.0f, 6.0f * (time - 1.0f)) * fabsf(sinf(time * DirectX::XM_PI * 3.5f));
}

float math::ease::out_bounce(float time)
{
	return 1.0f - powf(2.0f, -6.0f * time) * fabsf(cosf(time * DirectX::XM_PI * 3.5f));
}

float math::ease::in_out_bounce(float time)
{
	if (time < 0.5f)
	{
		return 8.0f * powf(2.0f, 8.0f * (time - 1.0f)) * fabsf(sinf(time * DirectX::XM_PI * 7.0f));
	}

	return 1.0f - 8.0f * powf(2.0f, -8.0f * time) * fabsf(sinf(time * DirectX::XM_PI * 7.0f));
}

float math::ease::calculate(float time, math::ease::type type)
{
	switch (type)
	{
		case math::ease::type::in_sine: return in_sine(time);
		case math::ease::type::out_sine: return out_sine(time);
		case math::ease::type::in_out_sine: return in_out_sine(time);

		case math::ease::type::in_quad: return in_quad(time);
		case math::ease::type::out_quad: return out_quad(time);
		case math::ease::type::in_out_quad: return in_out_quad(time);

		case math::ease::type::in_cubic: return in_cubic(time);
		case math::ease::type::out_cubic: return out_cubic(time);
		case math::ease::type::in_out_cubic: return in_out_cubic(time);

		case math::ease::type::in_quart: return in_quart(time);
		case math::ease::type::out_quart: return out_quart(time);
		case math::ease::type::in_out_quart: return in_out_quart(time);

		case math::ease::type::in_quint: return in_quint(time);
		case math::ease::type::out_quint: return out_quint(time);
		case math::ease::type::in_out_quint: return in_out_quint(time);

		case math::ease::type::in_expo: return in_expo(time);
		case math::ease::type::out_expo: return out_expo(time);
		case math::ease::type::in_out_expo: return in_out_expo(time);

		case math::ease::type::in_circ: return in_circ(time);
		case math::ease::type::out_circ: return out_circ(time);
		case math::ease::type::in_out_circ: return in_out_circ(time);

		case math::ease::type::in_back: return in_back(time);
		case math::ease::type::out_back: return out_back(time);
		case math::ease::type::in_out_back: return in_out_back(time);

		case math::ease::type::in_elastic: return in_elastic(time);
		case math::ease::type::out_elastic: return out_elastic(time);
		case math::ease::type::in_out_elastic: return in_out_elastic(time);

		case math::ease::type::in_bounce: return in_bounce(time);
		case math::ease::type::out_bounce: return out_bounce(time);
		case math::ease::type::in_out_bounce: return in_out_bounce(time);

		case math::ease::type::lerp: return math::saturate(time);
	}

	return time;
}

bool math::is_sharp_angle(const DirectX::XMVECTOR& vectorA, const DirectX::XMVECTOR& vectorB)
{
	return DirectX::XMVectorGetX(DirectX::XMVector3Dot(vectorA, vectorB)) > 0.0f;
}

bool math::is_sharp_angle(const DirectX::XMFLOAT3& pointA, const DirectX::XMFLOAT3& pointB, const DirectX::XMFLOAT3& pointC)
{
	DirectX::XMVECTOR PointA = DirectX::XMLoadFloat3(&pointA);
	DirectX::XMVECTOR PointB = DirectX::XMLoadFloat3(&pointB);
	DirectX::XMVECTOR PointC = DirectX::XMLoadFloat3(&pointC);
	return is_sharp_angle(DirectX::XMVectorSubtract(PointA, PointB), DirectX::XMVectorSubtract(PointC, PointB));
}


//	クォータニオンからオイラー角に変換
DirectX::XMFLOAT3 math::convert_rotation_matrix_to_euler(DirectX::XMFLOAT4X4 rotation)
{
	//	ZXY回転
	DirectX::XMFLOAT3 euler;
	if (1.0f - fabs(rotation.m[2][1]) < 1.0e-6f)
	{
		euler.x = rotation.m[2][1] < 0 ? DirectX::XM_PIDIV2 : -DirectX::XM_PIDIV2;
		euler.y = atan2f(rotation.m[1][0], rotation.m[0][0]);
		euler.z = 0;
	}
	else
	{
		euler.x = asinf(-rotation.m[2][1]);
		euler.y = atan2f(rotation.m[2][0], rotation.m[2][2]);
		euler.z = atan2f(rotation.m[0][1], rotation.m[1][1]);
	}
	return euler;
}

DirectX::XMFLOAT3 math::calculate_right(DirectX::XMFLOAT4 quaternion)
{
	float xx = quaternion.x * quaternion.y;
	float xz = quaternion.x * quaternion.z;
	float yy = quaternion.y * quaternion.y;
	float zz = quaternion.z * quaternion.z;
	float wy = quaternion.w * quaternion.y;
	float wz = quaternion.w * quaternion.z;

	return	DirectX::XMFLOAT3(1.0f - 2.0f * yy - 2.0f * zz, 2.0f * xx + 2.0f * wz, 2.0f * xz - 2.0f * wy);
}

DirectX::XMFLOAT3 math::calculate_up(DirectX::XMFLOAT4 quaternion)
{
	float xx = quaternion.x * quaternion.x;
	float xy = quaternion.x * quaternion.y;
	float yz = quaternion.y * quaternion.z;
	float zz = quaternion.z * quaternion.z;
	float wx = quaternion.w * quaternion.x;
	float wz = quaternion.w * quaternion.z;

	return	DirectX::XMFLOAT3(2.0f * xy - 2.0f * wz, 1.0f - 2.0f * xx - 2.0f * zz, 2.0f * yz + 2.0f * wx);
}

DirectX::XMFLOAT3 math::calculate_front(DirectX::XMFLOAT4 quaternion)
{
	float xx = quaternion.x * quaternion.x;
	float xz = quaternion.x * quaternion.z;
	float yy = quaternion.y * quaternion.y;
	float yz = quaternion.y * quaternion.z;
	float wx = quaternion.w * quaternion.x;
	float wy = quaternion.w * quaternion.y;

	return	DirectX::XMFLOAT3(2.0f * xz + 2.0f * wy, 2.0f * yz - 2.0f * wx, 1.0f - 2.0f * xx - 2.0f * yy);
}

DirectX::XMFLOAT3 math::convert_quaternion_to_euler(DirectX::XMFLOAT4 quaternion)
{
	DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&quaternion));
	DirectX::XMFLOAT4X4 rotation;
	DirectX::XMStoreFloat4x4(&rotation, Rotation);
	return math::convert_rotation_matrix_to_euler(rotation);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
math::rotator::rotator(DirectX::XMFLOAT4 quaternion)
: rotator(convert_quaternion_to_euler(quaternion))
{
}

math::rotator::rotator(DirectX::XMFLOAT4X4 rotation)
: rotator(convert_rotation_matrix_to_euler(rotation))
{
}

DirectX::XMFLOAT3 math::rotator::get_right() const
{
	DirectX::XMFLOAT4 quaternion;
	DirectX::XMStoreFloat4(&quaternion, DirectX::XMQuaternionRotationRollPitchYaw(x, y, z));
	return calculate_right(quaternion);
}

DirectX::XMFLOAT3 math::rotator::get_up() const
{
	DirectX::XMFLOAT4 quaternion;
	DirectX::XMStoreFloat4(&quaternion, DirectX::XMQuaternionRotationRollPitchYaw(x, y, z));
	return calculate_up(quaternion);
}

DirectX::XMFLOAT3 math::rotator::get_front() const
{
	DirectX::XMFLOAT4 quaternion;
	DirectX::XMStoreFloat4(&quaternion, DirectX::XMQuaternionRotationRollPitchYaw(x, y, z));
	return calculate_front(quaternion);
}

DirectX::XMFLOAT4 math::rotator::convert_quaternion() const
{
	DirectX::XMFLOAT4 quaternion;
	DirectX::XMStoreFloat4(&quaternion, DirectX::XMQuaternionRotationRollPitchYaw(x, y, z));
	return quaternion;
}

void math::rotator::set_parameter_from_direction(DirectX::XMFLOAT3 direction)
{
	float s = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	x = -asinf(direction.y / s);
	y = atan2f(direction.x / s, direction.z / s);
	z = 0.0f;
}

math::rotator::operator DirectX::XMFLOAT4() const
{
	return convert_quaternion();
}

math::rotator::operator DirectX::XMFLOAT3() const
{
	return {x, y, z};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
math::compute_fade::out_calculator::out_calculator(time_pair fade_out)
: fade_out(fade_out)
{
}

float math::compute_fade::out_calculator::compute(float time)
{
	float factor = math::ease::calculate((time - fade_out.start) / (fade_out.end - fade_out.start), fade_out.ease_type);
	return math::saturate(factor);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
math::compute_fade::in_calculator::in_calculator(time_pair fade_in)
: fade_in(fade_in)
{
}

float math::compute_fade::in_calculator::compute(float time)
{
	float factor = math::ease::calculate(1.0f - (time - fade_in.start) / (fade_in.end - fade_in.start), fade_in.ease_type);
	return math::saturate(factor);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
math::compute_fade::calculator::calculator(time_pair fade_out, time_pair fade_in)
: fade_in(fade_in)
, fade_out(fade_out)
{
}

float math::compute_fade::calculator::compute(float time, bool* end_flag)
{
	if (end_flag)
		*end_flag = false;
	float factor = 0.0f;
	if (fade_out.start <= time && time <= fade_out.end)
	{
		factor = math::ease::calculate((time - fade_out.start) / (fade_out.end - fade_out.start), fade_out.ease_type);
	}
	else if (fade_out.end <= time && time <= fade_in.start)
	{
		factor = 1.0f;
	}
	else if (fade_in.start <= time && time <= fade_in.end)
	{
		factor = math::ease::calculate(1.0f - (time - fade_in.start) / (fade_in.end - fade_in.start), fade_in.ease_type);
	}
	else if (fade_in.end <= time && end_flag)
	{
		*end_flag = true;
	}
	return factor;
}