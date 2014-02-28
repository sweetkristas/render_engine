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

#include "Geometry.hpp"
#include "WindowManagerFwd.hpp"

namespace Graphics
{
	class PixelFormat
	{
	public:
		enum {
			PIXEL_FORMAT_UNKNOWN,
			PIXEL_FORMAT_L8,
			PIXEL_FORMAT_BYTE_L = PIXEL_FORMAT_L8,
			PIXEL_FORMAT_L16,
			PIXEL_FORMAT_SHORT_L = PIXEL_FORMAT_L16,
			PIXEL_FORMAT_A8,
			PIXEL_FORMAT_BYTE_A = PIXEL_FORMAT_A8,
			PIXEL_FORMAT_A4L4,
			PIXEL_FORMAT_BYTE_LA,
			PIXEL_FORMAT_R5G6B5,
			PIXEL_FORMAT_B5G6R5,
			PIXEL_FORMAT_R3G3B2,
			PIXEL_FORMAT_A4R4G4B4,
			PIXEL_FORMAT_A1R5G5B5,
			PIXEL_FORMAT_R8G8B8,
			PIXEL_FORMAT_B8G8R8,
			PIXEL_FORMAT_A8R8G8B8,
			PIXEL_FORMAT_A8B8G8R8,
			PIXEL_FORMAT_B8G8R8A8,
			PIXEL_FORMAT_R8G8B8A8,
			PIXEL_FORMAT_BYTE_RGB = PIXEL_FORMAT_B8G8R8,
			PIXEL_FORMAT_BYTE_BGR = PIXEL_FORMAT_R8G8B8,
			PIXEL_FORMAT_BYTE_BGRA = PIXEL_FORMAT_A8R8G8B8,
			PIXEL_FORMAT_BYTE_RGBA = PIXEL_FORMAT_A8B8G8R8,
			PIXEL_FORMAT_A2R10G10B10,
			PIXEL_FORMAT_A2B10G10R10,
			// 8bits red
			PIXEL_FORMAT_R8,
			// 16 bits, 8 bits read, 8 bits green
			PIXEL_FORMAT_RG8,

			// 4:2:2 YCbCr
			PIXEL_FORMAT_YV12,
			//! Number of pixel formats currently defined
			PIXEL_FORMAT_COUNT
		};
	private:
		size_t bits_red_;
		size_t bits_green_;
		size_t bits_blue_;
		size_t bits_alpha_;

		size_t shift_red_;
		size_t shift_green_;
		size_t shift_blue_;
		size_t shift_alpha_;
	};

	class Surface
	{
	public:
		virtual ~Surface();
		virtual const void* Pixels() const = 0;
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

		virtual bool HasData() const = 0;

		virtual bool SetClipRect(int x, int y, size_t width, size_t height) = 0;
		virtual void GetClipRect(int& x, int& y, size_t& width, size_t& height) = 0;
		virtual bool SetClipRect(const rect& r) = 0;
		virtual const rect& GetClipRect() = 0;
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
