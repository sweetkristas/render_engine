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

#include <tuple>

#include "surface.hpp"
#include "SDL.h"

namespace KRE
{
	class pixel_format_sdl : public pixel_format
	{
	public:
		pixel_format_sdl(const SDL_PixelFormat* pf);
		virtual ~pixel_format_sdl();

		uint8_t bits_per_pixel() const override;
		uint8_t bytes_per_pixel() const override;

		bool is_yuv_planar() const override;
		bool is_yuv_packed() const override;
		bool yuv_height_reversed() const override;
		bool is_interlaced() const override;
		
		bool is_rgb() const override;
		bool has_red_channel() const override;
		bool has_green_channel() const override;
		bool has_blue_channel() const override;
		bool has_alpha_channel() const override;
		bool has_luminance() const override;

		uint32_t red_mask() const override;
		uint32_t green_mask() const override;
		uint32_t blue_mask() const override;
		uint32_t alpha_mask() const override;
		uint32_t luminance_mask() const override;

		uint8_t red_bits() const override;
		uint8_t green_bits() const override;
		uint8_t blue_bits() const override;
		uint8_t alpha_bits() const override;
		uint8_t luminance_bits() const override;

		uint32_t red_shift() const override;
		uint32_t green_shift() const override;
		uint32_t blue_shift() const override;
		uint32_t alpha_shift() const override;
		uint32_t luminance_shift() const override;

		PixelFormat get_format() const override;

		std::tuple<int,int> extract_rgba(const void* pixels, int ndx, uint32_t& red, uint32_t& green, uint32_t& blue, uint32_t& alpha) override;
		void encode_rgba(void* pixels, uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha) override; 

		bool has_palette() const override;
	private:
		const SDL_PixelFormat* pf_;
	};


	class surface_sdl : public surface
	{
	public:
		surface_sdl(unsigned width, 
			unsigned height, 
			unsigned bpp, 
			unsigned row_pitch, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask, 
			void* pixels);
		surface_sdl(unsigned width, 
			unsigned height, 
			unsigned bpp, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask);
		surface_sdl(SDL_Surface* surface);
		surface_sdl(unsigned width, unsigned height, PixelFormat format);
		virtual ~surface_sdl();
		const void* pixels() const override;
		void write_pixels(unsigned bpp, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask,
			const void* pixels) override;
		void write_pixels(const void* pixels) override;
		unsigned width() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			return surface_->w;
		}
		unsigned height() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			return surface_->h;
		}
		unsigned row_pitch() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			return surface_->pitch;
		}

		virtual bool has_data() const override {
			if(surface_ == NULL) {
				return false;
			}
			return has_data_;
		}

		void set_blend_mode(SurfaceBlend bm) override;
		SurfaceBlend get_blend_mode() const override;

		bool set_clip_rect(int x, int y, unsigned width, unsigned height) override;
		void get_clip_rect(int& x, int& y, unsigned& width, unsigned& height) override;
		bool set_clip_rect(const rect& r) override;
		const rect get_clip_rect() override;

		static surface_ptr CreateFromFile(const std::string&, PixelFormat fmt, surfaceconvertFn fn);

		void lock() override;
		void unlock() override;
	private:
		surface_ptr handle_convert(PixelFormat fmt, surfaceconvertFn convert) override;

		SDL_Surface* surface_;
		bool has_data_;
		surface_sdl();
	};
}
