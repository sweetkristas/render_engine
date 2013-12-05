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

		class init_error : public std::exception
		{
		public:
			init_error() : exception(), msg_(SDL_GetError())
			{}
			init_error(const std::string& msg) : exception(), msg_(msg)
			{}
			virtual ~init_error() throw()
			{}
			virtual const char* what() const throw() { return msg_.c_str(); }
		private:
			std::string msg_;
		};

		class SDL
		{
		public:
			SDL(Uint32 flags = SDL_INIT_VIDEO)
			{
				if (SDL_Init(flags) < 0) {
					std::stringstream ss;
					ss << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
					throw init_error(ss.str());
				}
			}

			virtual ~SDL()
			{
				SDL_Quit();
			}
		};

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
						LOG_MSG(LOG_WARN, "MSAA(" << multi_samples() << ") requested but mutlisample buffer couldn't be allocated.");
					} else {
						size_t msaa = next_pow2(multi_samples());
						if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa) != 0) {
							LOG_MSG(LOG_INFO, "Requested MSAA of " << msaa << " but couldn't allocate");
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
				SDL_DestroyRenderer(renderer_);
				renderer_ = NULL;
				if(context_) {
					SDL_GL_DeleteContext(context_);
					context_ = NULL;
				}
				SDL_DestroyWindow(wnd);
			});
			ASSERT_LOG(window_ != NULL, "FATAL: Failed to create window: " << SDL_GetError());
			if(display_->id() == DisplayDevice::DISPLAY_DEVICE_OPENGL) {
				context_ = SDL_GL_CreateContext(window_.get());	
				ASSERT_LOG(context_ != NULL, "FATAL: Failed to GL Context: " << SDL_GetError());
			}
			Uint32 rnd_flags = SDL_RENDERER_ACCELERATED;
			if(vsync()) {
				rnd_flags |= SDL_RENDERER_PRESENTVSYNC;
			}
			if(renderer_hint_.size() > 4 && renderer_hint_.substr(0,4) == "sdl:") {
				SDL_SetHint(SDL_HINT_RENDER_DRIVER, renderer_hint_.substr(5).c_str());
			}
			renderer_ = SDL_CreateRenderer(window_.get(), -1, rnd_flags);
			ASSERT_LOG(renderer_ != NULL, "FATAL: Failed to create renderer: " << SDL_GetError());

			display_->init(width_, height_);
			display_->print_device_info();
		}

		void destroy_window() {
			display_.reset();
			window_.reset();
		}

		void set_window_icon(const std::string& name) {
			// XXX SDL_SetWindowIcon(window_.get(), wm_icon.get());
		}
		
		bool set_window_size(size_t width, size_t height) {
		}

		bool auto_window_size(size_t& width, size_t& height) {
		}

		void set_window_title(const std::string& title) {
			ASSERT_LOG(window_ != NULL, "FATAL: Window is null");
			SDL_SetWindowTitle(window_.get(), title.c_str());		
		}
	protected:
		void change_fullscreen_mode() {
			// XXX
		}
	private:
		DisplayDevicePtr display_;
		SDL_WindowPtr window_;
		SDL_Renderer* renderer_;
		SDL_GLContext context_;
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
	}

	WindowManager::~WindowManager()
	{
	}

	void WindowManager::enable_16bpp(bool bpp=true) {
		use_16bpp_ = bpp;
	}

	void WindowManager::enable_multisampling(bool multi_sampling=true, size_t samples=4) {
		use_multi_sampling_ = multi_sampling;
		samples_ = samples;
	}

	void WindowManager::enable_resizeable_window(bool en=true) {
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

	WindowManagerPtr WindowManager::factory(const std::string& title, const std::string& wnd_hint, const std::string& rend_hint)
	{
		// We really only support one sub-class of the window manager
		// at the moment, so we just return it. We could use hint in the
		// future if we had more.
		return WindowManagerPtr(new SDLWindowManager(title, rend_hint));
	}
}