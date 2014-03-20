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

#include <cstdint>

namespace KRE
{
	class Color
	{
	public:
		Color();
		~Color();
		Color(const double r, const double g, const double b, const double a=1.0);
		Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255);

		double r() const { return color_[0]; }
		double g() const { return color_[1]; }
		double b() const { return color_[2]; }
		double a() const { return color_[3]; }

		double red() const { return color_[0]; }
		double green() const { return color_[1]; }
		double blue() const { return color_[2]; }
		double alpha() const { return color_[3]; }
		
		const float* AsFloatVector() const {
			return color_;
		}

	private:
		float color_[4];
	};
}
