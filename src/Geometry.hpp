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

#include <vector>

#include "SDL.h"

namespace Geometry
{
	template<typename T>
	struct Point {
		explicit Point(T x=0, T y=0) : x(x), y(y)
		{}

		explicit Point(const std::vector<T>& v);

		union {
			struct { T x, y; };
			T buf[2];
		};
	};

	template<typename T>
	bool operator==(const Point<T>& a, const Point<T>& b);
	template<typename T>
	bool operator!=(const Point<T>& a, const Point<T>& b);
	template<typename T>
	bool operator<(const Point<T>& a, const Point<T>& b);

	template<typename T>
	class Rect
	{
	public:
		explicit Rect(T x=0, T y=0, T w=0, T h=0);
		explicit Rect(const SDL_Rect& r);
		//explicit Rect(const std::vector<T>& v);
		//explicit Rect(const std::string& s);
		//explicit Rect(const variant& v);
		static Rect FromCoordinates(T x1, T y1, T x2, T y2);
		static Rect from_coordinates(T x1, T y1, T x2, T y2) {
			return FromCoordinates(x1,y1,x2,y2);
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

		const Point& top_left() const { return top_left_; }
		const Point& bottom_right() const { return bottom_right_; }

		void operator+=(const Point& p) {
			top_left_.x += p.x;
			top_left_.y += p.y;
			bottom_right_.x += p.x;
			bottom_right_.y += p.y;
		}
		void operator-=(const Point& p) {
			top_left_.x -= p.x;
			top_left_.y -= p.y;
			bottom_right_.x -= p.x;
			bottom_right_.y -= p.y;
		}

		SDL_Rect sdl_rect() const {
			SDL_Rect r = {x(), y(), w(), h()};
			return r;
		}
	private:
		Point top_left_;
		Point bottom_right_;
	};
}

typedef Geometry::Point<int> point;
typedef Geometry::Point<float> pointf;

typedef Geometry::Rect<int> rect;
typedef Geometry::Rect<float> rectf;

