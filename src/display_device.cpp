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
#include "display_device.hpp"
#include "logger.hpp"

namespace KRE
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

		typedef std::map<std::string, std::function<display_device_ptr()>> display_deviceRegistry;
		display_deviceRegistry& get_display_registry()
		{
			static display_deviceRegistry res;
			return res;
		}

		display_device_ptr& current_display_device()
		{
			static display_device_ptr res;
			return res;
		};
	}

	display_device::display_device()
	{
	}

	display_device::~display_device()
	{
	}

	void display_device::set_clear_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		set_clear_color(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
	}

	display_device_ptr display_device::factory(const std::string& type)
	{
		ASSERT_LOG(!get_display_registry().empty(), "No display device drivers registered.");
		auto it = get_display_registry().find(type);
		if(it == get_display_registry().end()) {			
			LOG_WARN("Requested display driver '" << type << "' not found, using default: " << get_display_registry().begin()->first);
			current_display_device() = get_display_registry().begin()->second();
			return get_display_registry().begin()->second();
		}
		current_display_device() = it->second();
		return it->second();
	}

	display_device_ptr display_device::get_current()
	{
		ASSERT_LOG(current_display_device() != NULL, "display device is NULL");
		return current_display_device();
	}

	void display_device::register_factory_function(const std::string& type, std::function<display_device_ptr()> create_fn)
	{
		auto it = get_display_registry().find(type);
		if(it != get_display_registry().end()) {
			LOG_WARN("Overwriting the Display Device Driver: " << type);
		}
		get_display_registry()[type] = create_fn;
	}

	void display_device::blit_texture(const texture_ptr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch)
	{
		get_current()->do_blit_texture(tex, dstx, dsty, dstw, dsth, rotation, srcx, srcy, srcw, srch);
	}

	attribute_set_ptr display_device::create_attribute_set(bool hardware_hint, bool indexed, bool instanced)
	{
		if(hardware_hint) {
			auto as = display_device::get_current()->handle_create_attribute_set(indexed, instanced);
			if(as) {
				return as;
			}
		}
		return attribute_set_ptr(new attribute_set(indexed, instanced));
	}

	hardware_attribute_ptr display_device::create_attribute_buffer(bool hw_backed, attribute_base* parent)
	{
		if(hw_backed) {
			auto attrib = display_device::get_current()->handle_create_attribute(parent);
			if(attrib) {
				return attrib;
			}
		}
		return hardware_attribute_ptr(new hardware_attribute_impl(parent));
	}

	render_target_ptr display_device::render_target_instance(size_t width, size_t height, 
		size_t color_plane_count, 
		bool depth, 
		bool stencil, 
		bool use_multi_sampling, 
		size_t multi_samples)
	{
		return get_current()->handle_create_render_target(width, height, 
			color_plane_count, 
			depth, 
			stencil, 
			use_multi_sampling, 
			multi_samples);
	}

	texture_ptr display_device::create_texture(const surface_ptr& surface, const variant& node)
	{
		return get_current()->handle_create_texture(surface, node);
	}

	texture_ptr display_device::create_texture(const surface_ptr& surface, texture::Type type, int mipmap_levels)
	{
		return get_current()->handle_create_texture(surface, type, mipmap_levels);
	}

	texture_ptr display_device::create_texture(unsigned width, PixelFormat fmt)
	{
		return get_current()->handle_create_texture(width, fmt);
	}

	texture_ptr display_device::create_texture(unsigned width, unsigned height, PixelFormat fmt, texture::Type type)
	{
		return get_current()->handle_create_texture(width, height, fmt, type);
	}

	texture_ptr display_device::create_texture(unsigned width, unsigned height, unsigned depth, PixelFormat fmt)
	{
		return get_current()->handle_create_texture(width, height, depth, fmt);
	}

	texture_ptr display_device::create_texture(const std::string& filename, texture::Type type, int mipmap_levels)
	{
		return get_current()->handle_create_texture(filename, type, mipmap_levels);
	}

	MaterialPtr display_device::create_material(const variant& node)
	{
		return get_current()->handle_create_material(node);
	}

	MaterialPtr display_device::create_material(const std::string& name, const std::vector<texture_ptr>& textures, const BlendMode& blend, bool fog, bool lighting, bool depth_write, bool depth_check)
	{
		return get_current()->handle_create_material(name, textures, blend, fog, lighting, depth_write, depth_check);
	}

	render_target_ptr display_device::render_target_instance(const variant& node)
	{
		return display_device::get_current()->handle_create_render_target(node);
	}

	bool display_device::check_for_feature(display_device_capabilties cap)
	{
		return display_device::get_current()->do_check_for_feature(cap);
	}

	display_device_def::display_device_def(const std::vector<attribute_set_ptr>& as)
		: attributes_(as)//, uniforms_(us)
	{
	}

	display_device_def::~display_device_def()
	{
	}

	void display_device_def::set_hint(const std::string& hint_name, const std::string& hint)
	{
		hint_list hint_list(1,hint);
		hints_.insert(std::make_pair(hint_name, hint_list));
	}

	void display_device_def::set_hint(const std::string& hint_name, const hint_list& hint)
	{
		hints_[hint_name] = hint;
	}

	display_device_data::display_device_data()
	{
	}

	display_device_data::~display_device_data()
	{
	}
}
