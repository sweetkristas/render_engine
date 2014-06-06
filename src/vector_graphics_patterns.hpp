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
#include "color.hpp"
#include "vector_graphics_fwd.hpp"
#include "util.hpp"

namespace KRE
{
	namespace vector
	{
		enum class PatternType {
			PATTERN_TYPE_SOLID,
			PATTERN_TYPE_SURFACE,
			PATTERN_TYPE_LINEAR,
			PATTERN_TYPE_RADIAL,
			PATTERN_TYPE_MESH,
		};

		class pattern
		{
		public:
			virtual ~pattern();
			PatternType type() const { return type_; }
		protected:
			pattern(PatternType type);		
		private:
			DISALLOW_COPY_ASSIGN_AND_DEFAULT(pattern);

			PatternType type_;
		};

		class solid_pattern : public pattern
		{
		public:
			solid_pattern();
			solid_pattern(const double r, const double g, const double b, const double a=1.0);
			solid_pattern(const int r, const int g, const int b, const int a=255);
			virtual ~solid_pattern();
			double red() const { return color_.red(); }
			double green() const { return color_.green(); }
			double blue() const { return color_.blue(); }
			double alpha() const { return color_.alpha(); }
			const color& color() const { return color_; }
		private:
			DISALLOW_COPY_AND_ASSIGN(solid_pattern);

			color color_;
		};

		/*class surface_pattern : public pattern
		{
		public:
			surface_pattern(const SurfacePtr& surface);
			virtual ~surface_pattern();
		private:
		};*/

		typedef std::pair<double,color> color_stop;

		class linear_pattern : public pattern
		{
		public:
			linear_pattern(const double x1, const double y1, const double x2, const double y2);
			virtual ~linear_pattern();
			void add_color_stop(double offset, const color& color);
			void add_color_stop(double offset, const double r, const double g, const double b, const double a=1.0);
			void add_color_stop(double offset, const int r, const int g, const int b, const int a=255);
			const std::vector<color_stop>& get_color_stops() const { return color_stops_; }
		private:
			DISALLOW_COPY_ASSIGN_AND_DEFAULT(linear_pattern);
			double x1_;
			double y1_;
			double x2_;
			double y2_;
			std::vector<color_stop> color_stops_;
		};

		class radial_pattern : public pattern
		{
		public:
			radial_pattern(const double cx1, const double cy1, const double r1, const double cx2, const double cy2, const double r2);
			virtual ~radial_pattern();
			void add_color_stop(double offset, const color& color);
			void add_color_stop(double offset, const double r, const double g, const double b, const double a=1.0);
			void add_color_stop(double offset, const int r, const int g, const int b, const int a=255);
			const std::vector<color_stop>& get_color_stops() const { return color_stops_; }
		private:
			DISALLOW_COPY_ASSIGN_AND_DEFAULT(radial_pattern);
			std::vector<color_stop> color_stops_;
		};

		class mesh_patch
		{
		public:
			mesh_patch();
			virtual ~mesh_patch();
			void move_to(const double x, const double y);
			void line_to(const double x, const double y);
			void curve_to(const double x1, const double y1, const double x2, const double y2, const double ex, const double ey);
			void set_control_point(const size_t n, const double x, const double y);
			void set_corner_color(const size_t corner, const double r, const double g, const double b, const double a=1.0);
			void set_corner_color(const size_t corner, const int r, const int g, const int b, const int a=255);
			void set_corner_color(const size_t corner, const color& color);
		private:
			DISALLOW_COPY_AND_ASSIGN(mesh_patch);
		};
		typedef std::shared_ptr<mesh_patch> mesh_patch_ptr;

		class mesh_pattern : public pattern
		{
		public:
			mesh_pattern();
			virtual ~mesh_pattern();
			void add_patch(const mesh_patch_ptr& patch);
		private:
			DISALLOW_COPY_AND_ASSIGN(mesh_pattern);
			std::vector<mesh_patch_ptr> patches_;
		};
	}
}
