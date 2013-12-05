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

#include "GL/gl.h"
#include "DisplayDeviceOpenGL.hpp"

namespace graphics
{
	DisplayDeviceOpenGL::DisplayDeviceOpenGL()
	{
	}

	DisplayDeviceOpenGL::~DisplayDeviceOpenGL()
	{
	}

	void DisplayDeviceOpenGL::init(size_t width, size_t height)
	{
		glViewPort(0, 0, width, height);
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

	void DisplayDeviceOpenGL::swap()
	{
		// This is a no-action.
	}
}
