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

#include <map>

#include "asserts.hpp"
#include "DisplayDevice.hpp"
#include "logger.hpp"

namespace Graphics
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

		typedef std::map<std::string, std::function<DisplayDevicePtr()>> DisplayDeviceRegistry;
		DisplayDeviceRegistry& get_display_registry()
		{
			static DisplayDeviceRegistry res;
			return res;
		}
	}

	DisplayDevice::DisplayDevice()
	{
	}

	DisplayDevice::~DisplayDevice()
	{
	}

	void DisplayDevice::set_clear_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		set_clear_color(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
	}

	DisplayDevicePtr DisplayDevice::factory(const std::string& type)
	{
		ASSERT_LOG(!get_display_registry().empty(), "No display device drivers registered.");
		auto it = get_display_registry().find(type);
		if(it == get_display_registry().end()) {			
			LOG_WARN("Requested display driver '" << type << "' not found, using default: " << get_display_registry().begin()->first);
			return get_display_registry().begin()->second();
		}
		return it->second();
	}

	void DisplayDevice::RegisterFactoryFunction(const std::string& type, std::function<DisplayDevicePtr()> create_fn)
	{
		auto it = get_display_registry().find(type);
		if(it != get_display_registry().end()) {
			LOG_WARN("Overwriting the Display Device Driver: " << type);
		}
		get_display_registry()[type] = create_fn;
	}

	DisplayDeviceDef::DisplayDeviceDef(const Render::RenderVariableList& arv, const Render::RenderVariableList& urv)
		: attrib_render_vars_(arv), uniform_render_vars_(urv)
	{
	}

	DisplayDeviceDef::~DisplayDeviceDef()
	{
	}

	void DisplayDeviceDef::SetHint(const std::string& hint_name, const std::string& hint)
	{
		HintList hint_list(1,hint);
		hints_.insert(std::make_pair(hint_name, hint_list));
	}

	void DisplayDeviceDef::SetHint(const std::string& hint_name, const HintList& hint)
	{
		hints_[hint_name] = hint;
	}

	DisplayDeviceData::DisplayDeviceData()
	{
	}

	DisplayDeviceData::~DisplayDeviceData()
	{
	}
}
