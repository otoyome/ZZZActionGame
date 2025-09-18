#pragma once

#include "system_utility.h"

//	バッファビュー
struct buffer_view
{
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	std::vector<BYTE> row_data;
	size_t stride_in_bytes = 0;
	size_t size_in_bytes = 0;
	size_t offset_in_bytes = 0;
	size_t count() const
	{
		return size_in_bytes / stride_in_bytes;
	}
	size_t offset() const
	{
		return offset_in_bytes / stride_in_bytes;
	}
	buffer_view() = default;
	~buffer_view() {}

	template<class Archive>
	void serialize(Archive& archive, int version)
	{
		archive(CEREAL_NVP(format),
				CEREAL_NVP(row_data),
				CEREAL_NVP(stride_in_bytes),
				CEREAL_NVP(size_in_bytes),
				CEREAL_NVP(offset_in_bytes));
	}
};

//	識別子
struct semantic
{
	std::string name;
	UINT index;
	semantic() {}
	semantic(std::string_view name, UINT index) : name(name), index(index) {}
	bool operator==(const semantic& l) const
	{
		return name == l.name && index == l.index;
	}

	template<class Archive>
	void serialize(Archive& archive, int version)
	{
		archive(CEREAL_NVP(name),
				CEREAL_NVP(index));
	}

	std::size_t operator()(const semantic& item) const
	{
		std::size_t h1 = std::hash<std::string>()(item.name);
		std::size_t h2 = std::hash<UINT>()(item.index);
		return h1 ^ h2;
	}
};

//	描画バッファ情報
struct buffer_datas
{
	const std::unordered_map<semantic, buffer_view>* vertex_buffer_views;
	const buffer_view* index_buffer_view;
	std::vector<DirectX::XMFLOAT4X4> transform_matrices;
	UINT stride_transform_by_index;
	DirectX::XMFLOAT3 bounds_min, bounds_max;
};


namespace std
{
    template <>
    struct hash<semantic>
	{
		std::size_t operator()(const semantic& item) const
		{
			std::size_t h1 = std::hash<std::string>()(item.name);
			std::size_t h2 = std::hash<UINT>()(item.index);
			return h1 ^ h2;
		}
	};
}	//	namespace std
