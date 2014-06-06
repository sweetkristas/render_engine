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

#include "display_device.hpp"
#include "Material.hpp"
#include "shaders_ogl.hpp"

namespace KRE
{
	class display_device_ogl : public display_device
	{
	public:
		display_device_ogl();
		~display_device_ogl();

		display_device_id id() const override { return display_device_id::DISPLAY_DEVICE_OPENGL; }

		void swap() override;
		void clear(uint32_t clr) override;

		void set_clear_color(float r, float g, float b, float a) override;
		void set_clear_color(const color& color) override;

		void render(const renderable_ptr& r) override;

		void init(size_t width, size_t height) override;
		void print_device_info() override;

		virtual display_device_data_ptr create_display_device_data(const display_device_def& def) override;

		canvas_ptr get_canvas() override;

	private:
		display_device_ogl(const display_device_ogl&);

		attribute_set_ptr handle_create_attribute_set(bool indexed, bool instanced) override;
		hardware_attribute_ptr handle_create_attribute(attribute_base* parent) override;

		render_target_ptr handle_create_render_target(size_t width, size_t height, 
			size_t color_plane_count, 
			bool depth, 
			bool stencil, 
			bool use_multi_sampling, 
			size_t multi_samples) override;
		render_target_ptr handle_create_render_target(const variant& node) override;
		void do_blit_texture(const texture_ptr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch) override;

		bool do_check_for_feature(display_device_capabilties cap) override;

		texture_ptr handle_create_texture(const std::string& filename, texture::Type type, int mipmap_levels) override;
		texture_ptr handle_create_texture(const surface_ptr& surface, const variant& node) override;
		texture_ptr handle_create_texture(const surface_ptr& surface, texture::Type type, int mipmap_levels) override;
		texture_ptr handle_create_texture(unsigned width, PixelFormat fmt) override;
		texture_ptr handle_create_texture(unsigned width, unsigned height, PixelFormat fmt, texture::Type type=texture::Type::TEXTURE_2D) override;
		texture_ptr handle_create_texture(unsigned width, unsigned height, unsigned depth, PixelFormat fmt) override;

		MaterialPtr handle_create_material(const variant& node) override;
		MaterialPtr handle_create_material(const std::string& name, const std::vector<texture_ptr>& textures, const BlendMode& blend=BlendMode(), bool fog=false, bool lighting=false, bool depth_write=false, bool depth_check=false) override;
	};
}
