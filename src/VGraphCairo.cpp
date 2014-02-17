/*
	Copyright (C) 2003-2013 by Kristina Simpson <sweet.kristas@gmail.com>
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented
		{
		}
 you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgement in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#include "asserts.hpp"
#include "logger.hpp"
#include "VGraphCairo.hpp"

namespace Graphics
{
	namespace Vector
	{
		CairoContext::CairoContext(int width, int height)
		{
			surface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
			auto status = cairo_surface_status(surface_);
			ASSERT_LOG(status == CAIRO_STATUS_SUCCESS, "Unable to create cairo surface: " << cairo_status_to_string(status));

			context_ = cairo_create(surface_);
			status = cairo_status(context_);
			ASSERT_LOG(status == CAIRO_STATUS_SUCCESS, "Unable to create cairo instance: " << cairo_status_to_string(status));
		}

		CairoContext::~CairoContext()
		{
			cairo_destroy(context_);
			cairo_surface_destroy(surface_);
		}

		void CairoContext::Save()
		{
			cairo_save(context_);
		}

		void CairoContext::Restore()
		{
			cairo_restore(context_);
		}


		void CairoContext::PushGroup()
		{
			cairo_push_group(context_);
		}

		void CairoContext::PopGroup(const bool to_source)
		{
			if(to_source) {
				cairo_pop_group_to_source(context_);
			} else {
				cairo_pop_group(context_);
			}
		}


		void CairoContext::SetSourceColor(const double r, const double g, const double b, const double a)
		{
			cairo_set_source_rgba(context_, r, g, b, a);
		}

		void CairoContext::SetSourceColor(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
		{
			cairo_set_source_rgba(context_, r/255.0, g/255.0, b/255.0, a/255.0);
		}

		void CairoContext::SetSourceColor(const double r, const Color& color)
		{
			cairo_set_source_rgba(context_, color.r(), color.g(), color.b(), color.a());
		}

		void CairoContext::SetSource(const PatternPtr& p)
		{
			// XXX
			//auto pattern = std::dynamic_pointer_cast<CairoPattern>(p);
			//pattern = 
		}

		PatternPtr CairoContext::GetSource() const
		{
			// XXX 
			return PatternPtr();
		}


		void CairoContext::SetFillRule(const FillRule fr)
		{
			cairo_set_fill_rule(context_, cairo_fill_rule_t(fr));
		}

		FillRule CairoContext::GetFillRule() const
		{
			auto fr = cairo_get_fill_rule(context_);
			return FillRule(fr);
		}


		void CairoContext::SetLineCap(const LineCap lc)
		{
			cairo_set_line_cap(context_, cairo_line_cap_t(lc));
		}

		LineCap CairoContext::GetLineCap() const
		{
			auto lc = cairo_get_line_cap(context_);
			return LineCap(lc);
		}


		void CairoContext::SetLineJoin(const LineJoin lj)
		{
			cairo_set_line_join(context_, cairo_line_join_t(lj));
		}

		LineJoin CairoContext::GetLineJoin() const
		{
			return LineJoin(cairo_get_line_join(context_));
		}


		void CairoContext::SetLineWidth(const double width)
		{
			cairo_set_line_width(context_, width);
		}

		double CairoContext::GetLineWidth() const
		{
			return cairo_get_line_width(context_);
		}


		void CairoContext::SetMiterLimit(const double limit)
		{
			cairo_set_miter_limit(context_, limit);
		}

		double CairoContext::GetMiterLimit() const
		{
			return cairo_get_miter_limit(context_);
		}


		void CairoContext::SetDashStyle(const std::vector<double>& dashes, const double offset)
		{
			cairo_set_dash(context_, &dashes[0], dashes.size(), offset);
		}

		const std::vector<double> CairoContext::GetDashStyle() const
		{
			int cnt = cairo_get_dash_count(context_);
			std::vector<double> dashes(cnt);
			double offset;
			cairo_get_dash(context_, &dashes[0], &offset);
			return dashes;
		}

		void CairoContext::SetDashOffset(double offset)
		{
			// XXX
		}

		double CairoContext::GetDashOffset() const
		{
			int cnt = cairo_get_dash_count(context_);
			std::vector<double> dashes(cnt);
			double offset;
			cairo_get_dash(context_, &dashes[0], &offset);
			return offset;
		}

				
		void CairoContext::Paint(const double alpha)
		{
			cairo_paint_with_alpha(context_, alpha);
		}


		void CairoContext::Fill(const bool preserve)
		{
			if(preserve) {
				cairo_fill_preserve(context_);
			} else {
				cairo_fill(context_);
			}
		}

		void CairoContext::FillExtents(double& x1, double& y1, double& x2, double& y2)
		{
			cairo_fill_extents(context_, &x1, &y1, &x2, &y2);
		}

		bool CairoContext::InFill(const double x, const double y)
		{
			return cairo_in_fill(context_, x, y);
		}


		void CairoContext::Stroke(const bool preserve)
		{
			if(preserve) {
				cairo_stroke_preserve(context_);
			} else {
				cairo_stroke(context_);
			}
		}

		void CairoContext::StrokeExtents(double& x1, double& y1, double& x2, double& y2)
		{
			cairo_stroke_extents(context_, &x1, &y1, &x2, &y2);
		}

		bool CairoContext::InStroke(const double x, const double y)
		{
			return cairo_in_stroke(context_, x, y);
		}


		void CairoContext::Clip(const bool preserve)
		{
			if(preserve) {
				cairo_clip_preserve(context_);
			} else {
				cairo_clip(context_);
			}
		}

		void CairoContext::ClipExtents(double& x1, double& y1, double& x2, double& y2)
		{
			cairo_clip_extents(context_, &x1, &y1, &x2, &y2);
		}

		bool CairoContext::InClip(const double x, const double y)
		{
			return cairo_in_clip(context_, x, y);
		}

		void CairoContext::ClipReset()
		{
			cairo_reset_clip(context_);
		}


		//void set_antialiasing(const AntiAliasing aa)
		//AntiAliasing get_antialiasing() const


		void CairoContext::GetCurrentPoint(double& x, double& y)
		{
			cairo_get_current_point(context_, &x, &y);
		}

		bool CairoContext::HasCurrentPoint()
		{
			return cairo_has_current_point(context_);
		}

	}
}
