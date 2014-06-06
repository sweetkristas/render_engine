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

#include <set>
#include "asserts.hpp"
#include "display_device.hpp"
#include "texture.hpp"

namespace KRE
{
	namespace 
	{
		typedef std::set<texture*> textureRegistryType;
		textureRegistryType& texture_registry()
		{
			static textureRegistryType res;
			return res;
		}

		void add_to_texture_registry(texture* tex) 
		{
			texture_registry().insert(tex);
		}

		void remove_from_texture_registery(texture* tex)
		{
			auto it = texture_registry().find(tex);
			ASSERT_LOG(it != texture_registry().end(), "tried to erase texture from registry that doesn't exist");
			texture_registry().erase(it);
		}
	}

	texture::texture(const surface_ptr& surface, const variant& node)
		: type_(Type::TEXTURE_2D), 
		mipmaps_(0), 
		max_anisotropy_(1),
		lod_bias_(0.0f),
		surface_(surface),
		surface_width_(surface->width()),
		surface_height_(surface->height()),
		width_(0),
		height_(0),
		depth_(0)
	{
		internal_init();
		if(node.has_key("type")) {
			const std::string& type = node["type"].as_string();
			if(type == "1d") {
				type_ = Type::TEXTURE_1D;
			} else if(type == "2d") {
				type_ = Type::TEXTURE_2D;
			} else if(type == "3d") {
				type_ = Type::TEXTURE_3D;
			} else if(type == "cubic") {
				type_ = Type::TEXTURE_CUBIC;
			} else {
				ASSERT_LOG(false, "Unrecognised texture type '" << type << "'. Valid values are 1d,2d,3d and cubic.");
			}
		}
		if(node.has_key("mipmaps")) {
			ASSERT_LOG(node["mipmaps"].is_int(), "'mipmaps' not an integer type, found: " << node["mipmaps"].type_as_string());
			mipmaps_ = int(node["mipmaps"].as_int());
		}
		if(node.has_key("lod_bias")) {
			ASSERT_LOG(node["lod_bias"].is_numeric(), "'lod_bias' not a numeric type, found: " << node["lod_bias"].type_as_string());
			lod_bias_ = node["lod_bias"].as_float();
		}
		if(node.has_key("max_anisotropy")) {
			ASSERT_LOG(node["max_anisotropy"].is_int(), "'max_anisotropy' not an integer type, found: " << node["max_anisotropy"].type_as_string());
			max_anisotropy_ = int(node["max_anisotropy"].as_int());
		}
		if(node.has_key("filtering")) {
			if(node["filtering"].is_string()) {
				const std::string& filtering = node["filtering"].as_string();
				if(filtering == "none") {
					filtering_[0] = filtering::POINT;
					filtering_[1] = filtering::POINT;
					filtering_[2] = filtering::NONE;
				} else if(filtering == "bilinear") {
					filtering_[0] = filtering::LINEAR;
					filtering_[1] = filtering::LINEAR;
					filtering_[2] = filtering::POINT;
				} else if(filtering == "trilinear") {
					filtering_[0] = filtering::LINEAR;
					filtering_[1] = filtering::LINEAR;
					filtering_[2] = filtering::LINEAR;
				} else if(filtering == "anisotropic") {
					filtering_[0] = filtering::ANISOTROPIC;
					filtering_[1] = filtering::ANISOTROPIC;
					filtering_[2] = filtering::LINEAR;
				} else {
					ASSERT_LOG(false, "'filtering' must be either 'none','bilinear','trilinear' or 'anisotropic'. Found: " << filtering);
				}
			} else if(node["filtering"].is_list()) {
				size_t list_size = node["filtering"].num_elements();
				ASSERT_LOG(list_size == 3, "Size of list for 'filtering' attribute must be 3 elements. Found: " << list_size);
				for(size_t n = 0; n != 3; ++n) {
					ASSERT_LOG(node["filtering"][n].is_string(), "Element " << n << " of filtering is not a string: " << node["filtering"][0].type_as_string());
					const std::string& f = node["filtering"][n].as_string();
					if(f == "none") {
						filtering_[n] = filtering::NONE;
					} else if(f == "point") {
						filtering_[n] = filtering::POINT;
					} else if(f == "linear") {
						filtering_[n] = filtering::LINEAR;
					} else if(f == "anisotropic") {
						filtering_[n] = filtering::ANISOTROPIC;
					} else {
						ASSERT_LOG(false, "filtering element(" << n << ") invalid: " << f);
					}
				}
			} else {
				ASSERT_LOG(false, "'filtering' must be either a string value or list of strings. Found: " << node["filtering"].type_as_string());
			}
		}
		if(node.has_key("address_mode")) {
			if(node["address_mode"].is_list()) {
				size_t list_size = node["address_mode"].num_elements();
				ASSERT_LOG(list_size >= 1 && list_size <= 3, "Size of list for 'address_mode' attribute must be between 1 and 3 elements. Found: " << list_size);
				size_t n = 0;
				for(; n != list_size; ++n) {
					ASSERT_LOG(node["address_mode"][n].is_string(), "Element " << n << " of 'address_mode' attribute is not a string: " << node["address_mode"][0].type_as_string());
					const std::string& am = node["address_mode"][n].as_string();
					if(am == "wrap") {
						address_mode_[n] = address_mode::WRAP;
					} else if(am == "clamp") {
						address_mode_[n] = address_mode::CLAMP;
					} else if(am == "mirror") {
						address_mode_[n] = address_mode::MIRROR;
					} else if(am == "border") {
						address_mode_[n] = address_mode::BORDER;
					} else {
						ASSERT_LOG(false, "address_mode element(" << n << ") invalid: " << am);
					}
				}
				for(; n < 3; ++n) {
					address_mode_[n] = address_mode::WRAP;
				}
			} else {
				ASSERT_LOG(false, "'filtering' must be a list of strings. Found: " << node["filtering"].type_as_string());
			}
		}
		if(node.has_key("border_color")) {
			border_color_ = color(node["border_color"]);
		}
		if(node.has_key("rect")) {
			ASSERT_LOG(node["rect"].is_list(), "'rect' attribute must be a list of numbers.");
			ASSERT_LOG(node["rect"].num_elements() >= 4, "'rect' attribute must have at least 4 elements.");
		}
	}

	texture::texture(const surface_ptr& surface, Type type, int mipmap_levels)
		: type_(type), 
		mipmaps_(mipmap_levels), 
		max_anisotropy_(1),
		lod_bias_(0.0f),
		surface_(surface),
		surface_width_(surface->width()),
		surface_height_(surface->height()),
		width_(0),
		height_(0),
		depth_(0)
	{
		internal_init();
	}

	texture::texture(unsigned width, 
		unsigned height, 
		unsigned depth,
		PixelFormat fmt, 
		Type type)
		: type_(type), 
		mipmaps_(0), 
		max_anisotropy_(1),
		lod_bias_(0.0f),
		surface_width_(0),
		surface_height_(0),
		width_(width),
		height_(height),
		depth_(depth)
	{
		internal_init();
	}
	
	texture::~texture()
	{
		remove_from_texture_registery(this);
	}

	void texture::internal_init()
	{
		add_to_texture_registry(this);

		for(auto& am : address_mode_) {
			am = address_mode::CLAMP;
		}
		filtering_[0] = filtering::POINT;
		filtering_[1] = filtering::POINT;
		filtering_[2] = filtering::NONE;

		// XXX For reasons (i.e. some video cards are problematic either hardware/drivers)
		// we are forced to use power-of-two textures anyway if we want mip-mapping and
		// address modes other than CLAMP.
		if(!display_device::check_for_feature(display_device_capabilties::NPOT_TEXTURES)) {
			width_ = next_power_of_2(surface_width_);
			height_ = next_power_of_2(surface_height_);
			depth_ = 0;
		} else {
			width_ = surface_width_;
			height_ = surface_height_;
			depth_ = 0;
		}
	}

	void texture::set_address_modes(address_mode u, address_mode v, address_mode w, const color& bc)
	{
		address_mode_[0] = u;
		address_mode_[1] = v;
		address_mode_[2] = w;
		border_color_ = bc;
		init();
	}

	void texture::set_address_modes(const address_mode uvw[3], const color& bc)
	{
		for(int n = 0; n < 3; ++n) {
			address_mode_[n] = uvw[n];
		}
		border_color_ = bc;
		init();
	}

	void texture::set_filtering(filtering min, filtering max, filtering mip)
	{
		filtering_[0] = min;
		filtering_[1] = max;
		filtering_[2] = mip;
		init();
	}

	void texture::set_filtering(const filtering f[3])
	{
		for(int n = 0; n < 3; ++n) {
			filtering_[n] = f[n];
		}
		init();
	}

	void texture::rebuild_all()
	{
		for(auto tex : texture_registry()) {
			tex->rebuild();
		}
	}

	void texture::set_texture_dimensions(unsigned w, unsigned h, unsigned d)
	{
		width_ = w;
		height_ = h;
		depth_ = d;
	}
}
