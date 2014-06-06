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

#include <cairo/cairo.h>

#include "geometry.hpp"
#include "vector_graphics.hpp"

namespace KRE
{
	namespace vector
	{
		class cairo_path;

		class cairo_context : public context
		{
		public:
			cairo_context(int width, int height);
			virtual ~cairo_context();

			virtual void save() override;
			virtual void restore() override;

			virtual void push_group() override;
			virtual void pop_group(const bool to_source=false) override;

			virtual void set_source_color(const double r, const double g, const double b, const double a=1.0) override;
			virtual void set_source_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255) override;
			virtual void set_source_color(const double r, const color& color) override;
			virtual void set_source(const pattern_ptr& p) override;
			virtual pattern_ptr get_source() const override;

			virtual void set_fill_rule(const FillRule fr) override;
			virtual FillRule get_fill_rule() const override;

			virtual void set_line_cap(const LineCap lc) override;
			virtual LineCap get_line_cap() const override;

			virtual void set_line_join(const LineJoin lj) override;
			virtual LineJoin get_line_join() const override;

			virtual void set_line_width(const double width) override;
			virtual double get_line_width() const override;

			virtual void set_miter_limit(const double limit) override;
			virtual double get_miter_limit() const override;

			virtual void set_dash_style(const std::vector<double>& dashes, const double offset=0) override;
			virtual const std::vector<double> get_dash_style() const override;
			virtual void set_dash_offset(double offset) override;
			virtual double get_dash_offset() const override;
				
			virtual void paint(const double alpha=1.0) override;

			virtual void fill(const bool preserve=false) override;
			virtual void fill_extents(double& x1, double& y1, double& x2, double& y2) override;
			virtual bool in_fill(const double x, const double y) override;

			virtual void stroke(const bool preserve=false) override;
			virtual void stroke_extents(double& x1, double& y1, double& x2, double& y2) override;
			virtual bool in_stroke(const double x, const double y) override;

			virtual void clip(const bool preserve=false) override;
			virtual void clip_extents(double& x1, double& y1, double& x2, double& y2) override;
			virtual bool in_clip(const double x, const double y) override;
			virtual void clip_reset() override;

			//virtual void set_antialiasing(const AntiAliasing aa) override;
			//virtual AntiAliasing get_antialiasing() const override;

			virtual void get_current_point(double& x, double& y) override;
			virtual bool has_current_point() override;

			virtual void path_extents(double& x1, double& y1, double& x2, double& y2) override;

			virtual path_ptr new_path() override;
			virtual void add_path(const path_ptr& path) override;
			virtual void add_sub_path(const path_ptr& path) override;
		
			virtual void pre_render() override;

			virtual display_device_def attach(const display_device_ptr& dd);
		protected:
			cairo_t* GetContext() { return context_; }
			cairo_surface_t* get_surface() { return surface_; }
		private:
			cairo_t* context_;
			cairo_surface_t* surface_;

			std::vector<path_ptr> path_list_;

			rectf draw_rect_;
			texture_ptr tex_;
			std::shared_ptr<attribute<vertex_texcoord>> attribs_;

			friend class CairoPath;
		};
	}
}
