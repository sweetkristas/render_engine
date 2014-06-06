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

#ifndef _USE_MATH_DEFINES
#	define _USE_MATH_DEFINES	1
#endif 

#include <cmath>
#include <string>
#include <vector>

#include "color.hpp"
#include "scene_object.hpp"
#include "vector_graphics_fwd.hpp"
#include "util.hpp"

#ifndef M_PI
#	define M_PI	3.14159265358979323846
#endif

namespace KRE
{
	namespace vector
	{
		enum FillRule {
			FILL_RULE_WINDING,
			FILL_RULE_EVEN_ODD,
		};

		enum LineCap {
			LINE_CAP_BUTT,
			LINE_CAP_ROUND,
			LINE_CAP_SQUARE,
		};

		enum LineJoin {
			LINE_JOIN_MITER,
			LINE_JOIN_ROUND,
			LINE_JOIN_BEVEL,
		};

		class path
		{
		public:
			path();
			path(const double x, const double y, bool relative=false);
			virtual ~path();

			virtual void move_to(const double x, const double y, const bool relative=false) = 0;
			virtual void line_to(const double x, const double y, const bool relative=false) = 0;

			// Helper function equivalent to drawing an arc between 0.0 and 2*M_PI
			virtual void circle(const double x, const double y, const double r) = 0;
			virtual void line(const double x1, const double y1, const double x2, const double y2) = 0;
			virtual void rectangle(const double x, const double y, const double width, const double height) = 0;

			virtual void arc(const double cx, const double cy, const double radius, const double start_angle, const double end_angle, bool negative=false) = 0;
				
			// Adds a Cubic Bézier curve to the current path from the current position to the end position
			// (ex,ey) using the control points (x1,y1) and (x2,y2)
			// If relative is true then the curve is drawn with all positions relative to the current point.
			virtual void cubic_curve_to(const double x1, const double y1, const double x2, const double y2, const double ex, const double ey, bool relative=false) = 0;
			// Adds a Quadratic Bézier curve to the current path from the current position to the end position
			// (ex,ey) using the control point (x1,y1)
			// If relative is true then the curve is drawn with all positions relative to the current point.
			virtual void quadratic_curve_to(const double x1, const double y1, const double ex, const double ey, bool relative=false) = 0;

			//virtual void glyph_pPath(const std::vector<glyph>& g);
			virtual void text_path(const std::string& s) = 0;

			virtual void close_path() = 0;
		private:
			DISALLOW_COPY_AND_ASSIGN(path);
		};


		class context : public scene_object
		{
		public:
			context(int width, int height);
			virtual ~context();

			int width() const { return width_; }
			int height() const { return height_; }
				
			virtual void save() = 0;
			virtual void restore() = 0;

			virtual void push_group() = 0;
			virtual void pop_group(const bool to_source=false) = 0;

			virtual void set_source_color(const double r, const double g, const double b, const double a=1.0) = 0;
			virtual void set_source_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255) = 0;
			virtual void set_source_color(const double r, const color& color) = 0;
			virtual void set_source(const pattern_ptr& p) = 0;
			virtual pattern_ptr get_source() const = 0;

			virtual void set_fill_rule(const FillRule fr) = 0;
			virtual FillRule get_fill_rule() const = 0;

			virtual void set_line_cap(const LineCap lc) = 0;
			virtual LineCap get_line_cap() const = 0;

			virtual void set_line_join(const LineJoin lj) = 0;
			virtual LineJoin get_line_join() const = 0;

			virtual void set_line_width(const double width) = 0;
			virtual double get_line_width() const = 0;

			virtual void set_miter_limit(const double limit) = 0;
			virtual double get_miter_limit() const = 0;

			virtual void set_dash_style(const std::vector<double>& dashes, const double offset=0) = 0;
			virtual const std::vector<double> get_dash_style() const = 0;
			virtual void set_dash_offset(double offset) = 0;
			virtual double get_dash_offset() const = 0;
				
			virtual void paint(const double alpha=1.0) = 0;

			virtual void fill(const bool preserve=false) = 0;
			virtual void fill_extents(double& x1, double& y1, double& x2, double& y2) = 0;
			virtual bool in_fill(const double x, const double y) = 0;

			virtual void stroke(const bool preserve=false) = 0;
			virtual void stroke_extents(double& x1, double& y1, double& x2, double& y2) = 0;
			virtual bool in_stroke(const double x, const double y) = 0;

			virtual void clip(const bool preserve=false) = 0;
			virtual void clip_extents(double& x1, double& y1, double& x2, double& y2) = 0;
			virtual bool in_clip(const double x, const double y) = 0;
			virtual void clip_reset() = 0;

			//virtual void set_antialiasing(const AntiAliasing aa);
			//virtual AntiAliasing get_antialiasing() const;

			virtual void get_current_point(double& x, double& y) = 0;
			virtual bool has_current_point() = 0;

			virtual path_ptr new_path() = 0;
			virtual void add_path(const path_ptr& path) = 0;
			virtual void add_sub_path(const path_ptr& path) = 0;

			virtual void path_extents(double& x1, double& y1, double& x2, double& y2) = 0;

			// Sometimes we may wish to draw some vector graphics in the context of the 2D Canvas
			// We can do so through this member function.
			// XXX should maybe make this a little more abstracted so that it seperates concerns
			// between drawing as a scene_object and Drawing to the Canvas.
			//virtual void CanvasDraw() const = 0;

			static context_ptr create_instance(const std::string& hint, int width, int height);
		protected:
			context();
		private:
			DISALLOW_COPY_AND_ASSIGN(context);

			int width_;
			int height_;
		};
	}
}
