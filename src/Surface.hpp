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
		PixelFormat();
		virtual ~PixelFormat();

		virtual uint8_t BitsPerPixel() const = 0;
		virtual uint8_t BytesPerPixel() const = 0;

		virtual bool IsYuvPlanar() const = 0;
		virtual bool IsYuvPacked() const = 0;
		virtual bool YuvHeightReversed() const = 0;
		virtual bool IsInterlaced() const = 0;
		
		virtual bool IsRGB() const = 0;
		virtual bool HasRedChannel() const = 0;
		virtual bool HasGreenChannel() const = 0;
		virtual bool HasBlueChannel() const = 0;
		virtual bool HasAlphaChannel() const = 0;
		virtual bool HasLuminance() const = 0;

		virtual uint32_t RedMask() const = 0;
		virtual uint32_t GreenMask() const = 0;
		virtual uint32_t BlueMask() const = 0;
		virtual uint32_t AlphaMask() const = 0;
		virtual uint32_t LuminanceMask() const = 0;

		virtual uint8_t RedBits() const = 0;
		virtual uint8_t GreenBits() const = 0;
		virtual uint8_t BlueBits() const = 0;
		virtual uint8_t AlphaBits() const = 0;
		virtual uint8_t LuminanceBits() const = 0;

		virtual bool HasPalette() const = 0;
	private:
		PixelFormat(const PixelFormat&);
	};

	typedef std::shared_ptr<PixelFormat> PixelFormatPtr;

	class Surface
	{
	public:
		virtual ~Surface();
		virtual const void* Pixels() const = 0;
		virtual size_t width() = 0;
		virtual size_t height() = 0;
		virtual size_t row_pitch() = 0;

		virtual void WritePixels(size_t bpp, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask,
			const void* pixels) = 0;
		virtual void WritePixels(const void* pixels) = 0;

		PixelFormatPtr GetPixelFormat();

		virtual void Lock() = 0;
		virtual void Unlock() = 0;

		virtual bool HasData() const = 0;

		enum BlendMode {
			BLEND_MODE_NONE,
			BLEND_MODE_BLEND,
			BLEND_MODE_ADD,
			BLEND_MODE_MODULATE,
		};
		virtual void SetBlendMode(BlendMode bm) = 0;
		virtual BlendMode GetBlendMode() const = 0;

		virtual bool SetClipRect(int x, int y, size_t width, size_t height) = 0;
		virtual void GetClipRect(int& x, int& y, size_t& width, size_t& height) = 0;
		virtual bool SetClipRect(const rect& r) = 0;
		virtual const rect GetClipRect() = 0;
	protected:
		Surface();
		void SetPixelFormat(PixelFormatPtr pf);
	private:
		PixelFormatPtr pf_;
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
