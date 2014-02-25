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
#include "Color.hpp"
#include "Surface.hpp"
#include "variant.hpp"

namespace Graphics
{
	class Texture
	{
	public:
		enum TextureType {
			TEXTURE_1D,
			TEXTURE_2D,
			TEXTURE_3D,
			TEXTURE_CUBIC,
		};
		enum AddressMode {
			WRAP,
			CLAMP,
			MIRROR,
			BORDER,
		};
		enum Filtering {
			NONE,
			POINT,
			LINEAR,
			ANISOTROPIC,
		};
		Texture(const variant& node);
		Texture(const std::string& filename, 
			TextureType type=TextureType::TEXTURE_2D, 
			int mipmap_levels=0);
		Texture(const SurfacePtr& surface, 
			TextureType type=TextureType::TEXTURE_2D, 
			int mipmap_levels=0);
		virtual ~Texture();

		void SetAddressModes(AddressMode u, AddressMode v=AddressMode::WRAP, AddressMode w=AddressMode::WRAP);
		void SetAddressModes(const AddressMode uvw[3]);

		void SetFiltering(Filtering min, Filtering max, Filtering mip);
		void SetFiltering(const Filtering f[3]);

		TextureType GetType() const { return type_; }
		int GetMipMapLevels() const { return mipmaps_; }
		int GetMaxAnisotropy() const { return max_anisotropy_; }
		AddressMode GetAddressModeU() const { return address_mode_[0]; }
		AddressMode GetAddressModeV() const { return address_mode_[1]; }
		AddressMode GetAddressModeW() const { return address_mode_[2]; }
		Filtering GetFilteringMin() const { return filtering_[0]; }
		Filtering GetFilteringMax() const { return filtering_[1]; }
		Filtering GetFilteringMip() const { return filtering_[2]; }
		const Color& GetBorderColor() const { return border_color_; }
		float GetLodBias() const { return lod_bias_; }

		void InternalInit();

		virtual void Init() = 0;

		virtual void Update(int x, int y, int width, int height, int stride, void* pixels) = 0;
	private:
		TextureType type_;
		int mipmaps_;
		AddressMode address_mode_[3]; // u,v,w
		Filtering filtering_[3]; // minification, magnification, mip
		Color border_color_;
		int max_anisotropy_;
		float lod_bias_;
		Texture();
	};

	typedef std::shared_ptr<Texture> TexturePtr;
}
