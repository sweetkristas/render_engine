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

#include <map>
#include <memory>
#include <string>

#include "attribute_set.hpp"
#include "canvas.hpp"
#include "color.hpp"
#include "display_device_fwd.hpp"
#include "Material.hpp"
#include "renderable.hpp"
#include "renderTarget.hpp"
#include "variant.hpp"

namespace KRE
{
	typedef std::vector<std::string> hint_list;
	typedef std::map<std::string,hint_list> hint_map;
	class display_device_def
	{
	public:
		display_device_def(const std::vector<attribute_set_ptr>& as/*, const std::vector<UniformSetPtr>& us*/);
		~display_device_def();

		const std::vector<attribute_set_ptr>& get_attribute_set() const { return attributes_; }
		//const std::vector<UniformSetPtr>& get_uniform_set() const { return uniforms_; }

		void set_hint(const std::string& hint_name, const std::string& hint);
		void set_hint(const std::string& hint_name, const hint_list& hint);
		hint_map get_hints() const { return hints_; }
	private:
		hint_map hints_;
		const std::vector<attribute_set_ptr>& attributes_;
		//const std::vector<UniformSetPtr>& uniforms_;
	};

	class display_device_data
	{
	public:
		display_device_data();
		virtual ~display_device_data();
	private:
		display_device_data(const display_device_data&);
	};

	enum class display_device_capabilties
	{
		NPOT_TEXTURES,
	};

	class display_device
	{
	public:
		enum class display_device_id {
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

		display_device();
		virtual ~display_device();

		virtual display_device_id id() const = 0;

		virtual void set_clear_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		virtual void set_clear_color(float r, float g, float b, float a) = 0;
		virtual void set_clear_color(const color& color) = 0;

		virtual void clear(uint32_t clr) = 0;
		virtual void swap() = 0;

		virtual void init(size_t width, size_t height) = 0;
		virtual void print_device_info() = 0;

		virtual void render(const renderable_ptr& r) = 0;

		static texture_ptr create_texture(const std::string& filename, 
			texture::Type type=texture::Type::TEXTURE_2D, 
			int mipmap_levels=0);

		static texture_ptr create_texture(const surface_ptr& surface, const variant& node);
		static texture_ptr create_texture(const surface_ptr& surface, 
			texture::Type type=texture::Type::TEXTURE_2D, 
			int mipmap_levels=0);
		static texture_ptr create_texture(unsigned width, PixelFormat fmt);
		static texture_ptr create_texture(unsigned width, unsigned height, PixelFormat fmt, texture::Type type=texture::Type::TEXTURE_2D);
		static texture_ptr create_texture(unsigned width, unsigned height, unsigned depth, PixelFormat fmt);

		virtual canvas_ptr get_canvas() = 0;

		static void blit_texture(const texture_ptr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch);

		static MaterialPtr create_material(const variant& node);
		static MaterialPtr create_material(const std::string& name, const std::vector<texture_ptr>& textures, const BlendMode& blend=BlendMode(), bool fog=false, bool lighting=false, bool depth_write=false, bool depth_check=false);

		static render_target_ptr render_target_instance(size_t width, size_t height, 
			size_t color_plane_count=1, 
			bool depth=false, 
			bool stencil=false, 
			bool use_multi_sampling=false, 
			size_t multi_samples=0);
		static render_target_ptr render_target_instance(const variant& node);

		virtual display_device_data_ptr create_display_device_data(const display_device_def& def) = 0;

		static attribute_set_ptr create_attribute_set(bool hardware_hint=false, bool indexed=false, bool instanced=false);
		static hardware_attribute_ptr create_attribute_buffer(bool hw_backed, attribute_base* parent);

		static display_device_ptr factory(const std::string& type);

		static display_device_ptr get_current();

		static bool check_for_feature(display_device_capabilties cap);

		static void register_factory_function(const std::string& type, std::function<display_device_ptr()>);
	private:
		display_device(const display_device&);
		virtual attribute_set_ptr handle_create_attribute_set(bool indexed, bool instanced) = 0;
		virtual hardware_attribute_ptr handle_create_attribute(attribute_base* parent) = 0;

		virtual render_target_ptr handle_create_render_target(size_t width, size_t height, 
			size_t color_plane_count, 
			bool depth, 
			bool stencil, 
			bool use_multi_sampling, 
			size_t multi_samples) = 0;
		virtual render_target_ptr handle_create_render_target(const variant& node) = 0;
		
		virtual texture_ptr handle_create_texture(const std::string& filename, texture::Type type, int mipmap_levels) = 0;
		virtual texture_ptr handle_create_texture(const surface_ptr& surface, const variant& node) = 0;
		virtual texture_ptr handle_create_texture(const surface_ptr& surface, texture::Type type, int mipmap_levels) = 0;
		virtual texture_ptr handle_create_texture(unsigned width, PixelFormat fmt) = 0;
		virtual texture_ptr handle_create_texture(unsigned width, unsigned height, PixelFormat fmt, texture::Type type=texture::Type::TEXTURE_2D) = 0;
		virtual texture_ptr handle_create_texture(unsigned width, unsigned height, unsigned depth, PixelFormat fmt) = 0;

		virtual MaterialPtr handle_create_material(const variant& node) = 0;
		virtual MaterialPtr handle_create_material(const std::string& name, const std::vector<texture_ptr>& textures, const BlendMode& blend=BlendMode(), bool fog=false, bool lighting=false, bool depth_write=false, bool depth_check=false) = 0;

		virtual bool do_check_for_feature(display_device_capabilties cap) = 0;

		virtual void do_blit_texture(const texture_ptr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch) = 0;
	};

	template<class T>
	struct display_device_registrar
	{
		display_device_registrar(const std::string& type)
		{
			// register the class factory function 
			display_device::register_factory_function(type, []() -> display_device_ptr { return display_device_ptr(new T());});
		}
	};
}
