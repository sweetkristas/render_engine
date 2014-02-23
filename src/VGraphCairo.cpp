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
#include "WindowManager.hpp"

namespace Graphics
{
	namespace Vector
	{
		// XXX change this to generate and store a list of commands
		class CairoPath : public Path
		{
		public:
			CairoPath(CairoContext* context) : context_(context) {
				ASSERT_LOG(context_ != NULL, "Passed an null context");
			}
			virtual ~CairoPath() {
			}
			virtual void MoveTo(const double x, const double y, const bool relative=false) override {
				if(relative) {
					cairo_rel_move_to(context_->GetContext(), x, y);
				} else {
					cairo_move_to(context_->GetContext(), x, y);
				}
			}
			virtual void LineTo(const double x, const double y, const bool relative=false) override	{
				if(relative) {
					cairo_rel_line_to(context_->GetContext(), x, y);
				} else {
					cairo_line_to(context_->GetContext(), x, y);
				}
			}

			// Helper function equivalent to drawing an arc between 0.0 and 2*M_PI
			virtual void Circle(const double x, const double y, const double r) override {
				cairo_arc(context_->GetContext(), x, y, r, 0, 2*M_PI);
			}
			virtual void Line(const double x1, const double y1, const double x2, const double y2) override {
				cairo_move_to(context_->GetContext(), x1, y1);
				cairo_line_to(context_->GetContext(), x2, y2);
				cairo_close_path(context_->GetContext());
			}
			virtual void Rectangle(const double x, const double y, const double width, const double height) override {
				cairo_move_to(context_->GetContext(), x, y);
				cairo_rel_line_to(context_->GetContext(), width, 0);
				cairo_rel_line_to(context_->GetContext(), 0, height);
				cairo_rel_line_to(context_->GetContext(), -width, 0);
				cairo_close_path(context_->GetContext());
			}

			virtual void Arc(const double cx, const double cy, const double radius, const double start_angle, const double end_angle, bool negative=false) override {
				if(negative) {
					cairo_arc_negative(context_->GetContext(), cx, cy, radius, start_angle, end_angle);
				} else {
					cairo_arc(context_->GetContext(), cx, cy, radius, start_angle, end_angle);
				}
			}
				
			// Adds a Cubic Bézier curve to the current path from the current position to the end position
			// (ex,ey) using the control points (x1,y1) and (x2,y2)
			// If relative is true then the curve is drawn with all positions relative to the current point.
			virtual void CubicCurveTo(const double x1, const double y1, const double x2, const double y2, const double ex, const double ey, bool relative=false) override {
				if(relative) {
					cairo_rel_curve_to(context_->GetContext(), x1, y1, x2, y2, ex, ey);
				} else {
					cairo_curve_to(context_->GetContext(), x1, y1, x2, y2, ex, ey);
				}
			}
			// Adds a Quadratic Bézier curve to the current path from the current position to the end position
			// (ex,ey) using the control point (x1,y1)
			// If relative is true then the curve is drawn with all positions relative to the current point.
			virtual void QuadraticCurveTo(const double x1, const double y1, const double ex, const double ey, bool relative=false) override {
				ASSERT_LOG(context_->HasCurrentPoint(), "No current point defined.");
				double cx, cy;
				cairo_get_current_point(context_->GetContext(), &cx, &cy);

				double nx1 = x1;
				double ny1 = y1;
				double nex = ex;
				double ney = ey;
				if(relative) {
					nx1 += cx;
					ny1 += cy;
					nex += cx;
					ney += cy;
				}

				double cp1x = cx + (2.0*(nx1-cx))/3.0;
				double cp1y = cy + (2.0*(ny1-cy))/3.0;
				double cp2x = nex + (2.0*(nx1-nex))/3.0;
				double cp2y = ney + (2.0*(ny1-ney))/3.0;

				cairo_curve_to(context_->GetContext(), cp1x, cp1y, cp2x, cp2y, ex, ey);
			}

			//virtual void GlyphPath(const std::vector<Glyph>& g);
			//virtual void TextPath(const std::string& s);

			virtual void PathExtents(double& x1, double& y1, double& x2, double& y2) override {
				cairo_path_extents(context_->GetContext(), &x1, &y1, &x2, &y2);
			}

			virtual void ClosePath() override {
				cairo_close_path(context_->GetContext());
			}
		private:
			CairoContext* context_;
		};

		CairoContext::CairoContext(const WindowManagerPtr& wnd, int width, int height)
			: Context(width, height)
		{
			surface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
			auto status = cairo_surface_status(surface_);
			ASSERT_LOG(status == CAIRO_STATUS_SUCCESS, "Unable to create cairo surface: " << cairo_status_to_string(status));

			context_ = cairo_create(surface_);
			status = cairo_status(context_);
			ASSERT_LOG(status == CAIRO_STATUS_SUCCESS, "Unable to create cairo instance: " << cairo_status_to_string(status));

			int w = cairo_image_surface_get_width(surface_);
			int h = cairo_image_surface_get_height(surface_);
			auto fmt = cairo_image_surface_get_format(surface_);
			int stride = cairo_image_surface_get_stride(surface_);
			size_t bpp;
			size_t rmask, gmask, bmask, amask;
			switch(fmt) {
				case CAIRO_FORMAT_ARGB32:
					rmask = 0x00ff0000;
					gmask = 0x0000ff00;
					bmask = 0x000000ff;
					amask = 0xff000000;
					bpp = 32;
					break;
				case CAIRO_FORMAT_RGB24:
					rmask = 0x00ff0000;
					gmask = 0x0000ff00;
					bmask = 0x000000ff;
					amask = 0x00000000;
					bpp = 32;
					break;
				case CAIRO_FORMAT_A8:
					ASSERT_LOG(false, "CAIRO_FORMAT_A8 unsupported at this time");
					break;
				case CAIRO_FORMAT_A1:
					ASSERT_LOG(false, "CAIRO_FORMAT_A1 unsupported at this time");
					break;
				case CAIRO_FORMAT_RGB16_565:
					rmask = 0x0000f800;
					gmask = 0x000007e0;
					bmask = 0x0000001f;
					amask = 0x00000000;
					bpp = 16;
					break;
				case CAIRO_FORMAT_RGB30:
					rmask = 0x3ff00000;
					gmask = 0x000ffc00;
					bmask = 0x000003ff;
					amask = 0x00000000;
					bpp = 30;
					break;
				default:
					ASSERT_LOG(false, "Unrecognised cairo surface format: " << fmt);
			}
			auto surf = wnd->CreateSurface(w, h, bpp, stride, rmask, gmask, bmask, amask, cairo_image_surface_get_data(surface_));
			tex_ = wnd->GetDisplayDevice()->CreateTexture(surf, variant());
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
			return cairo_in_fill(context_, x, y) ? true : false;
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
			return cairo_in_stroke(context_, x, y) ? true : false;
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
			return cairo_in_clip(context_, x, y) ? true : false;
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
			return cairo_has_current_point(context_) ? true : false;
		}

		PathPtr CairoContext::NewPath()
		{
			return PathPtr(new CairoPath(this));
		}

		void CairoContext::AddPath(const PathPtr& path)
		{
		}

		void CairoContext::AddSubPath(const PathPtr& path)
		{
		}

		void CairoContext::Render(const WindowManagerPtr& wnd)
		{
			// XXX we should move this to a one off thing when creating the
			// texture then just stream it here. (even better only stream on change).
			// render to texture then draw that to wnd
			
			tex_->Update(0, 0, width(), height(), cairo_image_surface_get_stride(surface_), cairo_image_surface_get_data(surface_));
			wnd->BlitTexture(tex_, 0, 0, width(), height());
		}
	}
}
