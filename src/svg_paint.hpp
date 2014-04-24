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
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace svg
{
	class paint
	{
	public:
		paint(const std::string& s);
		virtual ~paint();
		bool has_color() const { return !no_color_; }
		uint8_t r() const { return r_; }
		uint8_t g() const { return g_; }
		uint8_t b() const { return b_; }
		uint8_t a() const { return a_; }
	private:
		uint8_t r_;
		uint8_t g_;
		uint8_t b_;
		uint8_t a_;
		bool no_color_;
	};

	typedef std::shared_ptr<paint> paint_ptr;
}
