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
#include "Texture.hpp"

namespace KRE
{
	Texture::Texture(const SurfacePtr& surface, const variant& node)
		: type_(TEXTURE_2D), 
		mipmaps_(0), 
		max_anisotropy_(1),
		lod_bias_(0.0f),
		surface_(surface),
		tex_width_(surface->width()),
		tex_height_(surface->height())
	{
		InternalInit();
		if(node.has_key("type")) {
			const std::string& type = node["type"].as_string();
			if(type == "1d") {
				type_ = TEXTURE_1D;
			} else if(type == "2d") {
				type_ = TEXTURE_2D;
			} else if(type == "3d") {
				type_ = TEXTURE_3D;
			} else if(type == "cubic") {
				type_ = TEXTURE_CUBIC;
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
					filtering_[0] = Filtering::POINT;
					filtering_[1] = Filtering::POINT;
					filtering_[2] = Filtering::NONE;
				} else if(filtering == "bilinear") {
					filtering_[0] = Filtering::LINEAR;
					filtering_[1] = Filtering::LINEAR;
					filtering_[2] = Filtering::POINT;
				} else if(filtering == "trilinear") {
					filtering_[0] = Filtering::LINEAR;
					filtering_[1] = Filtering::LINEAR;
					filtering_[2] = Filtering::LINEAR;
				} else if(filtering == "anisotropic") {
					filtering_[0] = Filtering::ANISOTROPIC;
					filtering_[1] = Filtering::ANISOTROPIC;
					filtering_[2] = Filtering::LINEAR;
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
						filtering_[n] = Filtering::NONE;
					} else if(f == "point") {
						filtering_[n] = Filtering::POINT;
					} else if(f == "linear") {
						filtering_[n] = Filtering::LINEAR;
					} else if(f == "anisotropic") {
						filtering_[n] = Filtering::ANISOTROPIC;
					} else {
						ASSERT_LOG(false, "Filtering element(" << n << ") invalid: " << f);
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
						address_mode_[n] = AddressMode::WRAP;
					} else if(am == "clamp") {
						address_mode_[n] = AddressMode::CLAMP;
					} else if(am == "mirror") {
						address_mode_[n] = AddressMode::MIRROR;
					} else if(am == "border") {
						address_mode_[n] = AddressMode::BORDER;
					} else {
						ASSERT_LOG(false, "address_mode element(" << n << ") invalid: " << am);
					}
				}
				for(; n < 3; ++n) {
					address_mode_[n] = AddressMode::WRAP;
				}
			} else {
				ASSERT_LOG(false, "'filtering' must be a list of strings. Found: " << node["filtering"].type_as_string());
			}
		}
		if(node.has_key("border_color")) {
			//border_color_ = Color(node["border_color"]);
			ASSERT_LOG(false, "'border_color' attribute not processed, need to implement variant processor for Color()");
		}
		if(node.has_key("rect")) {
			ASSERT_LOG(node["rect"].is_list(), "'rect' attribute must be a list of numbers.");
			ASSERT_LOG(node["rect"].num_elements() >= 4, "'rect' attribute must have at least 4 elements.");
			coords_ = rectf::FromCoordinates(node["rect"][0].as_float(), 
				node["rect"][1].as_float(), 
				node["rect"][2].as_float(), 
				node["rect"][3].as_float());
		}
	}

	Texture::Texture(const SurfacePtr& surface, TextureType type, int mipmap_levels)
		: type_(type), 
		mipmaps_(mipmap_levels), 
		max_anisotropy_(1),
		lod_bias_(0.0f),
		surface_(surface),
		tex_width_(surface->width()),
		tex_height_(surface->height())
	{
		InternalInit();
	}

	Texture::Texture(unsigned width, 
		unsigned height, 
		PixelFormat::PixelFormatConstant fmt, 
		Texture::TextureType type)
		: type_(type), 
		mipmaps_(0), 
		max_anisotropy_(1),
		lod_bias_(0.0f),
		tex_width_(width),
		tex_height_(height)
	{
		InternalInit();
	}

	void Texture::InternalInit()
	{
		for(auto& am : address_mode_) {
			am = AddressMode::WRAP;
		}
		filtering_[0] = Filtering::POINT;
		filtering_[1] = Filtering::POINT;
		filtering_[2] = Filtering::NONE;
	}
	
	Texture::~Texture()
	{
	}
	void Texture::SetAddressModes(AddressMode u, AddressMode v, AddressMode w)
	{
		address_mode_[0] = u;
		address_mode_[1] = v;
		address_mode_[2] = w;
		Init();
	}

	void Texture::SetAddressModes(const AddressMode uvw[3])
	{
		for(int n = 0; n < 3; ++n) {
			address_mode_[n] = uvw[n];
		}
		Init();
	}

	void Texture::SetFiltering(Filtering min, Filtering max, Filtering mip)
	{
		filtering_[0] = min;
		filtering_[1] = max;
		filtering_[2] = mip;
		Init();
	}

	void Texture::SetFiltering(const Filtering f[3])
	{
		for(int n = 0; n < 3; ++n) {
			filtering_[n] = f[n];
		}
		Init();
	}

	void Texture::SetRect(int x, int y, unsigned width, unsigned height)
	{
		coords_ = rectf(x/tex_width_, y/tex_height_, width/tex_width_, height/tex_height_);
	}

	void Texture::SetRect(const rect& r)
	{
		coords_ = rectf::FromCoordinates(r.x()/tex_width_,
			r.y()/tex_height_,
			r.x2()/tex_width_,
			r.y2()/tex_height_);
	}

	void Texture::SetRect(const rectf& r)
	{
		coords_ = rectf::FromCoordinates(r.x()/tex_width_,
			r.y()/tex_height_,
			r.x2()/tex_width_,
			r.y2()/tex_height_);
	}
}
