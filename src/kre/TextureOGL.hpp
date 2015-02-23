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

#pragma once

#include <GL/glew.h>

#include "Texture.hpp"

namespace KRE
{
	class OpenGLTexture : public Texture
	{
	public:
		explicit OpenGLTexture(const variant& node, const std::vector<SurfacePtr>& surfaces);
		explicit OpenGLTexture(const std::vector<SurfacePtr>& surfaces, TextureType type, int mipmap_levels);
		explicit OpenGLTexture(int count, int width, int height, int depth, PixelFormat::PF fmt, TextureType type);
		virtual ~OpenGLTexture();

		void bind() override;

		void init() override;
		unsigned id(int n) override;

		void update(int x, int width, void* pixels) override;
		void update(int x, int y, int width, int height, const int* stride, const void* pixels) override;
		void update(int x, int y, int width, int height, const std::vector<unsigned>& stride, const void* pixels) override;
		void update(int x, int y, int z, int width, int height, int depth, void* pixels) override;

		const unsigned char* colorAt(int x, int y) const override;

		TexturePtr clone() override;
		static void handleClearTextures();
	private:
		void createTexture(int n);
		void rebuild() override;
		void handleAddPalette(const SurfacePtr& palette) override;

		// For YUV family textures we need two more texture id's
		// since we hold them in seperate textures.
		// XXX if we're copying a texture we want to use a shared pointer so
		// we don't accidentally delete textures that are potentially still in use.
		// Still deciding whether to use a vector of shared_ptr<GLuint>
		// Whether to store the textures in a registry, with ref-counting.
		// or what we do here.
		struct TextureData {
			TextureData() 
				: id(), 
				  surface_format(PixelFormat::PF::PIXELFORMAT_UNKNOWN), 
				  palette(), 
				  format(GL_RGBA), 
				  internal_format(GL_RGBA), 
				  type(GL_UNSIGNED_BYTE) 
			{
			}
			std::shared_ptr<GLuint> id;
			PixelFormat::PF surface_format;
			std::vector<Color> palette;
			GLenum format;
			GLenum internal_format;
			GLenum type;
		};
		std::vector<TextureData> texture_data_;

		// Set for YUV style textures;
		bool is_yuv_planar_;
	};
}
