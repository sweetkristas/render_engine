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

#include <GL/glew.h>

#include "asserts.hpp"
#include "attribute_set_ogl.hpp"
#include "camera_object.hpp"
#include "canvas_ogl.hpp"
#include "display_device_ogl.hpp"
#include "FboOpenGL.hpp"
#include "LightObject.hpp"
#include "MaterialOpenGL.hpp"
#include "texture_ogl.hpp"

namespace KRE
{
	namespace
	{
		static display_device_registrar<display_device_ogl> ogl_register("opengl");
	}

	// These basically get attached to renderable's and we can retreive them during the
	// rendering process. So we store stuff like shader information and shader variables.
	class ogl_device_data : public display_device_data
	{
	public:
		ogl_device_data() {
		}
		~ogl_device_data() { 
		}
		void set_shader(shader::shader_program_ptr shader) {
			shader_ = shader;
		}
		shader::shader_program_ptr get_shader() const { return shader_; }
	private:
		shader::shader_program_ptr shader_;
		ogl_device_data(const ogl_device_data&);
	};

	class render_variable_device_data : public display_device_data
	{
	public:
		render_variable_device_data() {
		}
		~render_variable_device_data() {
		}
		render_variable_device_data(const shader::const_actives_map_iterator& it)
			: active_iterator_(it) {
		}

		void SetActiveMapIterator(const shader::const_actives_map_iterator& it) {
			active_iterator_ = it;
		}
		shader::const_actives_map_iterator GetActiveMapIterator() const { return active_iterator_; }
	private:
		shader::const_actives_map_iterator active_iterator_;
	};

	namespace 
	{
		GLenum convert_render_variable_type(attribute_desc::VariableType type)
		{
			switch(type) {
				case attribute_desc::VariableType::BOOL:							return GL_BYTE;
				case attribute_desc::VariableType::HALF_FLOAT:					return GL_HALF_FLOAT;
				case attribute_desc::VariableType::FLOAT:						return GL_FLOAT;
				case attribute_desc::VariableType::DOUBLE:						return GL_DOUBLE;
				case attribute_desc::VariableType::FIXED:						return GL_FIXED;
				case attribute_desc::VariableType::SHORT:						return GL_SHORT;
				case attribute_desc::VariableType::UNSIGNED_SHORT:				return GL_UNSIGNED_SHORT;
				case attribute_desc::VariableType::BYTE:							return GL_BYTE;
				case attribute_desc::VariableType::UNSIGNED_BYTE:				return GL_UNSIGNED_BYTE;
				case attribute_desc::VariableType::INT:							return GL_INT;
				case attribute_desc::VariableType::UNSIGNED_INT:					return GL_UNSIGNED_INT;
				case attribute_desc::VariableType::INT_2_10_10_10_REV:			return GL_INT_2_10_10_10_REV;
				case attribute_desc::VariableType::UNSIGNED_INT_2_10_10_10_REV:	return GL_UNSIGNED_INT_2_10_10_10_REV;
				case attribute_desc::VariableType::UNSIGNED_INT_10F_11F_11F_REV:	return GL_UNSIGNED_INT_10F_11F_11F_REV;
			}
			ASSERT_LOG(false, "Unrecognised value for variable type.");
			return GL_NONE;
		}

		GLenum convert_drawing_mode(attribute_set::DrawMode dm)
		{
			switch(dm) {
				case attribute_set::DrawMode::POINTS:			return GL_POINTS;
				case attribute_set::DrawMode::LINE_STRIP:		return GL_LINE_STRIP;
				case attribute_set::DrawMode::LINE_LOOP:			return GL_LINE_LOOP;
				case attribute_set::DrawMode::LINES:				return GL_LINES;
				case attribute_set::DrawMode::TRIANGLE_STRIP:	return GL_TRIANGLE_STRIP;
				case attribute_set::DrawMode::TRIANGLE_FAN:		return GL_TRIANGLE_FAN;
				case attribute_set::DrawMode::TRIANGLES:			return GL_TRIANGLES;
				case attribute_set::DrawMode::QUAD_STRIP:		return GL_QUAD_STRIP;
				case attribute_set::DrawMode::QUADS:				return GL_QUADS;
				case attribute_set::DrawMode::POLYGON:			return GL_POLYGON;
			}
			ASSERT_LOG(false, "Unrecognised value for drawing mode.");
			return GL_NONE;
		}

		GLenum convert_index_type(attribute_set::IndexType it) 
		{
			switch(it) {
				case attribute_set::IndexType::INDEX_NONE:		break;
				case attribute_set::IndexType::INDEX_UCHAR:		return GL_UNSIGNED_BYTE;
				case attribute_set::IndexType::INDEX_USHORT:		return GL_UNSIGNED_SHORT;
				case attribute_set::IndexType::INDEX_ULONG:		return GL_UNSIGNED_INT;
			}
			ASSERT_LOG(false, "Unrecognised value for index type.");
			return GL_NONE;
		}
	}

	display_device_ogl::display_device_ogl()
	{
	}

	display_device_ogl::~display_device_ogl()
	{
	}

	void display_device_ogl::init(size_t width, size_t height)
	{
		GLenum err = glewInit();
		ASSERT_LOG(err == GLEW_OK, "Could not initialise GLEW: " << glewGetErrorString(err));

		glViewport(0, 0, width, height);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Register with the render variable factory so we can create 
		// VBO backed render variables.
	}

	void display_device_ogl::print_device_info()
	{
		GLint minor_version;
		GLint major_version;
		glGetIntegerv(GL_MINOR_VERSION, &minor_version);
		glGetIntegerv(GL_MINOR_VERSION, &major_version);
		if(glGetError() != GL_NONE) {
			// fall-back to old glGetStrings method.
			const char* version_str = reinterpret_cast<const char*>(glGetString(GL_VERSION));
			std::cerr << "OpenGL version: " << version_str << std::endl;
		} else {
			std::cerr << "OpenGL version: " << major_version << "." << minor_version << std::endl;
		}
	}

	void display_device_ogl::clear(uint32_t clr)
	{
		glClear(clr & DISPLAY_CLEAR_COLOR ? GL_COLOR_BUFFER_BIT : 0 
			| clr & DISPLAY_CLEAR_DEPTH ? GL_DEPTH_BUFFER_BIT : 0 
			| clr & DISPLAY_CLEAR_STENCIL ? GL_STENCIL_BUFFER_BIT : 0);
	}

	void display_device_ogl::set_clear_color(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void display_device_ogl::set_clear_color(const color& color)
	{
		glClearColor(float(color.r()), float(color.g()), float(color.b()), float(color.a()));
	}

	void display_device_ogl::swap()
	{
		// This is a no-action.
	}

	display_device_data_ptr display_device_ogl::create_display_device_data(const display_device_def& def)
	{
		ogl_device_data* dd = new ogl_device_data();
		bool use_default_shader = true;
		for(auto& hints : def.get_hints()) {
			if(hints.first == "shader") {
				// Need to have retrieved more shader data here.
				dd->set_shader(shader::shader_program::factory(hints.second[0]));
				use_default_shader = false;
			}
			// ...
			// add more hints here if needed.
		}
		// If there is no shader hint, we will assume the default system shader.
		if(use_default_shader) {
			dd->set_shader(shader::shader_program::default_system_shader());
		}
		
		// XXX Set uniforms from block here.

		for(auto& as : def.get_attribute_set()) {
			for(auto& attr : as->get_attributes()) {
				for(auto& desc : attr->get_attr_desc()) {
					auto ddp = display_device_data_ptr(new render_variable_device_data(dd->get_shader()->get_attribute_iterator(desc.attr_name())));
					desc.set_display_data(ddp);
				}
			}
		}

		return display_device_data_ptr(dd);
	}

	void display_device_ogl::render(const renderable_ptr& r)
	{
		auto dd = std::static_pointer_cast<ogl_device_data>(r->get_display_data());
		ASSERT_LOG(dd != NULL, "Failed to cast display data to the type required(ogl_device_data).");
		auto shader = dd->get_shader();
		shader->make_active();

		// lighting can be switched on or off at a material level.
		// so we grab the return of the Material::Apply() function
		// to find whether to apply it or not.
		bool use_lighting = true;
		if(r->material()) {
			use_lighting = r->material()->Apply();
		}

		glm::mat4 pmat(1.0f);
		if(r->camera()) {
			// set camera here.
			pmat = r->camera()->projection_mat() * r->camera()->view_mat();
		}

		if(use_lighting) {
			for(auto lp : r->lights()) {
				/// xxx need to set lights here.
			}
		}

		if(r->get_render_target()) {
			r->get_render_target()->Apply();
		}

		if(shader->get_mvp_uniform() != shader->uniforms_iterator_end()) {
			pmat *= r->model_matrix();
			shader->set_uniform_value(shader->get_mvp_uniform(), glm::value_ptr(pmat));
		}

		if(shader->get_color_uniform() != shader->uniforms_iterator_end() && r->is_color_set()) {
			shader->set_uniform_value(shader->get_color_uniform(), r->get_color().AsFloatVector());
		}

		if(shader->get_tex_map_uniform() != shader->uniforms_iterator_end()) {
			shader->set_uniform_value(shader->get_tex_map_uniform(), 0);
		}

		// Loop through uniform render variables and set them.
		/*for(auto& urv : r->UniformrenderVariables()) {
			for(auto& rvd : urv->VariableDescritionList()) {
				auto rvdd = std::dynamic_pointer_cast<render_variable_device_data>(rvd->get_display_data());
				ASSERT_LOG(rvdd != NULL, "Unable to cast DeviceData to render_variable_device_data.");
				shader->set_uniform_value(rvdd->GetActiveMapIterator(), urv->value());
			}
		}*/

		// Need to figure the interaction with shaders.
		/// XXX Need to create a mapping between attributes and the index value below.
		for(auto as : r->get_attribute_set()) {
			GLenum draw_mode = convert_drawing_mode(as->get_draw_mode());
			std::vector<GLuint> enabled_attribs;

			for(auto& attr : as->get_attributes()) {
				auto attr_hw = attr->get_device_buffer_data();
				//auto attrogl = std::dynamic_pointer_cast<attributeOGL>(attr);
				attr_hw->bind();
				for(auto& attrdesc : attr->get_attr_desc()) {
					auto ddp = std::static_pointer_cast<render_variable_device_data>(attrdesc.get_display_data());
					ASSERT_LOG(ddp != NULL, "converting attribute device data was NULL.");
					glEnableVertexAttribArray(ddp->GetActiveMapIterator()->second.location);
					
					glVertexAttribPointer(ddp->GetActiveMapIterator()->second.location, 
						attrdesc.num_elements(), 
						convert_render_variable_type(attrdesc.var_type()), 
						attrdesc.normalise(), 
						attrdesc.stride(), 
						reinterpret_cast<const GLvoid*>(attr_hw->value() + attr->get_offset() + attrdesc.offset()));
					enabled_attribs.emplace_back(ddp->GetActiveMapIterator()->second.location);
				}
			}

			if(as->is_instanced()) {
				if(as->is_indexed()) {
					as->bind_index();
					// XXX as->get_index_array() should be as->get_index_array()+as->get_offset()
					glDrawElementsInstanced(draw_mode, as->get_count(), convert_index_type(as->get_index_type()), as->get_index_array(), as->get_instance_count());
					as->unbind_index();
				} else {
					glDrawArraysInstanced(draw_mode, as->get_offset(), as->get_count(), as->get_instance_count());
				}
			} else {
				if(as->is_indexed()) {
					as->bind_index();
					// XXX as->get_index_array() should be as->get_index_array()+as->get_offset()
					glDrawElements(draw_mode, as->get_count(), convert_index_type(as->get_index_type()), as->get_index_array());
					as->unbind_index();
				} else {
					glDrawArrays(draw_mode, as->get_offset(), as->get_count());
				}
			}

			for(auto attrib : enabled_attribs) {
				glDisableVertexAttribArray(attrib);
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		if(r->material()) {
			r->material()->Unapply();
		}
		if(r->get_render_target()) {
			r->get_render_target()->Unapply();
		}
	}

	texture_ptr display_device_ogl::handle_create_texture(const surface_ptr& surface, const variant& node)
	{
		return texture_ptr(new OpenGLtexture(surface, node));
	}

	texture_ptr display_device_ogl::handle_create_texture(const surface_ptr& surface, texture::Type type, int mipmap_levels)
	{
		return texture_ptr(new OpenGLtexture(surface, type, mipmap_levels));
	}

	texture_ptr display_device_ogl::handle_create_texture(unsigned width, PixelFormat fmt)
	{
		return texture_ptr(new OpenGLtexture(width, 0, fmt, texture::Type::TEXTURE_1D));
	}

	texture_ptr display_device_ogl::handle_create_texture(unsigned width, unsigned height, PixelFormat fmt, texture::Type type)
	{
		return texture_ptr(new OpenGLtexture(width, height, fmt, texture::Type::TEXTURE_2D));
	}
	
	texture_ptr display_device_ogl::handle_create_texture(unsigned width, unsigned height, unsigned depth, PixelFormat fmt)
	{
		return texture_ptr(new OpenGLtexture(width, height, fmt, texture::Type::TEXTURE_3D, depth));
	}

	texture_ptr display_device_ogl::handle_create_texture(const std::string& filename, texture::Type type, int mipmap_levels)
	{
		auto surface = surface::create(filename);
		return texture_ptr(new OpenGLtexture(surface, type, mipmap_levels));
	}

	MaterialPtr display_device_ogl::handle_create_material(const variant& node)
	{
		return MaterialPtr(new OpenGLMaterial(node));
	}

	MaterialPtr display_device_ogl::handle_create_material(const std::string& name, 
		const std::vector<texture_ptr>& textures, 
		const BlendMode& blend, 
		bool fog, 
		bool lighting, 
		bool depth_write, 
		bool depth_check)
	{
		return MaterialPtr(new OpenGLMaterial(name, textures, blend, fog, lighting, depth_write, depth_check));
	}

	render_target_ptr display_device_ogl::handle_create_render_target(size_t width, size_t height, 
			size_t color_plane_count, 
			bool depth, 
			bool stencil, 
			bool use_multi_sampling, 
			size_t multi_samples)
	{
		return render_target_ptr(new FboOpenGL(width, height, color_plane_count, depth, stencil, use_multi_sampling, multi_samples));
	}

	render_target_ptr display_device_ogl::handle_create_render_target(const variant& node)
	{
		return render_target_ptr(new FboOpenGL(node));
	}

	attribute_set_ptr display_device_ogl::handle_create_attribute_set(bool indexed, bool instanced)
	{
		return attribute_set_ptr(new attribute_set_ogl(indexed, instanced));
	}

	hardware_attribute_ptr display_device_ogl::handle_create_attribute(attribute_base* parent)
	{
		return hardware_attribute_ptr(new hardware_attribute_ogl(parent));
	}

	canvas_ptr display_device_ogl::get_canvas()
	{
		return canvas_ogl::get_instance();
	}
	
	bool display_device_ogl::do_check_for_feature(display_device_capabilties cap)
	{
		bool ret_val = false;
		switch(cap) {
		case display_device_capabilties::NPOT_TEXTURES:
			// XXX We could put a force npot textures check here.
			if(GLEW_ARB_texture_non_power_of_two) {
				ret_val = true;
			}
			break;
		default:
			ASSERT_LOG(false, "Unknown value for display_device_capabilties given.");
		}
		return ret_val;
	}

	// XXX Need a way to deal with blits with Camera/Lighting.
	void display_device_ogl::do_blit_texture(const texture_ptr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch)
	{
		auto texture = std::static_pointer_cast<OpenGLtexture>(tex);
		ASSERT_LOG(texture != NULL, "texture passed in was not of expected type.");

		const float tx1 = float(srcx) / texture->width();
		const float ty1 = float(srcy) / texture->height();
		const float tx2 = srcw == 0 ? 1.0f : float(srcx + srcw) / texture->width();
		const float ty2 = srch == 0 ? 1.0f : float(srcy + srch) / texture->height();
		const float uv_coords[] = {
			tx1, ty1,
			tx2, ty1,
			tx1, ty2,
			tx2, ty2,
		};

		const float vx1 = float(dstx);
		const float vy1 = float(dsty);
		const float vx2 = float(dstx + dstw);
		const float vy2 = float(dsty + dsth);
		const float vtx_coords[] = {
			vx1, vy1,
			vx2, vy1,
			vx1, vy2,
			vx2, vy2,
		};

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((vx1+vx2)/2.0f,(vy1+vy2)/2.0f,0.0f)) * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-(vx1+vy1)/2.0f,-(vy1+vy1)/2.0f,0.0f));
		glm::mat4 mvp = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f) * model;
		auto shader = shader::shader_program::default_system_shader();
		shader->make_active();
		texture->bind();
		shader->set_uniform_value(shader->get_mvp_uniform(), glm::value_ptr(mvp));
		shader->set_uniform_value(shader->get_color_uniform(), glm::value_ptr(glm::vec4(1.0f,1.0f,1.0f,1.0f)));
		shader->set_uniform_value(shader->get_tex_map_uniform(), 0);
		// XXX the following line are only temporary, obviously.
		shader->set_uniform_value(shader->get_uniform_iterator("discard"), 0);
		glEnableVertexAttribArray(shader->get_vertex_attribute()->second.location);
		glVertexAttribPointer(shader->get_vertex_attribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords);
		glEnableVertexAttribArray(shader->get_texcoord_attribute()->second.location);
		glVertexAttribPointer(shader->get_texcoord_attribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, uv_coords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(shader->get_texcoord_attribute()->second.location);
		glDisableVertexAttribArray(shader->get_vertex_attribute()->second.location);
	}
}
