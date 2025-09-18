#pragma once

#include "texture.h"

#include "system_utility.h"

namespace postprocess
{
	//	色調補正情報
	struct color_grading_part_data
	{
		DirectX::XMFLOAT3 hsv_weight = { 1, 1, 1 }; // HSV色空間での調整係数
		float dummy;
		DirectX::XMFLOAT3 color_multiplyer = { 1, 1, 1 }; // 乗算値
		float dummy2;
	};

	struct color_grading_data
	{
		float intensity = 0.0f; // 色調補正適応量
		float stage_depth_scale = 10.0f; // ステージ用の深度による適応にスケール
		DirectX::XMFLOAT2 dummy;

		//  キャラクター用
		color_grading_part_data character_data;

		//  ステージ用
		color_grading_part_data stage_data;
	};

	//	ラジアルブラー情報
	struct radial_blur_datas
	{
		float radius = 0.0f;
		int sampling_count = 10;
		DirectX::XMFLOAT2 center = { 0.5f, 0.5f };

		float mask_radius = 0.0f;
		DirectX::XMFLOAT3 dummy;
	};

	//	色の分割処理情報
	struct separate_color_datas
	{
		float intensity = 0.0f; // 色調補正適応量
		DirectX::XMFLOAT3 color_min; // 閾値以下の場合の色
		float threshold; // 色閾値
		DirectX::XMFLOAT3 color_max; // 閾値以上の場合の色
	};

	//	文字描画処理
	struct overwrite_text_posterize_color
	{
		float step = 1.0f;
		float animation_rate = 0.0f;
		float amount = 0.0f;
		float dummy;
	};
	struct overwrite_text_datas
	{
		float intensity = 0.0f; // 色調補正適応量
		float texcoord_scale = 1.0f; // 文字拡大値
		DirectX::XMFLOAT2 texcoord_offset = {0.0f, 0.0f}; // 文字移動
		DirectX::XMFLOAT4 color = {1.0f, 1.0f, 1.0f, 1.0f}; // 基本色

		overwrite_text_posterize_color posterize_data; // ポスタライズ情報
	};

	//	集中線処理
	struct concentrated_line_work_datas
	{
		float intensity = 0.0f; // 色調補正適応量
		float use_animation = 1.0f; // アニメーション使用フラグ
		float animation_speed = 10.0f; // アニメーション速度
		float pattern_seed = 10.0f; // パターンシード値

		float noise_scale = 500.0f; // ノイズ拡大値
		DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f); // 集中線色
		DirectX::XMFLOAT2 edge = DirectX::XMFLOAT2(0.5f, 1.0f);
		DirectX::XMFLOAT2 dummy;
	};

	//  揺らぎ情報
	struct distortion_datas
	{
		float power = 0.0f;
		float speed = 0.0f;
		float amount = 0.0f;
		float dummy = 0.0f;
	};

	//  sin波による揺らぎ情報
	struct sin_distortion_data
	{
		float height_amplitude = 0.0f;
		float height_angular_frequency = 0.0f;
		int height_postraize_count = 0;
		float amount = 0.0f;
	};

	//  ボロノイノイズによる揺らぎ情報
	struct voronoi_distortion_data
	{
		int square_count = 0;
		float amount = 0.0f;
		DirectX::XMFLOAT2 dummy;
	};
}

//	ポストプロセス情報
struct post_process_datas
{
	postprocess::color_grading_data color_grading;
	postprocess::radial_blur_datas radial_blur;
	postprocess::separate_color_datas separate_color;

	postprocess::overwrite_text_datas overwrite_text;
	texture_ptr overwrite_text_texture;

	postprocess::concentrated_line_work_datas concentrated_line_work;
	postprocess::distortion_datas distortion_data;
	postprocess::sin_distortion_data sin_distortion_datas[2];

	postprocess::voronoi_distortion_data voronoi_distortion_data;
};

//	ポストプロセスボリューム情報
class post_process_volume
{
public:
	enum class flags
	{
		active,	//	アクティブ状態かどうか
		unbind,	//	範囲制限をかけるかどうか(かけない場合は全画面)
	};

public:
	post_process_volume();
	~post_process_volume();

	bool is_flag(flags flag) const { return system_utility::is_bitflag(system_flags, 1 << static_cast<UINT>(flag)); }
	void set_flag(flags flag, bool value){ system_utility::set_bitflag(system_flags, 1 << static_cast<UINT>(flag), value); }

	DirectX::XMFLOAT3 get_position() const;
	void set_position(const DirectX::XMFLOAT3& position);
	DirectX::XMFLOAT3 get_inner_range() const;
	void set_inner_range(const DirectX::XMFLOAT3& inner_range);
	DirectX::XMFLOAT3 get_outer_range() const;
	void set_outer_range(const DirectX::XMFLOAT3& outer_range);
	const DirectX::XMFLOAT4& get_color() const;
	void set_color(const DirectX::XMFLOAT4& color);

	post_process_datas get_post_process_data() const;
	void set_post_process_data(post_process_datas& post_process_data);

private:
	UINT system_flags;
	post_process_datas post_process_data;

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 inner_range;
	DirectX::XMFLOAT3 outer_range;
	DirectX::XMFLOAT4 color;
};
ENUM_ATTR_BITFLAG(post_process_volume::flags);

using post_process_volume_ptr = std::shared_ptr<post_process_volume>;

//	ポストプロセスボリュームコンテナ
struct post_process_volume_container
{
	std::vector<post_process_volume_ptr> post_process_volumes;
};

//	ポストプロセスボリューム管理クラス
class post_process_volume_manager
{
	SINGLETON(post_process_volume_manager);

private:
	post_process_volume_manager();
	~post_process_volume_manager();

public:
	bool generate_post_process_volume(post_process_volume_ptr& out_post_process_volume);
	void get_container(post_process_volume_container& container);

private:
	std::vector<std::weak_ptr<post_process_volume>> post_process_volumes;
};
