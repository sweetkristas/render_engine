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
#include "FboOpenGL.hpp"

namespace Graphics
{
	FboOpenGL::FboOpenGL(size_t width, size_t height, 
		size_t color_plane_count, 
		bool depth, 
		bool stencil, 
		bool use_multi_sampling, 
		size_t multi_samples)
		: RenderTarget(width, height, color_plane_count, depth, stencil, use_multi_sampling, multi_samples),
		uses_ext_(false)
	{
		Init();
	}

	FboOpenGL::FboOpenGL(const variant& node)
		: RenderTarget(node),
		uses_ext_(false)
	{
		Init();
	}

	void FboOpenGL::Init()
	{
		// check for fbo support
		if(GLEW_ARB_framebuffer_object) {
			// XXX wip
		} else if(GLEW_EXT_framebuffer_object) {
			ASSERT_LOG(!(UsesMultiSampling() && !GLEW_EXT_framebuffer_multisample), "Multi-sample texture requested but hardware doesn't support multi-sampling.");
			ASSERT_LOG(!(DepthPlane() || StencilPlane() && !GLEW_EXT_packed_depth_stencil), "Depth or Stencil plane required but hardware doesn't support it.");
			uses_ext_ = true;
			// XXX wip
		}
	}

	FboOpenGL::~FboOpenGL()
	{
			// XXX wip
	}

	void FboOpenGL::Render()
	{
			// XXX wip
	}

	void FboOpenGL::SetAsRenderTarget()
	{
			// XXX wip
	}
}
