#include "font_data.h"

const std::vector<texture_ptr>& font_data::get_textures() const
{
	return textures;
}

const std::vector<font_data::character_info>& font_data::get_character_infos() const
{
	return character_infos;
}

const std::vector<WORD>& font_data::get_character_indices() const
{
	return character_indices;
}

font_data::font_data(std::string_view file_name)
: font_width(0)
, font_height(0)
, texture_count(0)
, textures()
, character_infos()
, character_indices()
{
	// .fnt 読み込み
	{
		FILE* fp = nullptr;
		fopen_s(&fp, file_name.data(), "rb");
		_ASSERT_EXPR_A(fp, "FNT File not found");

		fseek(fp, 0, SEEK_END);
		long fntSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		std::unique_ptr<char[]> fntData = std::make_unique<char[]>(fntSize);
		fread(fntData.get(), fntSize, 1, fp);
		fclose(fp);

		// (***.fnt)ヘッダ解析
		char* context = nullptr;	// 内部的に使用するので深く考えない
		const char* delimiter = " ,=\"\r\n";
		const char* delimiter2 = ",=\"\r\n";
		char* pToken = ::strtok_s(fntData.get(), delimiter, &context);
		_ASSERT_EXPR_A(::strcmp(pToken, "info") == 0, "FNT Format");

		// face
		::strtok_s(nullptr, delimiter, &context);
		const char* face = ::strtok_s(nullptr, delimiter2, &context);
		// size
		::strtok_s(nullptr, delimiter, &context);
		int size = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// bold
		::strtok_s(nullptr, delimiter, &context);
		int bold = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// italic
		::strtok_s(nullptr, delimiter, &context);
		int italic = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// charset
		::strtok_s(nullptr, delimiter, &context);
		const char* charset = ::strtok_s(nullptr, delimiter, &context);
		// unicode
		if (::strcmp(charset, "unicode") == 0)
			charset = "";
		else
			::strtok_s(nullptr, delimiter, &context);
		int unicode = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// stretchH
		::strtok_s(nullptr, delimiter, &context);
		int stretchH = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// smooth
		::strtok_s(nullptr, delimiter, &context);
		int smooth = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// aa
		::strtok_s(nullptr, delimiter, &context);
		int aa = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// padding
		::strtok_s(nullptr, delimiter, &context);
		int padding1 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		int padding2 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		int padding3 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		int padding4 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// spacing
		::strtok_s(nullptr, delimiter, &context);
		int spacing1 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		int spacing2 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// outline
		::strtok_s(nullptr, delimiter, &context);
		int outline = ::atoi(::strtok_s(nullptr, delimiter, &context));

		// common
		::strtok_s(nullptr, delimiter, &context);
		// lineHeight
		::strtok_s(nullptr, delimiter, &context);
		int lineHeight = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// base
		::strtok_s(nullptr, delimiter, &context);
		int base = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// scaleW
		::strtok_s(nullptr, delimiter, &context);
		int scaleW = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// scaleH
		::strtok_s(nullptr, delimiter, &context);
		int scaleH = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// pages
		::strtok_s(nullptr, delimiter, &context);
		int pages = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// packed
		::strtok_s(nullptr, delimiter, &context);
		int packed = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// alphaChnl
		::strtok_s(nullptr, delimiter, &context);
		int alphaChnl = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// redChnl
		::strtok_s(nullptr, delimiter, &context);
		int redChnl = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// greenChnl
		::strtok_s(nullptr, delimiter, &context);
		int greenChnl = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// blueChnl
		::strtok_s(nullptr, delimiter, &context);
		int blueChnl = ::atoi(::strtok_s(nullptr, delimiter, &context));

		// ディレクトリパス取得
		char dirname[256];
		::_splitpath_s(file_name.data(), nullptr, 0, dirname, 256, nullptr, 0, nullptr, 0);
		textures.resize(pages);
		for (int i = 0; i < pages; i++)
		{
			// page
			::strtok_s(nullptr, delimiter, &context);
			// id
			::strtok_s(nullptr, delimiter, &context);
			int id = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// file
			::strtok_s(nullptr, delimiter, &context);
			const char* file = ::strtok_s(nullptr, delimiter2, &context);

			// 相対パスの解決
			char fname[256];
			::_makepath_s(fname, 256, nullptr, dirname, file, nullptr);

			// テクスチャ読み込み
			texture_manager::instance()->load_texture_from_file(textures[i], fname);
		}

		// chars
		::strtok_s(nullptr, delimiter, &context);
		// count
		::strtok_s(nullptr, delimiter, &context);
		int count = ::atoi(::strtok_s(nullptr, delimiter, &context));

		// データ格納
		font_width = static_cast<float>(size);
		font_height = static_cast<float>(lineHeight);
		texture_count = pages;
		character_count = count + 1;
		character_infos.resize(character_count);
		character_indices.resize(0xFFFF);
		::memset(character_indices.data(), 0, sizeof(WORD) * character_indices.size());

		character_indices.at(0x00) = EndCode;
		character_indices.at(0x0a) = ReturnCode;
		character_indices.at(0x09) = TabCode;
		character_indices.at(0x20) = SpaceCode;

		// 文字情報解析
		int n = 1;
		for (int i = 0; i < count; i++)
		{
			// char
			::strtok_s(nullptr, delimiter, &context);
			// id
			::strtok_s(nullptr, delimiter, &context);
			int id = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// x
			::strtok_s(nullptr, delimiter, &context);
			int x = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// y
			::strtok_s(nullptr, delimiter, &context);
			int y = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// width
			::strtok_s(nullptr, delimiter, &context);
			int width = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// height
			::strtok_s(nullptr, delimiter, &context);
			int height = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// xoffset
			::strtok_s(nullptr, delimiter, &context);
			int xoffset = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// yoffset
			::strtok_s(nullptr, delimiter, &context);
			int yoffset = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// xadvance
			::strtok_s(nullptr, delimiter, &context);
			int xadvance = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// page
			::strtok_s(nullptr, delimiter, &context);
			int page = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// chnl
			::strtok_s(nullptr, delimiter, &context);
			int chnl = ::atoi(::strtok_s(nullptr, delimiter, &context));

			// 文字コード格納
			if (id >= 0x10000) continue;

			character_info& info = character_infos.at(n);

			character_indices.at(id) = static_cast<WORD>(i + 1);

			// 文字情報格納
			info.left = static_cast<float>(x) / static_cast<float>(scaleW);
			info.top = static_cast<float>(y) / static_cast<float>(scaleH);
			info.right = static_cast<float>(x + width) / static_cast<float>(scaleW);
			info.bottom = static_cast<float>(y + height) / static_cast<float>(scaleH);
			info.xoffset = static_cast<float>(xoffset);
			info.yoffset = static_cast<float>(yoffset);
			info.xadvance = static_cast<float>(xadvance);
			info.width = static_cast<float>(width);
			info.height = static_cast<float>(height);
			info.page = page;
			info.ascii = (id < 0x100);

			n++;
		}
	}
}

font_data::~font_data()
{
}

bool font_data_manager::load_from_file(font_data_ptr& out_font_data, std::string_view file_path)
{
	auto	key_name	= std::string(file_path);
	auto	it	= fonts.find(key_name);
	if (it != fonts.end()){
		auto ptr = it->second.lock();
		if(ptr){
			out_font_data = ptr;
			return	true;
		}
		fonts.erase(it);
	}

	out_font_data = std::make_shared<font_data>(file_path);
	fonts.insert_or_assign(key_name, out_font_data);
	return true;
}

font_data_ptr	font_data_manager::search(std::string_view file_path)
{
	auto	key_name = std::string(file_path);
	auto	it = fonts.find(key_name);
	if (it != fonts.end()) {
		auto ptr = it->second.lock();
		if (ptr) {
			return ptr;
		}
		fonts.erase(it);
	}
	return nullptr;
}

