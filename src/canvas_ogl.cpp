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

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "canvas_ogl.hpp"
#include "shaders_ogl.hpp"
#include "texture_ogl.hpp"

namespace KRE
{
	namespace
	{
		canvas_ptr& get_canvas_instance()
		{
			static canvas_ptr res = canvas_ptr(new canvas_ogl());
			return res;
		}
	}

	canvas_ogl::canvas_ogl()
	{
		handle_dimensions_changed();
	}

	canvas_ogl::~canvas_ogl()
	{
	}

	void canvas_ogl::handle_dimensions_changed()
	{
		mvp_ = glm::ortho(0.0f, float(width()), float(height()), 0.0f);
	}

	void canvas_ogl::blit_texture(const texture_ptr& tex, const rect& src, float rotation, const rect& dst, const color& color)
	{
		auto texture = std::static_pointer_cast<OpenGLtexture>(tex);
		ASSERT_LOG(texture != NULL, "texture passed in was not of expected type.");

		const float tx1 = float(src.x()) / texture->width();
		const float ty1 = float(src.y()) / texture->height();
		const float tx2 = src.w() == 0 ? 1.0f : float(src.x2()) / texture->width();
		const float ty2 = src.h() == 0 ? 1.0f : float(src.y2()) / texture->height();
		const float uv_coords[] = {
			tx1, ty1,
			tx2, ty1,
			tx1, ty2,
			tx2, ty2,
		};

		const float vx1 = float(dst.x());
		const float vy1 = float(dst.y());
		const float vx2 = float(dst.x2());
		const float vy2 = float(dst.y2());
		const float vtx_coords[] = {
			vx1, vy1,
			vx2, vy1,
			vx1, vy2,
			vx2, vy2,
		};

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((vx1+vx2)/2.0f,(vy1+vy2)/2.0f,0.0f)) * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-(vx1+vx2)/2.0f,-(vy1+vy2)/2.0f,0.0f));
		glm::mat4 mvp = mvp_ * model;
		auto shader = shader::shader_program::default_system_shader();
		shader->make_active();
		texture->bind();
		shader->set_uniform_value(shader->get_mvp_uniform(), glm::value_ptr(mvp));
		shader->set_uniform_value(shader->get_color_uniform(), color.AsFloatVector());
		shader->set_uniform_value(shader->get_tex_map_uniform(), 0);
		// XXX the following line are only temporary, obviously.
		//shader->set_uniform_value(shader->get_uniform_iterator("discard"), 0);
		glEnableVertexAttribArray(shader->get_vertex_attribute()->second.location);
		glVertexAttribPointer(shader->get_vertex_attribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords);
		glEnableVertexAttribArray(shader->get_texcoord_attribute()->second.location);
		glVertexAttribPointer(shader->get_texcoord_attribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, uv_coords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(shader->get_texcoord_attribute()->second.location);
		glDisableVertexAttribArray(shader->get_vertex_attribute()->second.location);
	}

	void canvas_ogl::blit_texture(const MaterialPtr& mat, float rotation, const rect& dst, const color& color)
	{
		const float vx1 = float(dst.x());
		const float vy1 = float(dst.y());
		const float vx2 = float(dst.x2());
		const float vy2 = float(dst.y2());
		const float vtx_coords[] = {
			vx1, vy1,
			vx2, vy1,
			vx1, vy2,
			vx2, vy2,
		};

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((vx1+vx2)/2.0f,(vy1+vy2)/2.0f,0.0f)) * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-(vx1+vy1)/2.0f,-(vy1+vy1)/2.0f,0.0f));
		glm::mat4 mvp = mvp_ * model;
		auto shader = shader::shader_program::default_system_shader();
		shader->make_active();
		shader->set_uniform_value(shader->get_mvp_uniform(), glm::value_ptr(mvp));
		shader->set_uniform_value(shader->get_color_uniform(), color.AsFloatVector());
		shader->set_uniform_value(shader->get_tex_map_uniform(), 0);

		for(auto it = mat->Gettexture().begin(); it != mat->Gettexture().end(); ++it) {
			auto texture = std::static_pointer_cast<OpenGLtexture>(*it);
			ASSERT_LOG(texture != NULL, "texture passed in was not of expected type.");

			auto uv_coords = mat->GetNormalisedtextureCoords(it);

			texture->bind();
			// XXX the following line are only temporary, obviously.
			//shader->set_uniform_value(shader->get_uniform_iterator("discard"), 0);
			glEnableVertexAttribArray(shader->get_vertex_attribute()->second.location);
			glVertexAttribPointer(shader->get_vertex_attribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords);
			glEnableVertexAttribArray(shader->get_texcoord_attribute()->second.location);
			glVertexAttribPointer(shader->get_texcoord_attribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, &uv_coords);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glDisableVertexAttribArray(shader->get_texcoord_attribute()->second.location);
			glDisableVertexAttribArray(shader->get_vertex_attribute()->second.location);
		}
	}

	canvas_ptr canvas_ogl::get_instance()
	{
		return get_canvas_instance();
	}
}
