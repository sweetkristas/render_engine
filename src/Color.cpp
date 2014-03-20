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

#include "Color.hpp"

namespace KRE
{
	Color::Color()
	{
		color_[0] = 1.0f;
		color_[1] = 1.0f;
		color_[2] = 1.0f;
		color_[3] = 1.0f;
	}

	Color::~Color()
	{
	}

	Color::Color(const double r, const double g, const double b, const double a)
	{
		color_[0] = float(r);
		color_[1] = float(g);
		color_[2] = float(b);
		color_[3] = float(a);
	}

	Color::Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
	{
		color_[0] = r/255.0f;
		color_[1] = g/255.0f;
		color_[2] = b/255.0f;
		color_[3] = a/255.0f;
	}
}