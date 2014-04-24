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
#include <functional>
#include <memory>
#include <tuple>

#include "Geometry.hpp"
#include "WindowManagerFwd.hpp"

namespace KRE
{
	enum class PixelFormat {
		PIXELFORMAT_UNKNOWN,
		PIXELFORMAT_INDEX1LSB,
		PIXELFORMAT_INDEX1MSB,
		PIXELFORMAT_INDEX4LSB,
		PIXELFORMAT_INDEX4MSB,
		PIXELFORMAT_INDEX8,
		PIXELFORMAT_RGB332,
		PIXELFORMAT_RGB444,
		PIXELFORMAT_RGB555,
		PIXELFORMAT_BGR555,
		PIXELFORMAT_ARGB4444,
		PIXELFORMAT_RGBA4444,
		PIXELFORMAT_ABGR4444,
		PIXELFORMAT_BGRA4444,
		PIXELFORMAT_ARGB1555,
		PIXELFORMAT_RGBA5551,
		PIXELFORMAT_ABGR1555,
		PIXELFORMAT_BGRA5551,
		PIXELFORMAT_RGB565,
		PIXELFORMAT_BGR565,
		PIXELFORMAT_RGB24,
		PIXELFORMAT_BGR24,
		PIXELFORMAT_RGB888,
		PIXELFORMAT_RGBX8888,
		PIXELFORMAT_BGR888,
		PIXELFORMAT_BGRX8888,
		PIXELFORMAT_ARGB8888,
		PIXELFORMAT_RGBA8888,
		PIXELFORMAT_ABGR8888,
		PIXELFORMAT_BGRA8888,
		PIXELFORMAT_RGB101010,
		PIXELFORMAT_ARGB2101010,
		PIXELFORMAT_YV12,
		PIXELFORMAT_IYUV,
		PIXELFORMAT_YUY2,
		PIXELFORMAT_UYVY,
		PIXELFORMAT_YVYU,
	};

	class pixel_format
	{
	public:
		pixel_format();
		virtual ~pixel_format();

		virtual uint8_t bits_per_pixel() const = 0;
		virtual uint8_t bytes_per_pixel() const = 0;

		virtual bool is_yuv_planar() const = 0;
		virtual bool is_yuv_packed() const = 0;
		virtual bool yuv_height_reversed() const = 0;
		virtual bool is_interlaced() const = 0;
		
		virtual bool is_rgb() const = 0;
		virtual bool has_red_channel() const = 0;
		virtual bool has_green_channel() const = 0;
		virtual bool has_blue_channel() const = 0;
		virtual bool has_alpha_channel() const = 0;
		virtual bool has_luminance() const = 0;

		virtual uint32_t red_mask() const = 0;
		virtual uint32_t green_mask() const = 0;
		virtual uint32_t blue_mask() const = 0;
		virtual uint32_t alpha_mask() const = 0;
		virtual uint32_t luminance_mask() const = 0;

		virtual uint32_t red_shift() const = 0;
		virtual uint32_t green_shift() const = 0;
		virtual uint32_t blue_shift() const = 0;
		virtual uint32_t alpha_shift() const = 0;
		virtual uint32_t luminance_shift() const = 0;

		virtual uint8_t red_bits() const = 0;
		virtual uint8_t green_bits() const = 0;
		virtual uint8_t blue_bits() const = 0;
		virtual uint8_t alpha_bits() const = 0;
		virtual uint8_t luminance_bits() const = 0;

		virtual bool has_palette() const = 0;

		virtual PixelFormat get_format() const = 0;

		virtual std::tuple<int,int> extract_rgba(const void* pixels, int ndx, uint32_t& red, uint32_t& green, uint32_t& blue, uint32_t& alpha) = 0;
		virtual void encode_rgba(void* pixels, uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha) = 0;
	private:
		pixel_format(const pixel_format&);
	};

	typedef std::shared_ptr<pixel_format> pixel_format_ptr;

	typedef std::function<void(uint32_t&,uint32_t&,uint32_t&,uint32_t&)> surfaceconvertFn;

	typedef std::function<surface_ptr(const std::string&, PixelFormat, surfaceconvertFn)> surface_creator_fn;

	enum class SurfaceBlend {
		BLEND_MODE_NONE,
		BLEND_MODE_BLEND,
		BLEND_MODE_ADD,
		BLEND_MODE_MODULATE,
	};

	class surface
	{
	public:
		virtual ~surface();
		virtual const void* pixels() const = 0;
		virtual unsigned width() = 0;
		virtual unsigned height() = 0;
		virtual unsigned row_pitch() = 0;

		virtual void write_pixels(unsigned bpp, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask,
			const void* pixels) = 0;
		virtual void write_pixels(const void* pixels) = 0;

		pixel_format_ptr get_pixel_format();

		virtual void lock() = 0;
		virtual void unlock() = 0;

		virtual bool has_data() const = 0;

		virtual void set_blend_mode(SurfaceBlend bm) = 0;
		virtual SurfaceBlend get_blend_mode() const = 0;

		virtual bool set_clip_rect(int x, int y, unsigned width, unsigned height) = 0;
		virtual void get_clip_rect(int& x, int& y, unsigned& width, unsigned& height) = 0;
		virtual bool set_clip_rect(const rect& r) = 0;
		virtual const rect get_clip_rect() = 0;
		surface_ptr convert(PixelFormat fmt, surfaceconvertFn convert=nullptr);

		static bool register_surface_creator(const std::string& name, surface_creator_fn fn);
		static void unregister_surface_creator(const std::string& name);
		static surface_ptr create(const std::string& filename, bool no_cache=false, PixelFormat fmt=PixelFormat::PIXELFORMAT_UNKNOWN, surfaceconvertFn convert=nullptr);
		static void reset_surface_cache();
	protected:
		surface();
		void set_pixel_format(pixel_format_ptr pf);
	private:
		virtual surface_ptr handle_convert(PixelFormat fmt, surfaceconvertFn convert) = 0;
		pixel_format_ptr pf_;
	};

	class surface_lock
	{
	public:
		surface_lock(const surface_ptr& surface);
		~surface_lock();
	private:
		surface_ptr surface_;
	};
}
