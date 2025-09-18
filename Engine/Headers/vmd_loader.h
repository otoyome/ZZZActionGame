#pragma	once

#include	"vmd_definision.h"

class	vmd_loader
{
public:
	inline	std::string_view	get_create_model_name() const { return	model_name; };
	inline	const	unsigned int&	get_end_frame() const { return	end_frame; };
	inline	const	std::vector<vmd_bone_frame>&	get_bone() const { return	bone_frame_list; };
	inline	const	std::vector<vmd_morph_frame>&	get_face() const { return	morph_frame_list; };
	inline	const	std::vector<vmd_camera_frame>&	get_camera() const { return	camera_frame_list; };
	inline	const	vmd_error_code&	get_error_code() const { return	error_code; };


public:
	vmd_loader();
	~vmd_loader();

	bool	load(void* buffer, unsigned int buffer_size);

private:
	void			set_error_code( vmd_error_code error_code );

	template<class	Type>
	Type* read_buffer()
	{
		Type* ret = reinterpret_cast<Type*>(data_buffer + incriment);
		incriment += sizeof(Type);
		assert(incriment <= data_buffer_size);
		return	ret;
	}
	template<class	Type>
	Type* read_buffer(size_t size)
	{
		Type* ret = reinterpret_cast<Type*>(data_buffer + incriment);
		incriment += size;
		assert(incriment <= data_buffer_size);
		return	ret;
	}

private:
	vmd_error_code					error_code;
	int								version;

	char*							data_buffer;
	size_t							data_buffer_size;
	size_t							incriment;

	unsigned int					end_frame;
	std::string						model_name;

	//	ƒf[ƒ^
	std::vector<vmd_bone_frame>		bone_frame_list;
	std::vector<vmd_morph_frame>	morph_frame_list;
	std::vector<vmd_camera_frame>	camera_frame_list;
};
