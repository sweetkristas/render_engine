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
#include "DisplayDevice.hpp"
#include "DisplayDeviceOpenGL.hpp"
#include "DisplayDeviceOpenGLFixed.hpp"
#include "DisplayDeviceSDL.hpp"
//#include "DisplayDeviceD3D.hpp"

namespace graphics
{
	namespace 
	{
		// A quick hack to do case insensitive case compare, doesn't support utf-8,
		// doesn't support unicode comparison between code-points.
		// But then it isn't intended to.
		bool icasecmp(const std::string& l, const std::string& r)
		{
			return l.size() == r.size()
				&& equal(l.cbegin(), l.cend(), r.cbegin(),
					[](std::string::value_type l1, std::string::value_type r1)
						{ return toupper(l1) == toupper(r1); });
		}	
	}

	DisplayDevice::DisplayDevice()
	{
	}

	DisplayDevice::~DisplayDevice()
	{
	}

	DisplayDevicePtr DisplayDevice::factory(const std::string& type)
	{
		if(icasecmp(type, "opengl")) {
			return DisplayDevicePtr(new DisplayDeviceOpenGL());
		} else if(icasecmp(type, "opengles")) {
		} else if(type.size() >= 3 && icasecmp(type.substr(0,3), "sdl")) {
		} else if(icasecmp(type, "direct3d")) {
			ASSERT_LOG(false, "FATAL: Use of Direct3D isn't directly supported");
		}
		ASSERT_LOG(false, "FATAL: Unknown display device type: " << type);
		return DisplayDevicePtr();
	}
}
