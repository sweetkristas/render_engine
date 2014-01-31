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

#include "asserts.hpp"
#include "SurfaceSDL.hpp"

namespace Graphics
{
	SurfaceSDL::SurfaceSDL(size_t width, 
		size_t height, 
		size_t bpp, 
		uint32_t rmask, 
		uint32_t gmask, 
		uint32_t bmask, 
		uint32_t amask) 
	{
		surface_ = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
		ASSERT_LOG(surface_ != NULL, "Error creating surface: " << SDL_GetError());
	}

	SurfaceSDL::SurfaceSDL(size_t width, 
		size_t height, 
		size_t bpp, 
		size_t row_pitch,
		uint32_t rmask, 
		uint32_t gmask, 
		uint32_t bmask, 
		uint32_t amask, 
		void* pixels)
	{
		ASSERT_LOG(pixels != NULL, "NULL value for pixels while creating surface.");
		surface_ = SDL_CreateRGBSurfaceFrom(pixels, width, height, bpp, row_pitch, rmask, gmask, bmask, amask);
		ASSERT_LOG(surface_ != NULL, "Error creating surface: " << SDL_GetError());
	}

	SurfaceSDL::~SurfaceSDL()
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null in destructor");
		SDL_FreeSurface(surface_);
	}

	void* SurfaceSDL::Pixels()
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		// technically surface_->locked is an internal implementation detail.
		// but we'll live with using it.
		if(SDL_MUSTLOCK(surface_) && !surface_->locked) {
			ASSERT_LOG(false, "Surface is marked as needing to be locked but is not locked on Pixels access.");
		}
		return surface_->pixels;
	}

	bool SurfaceSDL::SetClipRect(int x, int y, size_t width, size_t height)
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect r = {x,y,width,height};
		return SDL_SetClipRect(surface_, &r) != SDL_TRUE;
	}

	void SurfaceSDL::GetClipRect(int& x, int& y, size_t& width, size_t& height)
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect r;
		SDL_GetClipRect(surface_, &r);
		x = r.x;
		y = r.y;
		width = r.w;
		height = r.h;

	}

	/*bool SurfaceSDL::SetClipRect(const rect& r)
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect r = r.AsSDLRect();
		return SDL_SetClipRect(surface_, &r);
	}*/

	/*const rect& SurfaceSDL::GetClipRect()
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect r;
		SDL_GetClipRect(surface_, &r);
		return rect(r);
	}*/

	void SurfaceSDL::Lock() 
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		if(SDL_MUSTLOCK(surface_)) {
			auto res = SDL_LockSurface(surface_);
			ASSERT_LOG(res == 0, "Error calling SDL_LockSurface(): " << SDL_GetError());
		}
	}

	void SurfaceSDL::Unlock() 
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		if(SDL_MUSTLOCK(surface_)) {
			SDL_UnlockSurface(surface_);
		}
	}
}
