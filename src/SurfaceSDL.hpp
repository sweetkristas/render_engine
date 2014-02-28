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

namespace Graphics
{
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
		virtual ~SurfaceSDL();
		const void* Pixels() const override;
		size_t width() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			return surface_->w;
		}
		size_t height() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			return surface_->h;
		}
		size_t bits_per_pixel() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			ASSERT_LOG(surface_->format != NULL, "surface_->format is null");
			return surface_->format->BitsPerPixel;
		}
		size_t bytes_per_pixel() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			ASSERT_LOG(surface_->format != NULL, "surface_->format is null");
			return surface_->format->BytesPerPixel;
		}
		size_t row_pitch() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			return surface_->pitch;
		}
		uint32_t red_mask() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			ASSERT_LOG(surface_->format != NULL, "surface_->format is null");
			return surface_->format->Rmask;
		}
		uint32_t green_mask() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			ASSERT_LOG(surface_->format != NULL, "surface_->format is null");
			return surface_->format->Gmask;
		}
		uint32_t blue_mask() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			ASSERT_LOG(surface_->format != NULL, "surface_->format is null");
			return surface_->format->Bmask;
		}
		uint32_t alpha_mask() override {
			ASSERT_LOG(surface_ != NULL, "surface_ is null");
			ASSERT_LOG(surface_->format != NULL, "surface_->format is null");
			return surface_->format->Amask;
		}

		virtual bool HasData() const override {
			if(surface_ == NULL) {
				return false;
			}
			return has_data_;
		}

		bool SetClipRect(int x, int y, size_t width, size_t height) override;
		void GetClipRect(int& x, int& y, size_t& width, size_t& height) override;
		bool SetClipRect(const rect& r) override;
		const rect& GetClipRect() override;

		void Lock() override;
		void Unlock() override;
	private:
		SDL_Surface* surface_;
		bool has_data_;
		SurfaceSDL();
	};
}
