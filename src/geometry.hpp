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

#include <algorithm>
#include <vector>

#include "asserts.hpp"
#include "variant.hpp"
#include "SDL.h"

namespace geometry
{
	template<typename T>
	struct point_t {
		explicit point_t(T x=0, T y=0) : x(x), y(y)
		{}

		explicit point_t(const std::vector<T>& v);

		union {
			struct { T x, y; };
			T buf[2];
		};
	};

	template<typename T> inline
	bool operator==(const point_t<T>& a, const point_t<T>& b);
	template<typename T> inline
	bool operator!=(const point_t<T>& a, const point_t<T>& b);
	template<typename T> inline
	bool operator<(const point_t<T>& a, const point_t<T>& b);

	template<typename T>
	class rect_t
	{
	public:
		inline explicit rect_t(T x=0, T y=0, T w=0, T h=0);
		inline explicit rect_t(const SDL_Rect& r);
		explicit rect_t(const std::vector<T>& v);
		explicit rect_t(const std::string& s);
		explicit rect_t(const variant& v);
		static rect_t from_coordinates(T x1, T y1, T x2, T y2);

		void from_vector(const std::vector<T>& v) {
			switch(v.size()) {
				case 2:
					*this = rect_t<T>::from_coordinates(v[0], v[1], v[0], v[1]);
					break;
				case 3:
					*this = rect_t<T>::from_coordinates(v[0], v[1], v[2], v[1]);
					break;
				case 4:
					*this = rect_t<T>::from_coordinates(v[0], v[1], v[2], v[3]);
					break;
				default:
					*this = rect_t<T>();
					break;
			}
		}

		T x() const { return top_left_.x; }
		T y() const { return top_left_.y; }
		T x2() const { return bottom_right_.x; }
		T y2() const { return bottom_right_.y; }
		T w() const { return bottom_right_.x - top_left_.x; }
		T h() const { return bottom_right_.y - top_left_.y; }

		T mid_x() const { return (x() + x2())/2; }
		T mid_y() const { return (y() + y2())/2; }

		bool empty() const { return w() == 0 || h() == 0; }

		const point_t<T>& top_left() const { return top_left_; }
		const point_t<T>& bottom_right() const { return bottom_right_; }

		void operator+=(const point_t<T>& p) {
			top_left_.x += p.x;
			top_left_.y += p.y;
			bottom_right_.x += p.x;
			bottom_right_.y += p.y;
		}
		void operator-=(const point_t<T>& p) {
			top_left_.x -= p.x;
			top_left_.y -= p.y;
			bottom_right_.x -= p.x;
			bottom_right_.y -= p.y;
		}

		template<typename F>
		rect_t<F> as_type() const {
			return rect_t<F>::from_coordinates(F(top_left_.x), F(top_left_.y), F(bottom_right_.x), F(bottom_right_.y));
		}

		SDL_Rect sdl_rect() const {
			SDL_Rect r = {x(), y(), w(), h()};
			return r;
		}
	private:
		point_t<T> top_left_;
		point_t<T> bottom_right_;
	};

	template<> inline 
	rect_t<int>::rect_t(const variant& v)
	{
		if(v.is_list()) {
			std::vector<int> vec;
			for(size_t n = 0; n != v.num_elements(); ++n) {
				vec.push_back(int(v.as_int()));
			}
			from_vector(vec);
			return;
		} else if(v.is_map()) {
			ASSERT_LOG((v.has_key("x") && v.has_key("y") && v.has_key("w") && v.has_key("h"))
				|| v.has_key("x1") && v.has_key("y1") && v.has_key("x2") && v.has_key("y2"), 
				"map must have 'x','y','w','h' or 'x1','y1','x2','y2' attributes.");
			if(v.has_key("x")) {
				*this = rect_t<int>(int(v["x"].as_int()),int(v["y"].as_int()),int(v["w"].as_int()),int(v["h"].as_int()));
			} else {
				*this = rect_t<int>::from_coordinates(int(v["x1"].as_int()),int(v["y1"].as_int()),int(v["x2"].as_int()),int(v["y2"].as_int()));
			}
		} else {
			ASSERT_LOG(false, "Creating a rect from a variant must be list or map");
		}
	}

	template<> inline 
	rect_t<float>::rect_t(const variant& v)
	{
		if(v.is_list()) {
			std::vector<float> vec;
			for(size_t n = 0; n != v.num_elements(); ++n) {
				vec.push_back(float(v.as_float()));
			}
			from_vector(vec);
			return;
		} else if(v.is_map()) {
			ASSERT_LOG((v.has_key("x") && v.has_key("y") && v.has_key("w") && v.has_key("h"))
				|| v.has_key("x1") && v.has_key("y1") && v.has_key("x2") && v.has_key("y2"), 
				"map must have 'x','y','w','h' or 'x1','y1','x2','y2' attributes.");
			if(v.has_key("x")) {
				*this = rect_t<float>(v["x"].as_float(),v["y"].as_float(),v["w"].as_float(),v["h"].as_float());
			} else {
				*this = rect_t<float>::from_coordinates(v["x1"].as_float(),v["y1"].as_float(),v["x2"].as_float(),v["y2"].as_float());
			}
		} else {
			ASSERT_LOG(false, "Creating a rect from a variant must be list or map");
		}
	}
}

#include "geometry.inl"

typedef geometry::point_t<int> point;
typedef geometry::point_t<float> pointf;

typedef geometry::rect_t<int> rect;
typedef geometry::rect_t<float> rectf;

