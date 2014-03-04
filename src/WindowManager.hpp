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

#include <string>

#include "DisplayDevice.hpp"
#include "Renderable.hpp"
#include "Texture.hpp"
#include "WindowManagerFwd.hpp"

namespace Graphics
{
	class WindowManager
	{
	public:
		enum FullScreenMode {
			WINDOWED_MODE,
			FULLSCREEN_WINDOWED_MODE,
			FULLSCREEN_MODE,
		};
		explicit WindowManager(const std::string& title);
		virtual ~WindowManager();
		virtual void create_window(size_t width, size_t height) = 0;
		virtual void destroy_window() = 0;
		
		virtual bool set_window_size(size_t width, size_t height) = 0;
		virtual bool auto_window_size(size_t& width, size_t& height) = 0;

		virtual void set_window_title(const std::string& title) = 0;
		virtual void set_window_icon(const std::string& name) = 0;

		virtual void Render(const Render::RenderablePtr& r) = 0;

		virtual SurfacePtr CreateSurface(size_t width, 
			size_t height, 
			size_t bpp, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask) = 0;
		virtual SurfacePtr CreateSurface(size_t width, 
			size_t height, 
			size_t bpp, 
			size_t row_pitch, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask, 
			void* pixels) = 0;
		virtual SurfacePtr CreateSurface(const std::string& filename) = 0;

		virtual void swap() = 0;

		void map_mouse_position(size_t* x, size_t* y);

		void enable_16bpp(bool bpp=true);
		void enable_multisampling(bool multi_sampling=true, size_t samples=4);
		void enable_resizeable_window(bool en=true);
		void set_fullscreen_mode(FullScreenMode mode);
		void enable_vsync(bool en=true);

		bool use_16bpp() const { return use_16bpp_; }
		bool use_multi_sampling() const { return use_multi_sampling_; }
		size_t multi_samples() const { return samples_; }
		bool resizeable() const { return is_resizeable_; }
		FullScreenMode fullscreen_mode() const { return fullscreen_mode_; }
		bool vsync() const { return use_vsync_; }

		size_t width() const { return width_; }
		size_t height() const { return height_; }

		size_t logical_width() const { return logical_width_; }
		size_t logical_height() const { return logical_height_; }

		const std::string& title() const { return title_; }

		void set_clear_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255);
		void set_clear_color(float r, float g, float b, float a=1.0f);
		// color clear_color() const { return clear_color_; }

		DisplayDevicePtr GetDisplayDevice() { return display_; }

		TexturePtr CreateTexture(const variant& node);
		TexturePtr CreateTexture(const std::string& filename, 
			Texture::TextureType type=Texture::TextureType::TEXTURE_2D, 
			int mipmap_levels=0);
		TexturePtr CreateTexture(unsigned width, PixelFormat::PixelFormatConstant fmt);
		TexturePtr CreateTexture(unsigned width, unsigned height, PixelFormat::PixelFormatConstant fmt, Texture::TextureType type=Texture::TextureType::TEXTURE_2D);
		TexturePtr CreateTexture(unsigned width, unsigned height, unsigned depth, PixelFormat::PixelFormatConstant fmt);

		void BlitTexture(const TexturePtr& tex, int dstx, int dsty, int dstw, int dsth, float rotation=0.0f, int srcx=0, int srcy=0, int srcw=0, int srch=0);

		static WindowManagerPtr factory(const std::string& title, const std::string& wnd_hint="", const std::string& rend_hint="");
	protected:
		size_t width_;
		size_t height_;
		size_t logical_width_;
		size_t logical_height_;
		// XXX repolace the following with a proper color interface.
		float clear_color_[4];

		DisplayDevicePtr display_;

		virtual void change_fullscreen_mode() = 0;
		virtual void handle_set_clear_color() = 0;
	private:
		bool use_16bpp_;
		bool use_multi_sampling_;
		size_t samples_;
		bool is_resizeable_;
		FullScreenMode fullscreen_mode_;
		std::string title_;
		bool use_vsync_;

		WindowManager();
		WindowManager(const WindowManager&);
	};
}
