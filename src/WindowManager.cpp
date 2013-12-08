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

#include <cctype>
#include <sstream>

#include "asserts.hpp"
#include "DisplayDevice.hpp"
#include "logger.hpp"
#include "SDL.h"
#include "WindowManager.hpp"

namespace graphics
{
	namespace 
	{
		typedef std::shared_ptr<SDL_Window> SDL_WindowPtr;

		uint32_t next_pow2(uint32_t v) 
		{
			--v;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			return ++v;
		}
	}

	class SDLWindowManager : public WindowManager
	{
	public:
		SDLWindowManager(const std::string& title, const std::string& renderer_hint) 
			: WindowManager(title), 
			renderer_hint_(renderer_hint),
			renderer_(NULL),
			context_(NULL) {
			if(renderer_hint_.empty()) {
				renderer_hint_ = "opengl";
			}			
		}
		~SDLWindowManager() {
			destroy_window();
		}

		void create_window(size_t width, size_t height) {
			logical_width_ = width_ = width;
			logical_height_ = height_ = height;

			Uint32 wnd_flags = 0;

			display_ = DisplayDevice::factory(renderer_hint_);

			if(display_->id() == DisplayDevice::DISPLAY_DEVICE_OPENGL) {
				// We need to do extra SDL set-up for an OpenGL context.
				// Since these parameter's need to be set-up before context
				// creation.
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
				SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
				SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
				if(use_16bpp()) {
					SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
					SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
					SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
					SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
				} else {
					SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
					SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
					SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
					SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
				}
				if(use_multi_sampling()) {
					if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) != 0) {
						LOG_WARN("MSAA(" << multi_samples() << ") requested but mutlisample buffer couldn't be allocated.");
					} else {
						size_t msaa = next_pow2(multi_samples());
						if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa) != 0) {
							LOG_INFO("Requested MSAA of " << msaa << " but couldn't allocate");
						}
					}
				}
				wnd_flags |= SDL_WINDOW_OPENGL;
			}

			if(resizeable()) {
				wnd_flags |= SDL_WINDOW_RESIZABLE;
			}

			int x = SDL_WINDOWPOS_CENTERED;
			int y = SDL_WINDOWPOS_CENTERED;
			int w = width_;
			int h = height_;
			switch(fullscreen_mode()) {
				case WINDOWED_MODE:		break;
				case FULLSCREEN_WINDOWED_MODE:
					x = y = SDL_WINDOWPOS_UNDEFINED;
					w = h = 0;
					wnd_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
					break;
				case FULLSCREEN_MODE:
					x = y = SDL_WINDOWPOS_UNDEFINED;
					wnd_flags |= SDL_WINDOW_FULLSCREEN;
					break;
			}
			window_.reset(SDL_CreateWindow(title().c_str(), x, y, w, h, wnd_flags), [&](SDL_Window* wnd){
				if(display_->id() != DisplayDevice::DISPLAY_DEVICE_SDL) {
					SDL_DestroyRenderer(renderer_);
				}
				display_.reset();
				if(context_) {
					SDL_GL_DeleteContext(context_);
					context_ = NULL;
				}
				SDL_DestroyWindow(wnd);
			});

			if(display_->id() != DisplayDevice::DISPLAY_DEVICE_SDL) {
				Uint32 rnd_flags = SDL_RENDERER_ACCELERATED;
				if(vsync()) {
					rnd_flags |= SDL_RENDERER_PRESENTVSYNC;
				}
				renderer_ = SDL_CreateRenderer(window_.get(), -1, rnd_flags);
				ASSERT_LOG(renderer_ != NULL, "Failed to create renderer: " << SDL_GetError());				
			}

			ASSERT_LOG(window_ != NULL, "Failed to create window: " << SDL_GetError());
			if(display_->id() == DisplayDevice::DISPLAY_DEVICE_OPENGL) {
				context_ = SDL_GL_CreateContext(window_.get());	
				ASSERT_LOG(context_ != NULL, "Failed to GL Context: " << SDL_GetError());
			}

			display_->set_clear_color(clear_color_[0],clear_color_[1],clear_color_[2],clear_color_[3]);
			display_->init(width_, height_);
			display_->print_device_info();
			display_->clear(DisplayDevice::DISPLAY_CLEAR_ALL);
			swap();
		}

		void destroy_window() {
			window_.reset();
		}

		void swap() {
			// This is a little bit hacky -- ideally the display device should swap buffers.
			// But SDL provides a device independent way of doing it which is really nice.
			// So we use that.
			if(display_->id() == DisplayDevice::DISPLAY_DEVICE_OPENGL) {
				SDL_GL_SwapWindow(window_.get());
			} else {
				// default to delegating to the display device.
				display_->swap();
			}
		}

		void set_window_icon(const std::string& name) {
			// XXX SDL_SetWindowIcon(window_.get(), wm_icon.get());
		}
		
		bool set_window_size(size_t width, size_t height) {
			// XXX
			return false;
		}

		bool auto_window_size(size_t& width, size_t& height) {
			// XXX
			return false;
		}

		void handle_set_clear_color()
		{
			if(display_ != NULL) {
				display_->set_clear_color(clear_color_[0],clear_color_[1],clear_color_[2],clear_color_[3]);
			}
		}

		void set_window_title(const std::string& title) {
			ASSERT_LOG(window_ != NULL, "Window is null");
			SDL_SetWindowTitle(window_.get(), title.c_str());		
		}
	protected:
		void change_fullscreen_mode() {
			// XXX
		}
	private:		
		DisplayDevicePtr display_;
		SDL_WindowPtr window_;
		SDL_GLContext context_;
		SDL_Renderer* renderer_;
		std::string renderer_hint_;
		SDLWindowManager(const SDLWindowManager&);
	};

	WindowManager::WindowManager(const std::string& title)
		: width_(0), 
		height_(0),
		logical_width_(0),
		logical_height_(0),
		use_16bpp_(false),
		use_multi_sampling_(false),
		samples_(4),
		is_resizeable_(false),
		title_(title)
	{
		clear_color_[0] = 0.0f;
		clear_color_[1] = 0.0f;
		clear_color_[2] = 0.0f;
		clear_color_[3] = 1.0f;
	}

	WindowManager::~WindowManager()
	{
	}

	void WindowManager::enable_16bpp(bool bpp) {
		use_16bpp_ = bpp;
	}

	void WindowManager::enable_multisampling(bool multi_sampling, size_t samples) {
		use_multi_sampling_ = multi_sampling;
		samples_ = samples;
	}

	void WindowManager::enable_resizeable_window(bool en) {
		is_resizeable_ = en;
	}

	void WindowManager::set_fullscreen_mode(FullScreenMode mode)
	{
		bool modes_differ = fullscreen_mode_ != mode;
		fullscreen_mode_ = mode;
		if(modes_differ) {
			change_fullscreen_mode();
		}
	}

	void WindowManager::enable_vsync(bool en)
	{
		use_vsync_ = en;
	}

	void WindowManager::map_mouse_position(size_t* x, size_t* y) 
	{
		if(x) {
			*x = int(*x * double(logical_width_) / width_);
		}
		if(y) {
			*y = int(*y * double(logical_height_) / height_);
		}
	}

	void WindowManager::set_clear_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		clear_color_[0] = r/255.0f;
		clear_color_[1] = g/255.0f;
		clear_color_[2] = b/255.0f;
		clear_color_[3] = a/255.0f;
		handle_set_clear_color();
	}

	void WindowManager::set_clear_color(float r, float g, float b, float a)
	{
		clear_color_[0] = r;
		clear_color_[1] = g;
		clear_color_[2] = b;
		clear_color_[3] = a;
		handle_set_clear_color();
	}

	WindowManagerPtr WindowManager::factory(const std::string& title, const std::string& wnd_hint, const std::string& rend_hint)
	{
		// We really only support one sub-class of the window manager
		// at the moment, so we just return it. We could use hint in the
		// future if we had more.
		return WindowManagerPtr(new SDLWindowManager(title, rend_hint));
	}
}