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
#include "logger.hpp"
#include "texture_ogl.hpp"

namespace KRE
{
	namespace
	{
		GLenum get_gl_address_mode(texture::address_mode am)
		{
			switch(am) {
				case texture::address_mode::WRAP:	return GL_REPEAT;
				case texture::address_mode::CLAMP:	return GL_CLAMP_TO_EDGE;
				case texture::address_mode::MIRROR:	return GL_MIRRORED_REPEAT;
				case texture::address_mode::BORDER:	return GL_CLAMP_TO_BORDER;
			}
			return GL_CLAMP_TO_EDGE;
		}

		GLenum get_gl_texture_type(texture::Type tt) 
		{
			switch(tt) {
				case texture::Type::TEXTURE_1D:		return GL_TEXTURE_1D;
				case texture::Type::TEXTURE_2D:		return GL_TEXTURE_2D;
				case texture::Type::TEXTURE_3D:		return GL_TEXTURE_3D;
				case texture::Type::TEXTURE_CUBIC:	return GL_TEXTURE_CUBE_MAP;
			}
			return GL_TEXTURE_2D;
		}
	}

	OpenGLtexture::OpenGLtexture(const surface_ptr& surface, const variant& node)
		: texture(surface, node),
		format_(GL_RGBA),
		internal_format_(GL_RGBA),
		type_(GL_UNSIGNED_BYTE),
		pixel_format_(PixelFormat::PIXELFORMAT_UNKNOWN),
		is_yuv_planar_(false)
	{
		create_texture(get_surface()->get_pixel_format()->get_format());
		init();
	}

	OpenGLtexture::OpenGLtexture(const surface_ptr& surface, Type type,  int mipmap_levels)
		: texture(surface, type, mipmap_levels), 
		format_(GL_RGBA),
		internal_format_(GL_RGBA),
		type_(GL_UNSIGNED_BYTE),
		pixel_format_(PixelFormat::PIXELFORMAT_UNKNOWN),
		is_yuv_planar_(false)
	{
		create_texture(get_surface()->get_pixel_format()->get_format());
		init();
	}

	OpenGLtexture::OpenGLtexture(unsigned width, 
		unsigned height, 
		PixelFormat fmt, 
		Type type, 
		unsigned depth)
		: texture(width, height, depth, fmt, type),
		format_(GL_RGBA),
		internal_format_(GL_RGBA),
		type_(GL_UNSIGNED_BYTE),
		pixel_format_(PixelFormat::PIXELFORMAT_UNKNOWN),
		is_yuv_planar_(false)
	{
		set_texture_dimensions(width, height, depth);
		create_texture(fmt);
		init();
	}

	OpenGLtexture::~OpenGLtexture()
	{
		glDeleteTextures(is_yuv_planar_ ? 3 : 1, &texture_id_[0]);
	}

	void OpenGLtexture::update(int x, unsigned width, void* pixels)
	{
		ASSERT_LOG(is_yuv_planar_ == false, "1D texture update function called on YUV planar format.");
		glBindTexture(get_gl_texture_type(get_type()), texture_id_[0]);
		ASSERT_LOG(get_type() == Type::TEXTURE_1D, "Tried to do 1D texture update on non-1D texture");
		glTexSubImage1D(get_gl_texture_type(get_type()), 0, x, width, format_, type_, pixels);
	}

	// Add a 2D update function which has single stride, but doesn't support planar YUV.

	// Stride is the width of the image surface *in pixels*
	void OpenGLtexture::update(int x, int y, unsigned width, unsigned height, const std::vector<unsigned>& stride, void* pixels)
	{
		int num_textures = is_yuv_planar_ ? 2 : 0;
		for(int n = num_textures; n >= 0; --n) {
			glBindTexture(get_gl_texture_type(get_type()), texture_id_[n]);
			if(stride.size() > size_t(n)) {
				glPixelStorei(GL_UNPACK_ROW_LENGTH, stride[n]);
			}
			switch(get_type()) {
				case Type::TEXTURE_1D:
					LOG_WARN("Running 2D texture update on 1D texture.");
					ASSERT_LOG(is_yuv_planar_ == false, "update of 1D texture in YUV planar mode.");
					glTexSubImage1D(get_gl_texture_type(get_type()), 0, x, width, format_, type_, pixels);
					break;
				case Type::TEXTURE_2D:
					glTexSubImage2D(get_gl_texture_type(get_type()), 0, x, y, n>0?width/2:width, n>0?height/2:height, format_, type_, pixels);
					break;
				case Type::TEXTURE_3D:
					ASSERT_LOG(false, "Tried to do 2D texture update on 3D texture");
				case Type::TEXTURE_CUBIC:
					ASSERT_LOG(false, "No support for updating cubic textures yet.");
			}
		
			if(get_mipmap_levels() > 0 && get_type() > Type::TEXTURE_1D) {
				glGenerateMipmap(get_gl_texture_type(get_type()));
			}
		}
		if(!stride.empty()) {
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}
	}

	void OpenGLtexture::update(int x, int y, int z, unsigned width, unsigned height, unsigned depth, void* pixels)
	{
		ASSERT_LOG(is_yuv_planar_ == false, "3D texture update function called on YUV planar format.");
		glBindTexture(get_gl_texture_type(get_type()), texture_id_[0]);
		switch(get_type()) {
			case Type::TEXTURE_1D:
				LOG_WARN("Running 2D texture update on 1D texture. You may get unexpected results.");
				glTexSubImage1D(get_gl_texture_type(get_type()), 0, x, width, format_, type_, pixels);
				break;
			case Type::TEXTURE_2D:
				LOG_WARN("Running 3D texture update on 2D texture. You may get unexpected results.");
				glTexSubImage2D(get_gl_texture_type(get_type()), 0, x, y, width, height, format_, type_, pixels);
				break;
			case Type::TEXTURE_3D:
				glTexSubImage3D(get_gl_texture_type(get_type()), 0, x, y, z, width, height, depth, format_, type_, pixels);
			case Type::TEXTURE_CUBIC:
				ASSERT_LOG(false, "No support for updating cubic textures yet.");
		}
		if(get_mipmap_levels() > 0 && get_type() > Type::TEXTURE_1D) {
			glGenerateMipmap(get_gl_texture_type(get_type()));
		}
	}

	void OpenGLtexture::create_texture(const PixelFormat& fmt)
	{
		// Set the pixel format being used.
		pixel_format_ = fmt;

		// Change the format/internalFormat/type depending on the 
		// data we now about the surface.
		// XXX these need testing for correctness.
		switch(fmt) {
			case PixelFormat::PIXELFORMAT_INDEX1LSB:
			case PixelFormat::PIXELFORMAT_INDEX1MSB:
			case PixelFormat::PIXELFORMAT_INDEX4LSB:
			case PixelFormat::PIXELFORMAT_INDEX4MSB:
			case PixelFormat::PIXELFORMAT_INDEX8:
				ASSERT_LOG(false, "Invalid pixel format given, indexed formats no supported.");
				break;
			case PixelFormat::PIXELFORMAT_RGB332:
				format_ = GL_RGB;
				internal_format_ = GL_R3_G3_B2;
				type_ = GL_UNSIGNED_BYTE_3_3_2;
				break;
			case PixelFormat::PIXELFORMAT_RGB444:
				format_ = GL_RGB;
				internal_format_ = GL_RGB4;
				type_ = GL_UNSIGNED_SHORT;
				break;
			case PixelFormat::PIXELFORMAT_RGB555:
				format_ = GL_RGB;
				internal_format_ = GL_RGB5;
				type_ = GL_UNSIGNED_SHORT;
				break;
			case PixelFormat::PIXELFORMAT_BGR555:
				format_ = GL_BGR;
				internal_format_ = GL_RGB4;
				type_ = GL_UNSIGNED_SHORT;
				break;
			case PixelFormat::PIXELFORMAT_ARGB4444:
				format_ = GL_BGRA;
				internal_format_ = GL_RGBA4;
				type_ =  GL_UNSIGNED_SHORT_4_4_4_4_REV;
				break;
			case PixelFormat::PIXELFORMAT_RGBA4444:
				format_ = GL_RGBA;
				internal_format_ = GL_RGBA4;
				type_ =  GL_UNSIGNED_SHORT_4_4_4_4;
				break;
			case PixelFormat::PIXELFORMAT_ABGR4444:
				format_ = GL_RGBA;
				internal_format_ = GL_RGBA4;
				type_ =  GL_UNSIGNED_SHORT_4_4_4_4_REV;
				break;
			case PixelFormat::PIXELFORMAT_BGRA4444:
				format_ = GL_BGRA;
				internal_format_ = GL_RGBA4;
				type_ =  GL_UNSIGNED_SHORT_4_4_4_4;
				break;
			case PixelFormat::PIXELFORMAT_ARGB1555:
				format_ = GL_BGRA;
				internal_format_ = GL_RGB5_A1;
				type_ =  GL_UNSIGNED_SHORT_1_5_5_5_REV;
				break;
			case PixelFormat::PIXELFORMAT_RGBA5551:
				format_ = GL_RGBA;
				internal_format_ = GL_RGB5_A1;
				type_ =  GL_UNSIGNED_SHORT_5_5_5_1;
				break;
			case PixelFormat::PIXELFORMAT_ABGR1555:
				format_ = GL_RGBA;
				internal_format_ = GL_RGB5_A1;
				type_ =  GL_UNSIGNED_SHORT_1_5_5_5_REV;
				break;
			case PixelFormat::PIXELFORMAT_BGRA5551:
				format_ = GL_BGRA;
				internal_format_ = GL_RGB5_A1;
				type_ =  GL_UNSIGNED_SHORT_5_5_5_1;
				break;
			case PixelFormat::PIXELFORMAT_RGB565:
				format_ = GL_RGB;
				internal_format_ = GL_RGB;
				type_ =  GL_UNSIGNED_SHORT_5_6_5;
				break;
			case PixelFormat::PIXELFORMAT_BGR565:
				format_ = GL_RGB;
				internal_format_ = GL_RGB;
				type_ =  GL_UNSIGNED_SHORT_5_6_5_REV;
				break;
			case PixelFormat::PIXELFORMAT_RGB24:
				format_ = GL_RGB;
				internal_format_ = GL_RGB8;
				type_ =  GL_UNSIGNED_BYTE;
				break;
			case PixelFormat::PIXELFORMAT_BGR24:
				format_ = GL_BGR;
				internal_format_ = GL_RGB8;
				type_ =  GL_UNSIGNED_BYTE;
				break;
			case PixelFormat::PIXELFORMAT_RGB888:
				format_ = GL_RGB;
				internal_format_ = GL_RGB8;
				type_ =  GL_UNSIGNED_BYTE;
				break;
			case PixelFormat::PIXELFORMAT_RGBX8888:
				format_ = GL_RGB;
				internal_format_ = GL_RGB8;
				type_ =  GL_UNSIGNED_INT_8_8_8_8;
				break;
			case PixelFormat::PIXELFORMAT_BGR888:
				format_ = GL_BGR;
				internal_format_ = GL_RGB8;
				type_ =  GL_UNSIGNED_BYTE;
				break;
			case PixelFormat::PIXELFORMAT_BGRX8888:
				format_ = GL_RGB;
				internal_format_ = GL_RGB8;
				type_ =  GL_UNSIGNED_INT_8_8_8_8_REV;
				break;
			case PixelFormat::PIXELFORMAT_ARGB8888:
				format_ = GL_BGRA;
				internal_format_ = GL_RGBA8;
				type_ = GL_UNSIGNED_INT_8_8_8_8_REV;
				break;
			case PixelFormat::PIXELFORMAT_RGBA8888:
				format_ = GL_RGBA;
				internal_format_ = GL_RGBA8;
				type_ = GL_UNSIGNED_INT_8_8_8_8;
				break;
			case PixelFormat::PIXELFORMAT_ABGR8888:
				format_ = GL_RGBA;
				internal_format_ = GL_RGBA8;
				type_ = GL_UNSIGNED_INT_8_8_8_8_REV;
				break;
			case PixelFormat::PIXELFORMAT_BGRA8888:
				format_ = GL_BGRA;
				internal_format_ = GL_RGBA8;
				type_ = GL_UNSIGNED_INT_8_8_8_8;
				break;
			case PixelFormat::PIXELFORMAT_ARGB2101010:
				format_ = GL_BGRA;
				internal_format_ = GL_RGBA8;
				type_ = GL_UNSIGNED_INT_2_10_10_10_REV;
				break;
			case PixelFormat::PIXELFORMAT_RGB101010:
				format_ = GL_RGB;
				internal_format_ = GL_RGB10;
				type_ = GL_UNSIGNED_INT;
				break;
			case PixelFormat::PIXELFORMAT_YV12:
			case PixelFormat::PIXELFORMAT_IYUV:
				format_ = GL_LUMINANCE;
				internal_format_ = GL_LUMINANCE;
				type_ = GL_UNSIGNED_BYTE;
				is_yuv_planar_ = true;
				ASSERT_LOG(get_type() == Type::TEXTURE_2D, "YUV style pixel format only supported for 2D textures.");
				break;
			case PixelFormat::PIXELFORMAT_YUY2:
			case PixelFormat::PIXELFORMAT_UYVY:
			case PixelFormat::PIXELFORMAT_YVYU:
				ASSERT_LOG(false, "Still to implement YUV packed format textures");
				break;
			default:
				ASSERT_LOG(false, "Unrecognised pixel format");
		}

		int num_textures = is_yuv_planar_ ? 3 : 1;
		glGenTextures(num_textures, &texture_id_[0]);
		for(int n = 0; n != num_textures; ++n) {
			glBindTexture(get_gl_texture_type(get_type()), texture_id_[n]);

			unsigned w = n>0 ? width()/2 : width();
			unsigned h = n>0 ? height()/2 : height();
			unsigned d = n>0 ? depth()/2 : depth();

			const void* pixels = get_surface() ? get_surface()->pixels() : 0;
			switch(get_type()) {
				case Type::TEXTURE_1D:
					glTexImage1D(get_gl_texture_type(get_type()), 0, internal_format_, w, 0, format_, type_, pixels);
					break;
				case Type::TEXTURE_2D:
					glTexImage2D(get_gl_texture_type(get_type()), 0, internal_format_, w, h, 0, format_, type_, pixels);
					break;
				case Type::TEXTURE_3D:
					glTexImage3D(get_gl_texture_type(get_type()), 0, internal_format_, w, h, d, 0, format_, type_, pixels);
					break;
				case Type::TEXTURE_CUBIC:
					// If we are using a cubic texture 		
					ASSERT_LOG(false, "Implement texturing of cubic texture target");
			}
		}
	}

	void OpenGLtexture::init()
	{
		GLenum type = get_gl_texture_type(get_type());

		unsigned num_textures = is_yuv_planar_ ? 3 : 1;
		for(unsigned n = 0; n != num_textures; ++n) {
			glBindTexture(type, texture_id_[n]);

			glTexParameteri(type, GL_TEXTURE_WRAP_S, get_gl_address_mode(get_address_mode_u()));
			if(get_address_mode_u() == address_mode::BORDER) {
				glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, get_border_color().AsFloatVector());
			}
			if(get_type() > Type::TEXTURE_1D) {
				glTexParameteri(type, GL_TEXTURE_WRAP_T, get_gl_address_mode(get_address_mode_v()));
				if(get_address_mode_v() == address_mode::BORDER) {
					glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, get_border_color().AsFloatVector());
				}
			}
			if(get_type() > Type::TEXTURE_2D) {
				glTexParameteri(type, GL_TEXTURE_WRAP_R, get_gl_address_mode(get_address_mode_w()));
				if(get_address_mode_w() == address_mode::BORDER) {
					glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, get_border_color().AsFloatVector());
				}
			}

			if(get_lod_bias() > 1e-14 || get_lod_bias() < -1e-14) {
				glTexParameterf(type, GL_TEXTURE_LOD_BIAS, get_lod_bias());
			}
			if(get_mipmap_levels() > 0) {
				glTexParameteri(type, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(type, GL_TEXTURE_MAX_LEVEL, get_mipmap_levels());
			}

			if(get_mipmap_levels() > 0 && get_type() > Type::TEXTURE_1D) {
				// XXX for OGL >= 1.4 < 3 use: glTexParameteri(type, GL_GENERATE_MIPMAP, GL_TRUE)
				// XXX for OGL < 1.4 manually generate them with glTexImage2D
				// OGL >= 3 use glGenerateMipmap(type);
				glGenerateMipmap(type);
			}

			ASSERT_LOG(get_filtering_min() != filtering::NONE, "'none' is not a valid choice for the minifying filter.");
			ASSERT_LOG(get_filtering_max() != filtering::NONE, "'none' is not a valid choice for the maxifying filter.");
			ASSERT_LOG(get_filtering_mip() != filtering::ANISOTROPIC, "'anisotropic' is not a valid choice for the mip filter.");

			if(get_filtering_min() == filtering::POINT) {
				switch(get_filtering_mip()) {
					case filtering::NONE: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST); break;
					case filtering::POINT: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); break;
					case filtering::LINEAR: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); break;
					case filtering::ANISOTROPIC: ASSERT_LOG(false, "ANISOTROPIC invalid"); break;
				}
			} else if(get_filtering_min() == filtering::LINEAR || get_filtering_min() == filtering::ANISOTROPIC) {
				switch(get_filtering_mip()) {
					case filtering::NONE: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR); break;
					case filtering::POINT: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); break;
					case filtering::LINEAR: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); break;
					case filtering::ANISOTROPIC: ASSERT_LOG(false, "ANISOTROPIC invalid"); break;
				}
			}

			if(get_filtering_max() == filtering::POINT) {
				glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			} else {
				glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}

			if(get_filtering_max() == filtering::ANISOTROPIC || get_filtering_min() == filtering::ANISOTROPIC) {
				if(GL_EXT_texture_filter_anisotropic) {
					float largest_anisotropy;
					glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_anisotropy);
					glTexParameterf(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_anisotropy > get_max_anisotropy() ? get_max_anisotropy() : largest_anisotropy);
				}
			}
		}
	}

	void OpenGLtexture::bind() 
	{ 
		if(is_yuv_planar_) {
			int num_textures = is_yuv_planar_ ? 2 : 0;
			for(int n = num_textures; n > 0; --n) {
				glActiveTexture(GL_TEXTURE0 + n); 			
				glBindTexture(get_gl_texture_type(get_type()), texture_id_[n]);
			}
			glActiveTexture(GL_TEXTURE0);
		}
		glBindTexture(get_gl_texture_type(get_type()), texture_id_[0]);
	}

	unsigned OpenGLtexture::id()
	{
		return texture_id_[0];
	}

	void OpenGLtexture::rebuild()
	{
		// Delete the old id
		glDeleteTextures(is_yuv_planar_ ? 3 : 1, &texture_id_[0]);

		// Re-create the texture
		create_texture(pixel_format_);
		init();
	}
}
