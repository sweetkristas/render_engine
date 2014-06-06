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
#include "vector_graphics_cairo.hpp"
#include "display_device.hpp"

namespace KRE
{
	namespace vector
	{
		enum InstructionType {
			INS_UNKNOWN,
			INS_CLOSE_PATH,
			INS_MOVE_TO,
			INS_LINE_TO,
			INS_CURVE_TO,
			INS_QUAD_CURVE_TO,
			INS_ARC,
			INS_TEXT_PATH,
		};

		class path_instruction
		{
		public:
			virtual ~path_instruction() {}
			InstructionType get_type() const { return InstructionType(instruction_type_); }
			virtual void execute(cairo_t* context) = 0;
			virtual std::string as_string() const = 0;
		protected:
			path_instruction() {}
		private:
			enum {
				instruction_type_ = INS_UNKNOWN
			};
			path_instruction(const path_instruction&);
		};
		typedef std::shared_ptr<path_instruction> path_instructionPtr;

		class Closepath_instruction : public path_instruction
		{
		public:
			Closepath_instruction() {}
			virtual ~Closepath_instruction() {}
			void execute(cairo_t* context) override {
				cairo_close_path(context);
			}
			std::string as_string() const override { return "close_path"; }
		private:
			enum {
				instruction_type_ = INS_CLOSE_PATH
			};
		};

		class move_to_instruction : public path_instruction
		{
		public:
			move_to_instruction(const double x, const double y, bool relative=false) 
				: x_(x), y_(y), relative_(relative) {
			}
			virtual ~move_to_instruction() {}
			void execute(cairo_t* context) override {
				if(relative_) {
					cairo_rel_move_to(context, x_, y_);
				} else {
					cairo_move_to(context, x_, y_);
				}
			}
			std::string as_string() const override { return "move_to"; }
		private:
			enum {
				instruction_type_ = INS_MOVE_TO
			};
			double x_;
			double y_;
			bool relative_;
		};

		class line_to_instruction : public path_instruction
		{
		public:
			line_to_instruction(const double x, const double y, bool relative=false) 
				: x_(x), y_(y), relative_(relative) {
			}
			virtual ~line_to_instruction() {}
			void execute(cairo_t* context) override {
				if(relative_) {
					cairo_rel_line_to(context, x_, y_);
				} else {
					cairo_line_to(context, x_, y_);
				}
			}
			std::string as_string() const override { return "line_to"; }
		private:
			enum {
				instruction_type_ = INS_LINE_TO
			};
			double x_;
			double y_;
			bool relative_;
		};

		class arc_instruction : public path_instruction
		{
		public:
			arc_instruction(const double x, const double y, const double radius, const double start_angle, const double end_angle, bool negative=false) 
				: x_(x), y_(y), radius_(radius), start_angle_(start_angle), end_angle_(end_angle), negative_(negative) {
			}
			virtual ~arc_instruction() {}
			void execute(cairo_t* context) override {
				if(negative_) {
					cairo_arc_negative(context, x_, y_, radius_, start_angle_, end_angle_);
				} else {
					cairo_arc(context, x_, y_, radius_, start_angle_, end_angle_);
				}
			}
			std::string as_string() const override { return "arc"; }
		private:
			enum {
				instruction_type_ = INS_ARC
			};
			double x_;
			double y_;
			double radius_;
			double start_angle_;
			double end_angle_;
			bool negative_;
		};

		class cubic_curve_instruction : public path_instruction
		{
		public:
			cubic_curve_instruction(const double x1, const double y1, const double x2, const double y2, const double ex, const double ey, bool relative=false) 
				: cp_x1_(x1), cp_y1_(y1), cp_x2_(x2), cp_y2_(y2), ex_(ex), ey_(ey), relative_(relative) {
			}
			virtual ~cubic_curve_instruction() {}
			void execute(cairo_t* context) override {
				if(relative_) {
					cairo_rel_curve_to(context, cp_x1_, cp_y1_, cp_x2_, cp_y2_, ex_, ey_);
				} else {
					cairo_curve_to(context, cp_x1_, cp_y1_, cp_x2_, cp_y2_, ex_, ey_);
				}
			}
			std::string as_string() const override { return "cubic_bézier"; }
		private:
			enum {
				instruction_type_ = INS_CURVE_TO
			};
			// control point 1
			double cp_x1_;
			double cp_y1_;
			// control point 2
			double cp_x2_;
			double cp_y2_;
			// end point
			double ex_;
			double ey_;
			bool relative_;
		};

		class quadratic_curve_instruction : public path_instruction
		{
		public:
			quadratic_curve_instruction(const double x1, const double y1, const double ex, const double ey, bool relative=false) 
				: cp_x1_(x1), cp_y1_(y1), ex_(ex), ey_(ey), relative_(relative) {
			}
			virtual ~quadratic_curve_instruction() {}
			void execute(cairo_t* context) override {
				ASSERT_LOG(cairo_has_current_point(context) != 0, "No current point defined.");
				double cx, cy;
				cairo_get_current_point(context, &cx, &cy);

				double nx1 = cp_x1_;
				double ny1 = cp_y1_;
				double nex = ex_;
				double ney = ey_;
				if(relative_) {
					nx1 += cx;
					ny1 += cy;
					nex += cx;
					ney += cy;
				}

				double cp1x = cx + (2.0*(nx1-cx))/3.0;
				double cp1y = cy + (2.0*(ny1-cy))/3.0;
				double cp2x = nex + (2.0*(nx1-nex))/3.0;
				double cp2y = ney + (2.0*(ny1-ney))/3.0;

				cairo_curve_to(context, cp1x, cp1y, cp2x, cp2y, nex, ney);
			}
			std::string as_string() const override { return "quadratic_bézier"; }
		private:
			enum {
				instruction_type_ = INS_QUAD_CURVE_TO
			};
			// control point 1
			double cp_x1_;
			double cp_y1_;
			// end point
			double ex_;
			double ey_;
			bool relative_;
		};

		class textpath_instruction : public path_instruction
		{
		public:
			textpath_instruction(const std::string& text) : text_(text) {
			}
			virtual ~textpath_instruction() {
			}
			void execute(cairo_t* context) override {
				cairo_text_path(context, text_.c_str());
			}
			std::string as_string() const override { return "text_path"; }
		private:
			std::string text_;
		};

		// XXX change this to generate and store a list of commands
		class cairo_path : public path
		{
		public:
			cairo_path(cairo_context* context) : context_(context) {
				ASSERT_LOG(context_ != NULL, "Passed an null context");
			}
			virtual ~cairo_path() {
			}
			virtual void move_to(const double x, const double y, const bool relative=false) override {
				path_instructions_.emplace_back(new move_to_instruction(x, y, relative));
			}
			virtual void line_to(const double x, const double y, const bool relative=false) override	{
				path_instructions_.emplace_back(new line_to_instruction(x, y, relative));
			}

			// Helper function equivalent to drawing an arc between 0.0 and 2*M_PI
			virtual void circle(const double x, const double y, const double r) override {
				path_instructions_.emplace_back(new arc_instruction(x, y, r, 0.0, 2.0*M_PI));
			}
			virtual void line(const double x1, const double y1, const double x2, const double y2) override {
				path_instructions_.emplace_back(new move_to_instruction(x1, y1));
				path_instructions_.emplace_back(new line_to_instruction(x2, y2));
				path_instructions_.emplace_back(new Closepath_instruction());
			}
			virtual void rectangle(const double x, const double y, const double width, const double height) override {
				path_instructions_.emplace_back(new move_to_instruction(x, y));
				path_instructions_.emplace_back(new line_to_instruction(width, 0));
				path_instructions_.emplace_back(new line_to_instruction(0, height));
				path_instructions_.emplace_back(new line_to_instruction(-width, 0));
				path_instructions_.emplace_back(new Closepath_instruction());
			}

			virtual void arc(const double cx, const double cy, const double radius, const double start_angle, const double end_angle, bool negative=false) override {
				path_instructions_.emplace_back(new arc_instruction(cx, cy, radius, start_angle, end_angle, negative));
			}
				
			// Adds a Cubic Bézier curve to the current path from the current position to the end position
			// (ex,ey) using the control points (x1,y1) and (x2,y2)
			// If relative is true then the curve is drawn with all positions relative to the current point.
			virtual void cubic_curve_to(const double x1, const double y1, const double x2, const double y2, const double ex, const double ey, bool relative=false) override {
				path_instructions_.emplace_back(new cubic_curve_instruction(x1, y1, x2, y2, ex, ey, relative));
			}
			// Adds a Quadratic Bézier curve to the current path from the current position to the end position
			// (ex,ey) using the control point (x1,y1)
			// If relative is true then the curve is drawn with all positions relative to the current point.
			virtual void quadratic_curve_to(const double x1, const double y1, const double ex, const double ey, bool relative=false) override {
				path_instructions_.emplace_back(new quadratic_curve_instruction(x1, y1, ex, ey, relative));
			}

			//virtual void GlyphPath(const std::vector<Glyph>& g);
			virtual void text_path(const std::string& s) {
				path_instructions_.emplace_back(new textpath_instruction(s));
			}

			virtual void close_path() override {
				path_instructions_.emplace_back(new Closepath_instruction());
			}

			void execute(cairo_t* context) {
				std::cerr << "Executing path:";
				for(auto ins : path_instructions_) {
					std::cerr << " " << ins->as_string();
					ins->execute(context);
				}
				std::cerr << std::endl;
			}
		private:
			cairo_context* context_;
			std::vector<path_instructionPtr> path_instructions_;
		};

		cairo_context::cairo_context(int width, int height)
			: context(width, height),
			draw_rect_(0.0f, 0.0f, float(width), float(height))
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

			PixelFormat pffmt;
			switch(fmt) {
				case CAIRO_FORMAT_A8:
					ASSERT_LOG(false, "CAIRO_FORMAT_A8 unsupported at this time");
					break;
				case CAIRO_FORMAT_A1:
					ASSERT_LOG(false, "CAIRO_FORMAT_A1 unsupported at this time");
					break;
				case CAIRO_FORMAT_ARGB32:	pffmt = PixelFormat::PIXELFORMAT_ARGB8888;	break;
				case CAIRO_FORMAT_RGB24:	pffmt = PixelFormat::PIXELFORMAT_RGB888;	break;
				case CAIRO_FORMAT_RGB16_565:pffmt = PixelFormat::PIXELFORMAT_RGB565;	break;
				case CAIRO_FORMAT_RGB30:	pffmt = PixelFormat::PIXELFORMAT_RGB101010;	break;
				default:
					ASSERT_LOG(false, "Unrecognised cairo surface format: " << fmt);
			}
			tex_ = display_device::create_texture(w, h, pffmt);
			tex_->set_address_modes(texture::address_mode::CLAMP, texture::address_mode::CLAMP);
			auto mat = display_device::create_material("cairo_context", std::vector<texture_ptr>(1,tex_));
			set_material(mat);

			auto as = display_device::create_attribute_set();
			attribs_.reset(new attribute<vertex_texcoord>(AccessFreqHint::DYNAMIC, AccessTypeHint::DRAW));
			attribs_->set_attr_desc(attribute_desc(attribute_desc::Type::POSITION, 2, attribute_desc::VariableType::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, vtx)));
			attribs_->set_attr_desc(attribute_desc(attribute_desc::Type::TEXTURE,  2, attribute_desc::VariableType::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, tc)));
			as->add_attribute(attribute_base_ptr(attribs_));
			as->set_draw_mode(attribute_set::DrawMode::TRIANGLE_STRIP);
			add_attribute_set(as);

			float offs_x = 0.0f;
			float offs_y = 0.0f;
			offs_x = -draw_rect_.w()/2.0f;
			offs_y = -draw_rect_.h()/2.0f;
			// XXX we should only do this if things changed.
			const float vx1 = draw_rect_.x() + offs_x;
			const float vy1 = draw_rect_.y() + offs_y;
			const float vx2 = draw_rect_.x2() + offs_x;
			const float vy2 = draw_rect_.y2() + offs_y;

			rectf r = material()->GetNormalisedtextureCoords(material()->Gettexture().begin());

			std::vector<vertex_texcoord> vertices;
			vertices.emplace_back(glm::vec2(vx1,vy1), glm::vec2(r.x(),r.y()));
			vertices.emplace_back(glm::vec2(vx2,vy1), glm::vec2(r.x2(),r.y()));
			vertices.emplace_back(glm::vec2(vx1,vy2), glm::vec2(r.x(),r.y2()));
			vertices.emplace_back(glm::vec2(vx2,vy2), glm::vec2(r.x2(),r.y2()));
			get_attribute_set().back()->set_count(vertices.size());
			attribs_->update(&vertices);
		}

		cairo_context::~cairo_context()
		{
			cairo_destroy(context_);
			cairo_surface_destroy(surface_);
		}

		void cairo_context::save()
		{
			cairo_save(context_);
		}

		void cairo_context::restore()
		{
			cairo_restore(context_);
		}


		void cairo_context::push_group()
		{
			cairo_push_group(context_);
		}

		void cairo_context::pop_group(const bool to_source)
		{
			if(to_source) {
				cairo_pop_group_to_source(context_);
			} else {
				cairo_pop_group(context_);
			}
		}


		void cairo_context::set_source_color(const double r, const double g, const double b, const double a)
		{
			cairo_set_source_rgba(context_, r, g, b, a);
		}

		void cairo_context::set_source_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
		{
			cairo_set_source_rgba(context_, r/255.0, g/255.0, b/255.0, a/255.0);
		}

		void cairo_context::set_source_color(const double r, const color& color)
		{
			cairo_set_source_rgba(context_, color.r(), color.g(), color.b(), color.a());
		}

		void cairo_context::set_source(const pattern_ptr& p)
		{
			// XXX
			//auto pattern = std::dynamic_pointer_cast<CairoPattern>(p);
			//pattern = 
		}

		pattern_ptr cairo_context::get_source() const
		{
			// XXX 
			return pattern_ptr();
		}


		void cairo_context::set_fill_rule(const FillRule fr)
		{
			cairo_set_fill_rule(context_, cairo_fill_rule_t(fr));
		}

		FillRule cairo_context::get_fill_rule() const
		{
			auto fr = cairo_get_fill_rule(context_);
			return FillRule(fr);
		}


		void cairo_context::set_line_cap(const LineCap lc)
		{
			cairo_set_line_cap(context_, cairo_line_cap_t(lc));
		}

		LineCap cairo_context::get_line_cap() const
		{
			auto lc = cairo_get_line_cap(context_);
			return LineCap(lc);
		}


		void cairo_context::set_line_join(const LineJoin lj)
		{
			cairo_set_line_join(context_, cairo_line_join_t(lj));
		}

		LineJoin cairo_context::get_line_join() const
		{
			return LineJoin(cairo_get_line_join(context_));
		}


		void cairo_context::set_line_width(const double width)
		{
			cairo_set_line_width(context_, width);
		}

		double cairo_context::get_line_width() const
		{
			return cairo_get_line_width(context_);
		}


		void cairo_context::set_miter_limit(const double limit)
		{
			cairo_set_miter_limit(context_, limit);
		}

		double cairo_context::get_miter_limit() const
		{
			return cairo_get_miter_limit(context_);
		}


		void cairo_context::set_dash_style(const std::vector<double>& dashes, const double offset)
		{
			cairo_set_dash(context_, &dashes[0], dashes.size(), offset);
		}

		const std::vector<double> cairo_context::get_dash_style() const
		{
			int cnt = cairo_get_dash_count(context_);
			std::vector<double> dashes(cnt);
			double offset;
			cairo_get_dash(context_, &dashes[0], &offset);
			return dashes;
		}

		void cairo_context::set_dash_offset(double offset)
		{
			// XXX
		}

		double cairo_context::get_dash_offset() const
		{
			int cnt = cairo_get_dash_count(context_);
			std::vector<double> dashes(cnt);
			double offset;
			cairo_get_dash(context_, &dashes[0], &offset);
			return offset;
		}

				
		void cairo_context::paint(const double alpha)
		{
			cairo_paint_with_alpha(context_, alpha);
		}


		void cairo_context::fill(const bool preserve)
		{
			if(preserve) {
				cairo_fill_preserve(context_);
			} else {
				cairo_fill(context_);
			}
		}

		void cairo_context::fill_extents(double& x1, double& y1, double& x2, double& y2)
		{
			cairo_fill_extents(context_, &x1, &y1, &x2, &y2);
		}

		bool cairo_context::in_fill(const double x, const double y)
		{
			return cairo_in_fill(context_, x, y) ? true : false;
		}


		void cairo_context::stroke(const bool preserve)
		{
			if(preserve) {
				cairo_stroke_preserve(context_);
			} else {
				cairo_stroke(context_);
			}
		}

		void cairo_context::stroke_extents(double& x1, double& y1, double& x2, double& y2)
		{
			cairo_stroke_extents(context_, &x1, &y1, &x2, &y2);
		}

		bool cairo_context::in_stroke(const double x, const double y)
		{
			return cairo_in_stroke(context_, x, y) ? true : false;
		}


		void cairo_context::clip(const bool preserve)
		{
			if(preserve) {
				cairo_clip_preserve(context_);
			} else {
				cairo_clip(context_);
			}
		}

		void cairo_context::clip_extents(double& x1, double& y1, double& x2, double& y2)
		{
			cairo_clip_extents(context_, &x1, &y1, &x2, &y2);
		}

		bool cairo_context::in_clip(const double x, const double y)
		{
			return cairo_in_clip(context_, x, y) ? true : false;
		}

		void cairo_context::clip_reset()
		{
			cairo_reset_clip(context_);
		}


		//void set_antialiasing(const AntiAliasing aa)
		//AntiAliasing get_antialiasing() const


		void cairo_context::get_current_point(double& x, double& y)
		{
			cairo_get_current_point(context_, &x, &y);
		}

		bool cairo_context::has_current_point()
		{
			return cairo_has_current_point(context_) ? true : false;
		}

		path_ptr cairo_context::new_path()
		{
			return path_ptr(new cairo_path(this));
		}

		void cairo_context::add_path(const path_ptr& path)
		{
			auto cpath = std::dynamic_pointer_cast<cairo_path>(path);
			ASSERT_LOG(cpath != NULL, "Couldn't convert path to appropriate type CairoPath");
			cpath->execute(context_);
		}

		void cairo_context::add_sub_path(const path_ptr& path)
		{
			auto cpath = std::dynamic_pointer_cast<cairo_path>(path);
			ASSERT_LOG(cpath != NULL, "Couldn't convert path to appropriate type CairoPath");
			cairo_new_sub_path(context_);
			cpath->execute(context_);
		}

		void cairo_context::pre_render() 
		{
			std::vector<unsigned> stride (1, cairo_image_surface_get_width(surface_));
			tex_->update(0, 0, width(), height(), stride, cairo_image_surface_get_data(surface_));
		}

		void cairo_context::path_extents(double& x1, double& y1, double& x2, double& y2) 
		{
			cairo_path_extents(context_, &x1, &y1, &x2, &y2);
		}

		display_device_def cairo_context::attach(const display_device_ptr& dd)
		{
			display_device_def def(get_attribute_set()/*, get_uniform_set()*/);
			return def;
		}
	}
}
