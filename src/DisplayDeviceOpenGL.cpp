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
#include "CameraObject.hpp"
#include "DisplayDeviceOpenGL.hpp"
#include "FboOpenGL.hpp"
#include "LightObject.hpp"
#include "MaterialOpenGL.hpp"
#include "RenderVariable.hpp"
#include "TextureOpenGL.hpp"

namespace Graphics
{
	namespace
	{
		static DisplayDeviceRegistrar<DisplayDeviceOpenGL> ogl_register("opengl");
	}

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
		GLenum ConvertRenderVariableType(Render::AttributeRenderVariableDesc::VariableType type)
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

		GLenum ConvertDrawingMode(Render::RenderVariable::DrawMode dm)
		{
			// relies on all the values in Render::Renderable::DrawMode being contiguous
			static std::vector<GLenum> res;
			if(res.empty()) {
				res.emplace_back(GL_POINTS);
				res.emplace_back(GL_LINE_STRIP);
				res.emplace_back(GL_LINE_LOOP);
				res.emplace_back(GL_LINES);
				res.emplace_back(GL_TRIANGLE_STRIP);
				res.emplace_back(GL_TRIANGLE_FAN);
				res.emplace_back(GL_TRIANGLES);
				res.emplace_back(GL_QUAD_STRIP);
				res.emplace_back(GL_QUADS);
				res.emplace_back(GL_POLYGON);
			}
			ASSERT_LOG(std::vector<GLenum>::size_type(dm) < res.size(), "ConvertDrawingMode: Unable to find type " << dm);
			return res[dm];
		}
	}

	DisplayDeviceOpenGL::DisplayDeviceOpenGL()
	{
	}

	DisplayDeviceOpenGL::~DisplayDeviceOpenGL()
	{
	}

	void DisplayDeviceOpenGL::Init(size_t width, size_t height)
	{
		GLenum err = glewInit();
		ASSERT_LOG(err == GLEW_OK, "Could not initialise GLEW: " << glewGetErrorString(err));

		glViewport(0, 0, width, height);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Register with the render variable factory so we can create 
		// VBO backed render variables.
	}

	void DisplayDeviceOpenGL::PrintDeviceInfo()
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

	void DisplayDeviceOpenGL::Clear(uint32_t clr)
	{
		glClear(clr & DISPLAY_CLEAR_COLOR ? GL_COLOR_BUFFER_BIT : 0 
			| clr & DISPLAY_CLEAR_DEPTH ? GL_DEPTH_BUFFER_BIT : 0 
			| clr & DISPLAY_CLEAR_STENCIL ? GL_STENCIL_BUFFER_BIT : 0);
	}

	void DisplayDeviceOpenGL::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void DisplayDeviceOpenGL::SetClearColor(const Color& color)
	{
		glClearColor(float(color.r()), float(color.g()), float(color.b()), float(color.a()));
	}

	void DisplayDeviceOpenGL::Swap()
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
				dd->SetShader(Shader::ShaderProgram::Factory(hints.second[0]));
				use_default_shader = false;
			}
			// ...
			// add more hints here if needed.
		}
		// If there is no shader hint, we will assume the default system shader.
		if(use_default_shader) {
			dd->SetShader(Shader::ShaderProgram::DefaultSystemShader());
		}

		// Process uniform render variables here
		for(auto& rv : def.GetUniformRenderVars()) {
			for(auto& rvd : rv->VariableDescritionList()) {
				auto& urvd = std::dynamic_pointer_cast<Render::UniformRenderVariableDesc>(rvd);
				ASSERT_LOG(urvd != NULL, "RenderVariableDesc was wrong type, couldn't cast to UniformRenderVariableDesc.");
					auto rvdd = new RenderVariableDeviceData(dd->GetShader()->GetUniformIterator(urvd->GetUniformTypeAsString()));
					rvd->SetDisplayData(DisplayDeviceDataPtr(rvdd));
			}
		}

		// Process attribute render variables here
		for(auto& rv : def.GetAttributeRenderVars()) {
			for(auto& rvd : rv->VariableDescritionList()) {
				auto& arvd = std::dynamic_pointer_cast<Render::AttributeRenderVariableDesc>(rvd);
				ASSERT_LOG(arvd != NULL, "RenderVariableDesc was wrong type, couldn't cast to AttributeRenderVariableDesc.");
				auto rvdd = new RenderVariableDeviceData(dd->GetShader()->GetAttributeIterator(arvd->GetVertexTypeAsString()));
				rvd->SetDisplayData(DisplayDeviceDataPtr(rvdd));
			}
		}
		return DisplayDeviceDataPtr(dd);
	}

	void DisplayDeviceOpenGL::Render(const Render::RenderablePtr& r)
	{
		auto dd = std::dynamic_pointer_cast<OpenGLDeviceData>(r->GetDisplayData());
		ASSERT_LOG(dd != NULL, "Failed to cast display data to the type required(OpenGLDeviceData).");
		auto shader = dd->GetShader();
		shader->MakeActive();

		// lighting can be switched on or off at a material level.
		// so we grab the return of the Material::Apply() function
		// to find whether to apply it or not.
		bool use_lighting = true;
		if(r->Material()) {
			use_lighting = r->Material()->Apply();
		}

		glm::mat4 pmat(1.0f);
		if(r->Camera()) {
			// set camera here.
			pmat = r->Camera()->ProjectionMat() * r->Camera()->ViewMat();
		}

		if(use_lighting) {
			for(auto lp : r->Lights()) {
				/// xxx need to set lights here.
			}
		}

		pmat *= r->ModelMatrix();
		shader->SetUniformValue(shader->GetMvpUniform(), glm::value_ptr(pmat));

		// Loop through uniform render variables and set them.
		for(auto& urv : r->UniformRenderVariables()) {
			for(auto& rvd : urv->VariableDescritionList()) {
				auto rvdd = std::dynamic_pointer_cast<RenderVariableDeviceData>(rvd->GetDisplayData());
				ASSERT_LOG(rvdd != NULL, "Unable to cast DeviceData to RenderVariableDeviceData.");
				shader->SetUniformValue(rvdd->GetActiveMapIterator(), urv->Value());
			}
		}

		// Need to figure the interaction with shaders.
		/// XXX Need to create a mapping between attributes and the index value below.
		for(auto arv : r->AttributeRenderVariables()) {
			GLenum draw_mode = ConvertDrawingMode(arv->GetDrawMode());
			std::vector<GLuint> enabled_attribs;

			for(auto& rvd : arv->VariableDescritionList()) {
				auto rvdd = std::dynamic_pointer_cast<RenderVariableDeviceData>(rvd->GetDisplayData());
				ASSERT_LOG(rvdd != NULL, "Unable to cast DeviceData to RenderVariableDeviceData.");
				auto& arvd = std::dynamic_pointer_cast<Render::AttributeRenderVariableDesc>(rvd);
				ASSERT_LOG(arvd != NULL, "RenderVariableDesc was wrong type, couldn't cast to AttributeRenderVariableDesc.");

				glEnableVertexAttribArray(rvdd->GetActiveMapIterator()->second.location);
				glVertexAttribPointer(rvdd->GetActiveMapIterator()->second.location, 
					arvd->NumElements(), 
					ConvertRenderVariableType(arvd->GetVariableType()), 
					arvd->Normalised(), 
					arvd->Stride(), 
					(void*)((intptr_t)arv->Value() + (intptr_t) + arvd->Offset()));
				enabled_attribs.emplace_back(rvdd->GetActiveMapIterator()->second.location);
			}

			if(arv->IsIndexedDraw()) {
				// XXX
				//glDrawElements(draw_mode, rv->Count(), rv->IndexType(), rv->Indicies());
			} else {
				// XXX we probably need to grab an offset parameter, from rv, instead of the 0 below.
				glDrawArrays(draw_mode, 0, arv->Count());
			}
			for(auto attrib : enabled_attribs) {
				glDisableVertexAttribArray(attrib);
			}
		}
		if(r->Material()) {
			r->Material()->Unapply();
		}
	}

	TexturePtr DisplayDeviceOpenGL::CreateTexture(const SurfacePtr& surface, const variant& node)
	{
		return TexturePtr(new OpenGLTexture(surface, node));
	}

	TexturePtr DisplayDeviceOpenGL::CreateTexture(const SurfacePtr& surface, Texture::TextureType type, int mipmap_levels)
	{
		return TexturePtr(new OpenGLTexture(surface, type, mipmap_levels));
	}

	TexturePtr DisplayDeviceOpenGL::CreateTexture(unsigned width, PixelFormat::PixelFormatConstant fmt)
	{
		return TexturePtr(new OpenGLTexture(width, 0, fmt, Texture::TEXTURE_1D));
	}

	TexturePtr DisplayDeviceOpenGL::CreateTexture(unsigned width, unsigned height, PixelFormat::PixelFormatConstant fmt, Texture::TextureType type)
	{
		return TexturePtr(new OpenGLTexture(width, height, fmt, Texture::TEXTURE_2D));
	}
	
	TexturePtr DisplayDeviceOpenGL::CreateTexture(unsigned width, unsigned height, unsigned depth, PixelFormat::PixelFormatConstant fmt)
	{
		return TexturePtr(new OpenGLTexture(width, height, fmt, Texture::TEXTURE_3D, depth));
	}

	MaterialPtr DisplayDeviceOpenGL::CreateMaterial(const variant& node)
	{
		return MaterialPtr(new OpenGLMaterial(node));
	}

	MaterialPtr DisplayDeviceOpenGL::CreateMaterial(const std::string& name, 
		const std::vector<TexturePtr>& textures, 
		const BlendMode& blend, 
		bool fog, 
		bool lighting, 
		bool depth_write, 
		bool depth_check)
	{
		return MaterialPtr(new OpenGLMaterial(name, textures, blend, fog, lighting, depth_write, depth_check));
	}

	RenderTargetPtr DisplayDeviceOpenGL::CreateRenderTarget(size_t width, size_t height, 
			size_t color_plane_count, 
			bool depth, 
			bool stencil, 
			bool use_multi_sampling, 
			size_t multi_samples)
	{
		return RenderTargetPtr(new FboOpenGL(width, height, color_plane_count, depth, stencil, use_multi_sampling, multi_samples));
	}

	RenderTargetPtr DisplayDeviceOpenGL::CreateRenderTarget(const variant& node)
	{
		return RenderTargetPtr(new FboOpenGL(node));
	}

	// XXX Need a way to deal with blits with Camera/Lighting.
	void DisplayDeviceOpenGL::BlitTexture(const TexturePtr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch)
	{
		auto texture = std::dynamic_pointer_cast<OpenGLTexture>(tex);
		ASSERT_LOG(texture != NULL, "Texture passed in was not of expected type.");

		const float tx1 = float(srcx) / texture->Width();
		const float ty1 = float(srcy) / texture->Height();
		const float tx2 = srcw == 0 ? 1.0f : float(srcx + srcw) / texture->Width();
		const float ty2 = srch == 0 ? 1.0f : float(srcy + srch) / texture->Height();
		const float uv_coords[] = {
			tx1, ty1,
			tx2, ty1,
			tx1, ty2,
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
		glm::mat4 mvp = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f) * model;
		auto shader = Shader::ShaderProgram::DefaultSystemShader();
		shader->MakeActive();
		texture->Bind();
		shader->SetUniformValue(shader->GetMvpUniform(), glm::value_ptr(mvp));
		shader->SetUniformValue(shader->GetColorUniform(), glm::value_ptr(glm::vec4(1.0f,1.0f,1.0f,1.0f)));
		shader->SetUniformValue(shader->GetTexMapUniform(), 0);
		// XXX the following line are only temporary, obviously.
		shader->SetUniformValue(shader->GetUniformIterator("discard"), 0);
		glEnableVertexAttribArray(shader->GetVertexAttribute()->second.location);
		glVertexAttribPointer(shader->GetVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vtx_coords);
		glEnableVertexAttribArray(shader->GetTexcoordAttribute()->second.location);
		glVertexAttribPointer(shader->GetTexcoordAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, uv_coords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(shader->GetTexcoordAttribute()->second.location);
		glDisableVertexAttribArray(shader->GetVertexAttribute()->second.location);
	}
}
