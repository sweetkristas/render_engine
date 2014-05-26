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

#include "asserts.hpp"
#include "vector_graphics.hpp"
#include "vector_graphics_cairo.hpp"
#include "vector_graphics_ogl.hpp"
#include "vector_graphics_oglf.hpp"

namespace KRE
{
	namespace vector
	{
		context::context()
			: scene_object("vector::context")
		{
		}

		context::context(int width, int height)
			: scene_object("vector::context"), 
			width_(width), height_(height)
			
		{
		}

		context::~context()
		{
		}

		context_ptr context::create_instance(const std::string& hint, int width, int height)
		{
			if(hint == "cairo") {
				return context_ptr(new cairo_context(width, height));
			} else if(hint == "opengl") {
				// XXX
				// return context_ptr(new opengl_context(width, height));
			} else if(hint == "opengl-fixed") {
				// XXX
				// return context_ptr(new opengl_fixed_context(width, height));
			} else {
				ASSERT_LOG(false, "Unrecognised hint to create vector graphics instance: " << hint);
			}
			return context_ptr();
		}

		path::path()
		{
		}

		path::~path()
		{
		}
	}
}
