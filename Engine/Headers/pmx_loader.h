#pragma	once

#include	"pmx_definision.h"

class	pmx_loader
{
public:
	inline	std::string_view	get_name() const { return	name; }
	inline	const	pmx_index_data& get_index_data() const { return	index_data; }
	inline	const	std::vector<pmx_vertex>&	get_vertices() const { return	vertices; }
	inline	const	std::vector<unsigned int>&	get_indices() const { return	indices; }
	inline	const	std::vector<std::string>&	get_textures() const { return	textures; }
	inline	const	std::vector<pmx_material>&	get_materials() const { return	materials; }
	inline	const	std::vector<pmx_bone>&	get_bones() const { return	bones; }
	inline	const	std::vector<pmx_morph>&	get_morphs() const { return	morphs; }
	inline	const	pmx_error_code&	get_error_code() const { return	error_code; }

public:
	pmx_loader();
	~pmx_loader();

	bool	load(void* buffer, size_t buffer_size, std::string_view folder_path);

private:
	void set_error_code(pmx_error_code error_code);
	unsigned int read_index(unsigned char size);
	void read_string(char* string, size_t string_length);

	template<class	Type>
	Type* read_buffer()
	{
		Type*	pRet	= reinterpret_cast<Type*>(data_buffer + incriment);
		incriment += sizeof(Type);
		assert(incriment <= data_buffer_size);
		return	pRet;
	}

	template<class	Type>
	Type* read_buffer(size_t _uSize)
	{
		Type* pRet = reinterpret_cast<Type*>(data_buffer + incriment);
		incriment += _uSize;
		assert(incriment <= data_buffer_size);
		return	pRet;
	}

private:
	pmx_error_code				error_code;
	int							version;

	char*						data_buffer;
	size_t						data_buffer_size;
	size_t						incriment;

	pmx_index_data				index_data;
	std::string					name;
	std::vector<pmx_vertex>		vertices;
	std::vector<unsigned int>	indices;
	std::vector<std::string>	textures;
	std::vector<pmx_material>	materials;
	std::vector<pmx_bone>		bones;
	std::vector<pmx_morph>		morphs;
};
