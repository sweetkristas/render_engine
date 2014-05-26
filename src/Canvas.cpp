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

#include "canvas.hpp"
#include "DisplayDevice.hpp"

namespace KRE
{
	canvas::canvas()
		: width_(0),
		height_(0)
	{
	}

	void canvas::set_dimensions(unsigned w, unsigned h)
	{
		width_ = w;
		height_ = h;
		handle_dimensions_changed();
	}

	canvas::~canvas()
	{
	}

	canvas_ptr canvas::get_instance()
	{
		return DisplayDevice::GetCurrent()->GetCanvas();
	}
}