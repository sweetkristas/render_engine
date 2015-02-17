/*
	Copyright (C) 2013-2014 by Kristina Simpson <sweet.kristas@gmail.com>
	
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

#include "Color.hpp"
#include "DisplayDevice.hpp"
#include "Texture.hpp"
#include "PixelFormat.hpp"
#include "Renderable.hpp"
#include "WindowManagerFwd.hpp"

namespace KRE
{
	enum class FullScreenMode {
		WINDOWED,
		FULLSCREEN_WINDOWED,
	};

	struct WindowMode
	{
		int width;
		int height;
		PixelFormatPtr pf;
		int refresh;
	};

	inline bool operator==(const WindowMode& lhs, const WindowMode& rhs) {
		return lhs.width == rhs.width && lhs.height == rhs.height;
	}

	class WindowManager
	{
	public:
		explicit WindowManager(const std::string& title);
		virtual ~WindowManager();
		void createWindow(int width, int height);
		void destroyWindow();
		
		virtual bool setWindowSize(int width, int height) = 0;
		virtual bool autoWindowSize(int& width, int& height) = 0;
		
		bool setLogicalWindowSize(int width, int height);

		virtual void setWindowTitle(const std::string& title) = 0;
		virtual void setWindowIcon(const std::string& name) = 0;

		virtual unsigned getWindowID() const = 0;

		virtual void render(const Renderable* r) const = 0;

		virtual void swap() = 0;

		void mapMousePosition(int* x, int* y);

		void enable16bpp(bool bpp=true);
		void enableMultisampling(bool multi_sampling=true, int samples=4);
		void enableResizeableWindow(bool en=true);
		void setFullscreenMode(FullScreenMode mode);
		void enableVsync(bool en=true);

		bool use16bpp() const { return use_16bpp_; }
		bool useMultiSampling() const { return use_multi_sampling_; }
		int multiSamples() const { return samples_; }
		bool resizeable() const { return is_resizeable_; }
		FullScreenMode fullscreenMode() const { return fullscreen_mode_; }
		bool vSync() const { return use_vsync_; }

		int width() const { return width_; }
		int height() const { return height_; }

		int logicalWidth() const { return logical_width_; }
		int logicalHeight() const { return logical_height_; }

		const std::string& getTitle() const { return title_; }

		void setClearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) const;
		void setClearColor(float r, float g, float b, float a=1.0f) const;
		void setClearColor(const Color& color) const;
		
		Color getClearColor() const { return clear_color_; }

		virtual void clear(ClearFlags f) = 0;

		virtual void setViewPort(int x, int y, int width, int height) = 0;

		void saveFrameBuffer(const std::string& filename);

		virtual std::vector<WindowMode> getWindowModes(std::function<bool(const WindowMode&)> mode_filter) = 0;

		void notifyNewWindowSize(int new_width, int new_height);

		static WindowManagerPtr createInstance(const std::string& title, const std::string& wnd_hint="", const std::string& rend_hint="");
		static std::vector<WindowManagerPtr> getWindowList();
		static WindowManagerPtr getWindowFromID(unsigned id);
		static WindowManagerPtr getMainWindow();
	protected:
		int width_;
		int height_;
		int logical_width_;
		int logical_height_;
		mutable Color clear_color_;

		DisplayDevicePtr display_;
	private:
		virtual void changeFullscreenMode() = 0;
		virtual void handleSetClearColor() const = 0;
		virtual bool handleLogicalWindowSizeChange() = 0;
		virtual bool handlePhysicalWindowSizeChange() = 0;
		virtual void doCreateWindow(int width, int height) = 0;
		virtual void doDestroyWindow() = 0;

		bool use_16bpp_;
		bool use_multi_sampling_;
		int samples_;
		bool is_resizeable_;
		FullScreenMode fullscreen_mode_;
		std::string title_;
		bool use_vsync_;

		WindowManager();
		WindowManager(const WindowManager&);
	};
}
