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
		uint32_t amask) : has_data_(false)
	{
		surface_ = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
		ASSERT_LOG(surface_ != NULL, "Error creating surface: " << SDL_GetError());
		SetPixelFormat(PixelFormatPtr(new SDLPixelFormat(surface_->format)));
	}

	SurfaceSDL::SurfaceSDL(size_t width, 
		size_t height, 
		size_t bpp, 
		size_t row_pitch,
		uint32_t rmask, 
		uint32_t gmask, 
		uint32_t bmask, 
		uint32_t amask, 
		void* pixels) : has_data_(true)
	{
		ASSERT_LOG(pixels != NULL, "NULL value for pixels while creating surface.");
		surface_ = SDL_CreateRGBSurfaceFrom(pixels, width, height, bpp, row_pitch, rmask, gmask, bmask, amask);
		ASSERT_LOG(surface_ != NULL, "Error creating surface: " << SDL_GetError());
		SetPixelFormat(PixelFormatPtr(new SDLPixelFormat(surface_->format)));
	}

	SurfaceSDL::~SurfaceSDL()
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null in destructor");
		SDL_FreeSurface(surface_);
	}

	const void* SurfaceSDL::Pixels() const
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		// technically surface_->locked is an internal implementation detail.
		// but we'll live with using it.
		if(SDL_MUSTLOCK(surface_) && !surface_->locked) {
			ASSERT_LOG(false, "Surface is marked as needing to be locked but is not locked on Pixels access.");
		}
		// Kind of a kludge, since someone could use this for a read not just a write.
		// XXXX Maybe a better scenario, would be to have a WritePixels() function that
		// takes a source pixel format and the data then converts and writes it.
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

	bool SurfaceSDL::SetClipRect(const rect& r)
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect sr = r.sdl_rect();
		return SDL_SetClipRect(surface_, &sr) == SDL_TRUE;
	}

	const rect SurfaceSDL::GetClipRect()
	{
		ASSERT_LOG(surface_ != NULL, "surface_ is null");
		SDL_Rect sr;
		SDL_GetClipRect(surface_, &sr);
		return rect(sr);
	}

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

	void SurfaceSDL::WritePixels(size_t bpp, 
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
		SetPixelFormat(PixelFormatPtr(new SDLPixelFormat(surface_->format)));
	}

	void SurfaceSDL::WritePixels(const void* pixels) 
	{
		SurfaceLock lock(SurfacePtr(this));
		memcpy(surface_->pixels, pixels, row_pitch() * width());
	}

	void SurfaceSDL::SetBlendMode(Surface::BlendMode bm) 
	{
		SDL_BlendMode sdl_bm;
		switch(bm) {
			case BLEND_MODE_NONE:	sdl_bm = SDL_BLENDMODE_NONE; break;
			case BLEND_MODE_BLEND:	sdl_bm = SDL_BLENDMODE_BLEND; break;
			case BLEND_MODE_ADD:	sdl_bm = SDL_BLENDMODE_ADD; break;
			case BLEND_MODE_MODULATE:	sdl_bm = SDL_BLENDMODE_MOD; break;
		}
		SDL_SetSurfaceBlendMode(surface_, sdl_bm);
	}

	Surface::BlendMode SurfaceSDL::GetBlendMode() const 
	{
		SDL_BlendMode sdl_bm;
		SDL_GetSurfaceBlendMode(surface_, &sdl_bm);
		switch(sdl_bm) {
		case SDL_BLENDMODE_NONE:	return BLEND_MODE_NONE;
		case SDL_BLENDMODE_BLEND:	return BLEND_MODE_BLEND;
		case SDL_BLENDMODE_ADD:		return BLEND_MODE_ADD;
		case SDL_BLENDMODE_MOD:		return BLEND_MODE_MODULATE;
		}
		ASSERT_LOG(false, "Unrecognised SDL blend mode: " << sdl_bm);
		return BLEND_MODE_NONE;
	}

	//////////////////////////////////////////////////////////////////////////
	// SDLPixelFormat
	namespace 
	{
		uint8_t count_bits_set(uint32_t v)
		{
			v = v - ((v >> 1) & 0x55555555);
			v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
			return uint8_t(((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24);
		}
	}

	SDLPixelFormat::SDLPixelFormat(const SDL_PixelFormat* pf)
		: pf_(pf)
	{
		ASSERT_LOG(pf != NULL, "SDLPixelFormat constructor passeda null pixel format.");
	}

	SDLPixelFormat::~SDLPixelFormat()
	{
	}

	uint8_t SDLPixelFormat::BitsPerPixel() const 
	{
		return pf_->BitsPerPixel;
	}

	uint8_t SDLPixelFormat::BytesPerPixel() const 
	{
		return pf_->BytesPerPixel;
	}

	bool SDLPixelFormat::IsYuvPlanar() const 
	{
		return pf_->format == SDL_PIXELFORMAT_YV12 || pf_->format == SDL_PIXELFORMAT_IYUV;
	}

	bool SDLPixelFormat::IsYuvPacked() const 
	{
		return pf_->format == SDL_PIXELFORMAT_YUY2 
			|| pf_->format == SDL_PIXELFORMAT_UYVY
			|| pf_->format == SDL_PIXELFORMAT_YVYU;
	}

	bool SDLPixelFormat::YuvHeightReversed() const 
	{
		return false;
	}

	bool SDLPixelFormat::IsInterlaced() const 
	{
		return false;
	}
		
	bool SDLPixelFormat::IsRGB() const 
	{
		return !SDL_ISPIXELFORMAT_FOURCC(pf_->format);
	}

	bool SDLPixelFormat::HasRedChannel() const 
	{
		return IsRGB() && pf_->Rmask != 0;
	}

	bool SDLPixelFormat::HasGreenChannel() const 
	{
		return IsRGB() && pf_->Gmask != 0;
	}

	bool SDLPixelFormat::HasBlueChannel() const 
	{
		return IsRGB() && pf_->Bmask != 0;
	}

	bool SDLPixelFormat::HasAlphaChannel() const 
	{
		return IsRGB() && pf_->Amask != 0;
	}

	bool SDLPixelFormat::HasLuminance() const 
	{
		return IsRGB() && pf_->Rmask != 0;
	}

	uint32_t SDLPixelFormat::RedMask() const 
	{
		ASSERT_LOG(IsRGB(), "Asked for RedMask of non-RGB surface.");
		return pf_->Rmask;
	}

	uint32_t SDLPixelFormat::GreenMask() const 
	{
		ASSERT_LOG(IsRGB(), "Asked for GreenMask of non-RGB surface.");
		return pf_->Gmask;
	}

	uint32_t SDLPixelFormat::BlueMask() const 
	{
		ASSERT_LOG(IsRGB(), "Asked for BlueMask of non-RGB surface.");
		return pf_->Bmask;
	}

	uint32_t SDLPixelFormat::AlphaMask() const 
	{
		return pf_->Amask;
	}

	uint32_t SDLPixelFormat::LuminanceMask() const 
	{
		ASSERT_LOG(IsRGB(), "Asked for LuminanceMask of non-RGB surface.");
		return pf_->Rmask;
	}

	uint8_t SDLPixelFormat::RedBits() const 
	{
		ASSERT_LOG(IsRGB(), "Asked for RedBits() of non-RGB surface.");
		return count_bits_set(pf_->Rmask);
	}

	uint8_t SDLPixelFormat::GreenBits() const 
	{
		ASSERT_LOG(IsRGB(), "Asked for GreenBits() of non-RGB surface.");
		return count_bits_set(pf_->Gmask);
	}

	uint8_t SDLPixelFormat::BlueBits() const 
	{
		ASSERT_LOG(IsRGB(), "Asked for BlueBits() of non-RGB surface.");
		return count_bits_set(pf_->Bmask);
	}

	uint8_t SDLPixelFormat::AlphaBits() const 
	{
		ASSERT_LOG(IsRGB(), "Asked for AlphaBits() of non-RGB surface.");
		return count_bits_set(pf_->Amask);
	}

	uint8_t SDLPixelFormat::LuminanceBits() const 
	{
		ASSERT_LOG(IsRGB(), "Asked for LuminanceBits() of non-RGB surface.");
		return count_bits_set(pf_->Rmask);
	}

	bool SDLPixelFormat::HasPalette() const 
	{
		return pf_->palette != NULL;
	}
}
