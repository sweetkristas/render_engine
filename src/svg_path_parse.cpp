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

/*
	From www.w3.org/TR/SVG/implnote.html#PathElementImplementationNotes

The S/s commands indicate that the first control point of the given cubic Bézier segment 
is calculated by reflecting the previous path segments second control point relative to 
the current point. The exact math is as follows. If the current point is (curx, cury) 
and the second control point of the previous path segment is (oldx2, oldy2), then the 
reflected point (i.e., (newx1, newy1), the first control point of the current path segment) is:

(newx1, newy1) = (curx - (oldx2 - curx), cury - (oldy2 - cury))
               = (2*curx - oldx2, 2*cury - oldy2)

*/

#include <iostream>
#include <list>

#include "asserts.hpp"
#include "svg_path_parse.hpp"

namespace svg
{
	const bool print_debug_info = false;

	path_command::path_command() : ins_(CLOSEPATH) {
		if(print_debug_info) {
			std::cerr << "XXX: Closepath" << std::endl;
		}
	}
	// move to/line to constructor
	path_command::path_command(PathInstruction ins, bool absolute, double x, double y) 
		: ins_(ins), 
		absolute_(absolute), 
		x_(x), y_(y) 
	{
		if(print_debug_info) {
			if(ins == MOVETO) {
				std::cerr << "XXX: MoveTo " << x << "," << y << (absolute ? " Abs" : " Rel") << std::endl;
			} else {
				std::cerr << "XXX: LineTo " << x << "," << y << std::endl;
			}
		}
	}
	// horz/vert line constructor
	path_command::path_command(PathInstruction ins, bool absolute, double v)
		: ins_(ins),
		absolute_(absolute),
		v_(v)
	{
		if(print_debug_info) {
			if(ins == LINETO_H) {
				std::cerr << "XXX: LineTo-H " << v << std::endl;
			} else {
				std::cerr << "XXX: LineTo-V " << v << std::endl;
			}
		}
	}
	// cubic bezier constructor
	path_command::path_command(bool absolute, bool smooth, double x, double y, double cp1x, double cp1y, double cp2x, double cp2y)
		: ins_(CUBIC_BEZIER), 
		absolute_(absolute), smooth_(smooth),
		x_(x), y_(y), 
		cp1x_(cp1x), cp1y_(cp1y), 
		cp2x_(cp2x), cp2y_(cp2y) 
	{
		if(print_debug_info) {
			if(smooth) {
				std::cerr << "XXX: CubicBezier " << x << "," << y << " " << cp1x << "," << cp1y << (absolute ? " Abs" : " Rel") << std::endl;
			} else {
				std::cerr << "XXX: CubicBezier " << x << "," << y << " " << cp1x << "," << cp1y << " " << cp2x << "," << cp2y << (absolute ? " Abs" : " Rel") << std::endl;
			}
		}
	}
	// quadratic bezier constructor
	path_command::path_command(bool absolute, bool smooth, double x, double y, double cp1x, double cp1y) 
		: ins_(QUADRATIC_BEZIER), 
		absolute_(absolute), smooth_(smooth),
		x_(x), y_(y), 
		cp1x_(cp1x), cp1y_(cp1y) 
	{
		if(print_debug_info) {
			if(smooth) {
				std::cerr << "XXX: QuadraticBezier " << x << "," << y << " " << (absolute ? " Abs" : " Rel") << std::endl;
			} else {
				std::cerr << "XXX: QuadraticBezier " << x << "," << y << " " << cp1x << "," << cp1y << (absolute ? " Abs" : " Rel") << std::endl;
			}
		}
	}
	// elliptical arc constructor
	path_command::path_command(bool absolute, double x, double y, double rx, double ry, double x_axis_rot, bool large_arc, bool sweep) 
		: ins_(ARC), absolute_(absolute), 
		x_(x), y_(y), 
		rx_(rx), ry_(ry), 
		x_axis_rotation_(x_axis_rot), 
		large_arc_flag_(large_arc), 
		sweep_flag_(sweep) 
	{
		if(print_debug_info) {
			std::cerr << "XXX: Arc " << x << "," << y << " " << rx << "," << ry << " " << x_axis_rot << " large-arc-flag:" << (large_arc?"true":"false") << " sweep-flag:" << (sweep?"true":"false") << (absolute ? " Abs" : " Rel") << std::endl;
		}
	}
	// Render to whatever back-end we have at the moment, on entry x&y are the current position.
	void path_command::render(float* x, float* y) 
	{
	}

	void path_command::cairo_render(cairo_t* cairo)
	{
		switch(ins_) {
			case MOVETO:
				if(absolute_) {
					cairo_move_to(cairo, x_, y_); 
				} else {
					cairo_rel_move_to(cairo, x_, y_);
				}
				break;
			case LINETO:
				if(absolute_) {
					cairo_line_to(cairo, x_, y_);
				} else {
					cairo_rel_line_to(cairo, x_, y_);
				}
				break;
			case LINETO_H:
				if(absolute_) {
					double cx, cy;
					cairo_get_current_point(cairo, &cx, &cy);
					cairo_line_to(cairo, v_, cy);
				} else {
					cairo_rel_line_to(cairo, v_, 0.0);
				}
				break;
			case LINETO_V:
				if(absolute_) {
					double cx, cy;
					cairo_get_current_point(cairo, &cx, &cy);
					cairo_line_to(cairo, cx, v_);
				} else {
					cairo_rel_line_to(cairo, 0.0, v_);
				}
				break;
			case CLOSEPATH:
				cairo_close_path(cairo);
				break;
			case CUBIC_BEZIER:
				if(smooth_) {
					ASSERT_LOG(false, "XXX: Implement smooth bezier's");
					//cp1x_ = ?;
					//cp1y_ = ?;
				}
				if(absolute_) {
					cairo_curve_to(cairo, cp1x_, cp1y_, cp2x_, cp2y_, x_, y_);
				} else {
					cairo_rel_curve_to(cairo, cp1x_, cp1y_, cp2x_, cp2y_, x_, y_);
				}
				break;
			case QUADRATIC_BEZIER: {
				if(smooth_) {
					ASSERT_LOG(false, "XXX: Implement smooth bezier's");
					//cp1x_ = ?;
					//cp1y_ = ?;
				}
				double c0x, c0y;
				double dx, dy;
				double acp1x, acp1y;
				cairo_get_current_point(cairo, &c0x, &c0y);
				// Simple quadratic -> cubic conversion.
				dx = x_;
				dy = y_;
				acp1x = cp1x_;
				acp1y = cp1y_;
				if(!absolute_) {
					dx += c0x;
					dy += c0y;
					acp1x += c0x;
					acp1y += c0y;
				}
				double cp1x = c0x + 2.0/3.0 * (acp1x - c0x);
				double cp1y = c0y + 2.0/3.0 * (acp1y - c0y);
				double cp2x = dx + 2.0/3.0 * (acp1x - dx);
				double cp2y = dy + 2.0/3.0 * (acp1y - dy);

				cairo_curve_to(cairo, cp1x, cp1y, cp2x, cp2y, x_, y_);
				break;
			}
			case ARC:
				std::cerr << "XXX:ERROR: Need to implement arc drawing" << std::endl;
				break;
		}
		auto status = cairo_status(cairo);
		ASSERT_LOG(status == CAIRO_STATUS_SUCCESS, "Cairo error: " << cairo_status_to_string(status));
	}


	class path_parser
	{
	public:
		path_parser(const std::string& s) : path_(s.begin(), s.end()) {
			do {
				if(path_.empty()) {
					throw parsing_exception("Found empty string");
				}
			} while(match_wsp_opt());
			match_moveto_drawto_command_groups();
			while(match_wsp_opt()) {
			}
			if(path_.size() > 0) {
				throw parsing_exception(formatter() << "Input data left after parsing: " << std::string(path_.begin(), path_.end()));
			}
		}
		bool match_wsp_opt()
		{
			if(path_.empty()) {
				return false;
			}
			char c = path_.front();
			if(c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				path_.pop_front();
				return true;
			}
			return false;
		}
		void match_wsp_star()
		{
			while(match_wsp_opt()) {
			}
		}
		void match_wsp_star_or_die()
		{
			do {
				if(path_.empty()) {
					throw parsing_exception("Found empty string");
				}
			} while(match_wsp_opt());
		}
		bool match(char c)
		{
			if(path_.empty()) {
				return false;
			}
			if(path_.front() == c) {
				path_.pop_front();
				return true;
			}
			return false;
		}
		bool match_moveto_drawto_command_groups()
		{
			if(path_.empty()) {
				return false;
			}
			match_moveto_drawto_command_group();
			match_wsp_star();
			return match_moveto_drawto_command_groups();
		}
		bool match_moveto_drawto_command_group()
		{
			if(!match_moveto()) {
				return false;
			}
			match_wsp_star();
			return match_drawto_commands();
		}
		bool match_moveto()
		{
			if(path_.empty()) {
				return false;
			}
			// ( "M" | "m" ) wsp* moveto-argument-sequence
			char c = path_.front();
			if(c == 'M' || c == 'm') {
				path_.pop_front();
				match_wsp_star_or_die();
				match_moveto_argument_sequence(c == 'M' ? true : false);
			} else {
				throw parsing_exception("Expected 'M' or 'm'");
			}
			return true;
		}
		bool match_moveto_argument_sequence(bool absolute)
		{
			double x, y;
			match_coordinate_pair(x, y);
			// emit
			cmds_.emplace_back(new path_command(path_command::MOVETO, absolute, x, y));
			match_comma_wsp_opt();
			return match_lineto_argument_sequence(absolute);
		}
		bool match_lineto_argument_sequence(bool absolute)
		{
			double x, y;
			if(match_coordinate_pair(x, y)) {
				// emit
				cmds_.emplace_back(new path_command(path_command::LINETO, absolute, x, y));
				match_comma_wsp_opt();
				match_lineto_argument_sequence(absolute);
			}
			return true;
		}
		bool match_coordinate_pair(double& x, double& y)
		{
			if(!match_coordinate(x)) {
				return false;
			}
			match_comma_wsp_opt();
			if(!match_coordinate(y)) {
				throw parsing_exception(formatter() << "Expected a second co-ordinate while parsing value: " << std::string(path_.begin(), path_.end()));
			}
			return true;
		}
		bool match_coordinate(double& v)
		{
			return match_number(v);
		}
		bool match_number(double& d)
		{
			std::string s(path_.begin(), path_.end());
			char* end;
			d = strtod(s.c_str(), &end);
			if(errno == ERANGE) {
				throw parsing_exception(formatter() << "Decode of numeric value out of range. " << s);
			}
			if(d == 0 && end == s.c_str()) {
				// No number to convert.
				return false;
			}
			auto it = path_.begin();
			std::advance(it, end - s.c_str());
			path_.erase(path_.begin(), it);
			return true;
		}
		bool match_comma_wsp_opt()
		{
			if(path_.empty()) {
				return false;
			}
			char c = path_.front();
			if(c == ',') {
				path_.pop_front();
				match_wsp_star();
			} else {
				if(!match_wsp_opt()) {
					return true;
				}
				match_wsp_star();
				c = path_.front();
				if(c != ',') {
					//throw parsing_exception("Expected COMMA");
					return true;
				}
				path_.pop_front();
				match_wsp_star();
			}
			return true;
		}
		void match_comma_wsp_or_die() {
			if(!match_comma_wsp_opt()) {
				throw parsing_exception("End of string found");
			}
		}
		bool match_drawto_commands()
		{
			if(!match_drawto_command()) {
				return false;
			}
			match_wsp_star();
			return match_drawto_commands();
		}
		bool match_drawto_command()
		{
			if(path_.empty()) {
				return false;
			}
			char c = path_.front();
			if(c == 'M' || c == 'm') {
				return false;
			}
			path_.pop_front();
			switch(c) {
				case 'Z': case 'z': 
					cmds_.emplace_back(new path_command()); 
					break;
				case 'L':  case 'l': 
					match_wsp_star();
					match_lineto_argument_sequence(c == 'L' ? true : false);
					break;
				case 'H': case 'h':
					match_wsp_star();
					match_single_coordinate_argument_sequence(path_command::LINETO_H, c == 'H' ? true : false);
					break;
				case 'V': case 'v':
					match_wsp_star();
					match_single_coordinate_argument_sequence(path_command::LINETO_V, c == 'V' ? true : false);
					break;
				case 'C': case 'c': case 'S': case 's':
					match_wsp_star();
					match_curveto_argument_sequence(c=='C'||c=='S'?true:false, c=='S'||c=='s'?true:false);
					break;
				case 'Q': case 'q': case 'T': case 't':
					match_wsp_star();
					match_bezierto_argument_sequence(c=='Q'||c=='T'?true:false, c=='T'||c=='t'?true:false);
					break;
				case 'A': case 'a':
					match_arcto_argument_sequence(c == 'A' ? true : false);
					break;
				default:
					throw parsing_exception(formatter() << "Unrecognised draw-to symbol: " << c);
			}			
			return true;
		}
		bool match_single_coordinate_argument_sequence(path_command::PathInstruction ins, bool absolute)
		{
			double v;
			if(!match_coordinate(v)) {
				return false;
			}
			// emit
			cmds_.emplace_back(new path_command(ins, absolute, v));
			match_wsp_star();
			return match_single_coordinate_argument_sequence(ins, absolute);
		}
		bool match_curveto_argument_sequence(bool absolute, bool smooth)
		{
			double x, y;
			double cp1x, cp1y;
			double cp2x, cp2y;
			if(!match_curveto_argument(smooth, x, y, cp1x, cp1y, cp2x, cp2y)) {
				return false;
			}
			// emit
			cmds_.emplace_back(new path_command(absolute, smooth, x, y, cp1x, cp1y, cp2x, cp2y));
			match_wsp_star();
			return match_curveto_argument_sequence(absolute, smooth);
		}
		bool match_curveto_argument(bool smooth, double& x, double& y, double& cp1x, double& cp1y, double& cp2x, double& cp2y) 
		{
			if(!smooth) {
				if(!match_coordinate_pair(cp1x, cp1y)) {
					return false;
				}
				if(!match_comma_wsp_opt()) {
					throw parsing_exception("End of string found");
				}
			} else {
				cp1x = cp1y = 0;
			}
			if(!match_coordinate_pair(cp2x, cp2y)) {
				if(smooth) {
					return false;
				} else {
					throw parsing_exception(formatter() << "Expected first pair of control points in curve: " << std::string(path_.begin(), path_.end()));
				}
			}
			if(!match_comma_wsp_opt()) {
				throw parsing_exception("End of string found");
			}
			if(!match_coordinate_pair(x, y)) {
				throw parsing_exception(formatter() << "Expected second pair of control points in curve: " << std::string(path_.begin(), path_.end()));
			}
			return true;
		}
		bool match_bezierto_argument_sequence(bool absolute, bool smooth)
		{
			double x, y;
			double cp1x, cp1y;
			if(!match_bezierto_argument(smooth, x, y, cp1x, cp1y)) {
				return false;
			}
			// emit
			cmds_.emplace_back(new path_command(absolute, smooth, x, y, cp1x, cp1y));
			match_wsp_star();
			return match_bezierto_argument_sequence(absolute, smooth);
		}
		bool match_bezierto_argument(bool smooth, double& x, double& y, double& cp1x, double& cp1y) 
		{
			if(smooth) {
				cp1x = cp1y = 0;
			} else {
				if(!match_coordinate_pair(cp1x, cp1y)) {
					return false;
				}
				if(!match_comma_wsp_opt()) {
					throw parsing_exception("End of string found");
				}
			}
			if(!match_coordinate_pair(x, y)) {
				if(smooth) {
					return false;
				} else {
					throw parsing_exception(formatter() << "Expected first pair of control points in curve: " << std::string(path_.begin(), path_.end()));
				}
			}
			return true;
		}
		bool match_arcto_argument_sequence(bool absolute)
		{
			double x, y;
			double rx, ry;
			double x_axis_rot;
			bool large_arc;
			bool sweep;
			if(!match_arcto_argument(x, y, rx, ry, x_axis_rot, large_arc, sweep)) {
				return false;
			}
			// emit
			cmds_.emplace_back(new path_command(absolute, x, y, rx, ry, x_axis_rot, large_arc, sweep));
			match_wsp_star();
			return match_arcto_argument_sequence(absolute);
		}
		bool match_arcto_argument(double& x, double& y, double& rx, double& ry, double& x_axis_rot, bool& large_arc, bool& sweep) 
		{
			if(!match_coordinate(rx)) {
				return false;
			}
			if(rx < 0) {
				throw parsing_exception(formatter() << "While parsing elliptic arc command found negative RX value: " << rx);
			}
			match_comma_wsp_or_die();
			if(!match_coordinate(ry)) {
				throw parsing_exception("Unmatched RY value while parsing elliptic arc command");
			}
			if(ry < 0) {
				throw parsing_exception(formatter() << "While parsing elliptic arc command found negative RY value: " << y);
			}
			match_comma_wsp_or_die();
			if(!match_coordinate(x_axis_rot)) {
				throw parsing_exception("Unmatched x_axis_rotation value while parsing elliptic arc command");
			}
			match_comma_wsp_or_die();
			double large_arc_flag;
			if(!match_number(large_arc_flag)) {
				throw parsing_exception("Unmatched large_arc_flag value while parsing elliptic arc command");
			}
			large_arc = large_arc_flag > 0 ? true : false;
			match_comma_wsp_or_die();
			double sweep_flag;
			if(!match_number(sweep_flag)) {
				throw parsing_exception("Unmatched sweep_flag value while parsing elliptic arc command");
			}
			sweep = sweep_flag > 0 ? true : false;
			match_comma_wsp_or_die();
			if(!match_coordinate_pair(x, y)) {
				throw parsing_exception(formatter() << "Expected X,Y points in curve: " << std::string(path_.begin(), path_.end()));
			}
			return true;
		}
		const std::vector<path_command_ptr>& get_command_list() const { return cmds_; }
	private:
		std::list<char> path_;
		std::vector<path_command_ptr> cmds_;
	};

	std::vector<path_command_ptr> parse_path(const std::string& s)
	{
		path_parser pp(s);
		return pp.get_command_list();
	}

}