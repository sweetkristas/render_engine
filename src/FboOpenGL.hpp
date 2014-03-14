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

#include <vector>
#include "RenderTarget.hpp"
#include "variant.hpp"

namespace Graphics
{
	struct vertex_texcoord
	{
		vertex_texcoord(const glm::vec2& v, const glm::vec2& tc) : vertex(v), texcoord(tc) {}
		glm::vec2 vertex;
		glm::vec2 texcoord;
	};

	class FboOpenGL : public Render::RenderTarget
	{
	public:
		explicit FboOpenGL(unsigned width, unsigned height, 
			unsigned color_plane_count=1, 
			bool depth=false, 
			bool stencil=false, 
			bool use_multi_sampling=false, 
			unsigned multi_samples=0);
		explicit FboOpenGL(const variant& node);
		virtual ~FboOpenGL();
		virtual void PreRender() override;
	private:
		virtual Graphics::DisplayDeviceDef Attach(const Graphics::DisplayDevicePtr& dd) override;
		virtual void HandleCreate() override;
		virtual void HandleApply() override;
		virtual void HandleUnapply() override;
		void GetDSInfo(GLenum& ds_attachment, GLenum& depth_stencil_internal_format);
		bool uses_ext_;
		std::shared_ptr<GLuint> depth_stencil_buffer_id_;
		std::shared_ptr<GLuint> framebuffer_id_;
		std::shared_ptr<GLuint> sample_framebuffer_id_;
		std::shared_ptr<std::vector<GLuint>> render_buffer_id_;

		std::shared_ptr<Render::AttributeRenderVariable<vertex_texcoord>> arv_;

		unsigned tex_width_;
		unsigned tex_height_;
		std::string shader_hint_;
		GLint viewport_[4];
	};
}
