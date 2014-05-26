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

#include "SDL_image.h"

#include "asserts.hpp"
#include "surface_sdl.hpp"

namespace KRE
{
	namespace
	{
		bool can_create_surfaces = surface::register_surface_creator("sdl", surface_sdl::CreateFromFile);

		Uint32 get_sdl_pixel_format(PixelFormat fmt)
		{
			switch(fmt) {
				case PixelFormat::PIXELFORMAT_INDEX1LSB:	    return SDL_PIXELFORMAT_INDEX1LSB;
				case PixelFormat::PIXELFORMAT_INDEX1MSB:	    return SDL_PIXELFORMAT_INDEX1MSB;
				case PixelFormat::PIXELFORMAT_INDEX4LSB:	    return SDL_PIXELFORMAT_INDEX4LSB;
				case PixelFormat::PIXELFORMAT_INDEX4MSB:	    return SDL_PIXELFORMAT_INDEX4MSB;
				case PixelFormat::PIXELFORMAT_INDEX8:	    return SDL_PIXELFORMAT_INDEX8;
				case PixelFormat::PIXELFORMAT_RGB332:	    return SDL_PIXELFORMAT_RGB332;
				case PixelFormat::PIXELFORMAT_RGB444:	    return SDL_PIXELFORMAT_RGB444;
				case PixelFormat::PIXELFORMAT_RGB555:	    return SDL_PIXELFORMAT_RGB555;
				case PixelFormat::PIXELFORMAT_BGR555:	    return SDL_PIXELFORMAT_BGR555;
				case PixelFormat::PIXELFORMAT_ARGB4444:	    return SDL_PIXELFORMAT_ARGB4444;
				case PixelFormat::PIXELFORMAT_RGBA4444:	    return SDL_PIXELFORMAT_RGBA4444;
				case PixelFormat::PIXELFORMAT_ABGR4444:	    return SDL_PIXELFORMAT_ABGR4444;
				case PixelFormat::PIXELFORMAT_BGRA4444:	    return SDL_PIXELFORMAT_BGRA4444;
				case PixelFormat::PIXELFORMAT_ARGB1555:	    return SDL_PIXELFORMAT_ARGB1555;
				case PixelFormat::PIXELFORMAT_RGBA5551:	    return SDL_PIXELFORMAT_RGBA5551;
				case PixelFormat::PIXELFORMAT_ABGR1555:	    return SDL_PIXELFORMAT_ABGR1555;
				case PixelFormat::PIXELFORMAT_BGRA5551:	    return SDL_PIXELFORMAT_BGRA5551;
				case PixelFormat::PIXELFORMAT_RGB565:	    return SDL_PIXELFORMAT_RGB565;
				case PixelFormat::PIXELFORMAT_BGR565:	    return SDL_PIXELFORMAT_BGR565;
				case PixelFormat::PIXELFORMAT_RGB24:	        return SDL_PIXELFORMAT_RGB24;
				case PixelFormat::PIXELFORMAT_BGR24:	        return SDL_PIXELFORMAT_BGR24;
				case PixelFormat::PIXELFORMAT_RGB888:	    return SDL_PIXELFORMAT_RGB888;
				case PixelFormat::PIXELFORMAT_RGBX8888:	    return SDL_PIXELFORMAT_RGBX8888;
				case PixelFormat::PIXELFORMAT_BGR888:	    return SDL_PIXELFORMAT_BGR888;
				case PixelFormat::PIXELFORMAT_BGRX8888:	    return SDL_PIXELFORMAT_BGRX8888;
				case PixelFormat::PIXELFORMAT_ARGB8888:	    return SDL_PIXELFORMAT_ARGB8888;
				case PixelFormat::PIXELFORMAT_RGBA8888:	    return SDL_PIXELFORMAT_RGBA8888;
				case PixelFormat::PIXELFORMAT_ABGR8888:	    return SDL_PIXELFORMAT_ABGR8888;
				case PixelFormat::PIXELFORMAT_BGRA8888:	    return SDL_PIXELFORMAT_BGRA8888;
				case PixelFormat::PIXELFORMAT_ARGB2101010:	return SDL_PIXELFORMAT_ARGB2101010;
				case PixelFormat::PIXELFORMAT_YV12:	        return SDL_PIXELFORMAT_YV12;
				case PixelFormat::PIXELFORMAT_IYUV:	        return SDL_PIXELFORMAT_IYUV;
				case PixelFormat::PIXELFORMAT_YUY2:	        return SDL_PIXELFORMAT_YUY2;
				case PixelFormat::PIXELFORMAT_UYVY:	        return SDL_PIXELFORMAT_UYVY;
				case PixelFormat::PIXELFORMAT_YVYU:	        return SDL_PIXELFORMAT_YVYU;
				default:
					ASSERT_LOG(false, "Unknown pixel format given");
			}
			return SDL_PIXELFORMAT_ABGR8888;
		}
	}

	surface_sdl::surface_sdl(unsigned width, 
		unsigned height, 
		unsigned bpp, 
		uint32_t rmask, 
		uint32_t gmask, 
		uint32_t bmask, 
		uint32_t amask) : has_data_(false)
	{
		surface_ = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
		ASSERT_LOG(surface_ != NULL, "Error creating surface: " << SDL_GetError());
		set_pixel_format(pixel_format_ptr(new pixel_format_sdl(surface_->format)));
	}

	surface_sdl::surface_sdl(unsigned width, 
		unsigned height, 
		unsigned bpp, 
		unsigned row_pitch,
		uint32_t rmask, 
		uint32_t gmask, 
		uint32_t bmask, 
		uint32_t amask, 
		void* pixels) : has_data_(true)
	{
		ASSERT_LOG(pixels != NULL, "NULL value for pixels while creating surface.");
		surface_ = SDL_CreateRGBSurfaceFrom(pixels, width, height, bpp, row_pitch, rmask, gmask, bmask, amask);
		ASSERT_LOG(surface_ != NULL, "Error creating surface: " << SDL_GetError());
		set_pixel_format(pixel_format_ptr(new pixel_format_sdl(surface_->format)));
	}

	surface_sdl::surface_sdl(SDL_Surface* surface)
		: surface_(surface)
	{
		ASSERT_LOG(surface_ != NULL, "Error creating surface: " << SDL_GetError());
		set_pixel_format(pixel_format_ptr(new pixel_format_sdl(surface_->format)));
	}

	surface_sdl::surface_sdl(unsigned width, unsigned height, PixelFormat format)
	{
		// XXX todo
	}

	surface_sdl::~surface_sdl()
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null in destructor");
		SDL_FreeSurface(surface_);
	}

	const void* surface_sdl::pixels() const
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		// technically surface_->locked is an internal implementation detail.
		// but we'll live with using it.
		if(SDL_MUSTLOCK(surface_) && !surface_->locked) {
			ASSERT_LOG(false, "surface is marked as needing to be locked but is not locked on Pixels access.");
		}
		return surface_->pixels;
	}

	bool surface_sdl::set_clip_rect(int x, int y, unsigned width, unsigned height)
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect r = {x,y,width,height};
		return SDL_SetClipRect(surface_, &r) != SDL_TRUE;
	}

	void surface_sdl::get_clip_rect(int& x, int& y, unsigned& width, unsigned& height)
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect r;
		SDL_GetClipRect(surface_, &r);
		x = r.x;
		y = r.y;
		width = r.w;
		height = r.h;

	}

	bool surface_sdl::set_clip_rect(const rect& r)
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect sr = r.sdl_rect();
		return SDL_SetClipRect(surface_, &sr) == SDL_TRUE;
	}

	const rect surface_sdl::get_clip_rect()
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect sr;
		SDL_GetClipRect(surface_, &sr);
		return rect(sr);
	}

	void surface_sdl::lock() 
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		if(SDL_MUSTLOCK(surface_)) {
			auto res = SDL_LockSurface(surface_);
			ASSERT_LOG(res == 0, "Error calling SDL_Locksurface(): " << SDL_GetError());
		}
	}

	void surface_sdl::unlock() 
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		if(SDL_MUSTLOCK(surface_)) {
			SDL_UnlockSurface(surface_);
		}
	}

	void surface_sdl::write_pixels(unsigned bpp, 
		uint32_t rmask, 
		uint32_t gmask, 
		uint32_t bmask, 
		uint32_t amask,
		const void* pixels)
	{
		SDL_FreeSurface(surface_);
		ASSERT_LOG(pixels != NULL, "NULL value for pixels while creating surface.");
		surface_ = SDL_CreateRGBSurfaceFrom(const_cast<void*>(pixels), width(), height(), bpp, row_pitch(), rmask, gmask, bmask, amask);
		ASSERT_LOG(surface_ != NULL, "Error creating surface: " << SDL_GetError());
		set_pixel_format(pixel_format_ptr(new pixel_format_sdl(surface_->format)));
	}

	void surface_sdl::write_pixels(const void* pixels) 
	{
		surface_lock lock(surface_ptr(this));
		memcpy(surface_->pixels, pixels, row_pitch() * width());
	}

	void surface_sdl::set_blend_mode(SurfaceBlend bm) 
	{
		SDL_BlendMode sdl_bm;
		switch(bm) {
			case SurfaceBlend::BLEND_MODE_NONE:	sdl_bm = SDL_BLENDMODE_NONE; break;
			case SurfaceBlend::BLEND_MODE_BLEND:	sdl_bm = SDL_BLENDMODE_BLEND; break;
			case SurfaceBlend::BLEND_MODE_ADD:	sdl_bm = SDL_BLENDMODE_ADD; break;
			case SurfaceBlend::BLEND_MODE_MODULATE:	sdl_bm = SDL_BLENDMODE_MOD; break;
		}
		SDL_SetSurfaceBlendMode(surface_, sdl_bm);
	}

	SurfaceBlend surface_sdl::get_blend_mode() const 
	{
		SDL_BlendMode sdl_bm;
		SDL_GetSurfaceBlendMode(surface_, &sdl_bm);
		switch(sdl_bm) {
		case SDL_BLENDMODE_NONE:	return SurfaceBlend::BLEND_MODE_NONE;
		case SDL_BLENDMODE_BLEND:	return SurfaceBlend::BLEND_MODE_BLEND;
		case SDL_BLENDMODE_ADD:		return SurfaceBlend::BLEND_MODE_ADD;
		case SDL_BLENDMODE_MOD:		return SurfaceBlend::BLEND_MODE_MODULATE;
		}
		ASSERT_LOG(false, "Unrecognised SDL blend mode: " << sdl_bm);
		return SurfaceBlend::BLEND_MODE_NONE;
	}

	//////////////////////////////////////////////////////////////////////////
	// pixel_format_sdl
	namespace 
	{
		uint8_t count_bits_set(uint32_t v)
		{
			v = v - ((v >> 1) & 0x55555555);
			v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
			return uint8_t(((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24);
		}
	}

	pixel_format_sdl::pixel_format_sdl(const SDL_PixelFormat* pf)
		: pf_(pf)
	{
		ASSERT_LOG(pf != NULL, "pixel_format_sdl constructor passeda null pixel format.");
	}

	pixel_format_sdl::~pixel_format_sdl()
	{
	}

	uint8_t pixel_format_sdl::bits_per_pixel() const 
	{
		return pf_->BitsPerPixel;
	}

	uint8_t pixel_format_sdl::bytes_per_pixel() const 
	{
		return pf_->BytesPerPixel;
	}

	bool pixel_format_sdl::is_yuv_planar() const 
	{
		return pf_->format == SDL_PIXELFORMAT_YV12 || pf_->format == SDL_PIXELFORMAT_IYUV;
	}

	bool pixel_format_sdl::is_yuv_packed() const 
	{
		return pf_->format == SDL_PIXELFORMAT_YUY2 
			|| pf_->format == SDL_PIXELFORMAT_UYVY
			|| pf_->format == SDL_PIXELFORMAT_YVYU;
	}

	bool pixel_format_sdl::yuv_height_reversed() const 
	{
		return false;
	}

	bool pixel_format_sdl::is_interlaced() const 
	{
		return false;
	}
		
	bool pixel_format_sdl::is_rgb() const 
	{
		return !SDL_ISPIXELFORMAT_FOURCC(pf_->format);
	}

	bool pixel_format_sdl::has_red_channel() const 
	{
		return is_rgb() && pf_->Rmask != 0;
	}

	bool pixel_format_sdl::has_green_channel() const 
	{
		return is_rgb() && pf_->Gmask != 0;
	}

	bool pixel_format_sdl::has_blue_channel() const 
	{
		return is_rgb() && pf_->Bmask != 0;
	}

	bool pixel_format_sdl::has_alpha_channel() const 
	{
		return is_rgb() && pf_->Amask != 0;
	}

	bool pixel_format_sdl::has_luminance() const 
	{
		return is_rgb() && pf_->Rmask != 0;
	}

	uint32_t pixel_format_sdl::red_mask() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for red_mask of non-RGB surface.");
		return pf_->Rmask;
	}

	uint32_t pixel_format_sdl::green_mask() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for green_mask of non-RGB surface.");
		return pf_->Gmask;
	}

	uint32_t pixel_format_sdl::blue_mask() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for blue_mask of non-RGB surface.");
		return pf_->Bmask;
	}

	uint32_t pixel_format_sdl::alpha_mask() const 
	{
		return pf_->Amask;
	}

	uint32_t pixel_format_sdl::luminance_mask() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for luminance_mask of non-RGB surface.");
		return pf_->Rmask;
	}

	uint8_t pixel_format_sdl::red_bits() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for red_bits() of non-RGB surface.");
		return count_bits_set(pf_->Rmask);
	}

	uint8_t pixel_format_sdl::green_bits() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for green_bits() of non-RGB surface.");
		return count_bits_set(pf_->Gmask);
	}

	uint8_t pixel_format_sdl::blue_bits() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for blue_bits() of non-RGB surface.");
		return count_bits_set(pf_->Bmask);
	}

	uint8_t pixel_format_sdl::alpha_bits() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for alpha_bits() of non-RGB surface.");
		return count_bits_set(pf_->Amask);
	}

	uint8_t pixel_format_sdl::luminance_bits() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for luminance_bits() of non-RGB surface.");
		return count_bits_set(pf_->Rmask);
	}

	uint32_t pixel_format_sdl::red_shift() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for red_shift() of non-RGB surface.");
		return pf_->Rshift;
	}

	uint32_t pixel_format_sdl::green_shift() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for green_shift() of non-RGB surface.");
		return pf_->Gshift;
	}

	uint32_t pixel_format_sdl::blue_shift() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for blue_shift() of non-RGB surface.");
		return pf_->Bshift;
	}

	uint32_t pixel_format_sdl::alpha_shift() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for alpha_shift() of non-RGB surface.");
		return pf_->Ashift;
	}

	uint32_t pixel_format_sdl::luminance_shift() const 
	{
		ASSERT_LOG(is_rgb(), "Asked for luminance_shift() of non-RGB surface.");
		return pf_->Rshift;
	}

	bool pixel_format_sdl::has_palette() const 
	{
		return pf_->palette != NULL;
	}

	PixelFormat pixel_format_sdl::get_format() const
	{
		switch(pf_->format) {
			case SDL_PIXELFORMAT_INDEX1LSB:	    return PixelFormat::PIXELFORMAT_INDEX1LSB;
			case SDL_PIXELFORMAT_INDEX1MSB:	    return PixelFormat::PIXELFORMAT_INDEX1MSB;
			case SDL_PIXELFORMAT_INDEX4LSB:	    return PixelFormat::PIXELFORMAT_INDEX4LSB;
			case SDL_PIXELFORMAT_INDEX4MSB:	    return PixelFormat::PIXELFORMAT_INDEX4MSB;
			case SDL_PIXELFORMAT_INDEX8:	    return PixelFormat::PIXELFORMAT_INDEX8;
			case SDL_PIXELFORMAT_RGB332:	    return PixelFormat::PIXELFORMAT_RGB332;
			case SDL_PIXELFORMAT_RGB444:	    return PixelFormat::PIXELFORMAT_RGB444;
			case SDL_PIXELFORMAT_RGB555:	    return PixelFormat::PIXELFORMAT_RGB555;
			case SDL_PIXELFORMAT_BGR555:	    return PixelFormat::PIXELFORMAT_BGR555;
			case SDL_PIXELFORMAT_ARGB4444:	    return PixelFormat::PIXELFORMAT_ARGB4444;
			case SDL_PIXELFORMAT_RGBA4444:	    return PixelFormat::PIXELFORMAT_RGBA4444;
			case SDL_PIXELFORMAT_ABGR4444:	    return PixelFormat::PIXELFORMAT_ABGR4444;
			case SDL_PIXELFORMAT_BGRA4444:	    return PixelFormat::PIXELFORMAT_BGRA4444;
			case SDL_PIXELFORMAT_ARGB1555:	    return PixelFormat::PIXELFORMAT_ARGB1555;
			case SDL_PIXELFORMAT_RGBA5551:	    return PixelFormat::PIXELFORMAT_RGBA5551;
			case SDL_PIXELFORMAT_ABGR1555:	    return PixelFormat::PIXELFORMAT_ABGR1555;
			case SDL_PIXELFORMAT_BGRA5551:	    return PixelFormat::PIXELFORMAT_BGRA5551;
			case SDL_PIXELFORMAT_RGB565:	    return PixelFormat::PIXELFORMAT_RGB565;
			case SDL_PIXELFORMAT_BGR565:	    return PixelFormat::PIXELFORMAT_BGR565;
			case SDL_PIXELFORMAT_RGB24:	        return PixelFormat::PIXELFORMAT_RGB24;
			case SDL_PIXELFORMAT_BGR24:	        return PixelFormat::PIXELFORMAT_BGR24;
			case SDL_PIXELFORMAT_RGB888:	    return PixelFormat::PIXELFORMAT_RGB888;
			case SDL_PIXELFORMAT_RGBX8888:	    return PixelFormat::PIXELFORMAT_RGBX8888;
			case SDL_PIXELFORMAT_BGR888:	    return PixelFormat::PIXELFORMAT_BGR888;
			case SDL_PIXELFORMAT_BGRX8888:	    return PixelFormat::PIXELFORMAT_BGRX8888;
			case SDL_PIXELFORMAT_ARGB8888:	    return PixelFormat::PIXELFORMAT_ARGB8888;
			case SDL_PIXELFORMAT_RGBA8888:	    return PixelFormat::PIXELFORMAT_RGBA8888;
			case SDL_PIXELFORMAT_ABGR8888:	    return PixelFormat::PIXELFORMAT_ABGR8888;
			case SDL_PIXELFORMAT_BGRA8888:	    return PixelFormat::PIXELFORMAT_BGRA8888;
			case SDL_PIXELFORMAT_ARGB2101010:	return PixelFormat::PIXELFORMAT_ARGB2101010;
			case SDL_PIXELFORMAT_YV12:	        return PixelFormat::PIXELFORMAT_YV12;
			case SDL_PIXELFORMAT_IYUV:	        return PixelFormat::PIXELFORMAT_IYUV;
			case SDL_PIXELFORMAT_YUY2:	        return PixelFormat::PIXELFORMAT_YUY2;
			case SDL_PIXELFORMAT_UYVY:	        return PixelFormat::PIXELFORMAT_UYVY;
			case SDL_PIXELFORMAT_YVYU:	        return PixelFormat::PIXELFORMAT_YVYU;
		}
		return PixelFormat::PIXELFORMAT_UNKNOWN;
	}

	surface_ptr surface_sdl::CreateFromFile(const std::string& filename, PixelFormat fmt, surfaceconvertFn fn)
	{
		auto surface = new surface_sdl(IMG_Load(filename.c_str()));
		// format means don't convert the surface from the loaded format.
		if(fmt != PixelFormat::PIXELFORMAT_UNKNOWN) {
			return surface->convert(fmt, fn);
		}
		return surface_ptr(surface);
	}

	std::tuple<int,int> pixel_format_sdl::extract_rgba(const void* pixels, int ndx, uint32_t& red, uint32_t& green, uint32_t& blue, uint32_t& alpha)
	{
		auto fmt = get_format();
		int pixel_shift_return = bytes_per_pixel();
		red = 0;
		green = 0;
		blue = 0;
		alpha = 255;
		switch(fmt) {
            case PixelFormat::PIXELFORMAT_INDEX1LSB: {
				ASSERT_LOG(pf_->palette != NULL, "Index type has no palette.");
				uint8_t px = *static_cast<const uint8_t*>(pixels) & (1 << ndx) >> ndx;
				ASSERT_LOG(px < pf_->palette->ncolors, "Index into palette invalid. " << px << " >= " << pf_->palette->ncolors);
				auto color = pf_->palette->colors[px];
				red = color.r;
				green = color.g;
				blue = color.b;
				alpha = color.a;
				if(ndx == 7) {
					ndx = 0;
				} else {
					pixel_shift_return = 0;
					++ndx;
				}
				break;
			}
            case PixelFormat::PIXELFORMAT_INDEX1MSB: {
				ASSERT_LOG(pf_->palette != NULL, "Index type has no palette.");
				uint8_t px = (*static_cast<const uint8_t*>(pixels) & (1 << (7-ndx))) >> (7-ndx);
				ASSERT_LOG(px < pf_->palette->ncolors, "Index into palette invalid. " << px << " >= " << pf_->palette->ncolors);
				auto color = pf_->palette->colors[px];
				red = color.r;
				green = color.g;
				blue = color.b;
				alpha = color.a;
				if(ndx == 7) {
					ndx = 0;
				} else {
					pixel_shift_return = 0;
					++ndx;
				}
				break;
			}
            case PixelFormat::PIXELFORMAT_INDEX4LSB: {
				ASSERT_LOG(pf_->palette != NULL, "Index type has no palette.");
				uint8_t px = (*static_cast<const uint8_t*>(pixels) & (0xf << ndx)) >> ndx;
				ASSERT_LOG(px < pf_->palette->ncolors, "Index into palette invalid. " << px << " >= " << pf_->palette->ncolors);
				auto color = pf_->palette->colors[px];
				red = color.r;
				green = color.g;
				blue = color.b;
				alpha = color.a;
				if(ndx == 4) {
					ndx = 0;
				} else {
					pixel_shift_return = 0;
					ndx = 4;
				}
				break;
			}
			case PixelFormat::PIXELFORMAT_INDEX4MSB: {
				ASSERT_LOG(pf_->palette != NULL, "Index type has no palette.");
				uint8_t px = (*static_cast<const uint8_t*>(pixels) & (0xf << (4-ndx))) >> (4-ndx);
				ASSERT_LOG(px < pf_->palette->ncolors, "Index into palette invalid. " << px << " >= " << pf_->palette->ncolors);
				auto color = pf_->palette->colors[px];
				red = color.r;
				green = color.g;
				blue = color.b;
				alpha = color.a;
				if(ndx == 4) {
					ndx = 0;
				} else {
					pixel_shift_return = 0;
					ndx = 4;
				}
				break;
			}
            case PixelFormat::PIXELFORMAT_INDEX8: {
				ASSERT_LOG(pf_->palette != NULL, "Index type has no palette.");
				uint8_t px = *static_cast<const uint8_t*>(pixels);
				ASSERT_LOG(px < pf_->palette->ncolors, "Index into palette invalid. " << px << " >= " << pf_->palette->ncolors);
				auto color = pf_->palette->colors[px];
				red = color.r;
				green = color.g;
				blue = color.b;
				alpha = color.a;
				pixel_shift_return = bytes_per_pixel();
				break;
			}

            case PixelFormat::PIXELFORMAT_RGB332:
            case PixelFormat::PIXELFORMAT_RGB444:
            case PixelFormat::PIXELFORMAT_RGB555:
            case PixelFormat::PIXELFORMAT_BGR555:
            case PixelFormat::PIXELFORMAT_ARGB4444:
            case PixelFormat::PIXELFORMAT_RGBA4444:
            case PixelFormat::PIXELFORMAT_ABGR4444:
            case PixelFormat::PIXELFORMAT_BGRA4444:
            case PixelFormat::PIXELFORMAT_ARGB1555:
            case PixelFormat::PIXELFORMAT_RGBA5551:
            case PixelFormat::PIXELFORMAT_ABGR1555:
            case PixelFormat::PIXELFORMAT_BGRA5551:
            case PixelFormat::PIXELFORMAT_RGB565:
            case PixelFormat::PIXELFORMAT_BGR565:
            case PixelFormat::PIXELFORMAT_RGB24:
            case PixelFormat::PIXELFORMAT_BGR24:
            case PixelFormat::PIXELFORMAT_RGB888:
            case PixelFormat::PIXELFORMAT_RGBX8888:
            case PixelFormat::PIXELFORMAT_BGR888:
            case PixelFormat::PIXELFORMAT_BGRX8888:
            case PixelFormat::PIXELFORMAT_ARGB8888:
            case PixelFormat::PIXELFORMAT_RGBA8888:
            case PixelFormat::PIXELFORMAT_ABGR8888:
            case PixelFormat::PIXELFORMAT_BGRA8888:
            case PixelFormat::PIXELFORMAT_ARGB2101010: {
				const uint32_t* px = static_cast<const uint32_t*>(pixels);
				if(has_red_channel()) {
					red = (*px) & red_mask() >> red_shift();
				}
				if(has_green_channel()) {
					green = (*px) & green_mask() >> green_shift();
				}
				if(has_blue_channel()) {
					blue = (*px) & blue_mask() >> blue_shift();
				}
				if(has_alpha_channel()) {
					alpha = (*px) & alpha_mask() >> alpha_shift();
				}
				break;
			}

            case PixelFormat::PIXELFORMAT_YV12:
            case PixelFormat::PIXELFORMAT_IYUV:
            case PixelFormat::PIXELFORMAT_YUY2:
            case PixelFormat::PIXELFORMAT_UYVY:
            case PixelFormat::PIXELFORMAT_YVYU:				
			default:
				ASSERT_LOG(false, "unsupported pixel format value for conversion.");
		}
		return std::make_tuple(pixel_shift_return, ndx);
	}

	void pixel_format_sdl::encode_rgba(void* pixels, uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
	{
		auto fmt = get_format();
		switch(fmt) {
            case PixelFormat::PIXELFORMAT_RGB332:
            case PixelFormat::PIXELFORMAT_RGB444:
            case PixelFormat::PIXELFORMAT_RGB555:
            case PixelFormat::PIXELFORMAT_BGR555:
            case PixelFormat::PIXELFORMAT_ARGB4444:
            case PixelFormat::PIXELFORMAT_RGBA4444:
            case PixelFormat::PIXELFORMAT_ABGR4444:
            case PixelFormat::PIXELFORMAT_BGRA4444:
            case PixelFormat::PIXELFORMAT_ARGB1555:
            case PixelFormat::PIXELFORMAT_RGBA5551:
            case PixelFormat::PIXELFORMAT_ABGR1555:
            case PixelFormat::PIXELFORMAT_BGRA5551:
            case PixelFormat::PIXELFORMAT_RGB565:
            case PixelFormat::PIXELFORMAT_BGR565:
            case PixelFormat::PIXELFORMAT_RGB24:
            case PixelFormat::PIXELFORMAT_BGR24:
            case PixelFormat::PIXELFORMAT_RGB888:
            case PixelFormat::PIXELFORMAT_RGBX8888:
            case PixelFormat::PIXELFORMAT_BGR888:
            case PixelFormat::PIXELFORMAT_BGRX8888:
            case PixelFormat::PIXELFORMAT_ARGB8888:
            case PixelFormat::PIXELFORMAT_RGBA8888:
            case PixelFormat::PIXELFORMAT_ABGR8888:
            case PixelFormat::PIXELFORMAT_BGRA8888:
            case PixelFormat::PIXELFORMAT_ARGB2101010: {
				uint32_t pixel = 0;
				if(has_red_channel()) {
					pixel = (red << red_shift()) & red_mask();
				}
				if(has_green_channel()) {
					pixel = (green << green_shift()) & green_mask();
				}
				if(has_blue_channel()) {
					pixel = (blue << blue_shift()) & blue_mask();
				}
				if(has_alpha_channel()) {
					pixel = (alpha << alpha_shift()) & alpha_mask();
				}
				uint32_t* px = static_cast<uint32_t*>(pixels);
				*px = pixel;
				break;
			}

            case PixelFormat::PIXELFORMAT_INDEX1LSB: 
            case PixelFormat::PIXELFORMAT_INDEX1MSB:
            case PixelFormat::PIXELFORMAT_INDEX4LSB:
            case PixelFormat::PIXELFORMAT_INDEX4MSB:
            case PixelFormat::PIXELFORMAT_INDEX8:
				ASSERT_LOG(false, "converting format to an indexed type not supported.");
				break;
            case PixelFormat::PIXELFORMAT_YV12:
            case PixelFormat::PIXELFORMAT_IYUV:
            case PixelFormat::PIXELFORMAT_YUY2:
            case PixelFormat::PIXELFORMAT_UYVY:
            case PixelFormat::PIXELFORMAT_YVYU:				
			default:
				ASSERT_LOG(false, "unsupported pixel format value for conversion.");
		}
	}

	surface_ptr surface_sdl::handle_convert(PixelFormat fmt, surfaceconvertFn convert)
	{
		ASSERT_LOG(fmt != PixelFormat::PIXELFORMAT_UNKNOWN, "unknown pixel format to convert to.");
		if(convert == nullptr) {
			std::shared_ptr<SDL_PixelFormat> pf = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(get_sdl_pixel_format(fmt)), [](SDL_PixelFormat* fmt) {
				SDL_FreeFormat(fmt);
			});
			auto surface = new surface_sdl(SDL_ConvertSurface(surface_, pf.get(), 0));
			return surface_ptr(surface);
		}

		surface_lock lock(surface_ptr(this));
		uint32_t red;
		uint32_t green;
		uint32_t blue;
		uint32_t alpha;

		// Create a destination surface
		auto dst = new surface_sdl(width(), height(), fmt);
		size_t dst_size = dst->row_pitch() * dst->height();
		uint8_t* dst_pixels = new uint8_t[dst_size];

		for(size_t h = 0; h != height(); ++h) {
			unsigned offs = 0;
			int ndx = 0;
			uint8_t* pixel_ptr = static_cast<uint8_t*>(surface_->pixels) + h*row_pitch();
			uint8_t* dst_pixel_ptr = static_cast<uint8_t*>(dst_pixels) + h*row_pitch();
			while(offs < width()) {
				std::tie(offs, ndx) = get_pixel_format()->extract_rgba(pixel_ptr + offs, ndx, red, green, blue, alpha);
				convert(red, green, blue, alpha);
				dst->get_pixel_format()->encode_rgba(dst_pixels, red, green, blue, alpha);
			}
		}
		dst->write_pixels(dst_pixels);
		delete[] dst_pixels;
		return surface_ptr(dst);
	}
}
