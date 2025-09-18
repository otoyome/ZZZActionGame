#pragma	once

#include <DirectXMath.h>

namespace math
{
	template<typename Type>
	inline Type step(Type y, Type x) {return x >= y;}

	template<typename Type>
	inline Type clamp(Type value, Type min, Type max)
	{
		if (value > max) return max;
		if (value < min) return min;
		return  value;
	}
	template<> inline DirectX::XMFLOAT2 clamp<DirectX::XMFLOAT2>(DirectX::XMFLOAT2 value, DirectX::XMFLOAT2 min, DirectX::XMFLOAT2 max) {return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y)};}
	template<> inline DirectX::XMFLOAT3 clamp<DirectX::XMFLOAT3>(DirectX::XMFLOAT3 value, DirectX::XMFLOAT3 min, DirectX::XMFLOAT3 max) {return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y), clamp(value.z, min.z, max.z)};}
	template<> inline DirectX::XMFLOAT4 clamp<DirectX::XMFLOAT4>(DirectX::XMFLOAT4 value, DirectX::XMFLOAT4 min, DirectX::XMFLOAT4 max) {return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y), clamp(value.z, min.z, max.z), clamp(value.w, min.w, max.w)};}
	template<> inline DirectX::XMINT2 clamp<DirectX::XMINT2>(DirectX::XMINT2 value, DirectX::XMINT2 min, DirectX::XMINT2 max) {return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y)};}
	template<> inline DirectX::XMINT3 clamp<DirectX::XMINT3>(DirectX::XMINT3 value, DirectX::XMINT3 min, DirectX::XMINT3 max) {return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y), clamp(value.z, min.z, max.z)};}
	template<> inline DirectX::XMINT4 clamp<DirectX::XMINT4>(DirectX::XMINT4 value, DirectX::XMINT4 min, DirectX::XMINT4 max) {return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y), clamp(value.z, min.z, max.z), clamp(value.w, min.w, max.w)};}
	template<> inline DirectX::XMUINT2 clamp<DirectX::XMUINT2>(DirectX::XMUINT2 value, DirectX::XMUINT2 min, DirectX::XMUINT2 max) {return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y)};}
	template<> inline DirectX::XMUINT3 clamp<DirectX::XMUINT3>(DirectX::XMUINT3 value, DirectX::XMUINT3 min, DirectX::XMUINT3 max) {return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y), clamp(value.z, min.z, max.z)};}
	template<> inline DirectX::XMUINT4 clamp<DirectX::XMUINT4>(DirectX::XMUINT4 value, DirectX::XMUINT4 min, DirectX::XMUINT4 max) {return {clamp(value.x, min.x, max.x), clamp(value.y, min.y, max.y), clamp(value.z, min.z, max.z), clamp(value.w, min.w, max.w)};}

	template<typename Type>
	Type saturate(Type value) {return clamp(value, static_cast<Type>(0), static_cast<Type>(1));}
	template<> inline DirectX::XMFLOAT2 saturate<DirectX::XMFLOAT2>(DirectX::XMFLOAT2 value) {return clamp(value, {0, 0}, {1, 1});}
	template<> inline DirectX::XMFLOAT3 saturate<DirectX::XMFLOAT3>(DirectX::XMFLOAT3 value) {return clamp(value, {0, 0, 0}, {1, 1, 1});}
	template<> inline DirectX::XMFLOAT4 saturate<DirectX::XMFLOAT4>(DirectX::XMFLOAT4 value) {return clamp(value, {0, 0, 0, 0}, {1, 1, 1, 1});}
	template<> inline DirectX::XMINT2 saturate<DirectX::XMINT2>(DirectX::XMINT2 value) {return clamp(value, {0, 0}, {1, 1});}
	template<> inline DirectX::XMINT3 saturate<DirectX::XMINT3>(DirectX::XMINT3 value) {return clamp(value, {0, 0, 0}, {1, 1, 1});}
	template<> inline DirectX::XMINT4 saturate<DirectX::XMINT4>(DirectX::XMINT4 value) {return clamp(value, {0, 0, 0, 0}, {1, 1, 1, 1});}
	template<> inline DirectX::XMUINT2 saturate<DirectX::XMUINT2>(DirectX::XMUINT2 value) {return clamp(value, {0, 0}, {1, 1});}
	template<> inline DirectX::XMUINT3 saturate<DirectX::XMUINT3>(DirectX::XMUINT3 value) {return clamp(value, {0, 0, 0}, {1, 1, 1});}
	template<> inline DirectX::XMUINT4 saturate<DirectX::XMUINT4>(DirectX::XMUINT4 value) {return clamp(value, {0, 0, 0, 0}, {1, 1, 1, 1});}

	template<typename Type>
	inline Type lerp(Type min, Type max, float factor) {return min + static_cast<Type>((max - min) * factor);}
	template<> inline int lerp(int min, int max, float factor) { return min + static_cast<int>(static_cast<float>(max - min) * factor); }
	template<> inline DirectX::XMFLOAT2 lerp<DirectX::XMFLOAT2>(DirectX::XMFLOAT2 min, DirectX::XMFLOAT2 max, float factor) {return { lerp(min.x, max.x, factor), lerp(min.y, max.y, factor) };}
	template<> inline DirectX::XMFLOAT3 lerp<DirectX::XMFLOAT3>(DirectX::XMFLOAT3 min, DirectX::XMFLOAT3 max, float factor) {return { lerp(min.x, max.x, factor), lerp(min.y, max.y, factor), lerp(min.z, max.z, factor) };}
	template<> inline DirectX::XMFLOAT4 lerp<DirectX::XMFLOAT4>(DirectX::XMFLOAT4 min, DirectX::XMFLOAT4 max, float factor) {return { lerp(min.x, max.x, factor), lerp(min.y, max.y, factor), lerp(min.z, max.z, factor), lerp(min.w, max.w, factor) };}
	template<> inline DirectX::XMINT2 lerp<DirectX::XMINT2>(DirectX::XMINT2 min, DirectX::XMINT2 max, float factor) { return { lerp(min.x, max.x, factor), lerp(min.y, max.y, factor) }; }
	template<> inline DirectX::XMINT3 lerp<DirectX::XMINT3>(DirectX::XMINT3 min, DirectX::XMINT3 max, float factor) { return { lerp(min.x, max.x, factor), lerp(min.y, max.y, factor), lerp(min.z, max.z, factor) }; }
	template<> inline DirectX::XMINT4 lerp<DirectX::XMINT4>(DirectX::XMINT4 min, DirectX::XMINT4 max, float factor) { return { lerp(min.x, max.x, factor), lerp(min.y, max.y, factor), lerp(min.z, max.z, factor), lerp(min.w, max.w, factor) }; }
	template<> inline DirectX::XMUINT2 lerp<DirectX::XMUINT2>(DirectX::XMUINT2 min, DirectX::XMUINT2 max, float factor) { return { lerp(min.x, max.x, factor), lerp(min.y, max.y, factor) }; }
	template<> inline DirectX::XMUINT3 lerp<DirectX::XMUINT3>(DirectX::XMUINT3 min, DirectX::XMUINT3 max, float factor) { return { lerp(min.x, max.x, factor), lerp(min.y, max.y, factor), lerp(min.z, max.z, factor) }; }
	template<> inline DirectX::XMUINT4 lerp<DirectX::XMUINT4>(DirectX::XMUINT4 min, DirectX::XMUINT4 max, float factor) { return { lerp(min.x, max.x, factor), lerp(min.y, max.y, factor), lerp(min.z, max.z, factor), lerp(min.w, max.w, factor) }; }

	DirectX::XMFLOAT2 bezier(DirectX::XMFLOAT2 s, DirectX::XMFLOAT2 c1, DirectX::XMFLOAT2 e, float value);
	DirectX::XMFLOAT3 bezier(DirectX::XMFLOAT3 s, DirectX::XMFLOAT3 c1, DirectX::XMFLOAT3 e, float value);
	DirectX::XMFLOAT4 slerp(DirectX::XMFLOAT4 s, DirectX::XMFLOAT4 e, float t);

	bool equal(DirectX::XMFLOAT2 a, DirectX::XMFLOAT2 b, float epsilon = FLT_EPSILON * 10);
	bool equal(DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b, float epsilon = FLT_EPSILON * 10);
	bool equal(DirectX::XMFLOAT4 a, DirectX::XMFLOAT4 b, float epsilon = FLT_EPSILON * 10);

	template<typename Type>
	inline Type smoothstep(Type min, Type max, Type value)
	{
		value = saturate((value - min) / (max - min));
		return (value * value * (static_cast<Type>(3) - (value + value)));
	}

	template<typename Type>
	inline Type linearstep(Type min, Type max, Type value)
	{
		return saturate((value - min) / (max - min));
	}

	float normalize_angle(float radians);

	float rand_range(float min, float max);

	namespace ease
	{
		float in_sine(float time);
		float out_sine(float time);
		float in_out_sine(float time);

		float in_quad(float time);
		float out_quad(float time);
		float in_out_quad(float time);

		float in_cubic(float time);
		float out_cubic(float time);
		float in_out_cubic(float time);

		float in_quart(float time);
		float out_quart(float time);
		float in_out_quart(float time);

		float in_quint(float time);
		float out_quint(float time);
		float in_out_quint(float time);

		float in_expo(float time);
		float out_expo(float time);
		float in_out_expo(float time);

		float in_circ(float time);
		float out_circ(float time);
		float in_out_circ(float time);

		float in_back(float time);
		float out_back(float time);
		float in_out_back(float time);

		float in_elastic(float time);
		float out_elastic(float time);
		float in_out_elastic(float time);

		float in_bounce(float time);
		float out_bounce(float time);
		float in_out_bounce(float time);

		enum class type
		{
			in_sine	= 0,
			out_sine,
			in_out_sine,
			in_quad,
			out_quad,
			in_out_quad,
			in_cubic,
			out_cubic,
			in_out_cubic,
			in_quart,
			out_quart,
			in_out_quart,
			in_quint,
			out_quint,
			in_out_quint,
			in_expo,
			out_expo,
			in_out_expo,
			in_circ,
			out_circ,
			in_out_circ,
			in_back,
			out_back,
			in_out_back,
			in_elastic,
			out_elastic,
			in_out_elastic,
			in_bounce,
			out_bounce,
			in_out_bounce,

			lerp,
		};

		float calculate(float time, type type);

	}
	//	∠ABCが鋭角か判定
	bool is_sharp_angle(const DirectX::XMVECTOR& vectorA, const DirectX::XMVECTOR& vectorB);
	bool is_sharp_angle(const DirectX::XMFLOAT3& pointA, const DirectX::XMFLOAT3& pointB, const DirectX::XMFLOAT3& pointC);

	DirectX::XMFLOAT3 convert_rotation_matrix_to_euler(DirectX::XMFLOAT4X4 rotation);
	DirectX::XMFLOAT3 convert_quaternion_to_euler(DirectX::XMFLOAT4 quaternion);
	DirectX::XMFLOAT3 calculate_right(DirectX::XMFLOAT4 quaternion);
	DirectX::XMFLOAT3 calculate_up(DirectX::XMFLOAT4 quaternion);
	DirectX::XMFLOAT3 calculate_front(DirectX::XMFLOAT4 quaternion);

	//	回転用ヘルパークラス
	class rotator
	{
	public:
		constexpr rotator() : x(0), y(0), z(0) {}
		constexpr rotator(float x, float y, float z) : x(x), y(y), z(z){}
		constexpr rotator(DirectX::XMFLOAT3 euler) : x(euler.x), y(euler.y), z(euler.z) {}
		rotator(DirectX::XMFLOAT4 quaternion);
		rotator(DirectX::XMFLOAT4X4 rotation);

		DirectX::XMFLOAT3 get_right() const;
		DirectX::XMFLOAT3 get_up() const;
		DirectX::XMFLOAT3 get_front() const;

		DirectX::XMFLOAT4 convert_quaternion() const;

		void set_parameter_from_direction(DirectX::XMFLOAT3 direction);

	public:
		operator DirectX::XMFLOAT4() const;
		operator DirectX::XMFLOAT3() const;

	public:
		float x;
		float y;
		float z;
	};

	//	簡易的なフェード係数計算用構造体
	namespace compute_fade
	{
		struct time_pair
		{
			float start;
			float end;
			math::ease::type ease_type = math::ease::type::lerp;
		};

		struct out_calculator
		{
			time_pair fade_out;
			out_calculator(time_pair fade_out);
			float compute(float time);
		};

		struct in_calculator
		{
			time_pair fade_in;
			in_calculator(time_pair fade_in);
			float compute(float time);
		};

		struct calculator
		{
			time_pair fade_in;
			time_pair fade_out;
			calculator(time_pair fade_out, time_pair fade_in);
			float compute(float time, bool* end_flag);
		};
	}	//	namespace compute_fade
}	//	namespace math



