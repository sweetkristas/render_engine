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

#include <memory>
#include <string>

namespace Graphics
{
	class DisplayDevice;
	typedef std::shared_ptr<DisplayDevice> DisplayDevicePtr;

	class DisplayDevice
	{
	public:
		enum DisplayDeviceId {
			// Display device is OpenGL 2.1 compatible, using shaders.
			DISPLAY_DEVICE_OPENGL,
			// Display device is OpenGLES 2.0, using shaders
			DISPLAY_DEVICE_OPENGLES,
			// Display device is OpenGL 1.1, fixed function pipeline
			DISPLAY_DEVICE_OPENGL_FIXED,
			// Display device is whatever SDL wants to use
			DISPLAY_DEVICE_SDL,
			// Display device is Direct3D
			DISPLAY_DEVICE_D3D,
		};
		enum ClearFlags {
			DISPLAY_CLEAR_COLOR		= 1,
			DISPLAY_CLEAR_DEPTH		= 2,
			DISPLAY_CLEAR_STENCIL	= 4,
			DISPLAY_CLEAR_ALL		= 0xffffffff,
		};

		DisplayDevice();
		virtual ~DisplayDevice();

		virtual DisplayDeviceId id() const = 0;

		virtual void set_clear_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		virtual void set_clear_color(float r, float g, float b, float a) = 0;

		virtual void clear(uint32_t clr) = 0;
		virtual void swap() = 0;

		virtual void init(size_t width, size_t height) = 0;
		virtual void print_device_info() = 0;

		static DisplayDevicePtr factory(const std::string& type);
	private:
		DisplayDevice(const DisplayDevice&);
	};
}
