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
#include "SurfaceSDL.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include "WindowManager.hpp"

namespace KRE
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

		DisplayDevicePtr& current_display_device()
		{
			static DisplayDevicePtr res;
			return res;
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
			current_display_device() = display_ = DisplayDevice::Factory(renderer_hint_);
		}
		~SDLWindowManager() {
			DestroyWindow();
		}

		void CreateWindow(size_t width, size_t height) override {
			logical_width_ = width_ = width;
			logical_height_ = height_ = height;

			Uint32 wnd_flags = 0;

			if(display_->ID() == DisplayDevice::DISPLAY_DEVICE_OPENGL) {
				// We need to do extra SDL set-up for an OpenGL context.
				// Since these parameter's need to be set-up before context
				// creation.
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
				SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
				SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
				if(Use16bpp()) {
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
				if(UseMultiSampling()) {
					if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) != 0) {
						LOG_WARN("MSAA(" << MultiSamples() << ") requested but mutlisample buffer couldn't be allocated.");
					} else {
						size_t msaa = next_pow2(MultiSamples());
						if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa) != 0) {
							LOG_INFO("Requested MSAA of " << msaa << " but couldn't allocate");
						}
					}
				}
				wnd_flags |= SDL_WINDOW_OPENGL;
			}

			if(Resizeable()) {
				wnd_flags |= SDL_WINDOW_RESIZABLE;
			}

			int x = SDL_WINDOWPOS_CENTERED;
			int y = SDL_WINDOWPOS_CENTERED;
			int w = width_;
			int h = height_;
			switch(FullscreenMode()) {
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
			window_.reset(SDL_CreateWindow(Title().c_str(), x, y, w, h, wnd_flags), [&](SDL_Window* wnd){
				if(display_->ID() != DisplayDevice::DISPLAY_DEVICE_SDL) {
					SDL_DestroyRenderer(renderer_);
				}
				display_.reset();
				if(context_) {
					SDL_GL_DeleteContext(context_);
					context_ = NULL;
				}
				SDL_DestroyWindow(wnd);
			});

			if(display_->ID() != DisplayDevice::DISPLAY_DEVICE_SDL) {
				Uint32 rnd_flags = SDL_RENDERER_ACCELERATED;
				if(Vsync()) {
					rnd_flags |= SDL_RENDERER_PRESENTVSYNC;
				}
				renderer_ = SDL_CreateRenderer(window_.get(), -1, rnd_flags);
				ASSERT_LOG(renderer_ != NULL, "Failed to create renderer: " << SDL_GetError());				
			}

			ASSERT_LOG(window_ != NULL, "Failed to create window: " << SDL_GetError());
			if(display_->ID() == DisplayDevice::DISPLAY_DEVICE_OPENGL) {
				context_ = SDL_GL_CreateContext(window_.get());	
				ASSERT_LOG(context_ != NULL, "Failed to GL Context: " << SDL_GetError());
			}

			display_->SetClearColor(clear_color_);
			display_->Init(width_, height_);
			display_->PrintDeviceInfo();
			display_->Clear(DisplayDevice::DISPLAY_CLEAR_ALL);
			Swap();
		}

		void DestroyWindow() override {
			window_.reset();
		}

		void Clear(DisplayDevice::ClearFlags f) override {
			display_->Clear(DisplayDevice::DISPLAY_CLEAR_ALL);
		}

		void Swap() override {
			// This is a little bit hacky -- ideally the display device should swap buffers.
			// But SDL provides a device independent way of doing it which is really nice.
			// So we use that.
			if(display_->ID() == DisplayDevice::DISPLAY_DEVICE_OPENGL) {
				SDL_GL_SwapWindow(window_.get());
			} else {
				// default to delegating to the display device.
				display_->Swap();
			}
		}

		void SetWindowIcon(const std::string& name) override {
			// XXX SDL_SetWindowIcon(window_.get(), wm_icon.get());
		}
		
		bool SetWindowSize(size_t width, size_t height) override {
			// XXX
			return false;
		}

		bool SetLogicalWindowSize(size_t width, size_t height) override {
			// XXX
			return false;
		}

		bool AutoWindowSize(size_t& width, size_t& height) override {
			// XXX
			return false;
		}

		SurfacePtr CreateSurface(size_t width, 
			size_t height, 
			size_t bpp, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask) override {
			return SurfacePtr(new SurfaceSDL(width, height, bpp, rmask, gmask, bmask, amask));
		}
		SurfacePtr CreateSurface(size_t width, 
			size_t height, 
			size_t bpp, 
			size_t row_pitch, 
			uint32_t rmask, 
			uint32_t gmask, 
			uint32_t bmask, 
			uint32_t amask, 
			void* pixels) override {
			// XXX feed into surface cache. 
			// will need to update cache if pixels change.
			return SurfacePtr(new SurfaceSDL(width, height, bpp, row_pitch, rmask, gmask, bmask, amask, pixels));
		}

		SurfacePtr CreateSurface(const std::string& filename) override {
			// XXX Here is were we can abstract image loading and provide an
			// image cache.
			// return SurfacePtr(WindowManager::LoadImage(filename));
			return Surface::Create(filename);
		}

		void SetWindowTitle(const std::string& title) override {
			ASSERT_LOG(window_ != NULL, "Window is null");
			SDL_SetWindowTitle(window_.get(), title.c_str());		
		}

		virtual void Render(const RenderablePtr& r) override {
			ASSERT_LOG(display_ != NULL, "No display to render to.");
			display_->Render(r);
		}
	protected:
	private:
		void HandleSetClearColor() {
			if(display_ != NULL) {
				display_->SetClearColor(clear_color_);
			}
		}
		void ChangeFullscreenMode() override {
			// XXX
		}
		bool HandleLogicalWindowSizeChange() override {
			// XXX
			return false;
		}

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
		title_(title),
		clear_color_(0.0f,0.0f,0.0f,1.0f)
	{
	}

	WindowManager::~WindowManager()
	{
	}

	void WindowManager::Enable16bpp(bool bpp) {
		use_16bpp_ = bpp;
	}

	void WindowManager::EnableMultisampling(bool multi_sampling, size_t samples) {
		use_multi_sampling_ = multi_sampling;
		samples_ = samples;
	}

	void WindowManager::EnableResizeableWindow(bool en) {
		is_resizeable_ = en;
	}

	void WindowManager::SetFullscreenMode(FullScreenMode mode)
	{
		bool modes_differ = fullscreen_mode_ != mode;
		fullscreen_mode_ = mode;
		if(modes_differ) {
			ChangeFullscreenMode();
		}
	}

	void WindowManager::EnableVsync(bool en)
	{
		use_vsync_ = en;
	}

	void WindowManager::MapMousePosition(size_t* x, size_t* y) 
	{
		if(x) {
			*x = int(*x * double(logical_width_) / width_);
		}
		if(y) {
			*y = int(*y * double(logical_height_) / height_);
		}
	}

	bool WindowManager::SetLogicalWindowSize(size_t width, size_t height)
	{
		logical_width_ = width;
		logical_height_ = height;
		return HandleLogicalWindowSizeChange();
	}

	void WindowManager::SetClearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		clear_color_ = Color(r,g,b,a);
		HandleSetClearColor();
	}

	void WindowManager::SetClearColor(float r, float g, float b, float a)
	{
		clear_color_ = Color(r,g,b,a);
		HandleSetClearColor();
	}

	void WindowManager::BlitTexture(const TexturePtr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch)
	{
		ASSERT_LOG(display_ != NULL, "No display to blit to.");
		display_->BlitTexture(tex, dstx, dsty, dstw, dsth, rotation, srcx, srcy, srcw, srch);

	}

	TexturePtr WindowManager::CreateTexture(unsigned width, PixelFormat::PixelFormatConstant fmt)
	{
		ASSERT_LOG(display_ != NULL, "No display to create texture.");
		return display_->CreateTexture(width, 0, fmt);
	}

	// Let's you create a texture which doesn't have an attached surface. 
	// Useful if you are calling Update() on the texture to update the contents.
	// Texture is created with streaming in mind.
	TexturePtr WindowManager::CreateTexture(unsigned width, unsigned height, PixelFormat::PixelFormatConstant fmt, Texture::TextureType type)
	{
		ASSERT_LOG(display_ != NULL, "No display to create texture.");
		return display_->CreateTexture(width, height, fmt, type);
	}

	TexturePtr WindowManager::CreateTexture(unsigned width, unsigned height, unsigned depth, PixelFormat::PixelFormatConstant fmt)
	{
		ASSERT_LOG(display_ != NULL, "No display to create texture.");
		return display_->CreateTexture(width, height, depth, fmt);
	}

	TexturePtr WindowManager::CreateTexture(const variant& node)
	{
		ASSERT_LOG(display_ != NULL, "No display to create texture.");
		ASSERT_LOG(node.has_key("image") || node.has_key("texture"), "Must have either 'image' or 'texture' attribute.");
		const std::string image_name = node.has_key("image") ? node["image"].as_string() : node["texture"].as_string();
		auto surface = CreateSurface(image_name);
		return display_->CreateTexture(surface, node);
	}

	TexturePtr WindowManager::CreateTexture(const std::string& filename, Texture::TextureType type, int mipmap_levels)
	{
		ASSERT_LOG(display_ != NULL, "No display to create texture.");
		auto surface = CreateSurface(filename);
		return display_->CreateTexture(surface, type, mipmap_levels);
	}

	MaterialPtr WindowManager::CreateMaterial(const variant& node)
	{
		ASSERT_LOG(display_ != NULL, "No display to create material.");
		return display_->CreateMaterial(node);
	}

	WindowManagerPtr WindowManager::Factory(const std::string& title, const std::string& wnd_hint, const std::string& rend_hint)
	{
		// We really only support one sub-class of the window manager
		// at the moment, so we just return it. We could use hint in the
		// future if we had more.
		return WindowManagerPtr(new SDLWindowManager(title, rend_hint));
	}

	DisplayDevicePtr WindowManager::GetDisplayDevice()
	{
		ASSERT_LOG(current_display_device() != NULL, "GetDisplayDevice() With no defined display device.");
		return current_display_device();
	}
}
