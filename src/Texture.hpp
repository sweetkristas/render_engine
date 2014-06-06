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
#include "color.hpp"
#include "geometry.hpp"
#include "surface.hpp"
#include "variant.hpp"

namespace KRE
{
	inline unsigned next_power_of_2(unsigned n) {
		--n;
		n = n|(n >> 1);
		n = n|(n >> 2);
		n = n|(n >> 4);
		n = n|(n >> 8);
		n = n|(n >> 16);
		++n;
		return n;
	}

	class texture
	{
	public:
		enum class Type {
			TEXTURE_1D,
			TEXTURE_2D,
			TEXTURE_3D,
			TEXTURE_CUBIC,
		};
		enum class address_mode {
			WRAP,
			CLAMP,
			MIRROR,
			BORDER,
		};
		enum class filtering {
			NONE,
			POINT,
			LINEAR,
			ANISOTROPIC,
		};
		texture(const surface_ptr& surface, const variant& node);
		texture(const surface_ptr& surface, 
			Type type=Type::TEXTURE_2D, 
			int mipmap_levels=0);
		texture(unsigned width, 
			unsigned height, 
			unsigned depth,
			PixelFormat fmt, 
			texture::Type type);
		virtual ~texture();

		void set_address_modes(address_mode u, address_mode v=address_mode::WRAP, address_mode w=address_mode::WRAP, const color& bc=color(0.0f,0.0f,0.0f));
		void set_address_modes(const address_mode uvw[3], const color& bc=color(0.0f,0.0f,0.0f));

		void set_filtering(filtering min, filtering max, filtering mip);
		void set_filtering(const filtering f[3]);

		void set_border_color(const color& bc);

		Type get_type() const { return type_; }
		int get_mipmap_levels() const { return mipmaps_; }
		int get_max_anisotropy() const { return max_anisotropy_; }
		address_mode get_address_mode_u() const { return address_mode_[0]; }
		address_mode get_address_mode_v() const { return address_mode_[1]; }
		address_mode get_address_mode_w() const { return address_mode_[2]; }
		filtering get_filtering_min() const { return filtering_[0]; }
		filtering get_filtering_max() const { return filtering_[1]; }
		filtering get_filtering_mip() const { return filtering_[2]; }
		const color& get_border_color() const { return border_color_; }
		float get_lod_bias() const { return lod_bias_; }

		void internal_init();

		unsigned width() const { return width_; }
		unsigned height() const { return height_; }
		unsigned depth() const { return depth_; }

		unsigned surface_width() const { return surface_width_; }
		unsigned surface_height() const { return surface_height_; }

		virtual void init() = 0;
		virtual void bind() = 0;
		virtual unsigned id() = 0;

		virtual void update(int x, unsigned width, void* pixels) = 0;
		virtual void update(int x, int y, unsigned width, unsigned height, const std::vector<unsigned>& stride, void* pixels) = 0;
		virtual void update(int x, int y, int z, unsigned width, unsigned height, unsigned depth, void* pixels) = 0;

		static void rebuild_all();
	protected:
		const surface_ptr& get_surface() const { return surface_; }
		void set_texture_dimensions(unsigned w, unsigned h, unsigned d=0);
	private:
		virtual void rebuild() = 0;

		Type type_;
		int mipmaps_;
		address_mode address_mode_[3]; // u,v,w
		filtering filtering_[3]; // minification, magnification, mip
		color border_color_;
		int max_anisotropy_;
		float lod_bias_;
		texture();
		surface_ptr surface_;
		
		unsigned surface_width_;
		unsigned surface_height_;

		// width/height/depth of the created texture -- may be a 
		// different size than the surface if things like only
		// allowing power-of-two textures is in effect.
		unsigned width_;
		unsigned height_;
		unsigned depth_;
	};

	typedef std::shared_ptr<texture> texture_ptr;
}
