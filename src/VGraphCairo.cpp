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

		void CairoContext::PopGroup(const bool to_source=false)
		{
			cairo_pop_group(context_);
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
			auto pattern = std::dynamic_pointer_cast<CairoPattern>(p);
			pattern = 
		}

		PatternPtr CairoContext::GetSource() const
		{
		}


		void CairoContext::SetFillRule(const FillRule fr)
		{
		}

		FillRule CairoContext::GetFillRule() const
		{
		}


		void CairoContext::SetLineCap(const LineCap lc)
		{
		}

		LineCap CairoContext::GetLineCap() const
		{
		}


		void CairoContext::SetLineJoin(const LineJoin lj)
		{
		}

		LineJoin CairoContext::GetLineJoin() const
		{
		}


		void CairoContext::SetLineWidth(const double width)
		{
		}

		double CairoContext::GetLineWidth() const
		{
		}


		void CairoContext::SetMiterLimit(const double limit)
		{
		}

		double CairoContext::GetMiterLimit() const
		{
		}


		void CairoContext::SetDashStyle(const std::vector<double>& dashes, const double offset=0)
		{
		}

		const std::vector<double>& CairoContext::GetDashStyle()
		{
		}

		void CairoContext::SetDashOffset(double offset)
		{
		}

		double CairoContext::GetDashOffset() const
		{
		}

				
		void CairoContext::Paint(const double alpha=1.0)
		{
		}


		void CairoContext::Fill(const bool preserve=false)
		{
		}

		void CairoContext::FillExtents(double& x1, double& y1, double& x2, double& y2)
		{
		}

		bool CairoContext::InFill(const double x, const double y)
		{
		}


		void CairoContext::Stroke(const bool preserve=false)
		{
		}

		void CairoContext::StrokeExtents(double& x1, double& y1, double& x2, double& y2)
		{
		}

		bool CairoContext::InStroke(const double x, const double y)
		{
		}


		void CairoContext::Clip(const bool preserve=false)
		{
		}

		void CairoContext::ClipExtents(double& x1, double& y1, double& x2, double& y2)
		{
		}

		bool CairoContext::InClip(const double x, const double y)
		{
		}

		void CairoContext::ClipReset()
		{
		}


		//void set_antialiasing(const AntiAliasing aa)
		//AntiAliasing get_antialiasing() const


		void CairoContext::GetCurrentPoint(double& x, double& y)
		{
		}

		bool CairoContext::HasCurrentPoint()
		{
		}

	}
}
