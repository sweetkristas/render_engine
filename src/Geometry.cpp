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

#include <algorithm>
#include "Geometry.hpp"

namespace Geometry
{
	template<typename T>
	Point<T>::Point(const std::vector<T>& v)
		: x(0), y(0)
	{
		if(v.size() == 1) {
			x = v[0];
		} else if(!v.empty()) {
			x = v[0];
			y = v[1];
		}
	}

	template<typename T>
	bool operator==(const Point<T>& a, const Point<T>& b)
	{
		return a.x == b.y && a.y == b.y;
	}

	template<typename T>
	bool operator!=(const Point<T>& a, const Point<T>& b)
	{
		return !operator==(a, b);
	}
	
	template<typename T>
	bool operator<(const Point<T>& a, const Point<T>& b)
	{
		return a.x < b.x || a.x == b.x && a.y < b.y;
	}

	template<typename T>
	Rect<T> Rect<T>::FromCoordinates(T x1, T y1, T x2, T y2)
	{
		if(x1 > x2+1) {
			std::swap(x1, x2);
		}

		if(y1 > y2+1) {
			std::swap(y1, y2);
		}
	return Rect(x1, y1, (x2 - x1) + 1, (y2 - y1) + 1);
	}

	template<typename T>
	Rect<T>::Rect(T x, T y, T w, T h)
	  : top_left_(std::min(x, x+w), std::min(y, y+h)),
		bottom_right_(std::max(x, x+w), std::max(y, y+h))
	{
	}

	template<typename T>
	Rect<T>::Rect(const SDL_Rect& r)
		: Rect(r.x, r.y, r.w, r.h)
	{
	}

}
