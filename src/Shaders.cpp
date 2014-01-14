#include <vector>

#include "asserts.hpp"
#include "Shaders.hpp"

namespace Shader
{
	Shader::Shader(GLenum type, const std::string& name, const std::string& code)
		: type_(type), shader_(0), name_(name)
	{
		bool compiled_ok = Compile(code);
		ASSERT_LOG(compiled_ok == true, "Error compiling shader for " << name_);
	}

	bool Shader::Compile(const std::string& code)
	{
		GLint compiled;
		if(shader_) {
			glDeleteShader(shader_);
			shader_ = 0;
		}

		ASSERT_LOG(glCreateShader != NULL, "Something bad happened with Glew shader not initialised.");
		shader_ = glCreateShader(type_);
		if(shader_ == 0) {
			std::cerr << "Enable to create shader." << std::endl;
			return false;
		}
		const char* shader_code = code.c_str();
		glShaderSource(shader_, 1, &shader_code, NULL);
		glCompileShader(shader_);
		glGetShaderiv(shader_, GL_COMPILE_STATUS, &compiled);
		if(!compiled) {
			GLint info_len = 0;
			glGetShaderiv(shader_, GL_INFO_LOG_LENGTH, &info_len);
			if(info_len > 1) {
				std::vector<char> info_log;
				info_log.resize(info_len);
				glGetShaderInfoLog(shader_, info_log.capacity(), NULL, &info_log[0]);
				std::string s(info_log.begin(), info_log.end());
				std::cerr << "Error compiling shader: " << s << std::endl;
			}
			glDeleteShader(shader_);
			shader_ = 0;
			return false;
		}
		return true;
	}

	ShaderProgram::ShaderProgram()
	{
	}

	ShaderProgram::ShaderProgram(const std::string& name, const ShaderDef& vs, const ShaderDef& fs)
	{
		Init(name, vs, fs);
	}

	void ShaderProgram::Init(const std::string& name, const ShaderDef& vs, const ShaderDef& fs)
	{
		name_ = name;
		vs_.reset(new Shader(GL_VERTEX_SHADER, vs.first, vs.second));
		fs_.reset(new Shader(GL_FRAGMENT_SHADER, fs.first, fs.second));
		bool linked_ok = Link();
		ASSERT_LOG(linked_ok == true, "Error linking program: " << name_);
	}

	GLuint ShaderProgram::GetAttribute(const std::string& attr) const
	{
		auto it = attribs_.find(attr);
		ASSERT_LOG(it != attribs_.end(), "Attribute \"" << attr << "\" not found in list.");
		return it->second.location;
	}

	GLuint ShaderProgram::GetUniform(const std::string& attr) const
	{
		auto it = uniforms_.find(attr);
		ASSERT_LOG(it != uniforms_.end(), "Uniform \"" << attr << "\" not found in list.");
		return it->second.location;
	}

	ConstActivesMapIterator ShaderProgram::GetAttributeIterator(const std::string& attr) const
	{
		auto it = attribs_.find(attr);
		ASSERT_LOG(it != attribs_.end(), "Attribute \"" << attr << "\" not found in list.");
		return it;
	}

	ConstActivesMapIterator ShaderProgram::GetUniformIterator(const std::string& attr) const
	{
		auto it = uniforms_.find(attr);
		ASSERT_LOG(it != uniforms_.end(), "Uniform \"" << attr << "\" not found in list.");
		return it;
	}

	bool ShaderProgram::Link()
	{
		if(object_) {
			glDeleteProgram(object_);
			object_ = 0;
		}
		object_ = glCreateProgram();
		ASSERT_LOG(object_ != 0, "Unable to create program object.");
		glAttachShader(object_, vs_->Get());
		glAttachShader(object_, fs_->Get());
		glLinkProgram(object_);
		GLint linked = 0;
		glGetProgramiv(object_, GL_LINK_STATUS, &linked);
		if(!linked) {
			GLint info_len = 0;
			glGetProgramiv(object_, GL_INFO_LOG_LENGTH, &info_len);
			if(info_len > 1) {
				std::vector<char> info_log;
				info_log.resize(info_len);
				glGetProgramInfoLog(object_, info_log.capacity(), NULL, &info_log[0]);
				std::string s(info_log.begin(), info_log.end());
				std::cerr << "Error linking object: " << s << std::endl;
			}
			glDeleteProgram(object_);
			object_ = 0;
			return false;
		}
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		return QueryUniforms() && QueryAttributes();
	}

	bool ShaderProgram::QueryUniforms()
	{
		GLint active_uniforms;
		glGetProgramiv(object_, GL_ACTIVE_UNIFORMS, &active_uniforms);
		GLint uniform_max_len;
		glGetProgramiv(object_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_max_len);
		std::vector<char> name;
		name.resize(uniform_max_len+1);
		for(int i = 0; i < active_uniforms; i++) {
			Actives u;
			GLsizei size;
			glGetActiveUniform(object_, i, name.size(), &size, &u.num_elements, &u.type, &name[0]);
			u.name = std::string(&name[0], &name[size]);
			u.location = glGetUniformLocation(object_, u.name.c_str());
			ASSERT_LOG(u.location >= 0, "Unable to determine the location of the uniform: " << u.name);
			uniforms_[u.name] = u;
		}
		return true;
	}

	bool ShaderProgram::QueryAttributes()
	{
		GLint active_attribs;
		glGetProgramiv(object_, GL_ACTIVE_ATTRIBUTES, &active_attribs);
		GLint attributes_max_len;
		glGetProgramiv(object_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributes_max_len);
		std::vector<char> name;
		name.resize(attributes_max_len+1);
		for(int i = 0; i < active_attribs; i++) {
			Actives a;
			GLsizei size;
			glGetActiveAttrib(object_, i, name.size(), &size, &a.num_elements, &a.type, &name[0]);
			a.name = std::string(&name[0], &name[size]);
			a.location = glGetAttribLocation(object_, a.name.c_str());
			ASSERT_LOG(a.location >= 0, "Unable to determine the location of the attribute: " << a.name);
			attribs_[a.name] = a;
		}
		return true;
	}

	void ShaderProgram::MakeActive()
	{
		glUseProgram(object_);
	}

	void ShaderProgram::SetUniform(ConstActivesMapIterator it, const GLint* value)
	{
		const Actives& u = it->second;
		ASSERT_LOG(value != NULL, "set_uniform(): value is NULL");
		switch(u.type) {
		case GL_INT:
		case GL_BOOL:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:	
			glUniform1i(u.location, *value); 
			break;
		case GL_INT_VEC2:	
		case GL_BOOL_VEC2:	
			glUniform2i(u.location, value[0], value[1]); 
			break;
		case GL_INT_VEC3:	
		case GL_BOOL_VEC3:	
			glUniform3iv(u.location, u.num_elements, value); 
			break;
		case GL_INT_VEC4: 	
		case GL_BOOL_VEC4:
			glUniform4iv(u.location, u.num_elements, value); 
			break;
		default:
			ASSERT_LOG(false, "Unhandled uniform type: " << it->second.type);
		}
	}

	void ShaderProgram::SetUniform(ConstActivesMapIterator it, const GLfloat* value)
	{
		const Actives& u = it->second;
		ASSERT_LOG(value != NULL, "set_uniform(): value is NULL");
		switch(u.type) {
		case GL_FLOAT: {
			glUniform1f(u.location, *value);
			break;
		}
		case GL_FLOAT_VEC2: {
			glUniform2fv(u.location, u.num_elements, value);
			break;
		}
		case GL_FLOAT_VEC3: {
			glUniform3fv(u.location, u.num_elements, value);
			break;
		}
		case GL_FLOAT_VEC4: {
			glUniform4fv(u.location, u.num_elements, value);
			break;
		}
		case GL_FLOAT_MAT2:	{
			glUniformMatrix2fv(u.location, u.num_elements, GL_FALSE, value);
			break;
		}
		case GL_FLOAT_MAT3: {
			glUniformMatrix3fv(u.location, u.num_elements, GL_FALSE, value);
			break;
		}
		case GL_FLOAT_MAT4: {
			glUniformMatrix4fv(u.location, u.num_elements, GL_FALSE, value);
			break;
		}
		default:
			ASSERT_LOG(false, "Unhandled uniform type: " << it->second.type);
		}	
	}
}
