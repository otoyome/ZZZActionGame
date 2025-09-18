#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "singleton.h"

enum class shader_load_type
{
	cso,
	file,

	max,
};

enum class shader_type
{
	vertex,
	hull,
	domain,
	geometry,
	pixel,
	compute,

	max,
};

class shader_base
{
public:
	shader_type get_type() const { return type; }

public:
	shader_base(shader_type type, shader_load_type load_type, std::string_view file_path);
	virtual ~shader_base(){}

	bool load();

protected:
	virtual bool load_from_cso(std::string_view file_path) = 0;
	virtual bool load_from_file(std::string_view file_path) = 0;

private:
	shader_type type;
	shader_load_type load_type;
	std::string file_path;
};

class vertex_shader : public shader_base
{
public:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> get_shader() const { return shader; }
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& get_input_element_desc() const { return input_element_desc; }
	Microsoft::WRL::ComPtr<ID3D11InputLayout> get_input_layout() const { return input_layout; }

public:
	vertex_shader(shader_load_type type, std::string_view file_path, std::vector<D3D11_INPUT_ELEMENT_DESC>& input_element_desc);
	~vertex_shader() override;

private:
	bool load_from_cso(std::string_view file_path) override;
	bool load_from_file(std::string_view file_path) override;

private:
	std::vector<D3D11_INPUT_ELEMENT_DESC> input_element_desc;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
};

class hull_shader : public shader_base
{
public:
	Microsoft::WRL::ComPtr<ID3D11HullShader> get_shader() const { return shader; }

public:
	hull_shader(shader_load_type type, std::string_view file_path);
	~hull_shader() override;

private:
	bool load_from_cso(std::string_view file_path) override;
	bool load_from_file(std::string_view file_path) override;

private:
	Microsoft::WRL::ComPtr<ID3D11HullShader> shader;
};

class domain_shader : public shader_base
{
public:
	Microsoft::WRL::ComPtr<ID3D11DomainShader> get_shader() const { return shader; }

public:
	domain_shader(shader_load_type type, std::string_view file_path);
	~domain_shader() override;

private:
	bool load_from_cso(std::string_view file_path) override;
	bool load_from_file(std::string_view file_path) override;

private:
	Microsoft::WRL::ComPtr<ID3D11DomainShader> shader;
};

class geometry_shader : public shader_base
{
public:
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> get_shader() const { return shader; }

public:
	geometry_shader(shader_load_type type, std::string_view file_path);
	~geometry_shader() override;

private:
	bool load_from_cso(std::string_view file_path) override;
	bool load_from_file(std::string_view file_path) override;

private:
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> shader;
};

class pixel_shader : public shader_base
{
public:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> get_shader() const { return shader; }

public:
	pixel_shader(shader_load_type type, std::string_view file_path);
	~pixel_shader() override;

private:
	bool load_from_cso(std::string_view file_path) override;
	bool load_from_file(std::string_view file_path) override;

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
};

class compute_shader : public shader_base
{
public:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> get_shader() const { return shader; }

public:
	compute_shader(shader_load_type type, std::string_view file_path);
	~compute_shader() override;

private:
	bool load_from_cso(std::string_view file_path) override;
	bool load_from_file(std::string_view file_path) override;

private:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> shader;
};

using shader_ptr = std::shared_ptr<shader_base>;

struct load_shader_cso
{
	std::string_view file_path;
	std::vector<D3D11_INPUT_ELEMENT_DESC> input_element_desc;
};

class shader_manager
{
	using shader_map = std::unordered_map<std::string, std::weak_ptr<shader_base>>;
	SINGLETON(shader_manager)

	template<class type>
	shader_type	load_shader_type(){ return shader_type::max; }
	template<>	shader_type	load_shader_type<vertex_shader>(){ return shader_type::vertex; }
	template<>	shader_type	load_shader_type<hull_shader>(){ return shader_type::hull; }
	template<>	shader_type	load_shader_type<domain_shader>(){ return shader_type::domain; }
	template<>	shader_type	load_shader_type<geometry_shader>(){ return shader_type::geometry; }
	template<>	shader_type	load_shader_type<pixel_shader>(){ return shader_type::pixel; }
	template<>	shader_type	load_shader_type<compute_shader>(){ return shader_type::compute; }

private:
	shader_manager(){}
	~shader_manager(){}

public:
	template<class type>
	bool	load_shader(std::shared_ptr<type>& out_shader, load_shader_cso data)
	{
		shader_ptr shader;
		if (load_shader(load_shader_type<type>(), shader, data))
		{
			out_shader	= std::static_pointer_cast<type>(shader);
			return	true;
		}
		return	false;
	}

	shader_ptr	search(std::string_view name);

private:
	bool	load_shader(shader_type type, shader_ptr& out_shader, load_shader_cso data);

private:
	shader_map	shaderes;
};
