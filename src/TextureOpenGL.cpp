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

#include "TextureOpenGL.hpp"

namespace Graphics
{
	/// XXXXX we need a way to specify texture wrap and min/mag filtering
	// for textures based on a variant type.

	OpenGLTexture::OpenGLTexture(const std::string& filename)
		: Texture(filename)
	{
		// XXX
	}

	OpenGLTexture::OpenGLTexture(const SurfacePtr& surface)
		: Texture(surface)
	{
		glGenTextures(1, &texture_id_);
		glBindTexture(GL_TEXTURE_2D, texture_id_);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glDeleteTextures(1, &texture_id_);
	}

	void OpenGLTexture::Update(int x, int y, int width, int height, int stride, void* pixels)
	{
		glBindTexture(GL_TEXTURE_2D, texture_id_);
		// XXX GL_UNSIGNED_BYTE may not be correct here?
		// May need to use GL_BGRA &  GL_UNSIGNED_INT_8_8_8_8_REV if input format is
		// ARGB
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA,  GL_UNSIGNED_BYTE, pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
