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
#include "TextureOpenGL.hpp"

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

	class RenderVariableDeviceData : public DisplayDeviceData
	{
	public:
		RenderVariableDeviceData() {
		}
		~RenderVariableDeviceData() {
		}
		RenderVariableDeviceData(const Shader::ConstActivesMapIterator& it)
			: active_iterator_(it) {
		}

		void SetActiveMapIterator(const Shader::ConstActivesMapIterator& it) {
			active_iterator_ = it;
		}
		Shader::ConstActivesMapIterator GetActiveMapIterator() const { return active_iterator_; }
	private:
		Shader::ConstActivesMapIterator active_iterator_;
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

		// XXX Process render variables here
		for(auto& rv : def.GetRenderVars()) {
			for(auto& rvd : rv->VariableDescritionList()) {
				switch(rvd.GetDescription()) {
					/// XXX the information returned from below needs to be matched with the corresponding render variable
					// so it can be applied during render -- it may be better if we augment Render variables to do this.
					case Render::RenderVariableDesc::DESC_ATTRIB: {
						auto rvdd = new RenderVariableDeviceData(dd->GetShader()->GetAttributeIterator(rvd.GetVertexTypeAsString()));
						rvd.SetDisplayData(DisplayDeviceDataPtr(rvdd));
						break;
					}
					case Render::RenderVariableDesc::DESC_UNIFORM: {
						auto rvdd = new RenderVariableDeviceData(dd->GetShader()->GetUniformIterator(rvd.GetUniformTypeAsString()));
						rvd.SetDisplayData(DisplayDeviceDataPtr(rvdd));
						break;
					}
					default:
						ASSERT_LOG(false, "Unrecognised render variable description type: " << rvd.GetDescription());
				}
			}
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
				auto rvdd = std::dynamic_pointer_cast<RenderVariableDeviceData>(rvd.GetDisplayData());
				ASSERT_LOG(rvdd != NULL, "Unable to cast DeviceData to RenderVariableDeviceData.");
				if(rvd.GetDescription() == Render::RenderVariableDesc::DESC_ATTRIB) {
					glVertexAttribPointer(rvdd->GetActiveMapIterator()->second.location, 
						rvd.NumElements(), 
						ConvertRenderVariableType(rvd.GetVariableType()), 
						rvd.Normalised(), 
						rvd.Stride(), 
						(void*)((intptr_t)rv->Value() + (intptr_t) + rvd.Offset()));
				} else {
					// XXX set uniform
					//dd->GetShader()->SetUniform(rvdd->GetActiveMapIterator(), rv->Value());
				}
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

	TexturePtr DisplayDeviceOpenGL::CreateTexture(const SurfacePtr& surface, Texture::TextureType type, int mipmap_levels)
	{
		return TexturePtr(new OpenGLTexture(surface, type, mipmap_levels));
	}

	void DisplayDeviceOpenGL::BlitTexture(const TexturePtr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch)
	{
		auto texture = std::dynamic_pointer_cast<OpenGLTexture>(tex);
		ASSERT_LOG(texture != NULL, "Texture passed in was not of expected type.");
		texture->Bind();

		const float tx1 = float(srcx) / texture->Width();
		const float ty1 = float(srcy) / texture->Height();
		const float tx2 = srcw == 0 ? 1.0f : float(srcx + srcw) / texture->Width();
		const float ty2 = srch == 0 ? 1.0f : float(srcy + srch) / texture->Height();
		const float uv_coords[] = {
			tx1, ty1,
			tx1, ty2,
			tx2, ty1,
			tx2, ty2,
		};

		const float vx1 = float(dstx);
		const float vy1 = float(dstx);
		const float vx2 = float(dstx + dstw);
		const float vy2 = float(dsty + dsth);
		const float vtx_coords[] = {
			vx1, vy1,
			vx2, vy1,
			vx1, vy2,
			vx2, vy2,
		};

		glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f));
		glm::mat4 pmat = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f);
		glm::mat4 mvp = pmat * /* view * */ model;

		auto shader = Shader::ShaderProgram::DefaultSystemShader();
		shader->MakeActive();
		shader->SetUniformValue(shader->GetMvpUniform(), glm::value_ptr(mvp));
		shader->SetUniformValue(shader->GetColorUniform(), glm::value_ptr(glm::vec4(1.0f,1.0f,1.0f,1.0f)));
		int value = 0;
		shader->SetUniformValue(shader->GetUniformIterator("discard"), &value);
		shader->SetUniformValue(shader->GetUniformIterator("tex_map"), &value);
		glEnableVertexAttribArray(shader->GetVertexAttribute()->second.location);
		glVertexAttribPointer(shader->GetVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords);
		glEnableVertexAttribArray(shader->GetTexcoordAttribute()->second.location);
		glVertexAttribPointer(shader->GetTexcoordAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, uv_coords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(shader->GetTexcoordAttribute()->second.location);
		glDisableVertexAttribArray(shader->GetVertexAttribute()->second.location);
	}
}
