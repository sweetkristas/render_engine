#include <vector>

#include "asserts.hpp"
#include "logger.hpp"
#include "Shaders.hpp"

namespace Shader
{
	namespace
	{
		const char* const default_vs = 
			"uniform mat4 u_mvp_matrix;\n"
			"attribute vec2 a_position;\n"
			"attribute vec4 a_color;\n"
			"attribute vec2 a_texcoord;\n"
			"uniform bool u_use_attrib_color;\n"
			"varying vec2 v_texcoord;\n"
			"varying vec4 v_color;\n"
			"void main()\n"
			"{\n"
			"    if(u_use_attrib_color) {\n"
			"        v_color = a_color;\n"
			"    }\n"
			"    v_texcoord = a_texcoord;\n"
			"    gl_Position = u_mvp_matrix * vec4(a_position,0.0,1.0);\n"
			"}\n";
		const char* const default_fs =
			"uniform sampler2D u_tex_map;\n"
			"varying vec4 v_color;\n"
			"varying vec2 v_texcoord;\n"
			"uniform bool u_use_attrib_color;\n"
			"uniform bool u_discard;\n"
			"uniform vec4 u_color;\n"
			"void main()\n"
			"{\n"
			"    vec4 color = texture2D(u_tex_map, v_texcoord);"
			"    if(u_use_attrib_color) {\n"
			"        color *= v_color;\n"
			"    }\n"
			"    if(u_discard && color[3] == 0.0) {\n"
			"        discard;\n"
			"    } else {\n"
			"        gl_FragColor = color * u_color;\n"
			"    }\n"
			"}\n";

		const struct { const char* alt_name; const char* name; } default_uniform_mapping[] =
		{
			{"mvp_matrix", "u_mvp_matrix"},
			{"color", "u_color"},
			{"discard", "u_discard"},
			{"use_attrib_color", "u_use_attrib_color"},
			{"tex_map", "u_tex_map"},
			{"tex_map0", "u_tex_map"},
		};
		const struct { const char* alt_name; const char* name; } default_attribue_mapping[] =
		{
			{"position", "a_position"},
			{"color", "a_color"},
			{"texcoord", "a_texcoord"},
		};

		const char* const simple_vs = 
			"uniform mat4 u_mvp_matrix;\n"
			"uniform float u_point_size;\n"
			"attribute vec2 a_position;\n"
			"attribute vec4 a_color;\n"
			"uniform bool u_use_attrib_color;\n"
			"varying vec4 v_color;\n"
			"void main()\n"
			"{\n"
			"    if(u_use_attrib_color) {\n"
			"        v_color = a_color;\n"
			"    }\n"
			"    gl_PointSize = u_point_size;\n"
			"    gl_Position = u_mvp_matrix * vec4(a_position,0.0,1.0);\n"
			"}\n";
		const char* const simple_fs =
			"varying vec4 v_color;\n"
			"uniform bool u_use_attrib_color;\n"
			"uniform bool u_discard;\n"
			"uniform vec4 u_color;\n"
			"void main()\n"
			"{\n"
			"    vec4 color(1.0,1.0,1.0,1.0);\n"
			"    if(u_use_attrib_color) {\n"
			"        color *= v_color;\n"
			"    }\n"
			"    if(u_discard && color[3] == 0.0) {\n"
			"        discard;\n"
			"    } else {\n"
			"        gl_FragColor = color * u_color;\n"
			"    }\n"
			"}\n";

		const struct { const char* alt_name; const char* name; } simple_uniform_mapping[] =
		{
			{"mvp_matrix", "u_mvp_matrix"},
			{"color", "u_color"},
			{"discard", "u_discard"},
			{"point_size", "u_point_size"},
			{"use_attrib_color", "u_use_attrib_color"},
		};
		const struct { const char* alt_name; const char* name; } simple_attribue_mapping[] =
		{
			{"position", "a_position"},
			{"color", "a_color"},
		};

		typedef std::map<std::string, ShaderProgramPtr> shader_factory_map;
		shader_factory_map& get_shader_factory()
		{
			static shader_factory_map res;
			if(res.empty()) {
				auto spp = ShaderProgramPtr(new ShaderProgram("default", 
					ShaderDef("default_vs", default_vs), 
					ShaderDef("default_fs", default_fs)));
				res["default"] = spp;
				for(auto& dum : default_uniform_mapping) {
					spp->SetAlternateUniformName(dum.name, dum.alt_name);
				}
				for(auto& dam : default_attribue_mapping) {
					spp->SetAlternateAttributeName(dam.name, dam.alt_name);
				}

				spp = ShaderProgramPtr(new ShaderProgram("simple", 
					ShaderDef("simple_vs", default_vs), 
					ShaderDef("simple_fs", default_fs)));
				res["simple"] = spp;
				for(auto& sum : simple_uniform_mapping) {
					spp->SetAlternateUniformName(sum.name, sum.alt_name);
				}
				for(auto& sam : simple_attribue_mapping) {
					spp->SetAlternateAttributeName(sam.name, sam.alt_name);
				}

				// XXX load some default shaders here.
			}
			return res;
		}
	}

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

	ShaderProgram::ShaderProgram(const std::string& name, const ShaderDef& vs, const ShaderDef& fs)
	{
		Init(name, vs, fs);
	}

	ShaderProgram::~ShaderProgram()
	{
		if(object_ != 0) {
			glDeleteShader(object_);
			object_ = 0;
		}
	}

	void ShaderProgram::Init(const std::string& name, const ShaderDef& vs, const ShaderDef& fs)
	{
		name_ = name;
		vs_.reset(new Shader(GL_VERTEX_SHADER, vs.first, vs.second));
		fs_.reset(new Shader(GL_FRAGMENT_SHADER, fs.first, fs.second));
		bool linked_ok = Link();
		ASSERT_LOG(linked_ok == true, "Error linking program: " << name_);
	}

	GLint ShaderProgram::GetAttributeOrDie(const std::string& attr) const
	{
		return GetAttributeIterator(attr)->second.location;
	}

	GLint ShaderProgram::GetUniformOrDie(const std::string& attr) const
	{
		return GetUniformIterator(attr)->second.location;
	}

	GLint ShaderProgram::GetAttribute(const std::string& attr) const
	{
		auto it = attribs_.find(attr);
		if(it != attribs_.end()) {
			return it->second.location;
		}
		auto alt_name_it = attribute_alternate_name_map_.find(attr);
		if(alt_name_it == attribute_alternate_name_map_.end()) {
			LOG_WARN("Attribute '" << attr << "' not found in alternate names list and is not a name defined in the shader.");
			return GLint(-1);
		}
		it = attribs_.find(alt_name_it->second);
		if(it == attribs_.end()) {
			LOG_WARN("Attribute \"" << alt_name_it->second << "\" not found in list, looked up from symbol " << attr);
			return GLint(-1);
		}
		return it->second.location;
	}

	GLint ShaderProgram::GetUniform(const std::string& attr) const
	{
		auto it = uniforms_.find(attr);
		if(it != uniforms_.end()) {
			return it->second.location;
		}
		auto alt_name_it = uniform_alternate_name_map_.find(attr);
		if(alt_name_it == uniform_alternate_name_map_.end()) {
			LOG_WARN("Uniform '" << attr << "' not found in alternate names list and is not a name defined in the shader.");
			return GLint(-1);
		}
		it = uniforms_.find(alt_name_it->second);
		if(it == uniforms_.end()) {
			LOG_WARN("Uniform \"" << alt_name_it->second << "\" not found in list, looked up from symbol " << attr);
			return GLint(-1);
		}
		return it->second.location;
	}

	ConstActivesMapIterator ShaderProgram::GetAttributeIterator(const std::string& attr) const
	{
		auto it = attribs_.find(attr);
		if(it == attribs_.end()) {
			auto alt_name_it = attribute_alternate_name_map_.find(attr);
			ASSERT_LOG(alt_name_it != attribute_alternate_name_map_.end(), 
				"Attribute '" << attr << "' not found in alternate names list and is not a name defined in the shader.");
			it = attribs_.find(alt_name_it->second);
			ASSERT_LOG(it != attribs_.end(), 
				"Attribute \"" << alt_name_it->second << "\" not found in list, looked up from symbol " << attr);
		}
		return it;
	}

	ConstActivesMapIterator ShaderProgram::GetUniformIterator(const std::string& attr) const
	{
		auto it = uniforms_.find(attr);
		if(it == uniforms_.end()) {
			auto alt_name_it = uniform_alternate_name_map_.find(attr);
			ASSERT_LOG(alt_name_it != uniform_alternate_name_map_.end(), 
				"Uniform '" << attr << "' not found in alternate names list and is not a name defined in the shader.");
			it = uniforms_.find(alt_name_it->second);
			ASSERT_LOG(it != uniforms_.end(), 
				"Uniform \"" << alt_name_it->second << "\" not found in list, looked up from symbol " << attr);
		}
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

	void ShaderProgram::SetAlternateUniformName(const std::string& name, const std::string& alt_name)
	{
		ASSERT_LOG(uniform_alternate_name_map_.find(alt_name) == uniform_alternate_name_map_.end(),
			"Trying to replace alternative uniform name: " << alt_name << " " << name);
		uniform_alternate_name_map_[alt_name] = name;
	}

	void ShaderProgram::SetAlternateAttributeName(const std::string& name, const std::string& alt_name)
	{
		ASSERT_LOG(attribute_alternate_name_map_.find(alt_name) == attribute_alternate_name_map_.end(),
			"Trying to replace alternative attribute name: " << alt_name << " " << name);
		attribute_alternate_name_map_[alt_name] = name;
	}

	ShaderProgramPtr ShaderProgram::Factory(const std::string& name)
	{
		auto& sf = get_shader_factory();
		auto it = sf.find(name);
		ASSERT_LOG(it != sf.end(), "Shader '" << name << "' not found in the list of shaders.");
		return it->second;
	}

	ShaderProgramPtr ShaderProgram::DefaultSystemShader()
	{
		auto& sf = get_shader_factory();
		auto it = sf.find("default");
		ASSERT_LOG(it != sf.end(), "No 'default' shader found in the list of shaders.");
		return it->second;
	}
}
