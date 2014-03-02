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
#include "TextureOpenGL.hpp"

namespace Graphics
{
	/// XXXXX we need a way to specify texture wrap and min/mag filtering
	// for textures based on a variant type.
	namespace
	{
		GLenum GetGLAddressMode(Texture::AddressMode am)
		{
			switch(am) {
				case Texture::AddressMode::WRAP:	return GL_REPEAT;
				case Texture::AddressMode::CLAMP:	return GL_CLAMP_TO_EDGE;
				case Texture::AddressMode::MIRROR:	return GL_MIRRORED_REPEAT;
				case Texture::AddressMode::BORDER:	return GL_CLAMP_TO_BORDER;
			}
			return GL_CLAMP_TO_EDGE;
		}

		GLenum GetGLTextureType(Texture::TextureType tt) 
		{
			switch(tt) {
				case Texture::TextureType::TEXTURE_1D:		return GL_TEXTURE_1D;
				case Texture::TextureType::TEXTURE_2D:		return GL_TEXTURE_2D;
				case Texture::TextureType::TEXTURE_3D:		return GL_TEXTURE_3D;
				case Texture::TextureType::TEXTURE_CUBIC:	return GL_TEXTURE_CUBE_MAP;
			}
			return GL_TEXTURE_2D;
		}
	}

	OpenGLTexture::OpenGLTexture(const variant& node)
		: Texture(node)
	{
		ASSERT_LOG(false, "Create OpenGL surface from filename.");

		ASSERT_LOG(node.has_key("image"), "Texture must have 'image' attribute.");
		ASSERT_LOG(node["image"].is_string() || node["image"].is_list(), "'image' attribute must be list of strings or string. Found: " << node["image"].type_as_string());
		// Create surface from filename
		// XXX
		ASSERT_LOG(false, "Create OpenGL surface from filename.");

		CreateTexture();
		Init();
	}

	OpenGLTexture::OpenGLTexture(const std::string& filename, TextureType type,  int mipmap_levels)
		: Texture(filename, type, mipmap_levels),
		width_(-1), 
		height_(-1),
		format_(GL_RGBA),
		internal_format_(GL_RGBA),
		type_(GL_UNSIGNED_BYTE)
	{
		// Create surface from filename
		// XXX
		ASSERT_LOG(false, "Create OpenGL surface from filename.");

		CreateTexture();
		Init();
	}

	OpenGLTexture::OpenGLTexture(const SurfacePtr& surface, TextureType type,  int mipmap_levels)
		: Texture(surface, type, mipmap_levels), 
		width_(surface->width()), 
		height_(surface->height()),
		format_(GL_RGBA),
		internal_format_(GL_RGBA),
		type_(GL_UNSIGNED_BYTE)
	{
		CreateTexture();
		Init();
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glDeleteTextures(1, &texture_id_);
	}

	void OpenGLTexture::Update(int x, int y, int width, int height, int stride, void* pixels)
	{
		glBindTexture(GetGLTextureType(GetType()), texture_id_);
		glTexSubImage2D(GetGLTextureType(GetType()), 0, x, y, width, height, format_, type_, pixels);
		if(GetMipMapLevels() > 0 && GetType() > TextureType::TEXTURE_1D) {
			glGenerateMipmap(GetGLTextureType(GetType()));
		}
		glBindTexture(GetGLTextureType(GetType()), 0);
	}

	void OpenGLTexture::CreateTexture()
	{
		glGenTextures(1, &texture_id_);
		glBindTexture(GetGLTextureType(GetType()), texture_id_);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		// Need to change the format/internalFormat/type depending on the 
		// data we now about the surface.

		//auto fmt = surface->GetFormat();
		//switch(fmt->GetPixelFormat()) {
		//	case Surface::PIXELFORMAT_ARGB8888:
				format_ = GL_BGRA;
				internal_format_ = GL_RGBA8;
				type_ = GL_UNSIGNED_INT_8_8_8_8_REV;
		//		break;
		//	default:
		//		ASSERT_LOG(false, "Unrecognised pixel format: " << fmt->GetPixelFormat());
		//}


		/// XXX Need to fill the image here. Or at least do it optionally.
		// Use glTexImage1D and glTexImage3D ????? as needed.
		glTexImage2D(GetGLTextureType(GetType()), 0, internal_format_, Width(), Height(), 0, format_, type_, 0);
		// If we are using a cubic texture 		
	}

	void OpenGLTexture::Init()
	{
		GLenum type = GetGLTextureType(GetType());

		glBindTexture(type, texture_id_);

		glTexParameteri(type, GL_TEXTURE_WRAP_S, GetGLAddressMode(GetAddressModeU()));
		if(GetAddressModeU() == AddressMode::BORDER) {
			glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, GetBorderColor().AsFloatVector());
		}
		if(GetType() > TextureType::TEXTURE_1D) {
			glTexParameteri(type, GL_TEXTURE_WRAP_T, GetGLAddressMode(GetAddressModeV()));
			if(GetAddressModeV() == AddressMode::BORDER) {
				glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, GetBorderColor().AsFloatVector());
			}
		}
		if(GetType() > TextureType::TEXTURE_2D) {
			glTexParameteri(type, GL_TEXTURE_WRAP_R, GetGLAddressMode(GetAddressModeW()));
			if(GetAddressModeW() == AddressMode::BORDER) {
				glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, GetBorderColor().AsFloatVector());
			}
		}
		
		ASSERT_LOG(GetFilteringMin() != Filtering::NONE, "'none' is not a valid choice for the minifying filter.");
		ASSERT_LOG(GetFilteringMax() != Filtering::NONE, "'none' is not a valid choice for the maxifying filter.");
		ASSERT_LOG(GetFilteringMip() != Filtering::ANISOTROPIC, "'anisotropic' is not a valid choice for the mip filter.");

		if(GetFilteringMin() == Filtering::POINT) {
			switch(GetFilteringMip()) {
				case Filtering::NONE: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST); break;
				case Filtering::POINT: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); break;
				case Filtering::LINEAR: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); break;
			}
		} else if(GetFilteringMin() == Filtering::LINEAR || GetFilteringMin() == Filtering::ANISOTROPIC) {
			switch(GetFilteringMip()) {
				case Filtering::NONE: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR); break;
				case Filtering::POINT: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); break;
				case Filtering::LINEAR: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); break;
			}
		}

		if(GetFilteringMax() == Filtering::POINT) {
			glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} else {
			glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		if(GetFilteringMax() == Filtering::ANISOTROPIC || GetFilteringMin() == Filtering::ANISOTROPIC) {
			if(GL_EXT_texture_filter_anisotropic) {
				float largest_anisotropy;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_anisotropy);
				glTexParameterf(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_anisotropy > GetMaxAnisotropy() ? GetMaxAnisotropy() : largest_anisotropy);
			}
		}

		if(GetLodBias() > 1e-14 || GetLodBias() < -1e-14) {
			glTexParameterf(type, GL_TEXTURE_LOD_BIAS, GetLodBias());
		}

		if(GetMipMapLevels() > 0 && GetType() > TextureType::TEXTURE_1D) {
			glGenerateMipmap(type);
		}

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindTexture(GetType(), 0);
	}

	void OpenGLTexture::Bind(int n) 
	{ 
		glActiveTexture(GL_TEXTURE0 + n); 
		glBindTexture(GetGLTextureType(GetType()), texture_id_);
	}
}
