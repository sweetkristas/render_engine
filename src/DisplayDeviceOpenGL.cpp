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
#include "DisplayDeviceOpenGL.hpp"
#include "RenderVariable.hpp"

namespace Graphics
{
	// These basically get attached to renderable's and we can retreive them during the
	// rendering process. So we store stuff like shader information and shader variables.
	class OpenGLDeviceData : public DisplayDeviceData
	{
	public:
		OpenGLDeviceData() {
		}
		~OpenGLDeviceData() { 
		}
		void SetShader(Shader::ShaderProgramPtr shader) {
			shader_ = shader;
		}
		Shader::ShaderProgramPtr GetShader() const { return shader_; }
	private:
		Shader::ShaderProgramPtr shader_;
		OpenGLDeviceData(const OpenGLDeviceData&);
	};


	namespace 
	{
		GLenum ConvertRenderVariableType(Render::RenderVariableDesc::VariableType type)
		{
			static std::vector<GLenum> res;
			if(res.empty()) {
				res.push_back(GL_BYTE);					// TYPE_BOOL
				res.push_back(GL_HALF_FLOAT);			// TYPE_HALF_FLOAT
				res.push_back(GL_FLOAT);				// TYPE_FLOAT
				res.push_back(GL_DOUBLE);				// TYPE_DOUBLE
				res.push_back(GL_FIXED);				// TYPE_FIXED
				res.push_back(GL_SHORT);				// TYPE_SHORT
				res.push_back(GL_UNSIGNED_SHORT);		// TYPE_UNSIGNED_SHORT
				res.push_back(GL_BYTE);					// TYPE_BYTE
				res.push_back(GL_UNSIGNED_BYTE);		// TYPE_UNSIGNED_BYTE
				res.push_back(GL_INT);					// TYPE_INT
				res.push_back(GL_UNSIGNED_INT);			// TYPE_UNSIGNED_INT
				res.push_back(GL_INT_2_10_10_10_REV);	// TYPE_INT_2_10_10_10
				res.push_back(GL_UNSIGNED_INT_2_10_10_10_REV);	// TYPE_UNSIGNED_INT_2_10_10_10
				res.push_back(GL_UNSIGNED_INT_10F_11F_11F_REV);	// TYPE_UNSIGNED_INT_10F_11F_11F
			}
			ASSERT_LOG(std::vector<GLenum>::size_type(type) < res.size(), "ConvertRenderVariableType: Unable to find type " << type);
			return res[type];
		}
	}

	DisplayDeviceOpenGL::DisplayDeviceOpenGL()
	{
	}

	DisplayDeviceOpenGL::~DisplayDeviceOpenGL()
	{
	}

	void DisplayDeviceOpenGL::init(size_t width, size_t height)
	{
		GLenum err = glewInit();
		ASSERT_LOG(err == GLEW_OK, "Could not initialise GLEW: " << glewGetErrorString(err));

		glViewport(0, 0, width, height);

		// Register with the render variable factory so we can create 
		// VBO backed render variables.
	}

	void DisplayDeviceOpenGL::print_device_info()
	{
	}

	void DisplayDeviceOpenGL::clear(uint32_t clr)
	{
		glClear(clr & DISPLAY_CLEAR_COLOR ? GL_COLOR_BUFFER_BIT : 0 
			| clr & DISPLAY_CLEAR_DEPTH ? GL_DEPTH_BUFFER_BIT : 0 
			| clr & DISPLAY_CLEAR_STENCIL ? GL_STENCIL_BUFFER_BIT : 0);
	}

	void DisplayDeviceOpenGL::set_clear_color(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void DisplayDeviceOpenGL::swap()
	{
		// This is a no-action.
	}

	DisplayDeviceDataPtr DisplayDeviceOpenGL::CreateDisplayDeviceData(const DisplayDeviceDef& def)
	{
		OpenGLDeviceData* dd = new OpenGLDeviceData();
		bool use_default_shader = true;
		for(auto& hints : def.GetHints()) {
			if(hints.first == "shader") {
				// Need to have retrieved more shader data here.
				dd->SetShader(Shader::ShaderProgram::Factory(hints.second));
				use_default_shader = false;
			}
			// ...
			// add more hints here if needed.
		}
		// If there is no shader hint, we will assume the default system shader.
		if(use_default_shader) {
			dd->SetShader(Shader::ShaderProgram::DefaultSystemShader());
		}

		// XXX Process vertex info here def.GetVertexInfo()
		for(auto& vertex_info : def.GetVertexInfo()) {
			auto shader = dd->GetShader();

			/// XXX the information returned from below needs to be matched with the corresponding render variable
			// so it can be applied during render -- it may be better if we augment Render variables to do this.
			shader->GetAttributeIterator(vertex_info);
		}
		return DisplayDeviceDataPtr(dd);
	}

	void DisplayDeviceOpenGL::render(const Render::RenderablePtr& r)
	{
		auto dd = std::dynamic_pointer_cast<OpenGLDeviceData>(r->GetDisplayData());
		ASSERT_LOG(dd != NULL, "Failed to cast display data to the type required(OpenGLDeviceData).");
		dd->GetShader()->MakeActive();

		if(r->Camera()) {
			/// xxx need to set camera here.
		}
		for(auto lp : r->Lights()) {
			/// xxx need to set lights here.
		}

		// Need to figure the interaction with shaders.
		/// XXX Need to create a mapping between attributes and the index value below.
		for(auto rv : r->RenderVariables()) {
			for(auto& rvd : rv->VariableDescritionList()) {
				GLuint index = 0;
				switch(rvd.GetVertexType()) {
					case Render::RenderVariableDesc::VERTEX_POSITION:
					case Render::RenderVariableDesc::VERTEX_COLOR:
					case Render::RenderVariableDesc::VERTEX_TEXTURE:
					case Render::RenderVariableDesc::VERTEX_NORMAL:
					case Render::RenderVariableDesc::VERTEX_UNKNOWN:
					break;
				}
				glVertexAttribPointer(index, 
					rvd.NumElements(), 
					ConvertRenderVariableType(rvd.GetVariableType()), 
					rvd.Normalised(), 
					rvd.Stride(), 
					(void*)rvd.Offset());
			}

			// XXX todo change this to a simple index look-up.
			GLenum draw_mode = GL_NONE;
			switch(rv->GetDrawMode()) {
				case Render::RenderVariable::POINTS:			draw_mode = GL_POINTS; break;
				case Render::RenderVariable::LINE_STRIP:		draw_mode = GL_LINE_STRIP; break;
				case Render::RenderVariable::LINE_LOOP:			draw_mode = GL_LINE_LOOP; break;
				case Render::RenderVariable::LINES:				draw_mode = GL_LINES; break;
				case Render::RenderVariable::TRIANGLE_STRIP:	draw_mode = GL_TRIANGLE_STRIP; break;
				case Render::RenderVariable::TRIANGLE_FAN:		draw_mode = GL_TRIANGLE_FAN; break;
				case Render::RenderVariable::TRIANGLES:			draw_mode = GL_TRIANGLES; break;
				case Render::RenderVariable::QUAD_STRIP:		draw_mode = GL_QUAD_STRIP; break;
				case Render::RenderVariable::QUADS:				draw_mode = GL_QUADS; break;
				case Render::RenderVariable::POLYGON:			draw_mode = GL_POLYGON; break;
			}
			if(rv->IsIndexedDraw()) {
				// XXX
				//glDrawElements(draw_mode, rv->Count(), rv->IndexType(), rv->Indicies());
			} else {
				// XXX we probably need to grab an offset parameter, from rv, instead of the 0 below.
				glDrawArrays(draw_mode, 0, rv->Count());
			}
		}
	}
}
