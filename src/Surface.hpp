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
#include <memory>

#include "WindowManagerFwd.hpp"

namespace Graphics
{
	class Surface
	{
	public:
		virtual ~Surface();
		virtual void* Pixels() = 0;
		virtual size_t width() = 0;
		virtual size_t height() = 0;
		virtual size_t bits_per_pixel() = 0;
		virtual size_t bytes_per_pixel() = 0;
		virtual size_t row_pitch() = 0;
		virtual uint32_t red_mask() = 0;
		virtual uint32_t green_mask() = 0;
		virtual uint32_t blue_mask() = 0;
		virtual uint32_t alpha_mask() = 0;

		virtual void Lock() = 0;
		virtual void Unlock() = 0;

		virtual bool SetClipRect(int x, int y, size_t width, size_t height) = 0;
		virtual void GetClipRect(int& x, int& y, size_t& width, size_t& height) = 0;
		//virtual bool SetClipRect(const rect& r) = 0;
		//virtual const rect& GetClipRect() = 0;
	protected:
		Surface();
	private:
	};

	class SurfaceLock
	{
	public:
		SurfaceLock(const SurfacePtr& surface);
		~SurfaceLock();
	private:
		SurfacePtr surface_;
	};
}
