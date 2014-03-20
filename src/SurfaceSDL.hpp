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

#include "Surface.hpp"
#include "SDL.h"

namespace KRE
{
	class SDLPixelFormat : public PixelFormat
	{
	public:
		SDLPixelFormat(const SDL_PixelFormat* pf);
		virtual ~SDLPixelFormat();

		uint8_t BitsPerPixel() const override;
		uint8_t BytesPerPixel() const override;

		bool IsYuvPlanar() const override;
		bool IsYuvPacked() const override;
		bool YuvHeightReversed() const override;
		bool IsInterlaced() const override;
		
		bool IsRGB() const override;
		bool HasRedChannel() const override;
		bool HasGreenChannel() const override;
		bool HasBlueChannel() const override;
		bool HasAlphaChannel() const override;
		bool HasLuminance() const override;

		uint32_t RedMask() const override;
		uint32_t GreenMask() const override;
		uint32_t BlueMask() const override;
		uint32_t AlphaMask() const override;
		uint32_t LuminanceMask() const override;

		uint8_t RedBits() const override;
		uint8_t GreenBits() const override;
		uint8_t BlueBits() const override;
		uint8_t AlphaBits() const override;
		uint8_t LuminanceBits() const override;

		PixelFormatConstant GetConstant() const override;

		bool HasPalette() const override;
	private:
		const SDL_PixelFormat* pf_;
	};


	class SurfaceSDL : public Surface
	{
	public:
		SurfaceSDL(size_t width, 
			size_t height, 
			size_t bpp, 
			size_t row_pitch, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask, 
			void* pixels);
		SurfaceSDL(size_t width, 
			size_t height, 
			size_t bpp, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask);
		SurfaceSDL(SDL_Surface* surface);
		virtual ~SurfaceSDL();
		const void* Pixels() const override;
		void WritePixels(size_t bpp, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask,
			const void* pixels) override;
		void WritePixels(const void* pixels) override;
		size_t width() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			return surface_->w;
		}
		size_t height() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			return surface_->h;
		}
		size_t row_pitch() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			return surface_->pitch;
		}

		virtual bool HasData() const override {
			if(surface_ == NULL) {
				return false;
			}
			return has_data_;
		}

		void SetBlendMode(BlendMode bm) override;
		BlendMode GetBlendMode() const override;

		bool SetClipRect(int x, int y, size_t width, size_t height) override;
		void GetClipRect(int& x, int& y, size_t& width, size_t& height) override;
		bool SetClipRect(const rect& r) override;
		const rect GetClipRect() override;

		static SurfacePtr CreateFromFile(const std::string&);

		void Lock() override;
		void Unlock() override;
	private:
		SDL_Surface* surface_;
		bool has_data_;
		SurfaceSDL();
	};
}
