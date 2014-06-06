/*
	Copyright (C) 2003-2013 by Kristina Simpson <sweet.kristas@gmail.com>
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgement in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>

namespace shader
{
	// Abstraction of vertex/geometry/fragment shader
	class shader
	{
	public:
		explicit shader(GLenum type, const std::string& name, const std::string& code);
		GLuint get() const { return shader_; }
		std::string name() const { return name_; }
	protected:
		bool compile(const std::string& code);
	private:
		GLenum type_;
		GLuint shader_;
		std::string name_;
		shader();
		shader(const shader&);
	};
	typedef std::unique_ptr<shader> shader_ptr;

	struct actives
	{
		// Name of variable.
		std::string name;
		// type of the uniform/attribute variable
		GLenum type;
		// If an array type, this is the maximum number of array elements used 
		// in the program. Value is 1 if type is not an array type.
		GLsizei num_elements;
		// Location of the active uniform/attribute
		GLint location;
	};

	typedef std::map<std::string, actives> actives_map;
	typedef actives_map::iterator actives_map_iterator;
	typedef actives_map::const_iterator const_actives_map_iterator;

	typedef std::pair<std::string,std::string> shader_def;

	class shader_program;
	typedef std::shared_ptr<shader_program> shader_program_ptr;

	class shader_program
	{
	public:
		shader_program(const std::string& name, const shader_def& va, const shader_def& fs);
		virtual ~shader_program();
		void init(const std::string& name, const shader_def& vs, const shader_def& fs);
		std::string name() const { return name_; }
		GLint get_attribute_or_die(const std::string& attr) const;
		GLint get_uniform_or_die(const std::string& attr) const;
		GLint get_attribute(const std::string& attr) const;
		GLint get_uniform(const std::string& attr) const;
		const_actives_map_iterator get_attribute_iterator(const std::string& attr) const;
		const_actives_map_iterator get_uniform_iterator(const std::string& attr) const;

		void set_actives();

		void set_uniform_value(const_actives_map_iterator it, const GLint);
		void set_uniform_value(const_actives_map_iterator it, const GLfloat);
		void set_uniform_value(const_actives_map_iterator it, const GLfloat*);
		void set_uniform_value(const_actives_map_iterator it, const GLint*);
		void set_uniform_value(const_actives_map_iterator it, const void*);

		void make_active();

		void set_alternate_uniform_name(const std::string& name, const std::string& alt_name);
		void set_alternate_attribute_name(const std::string& name, const std::string& alt_name);

		static shader_program_ptr factory(const std::string& name);
		static shader_program_ptr default_system_shader();

		const_actives_map_iterator get_color_uniform() const { return u_color_; }
		const_actives_map_iterator get_mvp_uniform() const { return u_mvp_; }
		const_actives_map_iterator get_tex_map_uniform() const { return u_tex_; }
		const_actives_map_iterator get_color_attribute() const { return a_color_; }
		const_actives_map_iterator get_vertex_attribute() const { return a_vertex_; }
		const_actives_map_iterator get_texcoord_attribute() const { return a_texcoord_; }
		
		const_actives_map_iterator uniforms_iterator_end() const { return uniforms_.end(); }
		const_actives_map_iterator attributes_iterator_end() const { return attribs_.end(); }

	protected:
		bool link();
		bool query_uniforms();
		bool query_attributes();

		std::vector<GLint> active_attributes_;
	private:
		std::string name_;
		shader_ptr vs_;
		shader_ptr fs_;
		GLuint object_;
		actives_map attribs_;
		actives_map uniforms_;
		std::map<std::string, std::string> uniform_alternate_name_map_;
		std::map<std::string, std::string> attribute_alternate_name_map_;

		// Store for common attributes and uniforms
		const_actives_map_iterator u_mvp_;
		const_actives_map_iterator u_color_;
		const_actives_map_iterator u_tex_;
		const_actives_map_iterator a_vertex_;
		const_actives_map_iterator a_texcoord_;
		const_actives_map_iterator a_color_;

		shader_program();
		shader_program(const shader_program&);
	};
}
