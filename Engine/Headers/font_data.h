#pragma once

#include "texture.h"

//	フォント情報
class font_data
{
public:
	static constexpr WORD NonCode = 0; // コードなし
	static constexpr WORD EndCode = 0xFFFF; // 終了コード
	static constexpr WORD ReturnCode = 0xFFFE; // 改行コード
	static constexpr WORD TabCode = 0xFFFD; // タブコード
	static constexpr WORD SpaceCode = 0xFFFC; // 空白コード

	struct character_info
	{
		float left; // テクスチャ左座標
		float top; // テクスチャ上座標
		float right; // テクスチャ右座標
		float bottom; // テクスチャ下座標
		float xoffset; // オフセットX
		float yoffset; // オフセットY
		float xadvance; // プロポーショナル幅
		float width; // 画像幅
		float height; // 画像高さ
		int page; // テクスチャ番号
		bool ascii; // アスキー文字
	};

public:
	float get_font_width() const { return font_width; }
	float get_font_height() const { return font_height; }
	int get_texture_count() const { return texture_count; }
	int get_character_count() const { return character_count; }
	const std::vector<texture_ptr>& get_textures() const;
	const std::vector<character_info>& get_character_infos() const;
	const std::vector<WORD>& get_character_indices() const;

public:
	font_data(std::string_view file_name);
	~font_data();

private:
	float font_width;
	float font_height;
	int texture_count;
	int character_count;
	std::vector<texture_ptr> textures;
	std::vector<character_info> character_infos;
	std::vector<WORD> character_indices;
};

using font_data_ptr = std::shared_ptr<font_data>;

class font_data_manager
{
	SINGLETON(font_data_manager)

	using font_data_map = std::unordered_map<std::string, std::weak_ptr<font_data>>;

public:
	bool load_from_file(font_data_ptr& out_font_data, std::string_view file_path);

	font_data_ptr	search(std::string_view name);

private:
	font_data_map fonts;
};
