/*
	Copyright (C) 2013-2014 by Kristina Simpson <sweet.kristas@gmail.com>
	
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
#include "DisplayDevice.hpp"
#include "TextureOGL.hpp"

namespace KRE
{
	namespace
	{
		const int maximum_palette_variations = 32;

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

		GLenum GetGLTextureType(TextureType tt) 
		{
			switch(tt) {
				case TextureType::TEXTURE_1D:		return GL_TEXTURE_1D;
				case TextureType::TEXTURE_2D:		return GL_TEXTURE_2D;
				case TextureType::TEXTURE_3D:		return GL_TEXTURE_3D;
				case TextureType::TEXTURE_CUBIC:	return GL_TEXTURE_CUBE_MAP;
			}
			return GL_TEXTURE_2D;
		}

		typedef std::map<SurfacePtr, std::weak_ptr<GLuint>> texture_id_cache;
		texture_id_cache& get_id_cache()
		{
			static texture_id_cache res;
			return res;
		}
	}

	OpenGLTexture::OpenGLTexture(const variant& node, const std::vector<SurfacePtr>& surfaces)
		: Texture(node, surfaces),
		  texture_data_(),
		  is_yuv_planar_(false)
	{
		int max_tex_units = DisplayDevice::getCurrent()->queryParameteri(DisplayDeviceParameters::MAX_TEXTURE_UNITS);
		if(max_tex_units > 0) {
			ASSERT_LOG(static_cast<int>(surfaces.size()) < max_tex_units, "Number of surfaces given exceeds maximum number of texture units for this hardware.");
		}
		
		texture_data_.resize(getSurfaces().size());
		int n = 0;
		for(auto& surf : getSurfaces()) {
			texture_data_[n].surface_format = surf->getPixelFormat()->getFormat();
			createTexture(n);
			++n;
		}
		init();
	}

	OpenGLTexture::OpenGLTexture(const std::vector<SurfacePtr>& surfaces, TextureType type, int mipmap_levels)
		: Texture(surfaces, type, mipmap_levels), 
		  texture_data_(),
		  is_yuv_planar_(false)
	{
		int max_tex_units = DisplayDevice::getCurrent()->queryParameteri(DisplayDeviceParameters::MAX_TEXTURE_UNITS);
		if(max_tex_units > 0) {
			ASSERT_LOG(static_cast<int>(surfaces.size()) < max_tex_units, "Number of surfaces given exceeds maximum number of texture units for this hardware.");
		}
		texture_data_.resize(getSurfaces().size());
		int n = 0;
		for(auto& surf : surfaces) {
			texture_data_[n].surface_format = surf->getPixelFormat()->getFormat();
			createTexture(n);
			++n;
		}
		init();
	}

	OpenGLTexture::OpenGLTexture(int count, int width, int height, int depth, PixelFormat::PF fmt, TextureType type)
		: Texture(count, width, height, depth, fmt, type),
		  texture_data_(),
		  is_yuv_planar_(false)
	{
		int max_tex_units = DisplayDevice::getCurrent()->queryParameteri(DisplayDeviceParameters::MAX_TEXTURE_UNITS);
		if(max_tex_units > 0) {
			ASSERT_LOG(count < max_tex_units, "Number of surfaces given exceeds maximum number of texture units for this hardware.");
		}
		texture_data_.resize(count);
		for(int n = 0; n != count; ++n) {
			texture_data_[n].surface_format = fmt;
			createTexture(n);
		}
		init();
	}

	OpenGLTexture::~OpenGLTexture()
	{
	}

	void OpenGLTexture::update(int n, int x, int width, void* pixels)
	{
		auto& td = texture_data_[n];
		ASSERT_LOG(is_yuv_planar_ == false, "Use updateYUV to update a YUV texture.");
		glBindTexture(GetGLTextureType(getType()), *td.id);
		ASSERT_LOG(getType() == TextureType::TEXTURE_1D, "Tried to do 1D texture update on non-1D texture");
		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, getUnpackAlignment());
		}
		glTexSubImage1D(GetGLTextureType(getType()), 0, x, width, td.format, td.type, pixels);
		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
	}

	// Add a 2D update function which has single stride, but doesn't support planar YUV.
	void OpenGLTexture::update2D(int n, int x, int y, int width, int height, int stride, const void* pixels)
	{
		ASSERT_LOG(is_yuv_planar_ == false, "Use updateYUV to update a YUV texture.");
		auto& td = texture_data_[n];
		glBindTexture(GetGLTextureType(getType()), *td.id);
		ASSERT_LOG(getType() == TextureType::TEXTURE_2D, "Tried to do 2D texture update on non-2D texture: " << static_cast<int>(getType()));
		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, getUnpackAlignment());
		}
		glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
		glTexSubImage2D(GetGLTextureType(getType()), 0, x, y, width, height, td.format, td.type, pixels);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
	}

	void OpenGLTexture::update(int n, int x, int y, int width, int height, const void* pixels)
	{
		ASSERT_LOG(is_yuv_planar_ == false, "Use updateYUV to update a YUV texture.");
		auto& td = texture_data_[n];
		glBindTexture(GetGLTextureType(getType()), *td.id);
		ASSERT_LOG(getType() == TextureType::TEXTURE_2D, "Tried to do 2D texture update on non-2D texture: " << static_cast<int>(getType()));
		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, getUnpackAlignment());
		}
		glTexSubImage2D(GetGLTextureType(getType()), 0, x, y, width, height, td.format, td.type, pixels);
		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
	}

	// Stride is the width of the image surface *in pixels*
	void OpenGLTexture::updateYUV(int x, int y, int width, int height, const std::vector<int>& stride, const void* pixels)
	{
		ASSERT_LOG(is_yuv_planar_, "updateYUV called on non YUV planar texture.");
		int num_textures = is_yuv_planar_ ? 2 : 0;
		for(int n = num_textures; n >= 0; --n) {
			auto& td = texture_data_[n];
			glBindTexture(GetGLTextureType(getType()), *td.id);
			if(static_cast<int>(stride.size()) > n) {
				glPixelStorei(GL_UNPACK_ROW_LENGTH, stride[n]);
			}
			if(getUnpackAlignment() != 4) {
				glPixelStorei(GL_UNPACK_ALIGNMENT, getUnpackAlignment());
			}
			switch(getType()) {
				case TextureType::TEXTURE_1D:
					LOG_WARN("Running 2D texture update on 1D texture.");
					ASSERT_LOG(is_yuv_planar_ == false, "Update of 1D Texture in YUV planar mode.");
					glTexSubImage1D(GetGLTextureType(getType()), 0, x, width, td.format, td.type, pixels);
					break;
				case TextureType::TEXTURE_2D:
					glTexSubImage2D(GetGLTextureType(getType()), 0, x, y, n>0?width/2:width, n>0?height/2:height, td.format, td.type, pixels);
					break;
				case TextureType::TEXTURE_3D:
					ASSERT_LOG(false, "Tried to do 2D texture update on 3D texture");
				case TextureType::TEXTURE_CUBIC:
					ASSERT_LOG(false, "No support for updating cubic textures yet.");
			}
		
			if(getMipMapLevels() > 0 && getType() > TextureType::TEXTURE_1D) {
				glGenerateMipmap(GetGLTextureType(getType()));
			}
		}
		if(!stride.empty()) {
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}
		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
	}

	void OpenGLTexture::update(int n, int x, int y, int z, int width, int height, int depth, void* pixels)
	{
		ASSERT_LOG(is_yuv_planar_ == false, "3D Texture Update function called on YUV planar format.");
		auto& td = texture_data_[n];
		glBindTexture(GetGLTextureType(getType()), *td.id);
		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, getUnpackAlignment());
		}
		switch(getType()) {
			case TextureType::TEXTURE_1D:
				LOG_WARN("Running 2D texture update on 1D texture. You may get unexpected results.");
				glTexSubImage1D(GetGLTextureType(getType()), 0, x, width, td.format, td.type, pixels);
				break;
			case TextureType::TEXTURE_2D:
				LOG_WARN("Running 3D texture update on 2D texture. You may get unexpected results.");
				glTexSubImage2D(GetGLTextureType(getType()), 0, x, y, width, height, td.format, td.type, pixels);
				break;
			case TextureType::TEXTURE_3D:
				glTexSubImage3D(GetGLTextureType(getType()), 0, x, y, z, width, height, depth, td.format, td.type, pixels);
			case TextureType::TEXTURE_CUBIC:
				ASSERT_LOG(false, "No support for updating cubic textures yet.");
		}
		if(getMipMapLevels() > 0 && getType() > TextureType::TEXTURE_1D) {
			glGenerateMipmap(GetGLTextureType(getType()));
		}
		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
	}

	void OpenGLTexture::handleAddPalette(const SurfacePtr& palette)
	{
		ASSERT_LOG(is_yuv_planar_ == false, "Can't create a palette for a YUV surface.");

		if(PixelFormat::isIndexedFormat(getFrontSurface()->getPixelFormat()->getFormat())) {
			// XXX is already an indexed format.
		} else {
			auto histogram = getFrontSurface()->getColorHistogram();
			int num_colors = histogram.size();
			ASSERT_LOG(num_colors < 256, "Can't convert surface to palettized version. Too many colors in source image: " << num_colors);
			LOG_DEBUG("Color count: " << num_colors);

			// Create palette from existing surface
			//LOG_DEBUG("Texture color values:");
			for(auto& hd : histogram) {
				// We're subverting the histogram data to store the index to the color, so we can use
				// it for easy index look up when we traverse the surface next.
				hd.second = texture_data_[0].palette.size();
				texture_data_[0].palette.emplace_back(hd.first);
				//std::cerr << (texture_data_[0].palette.size()-1) << " : " << texture_data_[0].palette.back() << "\n";
			}

			// Create a new indexed surface.
			auto surf = Surface::create(surfaceWidth(), surfaceHeight(), PixelFormat::PF::PIXELFORMAT_INDEX8);

			std::vector<uint8_t> new_pixels;
			new_pixels.resize(surf->rowPitch() * surf->height());
			for(auto px : *getFrontSurface()) {
				Color color(px.red, px.green, px.blue, px.alpha);
				auto it = histogram.find(color);
				ASSERT_LOG(it != histogram.end(), "Couldn't find the color in the surface. Something went terribly wrong.");
				new_pixels[px.x + px.y * surf->rowPitch()] = static_cast<uint8_t>(it->second);
			}
			surf->writePixels(&new_pixels[0], new_pixels.size());

			// save old palette
			auto old_palette = std::move(texture_data_[0].palette);

			// Reset the existing data so we can re-create it.
			texture_data_[0] = TextureData();
			texture_data_[0].surface_format = PixelFormat::PF::PIXELFORMAT_INDEX8;
			texture_data_[0].color_index_map = std::move(histogram);
			texture_data_[0].palette = std::move(old_palette);
			createTexture(0);

			// Set the surface to our new one.
			getSurfaces()[0] = surf;
		}

		SurfacePtr new_palette_surface;
		if(texture_data_.size() > 1) {
			// Already have a palette texture we can use.
			ASSERT_LOG(texture_data_[0].palette_row_index + 1 < maximum_palette_variations, "Only support a maximum of " << maximum_palette_variations << " palettes per texture.");
			new_palette_surface = getSurfaces()[1];
			ASSERT_LOG(new_palette_surface != nullptr, "There was no palette surface found, when there should have been.");
		} else {
			texture_data_.resize(2);
			// We create a surface with <maximum_palette_variations> rows, this allows for a maximum of <maximum_palette_variations> palettes.
			new_palette_surface = Surface::create(texture_data_[0].palette.size(), maximum_palette_variations, PixelFormat::PF::PIXELFORMAT_BGRA8888);
			getSurfaces().emplace_back(new_palette_surface);
			texture_data_[1].surface_format = PixelFormat::PF::PIXELFORMAT_BGRA8888;
			createTexture(1);

			// XXX need to add the original data as row 0 here.
		}

		const int palette_width = texture_data_[0].palette.size();
		// Create altered pixel data and update the surface/texture.
		std::vector<glm::u8vec4> new_pixels;
		new_pixels.reserve(palette_width);
		// Set the new pixel data same as current data.
		for(auto color : texture_data_[0].palette) {
			new_pixels.emplace_back(color.as_u8vec4(ColorByteOrder::BGRA));
		}
		if(palette->width() > palette->height()) {
			for(int x = 0; x != palette->width(); ++x) {
				Color normal_color = palette->getColorAt(x, 0);
				Color mapped_color = palette->getColorAt(x, 1);

				auto it = texture_data_[0].color_index_map.find(normal_color);
				if(it != texture_data_[0].color_index_map.end()) {
					// Found the color in the color map
					new_pixels[it->second] = mapped_color.as_u8vec4(ColorByteOrder::BGRA);
				}
			}
		} else {
			int colors_mapped = 0;
			//LOG_DEBUG("Palette color values:");
			for(int y = 0; y != palette->height(); ++y) {
				Color normal_color = palette->getColorAt(0, y);
				Color mapped_color = palette->getColorAt(1, y);
				//std::cerr << normal_color << " : " << mapped_color << "\n";

				auto it = texture_data_[0].color_index_map.find(normal_color);
				if(it != texture_data_[0].color_index_map.end()) {
					// Found the color in the color map
					new_pixels[it->second] = mapped_color.as_u8vec4(ColorByteOrder::BGRA);
					++colors_mapped;
				}
			}
			LOG_DEBUG("Mapped " << colors_mapped << " out of " << palette_width << " colors from palette");
		}

		// write altered pixel data to texture.
		update(1, 0, texture_data_[0].palette_row_index, palette_width, 1, &new_pixels[0]);
		// write altered pixel data to surface.
		unsigned char* px = reinterpret_cast<unsigned char*>(new_palette_surface->pixelsWriteable());
		memcpy(&px[texture_data_[0].palette_row_index * new_palette_surface->rowPitch()], &new_pixels[0], new_pixels.size() * sizeof(glm::u8vec4));

		// Update the palette row index so it points to the next free location.
		++texture_data_[0].palette_row_index;
	}

	void OpenGLTexture::createTexture(int n)
	{
		auto& td = texture_data_[n];
		auto surf = n < static_cast<int>(getSurfaces().size()) ? getSurfaces()[n] : SurfacePtr();

		// Change the format/internalFormat/type depending on the 
		// data we now about the surface.
		// XXX these need testing for correctness.
		switch(td.surface_format) {
			case PixelFormat::PF::PIXELFORMAT_INDEX1LSB:
			case PixelFormat::PF::PIXELFORMAT_INDEX1MSB:
			case PixelFormat::PF::PIXELFORMAT_INDEX4LSB:
			case PixelFormat::PF::PIXELFORMAT_INDEX4MSB:
			case PixelFormat::PF::PIXELFORMAT_INDEX8:
				if(texture_data_[n].palette.size() == 0) {
					texture_data_[n].palette = getSurfaces()[n]->getPalette();
				}
				td.format = GL_LUMINANCE;
				td.internal_format = GL_LUMINANCE;
				td.type = GL_UNSIGNED_BYTE;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGB332:
				td.format = GL_RGB;
				td.internal_format = GL_R3_G3_B2;
				td.type = GL_UNSIGNED_BYTE_3_3_2;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGB444:
				td.format = GL_RGB;
				td.internal_format = GL_RGB4;
				td.type = GL_UNSIGNED_SHORT;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGB555:
				td.format = GL_RGB;
				td.internal_format = GL_RGB5;
				td.type = GL_UNSIGNED_SHORT;
				break;
			case PixelFormat::PF::PIXELFORMAT_BGR555:
				td.format = GL_BGR;
				td.internal_format = GL_RGB4;
				td.type = GL_UNSIGNED_SHORT;
				break;
			case PixelFormat::PF::PIXELFORMAT_ARGB4444:
				td.format = GL_BGRA;
				td.internal_format = GL_RGBA4;
				td.type =  GL_UNSIGNED_SHORT_4_4_4_4_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGBA4444:
				td.format = GL_RGBA;
				td.internal_format = GL_RGBA4;
				td.type =  GL_UNSIGNED_SHORT_4_4_4_4;
				break;
			case PixelFormat::PF::PIXELFORMAT_ABGR4444:
				td.format = GL_RGBA;
				td.internal_format = GL_RGBA4;
				td.type =  GL_UNSIGNED_SHORT_4_4_4_4_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_BGRA4444:
				td.format = GL_BGRA;
				td.internal_format = GL_RGBA4;
				td.type =  GL_UNSIGNED_SHORT_4_4_4_4;
				break;
			case PixelFormat::PF::PIXELFORMAT_ARGB1555:
				td.format = GL_BGRA;
				td.internal_format = GL_RGB5_A1;
				td.type =  GL_UNSIGNED_SHORT_1_5_5_5_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGBA5551:
				td.format = GL_RGBA;
				td.internal_format = GL_RGB5_A1;
				td.type =  GL_UNSIGNED_SHORT_5_5_5_1;
				break;
			case PixelFormat::PF::PIXELFORMAT_ABGR1555:
				td.format = GL_RGBA;
				td.internal_format = GL_RGB5_A1;
				td.type =  GL_UNSIGNED_SHORT_1_5_5_5_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_BGRA5551:
				td.format = GL_BGRA;
				td.internal_format = GL_RGB5_A1;
				td.type =  GL_UNSIGNED_SHORT_5_5_5_1;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGB565:
				td.format = GL_RGB;
				td.internal_format = GL_RGB;
				td.type =  GL_UNSIGNED_SHORT_5_6_5;
				break;
			case PixelFormat::PF::PIXELFORMAT_BGR565:
				td.format = GL_RGB;
				td.internal_format = GL_RGB;
				td.type =  GL_UNSIGNED_SHORT_5_6_5_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGB24:
				td.format = GL_RGB;
				td.internal_format = GL_RGB8;
				td.type =  GL_UNSIGNED_BYTE;
				break;
			case PixelFormat::PF::PIXELFORMAT_BGR24:
				td.format = GL_BGR;
				td.internal_format = GL_RGB8;
				td.type =  GL_UNSIGNED_BYTE;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGB888:
				td.format = GL_RGB;
				td.internal_format = GL_RGB8;
				td.type =  GL_UNSIGNED_BYTE;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGBX8888:
				td.format = GL_RGB;
				td.internal_format = GL_RGB8;
				td.type =  GL_UNSIGNED_INT_8_8_8_8;
				break;
			case PixelFormat::PF::PIXELFORMAT_BGR888:
				td.format = GL_BGR;
				td.internal_format = GL_RGB8;
				td.type =  GL_UNSIGNED_BYTE;
				break;
			case PixelFormat::PF::PIXELFORMAT_BGRX8888:
				td.format = GL_BGRA;
				td.internal_format = GL_RGB8;
				td.type =  GL_UNSIGNED_INT_8_8_8_8;
				break;
			case PixelFormat::PF::PIXELFORMAT_ARGB8888:
				td.format = GL_BGRA;
				td.internal_format = GL_RGBA8;
				td.type = GL_UNSIGNED_INT_8_8_8_8_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_XRGB8888:
				// XX not sure these are correct or not
				td.format = GL_BGRA;
				td.internal_format = GL_RGB8;
				td.type = GL_UNSIGNED_INT_8_8_8_8_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGBA8888:
				td.format = GL_RGBA;
				td.internal_format = GL_RGBA8;
				td.type = GL_UNSIGNED_INT_8_8_8_8;
				break;
			case PixelFormat::PF::PIXELFORMAT_ABGR8888:
				td.format = GL_RGBA;
				td.internal_format = GL_RGBA8;
				td.type = GL_UNSIGNED_INT_8_8_8_8_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_BGRA8888:
				td.format = GL_BGRA;
				td.internal_format = GL_RGBA8;
				td.type = GL_UNSIGNED_INT_8_8_8_8;
				break;
			case PixelFormat::PF::PIXELFORMAT_ARGB2101010:
				td.format = GL_BGRA;
				td.internal_format = GL_RGBA8;
				td.type = GL_UNSIGNED_INT_2_10_10_10_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_RGB101010:
				td.format = GL_BGRA;
				td.internal_format = GL_RGB10;
				td.type = GL_UNSIGNED_INT_2_10_10_10_REV;
				break;
			case PixelFormat::PF::PIXELFORMAT_YV12:
			case PixelFormat::PF::PIXELFORMAT_IYUV:
				td.format = GL_LUMINANCE;
				td.internal_format = GL_LUMINANCE;
				td.type = GL_UNSIGNED_BYTE;
				is_yuv_planar_ = true;
				ASSERT_LOG(getType() == TextureType::TEXTURE_2D, "YUV style pixel format only supported for 2D textures.");
				break;
			case PixelFormat::PF::PIXELFORMAT_YUY2:
			case PixelFormat::PF::PIXELFORMAT_UYVY:
			case PixelFormat::PF::PIXELFORMAT_YVYU:
				ASSERT_LOG(false, "Still to implement YUV packed format textures");
				break;
			default:
				ASSERT_LOG(false, "Unrecognised pixel format");
		}

		ASSERT_LOG(is_yuv_planar_ == false, "XXX Need to fix yuv planar textures now!");
		//if(is_yuv_planar_) {
		//	texture_data_.resize(3);
		//}

		auto it = get_id_cache().find(surf);
		if(it != get_id_cache().end()) {
			auto cached_id = it->second.lock();
			if(cached_id != nullptr) {
				texture_data_[n].id = cached_id;
				return;
			}
			// if we couldn't lock the id fall through and create a new one
		}

		GLuint new_id;
		glGenTextures(1, &new_id);
		auto id_ptr = std::shared_ptr<GLuint>(new GLuint(new_id), [](GLuint* id) { glDeleteTextures(1, id); delete id; });
		texture_data_[n].id = id_ptr;
		if(surf) {
			get_id_cache()[surf] = id_ptr;
		}

		glBindTexture(GetGLTextureType(getType()), *texture_data_[n].id);

		unsigned w = is_yuv_planar_ && n>0 ? width()/2 : width();
		unsigned h = is_yuv_planar_ && n>0 ? height()/2 : height();
		unsigned d = is_yuv_planar_ && n>0 ? depth()/2 : depth();

		if(getUnpackAlignment() != 4) {
			glPixelStorei(GL_UNPACK_ALIGNMENT, getUnpackAlignment());
		}

		const void* pixels = surf != nullptr ? surf->pixels() : 0;
		switch(getType()) {
			case TextureType::TEXTURE_1D:
				if(pixels == nullptr) {
					glTexImage1D(GetGLTextureType(getType()), 0, td.internal_format, w, 0, td.format, td.type, 0);
				} else {
					glTexImage1D(GetGLTextureType(getType()), 0, td.internal_format, surf->width(), 0, td.format, td.type, pixels);
				}
				break;
			case TextureType::TEXTURE_2D:
				if(pixels == nullptr) {
					glTexImage2D(GetGLTextureType(getType()), 0, td.internal_format, w, h, 0, td.format, td.type, 0);
				} else {
					glTexImage2D(GetGLTextureType(getType()), 0, td.internal_format, surf->width(), surf->height(), 0, td.format, td.type, pixels);
				}
				break;
			case TextureType::TEXTURE_3D:
				// XXX this isn't correct fixme.
				if(pixels == nullptr) {
					glTexImage3D(GetGLTextureType(getType()), 0, td.internal_format, w, h, d, 0, td.format, td.type, 0);
				} else {
					glTexImage3D(GetGLTextureType(getType()), 0, td.internal_format, w, h, d, 0, td.format, td.type, pixels);
				}
				break;
			case TextureType::TEXTURE_CUBIC:
				// If we are using a cubic texture 		
				ASSERT_LOG(false, "Implement texturing of cubic texture target");
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}

	void OpenGLTexture::init()
	{
		GLenum type = GetGLTextureType(getType());

		for(auto& td : texture_data_) {
			glBindTexture(type, *td.id);

			glTexParameteri(type, GL_TEXTURE_WRAP_S, GetGLAddressMode(getAddressModeU()));
			if(getAddressModeU() == AddressMode::BORDER) {
				glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, getBorderColor().asFloatVector());
			}
			if(getType() > TextureType::TEXTURE_1D) {
				glTexParameteri(type, GL_TEXTURE_WRAP_T, GetGLAddressMode(getAddressModeV()));
				if(getAddressModeV() == AddressMode::BORDER) {
					glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, getBorderColor().asFloatVector());
				}
			}
			if(getType() > TextureType::TEXTURE_2D) {
				glTexParameteri(type, GL_TEXTURE_WRAP_R, GetGLAddressMode(getAddressModeW()));
				if(getAddressModeW() == AddressMode::BORDER) {
					glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, getBorderColor().asFloatVector());
				}
			}

			if(getLodBias() > 1e-14 || getLodBias() < -1e-14) {
				glTexParameterf(type, GL_TEXTURE_LOD_BIAS, getLodBias());
			}
			if(getMipMapLevels() > 0) {
				glTexParameteri(type, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(type, GL_TEXTURE_MAX_LEVEL, getMipMapLevels());
			}

			if(getMipMapLevels() > 0 && getType() > TextureType::TEXTURE_1D) {
				// XXX for OGL >= 1.4 < 3 use: glTexParameteri(type, GL_GENERATE_MIPMAP, GL_TRUE)
				// XXX for OGL < 1.4 manually generate them with glTexImage2D
				// OGL >= 3 use glGenerateMipmap(type);
				glGenerateMipmap(type);
			}

			ASSERT_LOG(getFilteringMin() != Filtering::NONE, "'none' is not a valid choice for the minifying filter.");
			ASSERT_LOG(getFilteringMax() != Filtering::NONE, "'none' is not a valid choice for the maxifying filter.");
			ASSERT_LOG(getFilteringMip() != Filtering::ANISOTROPIC, "'anisotropic' is not a valid choice for the mip filter.");

			if(getFilteringMin() == Filtering::POINT) {
				switch(getFilteringMip()) {
					case Filtering::NONE: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST); break;
					case Filtering::POINT: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); break;
					case Filtering::LINEAR: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); break;
					case Filtering::ANISOTROPIC: ASSERT_LOG(false, "ANISOTROPIC invalid"); break;
				}
			} else if(getFilteringMin() == Filtering::LINEAR || getFilteringMin() == Filtering::ANISOTROPIC) {
				switch(getFilteringMip()) {
					case Filtering::NONE: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR); break;
					case Filtering::POINT: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); break;
					case Filtering::LINEAR: glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); break;
					case Filtering::ANISOTROPIC: ASSERT_LOG(false, "ANISOTROPIC invalid"); break;
				}
			}

			if(getFilteringMax() == Filtering::POINT) {
				glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			} else {
				glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}

			if(getFilteringMax() == Filtering::ANISOTROPIC || getFilteringMin() == Filtering::ANISOTROPIC) {
				if(GL_EXT_texture_filter_anisotropic) {
					float largest_anisotropy;
					glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_anisotropy);
					glTexParameterf(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_anisotropy > getMaxAnisotropy() ? getMaxAnisotropy() : largest_anisotropy);
				}
			}
		}
	}

	void OpenGLTexture::bind() 
	{
		int n = texture_data_.size()-1;
		for(auto it = texture_data_.rbegin(); it != texture_data_.rend(); ++it) {
			glActiveTexture(GL_TEXTURE0 + n++);
			glBindTexture(GetGLTextureType(getType()), *it->id);
		}
	}

	unsigned OpenGLTexture::id(int n)
	{
		ASSERT_LOG(n < static_cast<int>(texture_data_.size()), "Requested texture id outside bounds.");
		return *texture_data_[n].id;
	}

	void OpenGLTexture::rebuild()
	{
		// Delete the old ids
		int num_tex = texture_data_.size();
		texture_data_.clear();
		texture_data_.resize(num_tex);

		// Re-create the texture
		for(int n = 0; n != num_tex; ++n) {
			createTexture(n);
		}
		init();
	}

	const unsigned char* OpenGLTexture::colorAt(int x, int y) const 
	{
		if(getFrontSurface() == nullptr) {
			// We could probably try a glTexImage fall-back here. But ugh, slow.
			return nullptr;
		}
		auto s = getFrontSurface();
		const unsigned char* pixels = reinterpret_cast<const unsigned char*>(s->pixels());
		return (pixels + (y*s->width() + x)*s->getPixelFormat()->bytesPerPixel());
	}

	TexturePtr OpenGLTexture::clone()
	{
		return TexturePtr(new OpenGLTexture(*this));
	}

	void OpenGLTexture::handleClearTextures()
	{
		get_id_cache().clear();
	}
}
