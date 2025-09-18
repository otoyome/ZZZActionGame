#pragma once

#include <directxmath.h>
#include <string>
#include <string_view>

class system_units
{
public:
	//	距離において、1.0f をどういう単位で取り扱うかを決める定数
	enum class length_units
	{
		micro_meter,
		milli_meter,
		centi_meter,
		meter,

		unit,	//	1.0unit = 10cm
	};

	//	1秒間に何回更新するか
	static constexpr int frame_per_sec = 60;

	static constexpr std::string_view length_unit_text(length_units unit)
	{
		switch (unit)
		{
			case length_units::micro_meter:	return	"um";
			case length_units::milli_meter:	return	"mm";
			case length_units::centi_meter:	return	"cm";
			case length_units::meter:		return	"m";
			case length_units::unit:		return	"unit";
		}
		return	"unknwon";
	}

private:
	static constexpr float length_factor(length_units origin, length_units dest)
	{
		switch (origin)
		{
			case length_units::micro_meter:
			{
				switch (dest)
				{
					case length_units::milli_meter:	return 0.001f;
					case length_units::centi_meter:	return 0.0001f;
					case length_units::meter:		return 0.0000001f;
					case length_units::unit:		return 0.001f;
				}
				break;
			}
			case length_units::milli_meter:
			{
				switch (dest)
				{
					case length_units::micro_meter:	return 1000.0f;
					case length_units::centi_meter:	return 0.1f;
					case length_units::meter:		return 0.001f;
					case length_units::unit:		return 0.01f;
				}
				break;
			}
			case length_units::centi_meter:
			{
				switch (dest)
				{
					case length_units::micro_meter:	return 10000.0f;
					case length_units::milli_meter:	return 10.0f;
					case length_units::meter:		return 0.01f;
					case length_units::unit:		return 0.1f;
				}
				break;
			}
			case length_units::meter:
			{
				switch (dest)
				{
					case length_units::micro_meter:	return 1000000.0f;
					case length_units::milli_meter:	return 1000.0f;
					case length_units::centi_meter:	return 100.0f;
					case length_units::unit:		return 10.0f;
				}
				break;
			}
		}
		return 1.0f;
	}

public:
	//	マイクロメートル <> ミリメートル変換
	static constexpr float convert_um_to_mm(float value) { return value * length_factor(length_units::micro_meter, length_units::milli_meter); }
	static constexpr float convert_mm_to_um(float value) { return value * length_factor(length_units::milli_meter, length_units::micro_meter); }

	//	ミリメートル <> センチメートル変換
	static constexpr float convert_mm_to_cm(float value) { return value * length_factor(length_units::milli_meter, length_units::centi_meter); }
	static constexpr float convert_cm_to_mm(float value) { return value * length_factor(length_units::centi_meter, length_units::milli_meter); }

	//	メートル <> センチメートル変換
	static constexpr float convert_cm_to_m(float value){ return value * length_factor(length_units::centi_meter, length_units::meter); }
	static constexpr float convert_m_to_cm(float value){ return value * length_factor(length_units::meter, length_units::centi_meter); }

	//	ミリメートル <> メートル変換
	static constexpr float convert_mm_to_m(float value) { return value * length_factor(length_units::milli_meter, length_units::meter); }
	static constexpr float convert_m_to_mm(float value) { return value * length_factor(length_units::meter, length_units::milli_meter); }

	//	各単位系での距離をゲーム単位へ変換
	static constexpr float convert_um_to_unit(float value) { return value * length_factor(length_units::micro_meter, length_units::unit); }
	static constexpr float convert_mm_to_unit(float value) { return value * length_factor(length_units::milli_meter, length_units::unit); }
	static constexpr float convert_cm_to_unit(float value) { return value * length_factor(length_units::centi_meter, length_units::unit); }
	static constexpr float convert_m_to_unit(float value) { return value * length_factor(length_units::meter, length_units::unit); }
	static constexpr float convert_unit_to_um(float value) { return value * length_factor(length_units::unit, length_units::micro_meter); }
	static constexpr float convert_unit_to_mm(float value) { return value * length_factor(length_units::unit, length_units::milli_meter); }
	static constexpr float convert_unit_to_cm(float value) { return value * length_factor(length_units::unit, length_units::centi_meter); }
	static constexpr float convert_unit_to_m(float value) { return value * length_factor(length_units::unit, length_units::meter); }

	static constexpr DirectX::XMFLOAT2 convert_m_to_unit(DirectX::XMFLOAT2 value)
	{
		return {convert_m_to_unit(value.x), convert_m_to_unit(value.y)};
	}
	static constexpr DirectX::XMFLOAT3 convert_m_to_unit(DirectX::XMFLOAT3 value)
	{
		return {convert_m_to_unit(value.x), convert_m_to_unit(value.y), convert_m_to_unit(value.z)};
	}
	static constexpr DirectX::XMFLOAT4 convert_m_to_unit(DirectX::XMFLOAT4 value)
	{
		return {convert_m_to_unit(value.x), convert_m_to_unit(value.y), convert_m_to_unit(value.z), convert_m_to_unit(value.w)};
	}

public:
	//	秒 <> ミリ秒変換
	static constexpr float convert_sec_to_millisec(float value) { return value * 1000.0f; }
	static constexpr float convert_millisec_to_sec(float value) { return value / 1000.0f; }
	//	秒 <> 分変換
	static constexpr float convert_sec_to_minute(float value) { return value * 60.0f; }
	static constexpr float convert_minute_to_sec(float value) { return value / 60.0f; }
	//	秒 <> 時変換
	static constexpr float convert_sec_to_hour(float value) { return value * 360.0f; }
	static constexpr float convert_hour_to_sec(float value) { return value / 360.0f; }

	//	フレーム <> 秒
	static constexpr float convert_sec_to_fps(float value) { return value * frame_per_sec; }
	static constexpr float convert_fps_to_sec(float value) { return value / frame_per_sec; }
};
