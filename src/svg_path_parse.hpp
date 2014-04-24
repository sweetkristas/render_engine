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

#include <cairo/cairo.h>
#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "formatter.hpp"

namespace svg
{
	class path_command
	{
	public:
		enum PathInstruction {
			MOVETO,
			LINETO,
			LINETO_H,
			LINETO_V,
			CLOSEPATH,
			CUBIC_BEZIER,
			QUADRATIC_BEZIER,
			ARC,
		};
		path_command();
		// move to/line to constructor
		path_command(PathInstruction ins, bool absolute, double x, double y);
		// horiz/vert lines constructor
		path_command(PathInstruction ins, bool absolute, double v);
		// cubic bezier constructor
		path_command(bool absolute, bool smooth, double x, double y, double cp1x, double cp1y, double cp2x, double cp2y);
		// quadratic bezier constructor
		path_command(bool absolute, bool smooth, double x, double y, double cp1x, double cp1y);
		// elliptical arc constructor
		path_command(bool absolute, double x, double y, double rx, double ry, double x_axis_rot, bool large_arc, bool sweep);
		// Render to whatever back-end we have at the moment, on entry x&y are the current position.
		// XXX replace x/y with a context
		void render(float* x, float* y);
		void cairo_render(cairo_t* t);
	private:
		PathInstruction ins_;
		bool absolute_;
		bool smooth_;
		double x_;
		double y_;
		double v_;
		// control point for cubic/quadratic bezier
		double cp1x_;
		double cp1y_;
		// control point for cubic bezier
		double cp2x_;
		double cp2y_;
		// arc ellipse radii
		double rx_;
		double ry_;
		// arc x axis rotation
		double x_axis_rotation_;
		bool large_arc_flag_;
		bool sweep_flag_;
	};
	typedef std::shared_ptr<path_command> path_command_ptr;

	class parsing_exception : public std::exception
	{
	public:
		parsing_exception(const std::string& ss) : s_(ss) {}
		virtual ~parsing_exception() override {}
		virtual const char* what() const override {
			return s_.c_str();
		}
	private:
		std::string s_;
	};

	std::vector<path_command_ptr> parse_path(const std::string& s);
}
