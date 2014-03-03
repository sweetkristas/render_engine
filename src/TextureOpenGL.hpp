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

#include <GL/glew.h>

#include "Texture.hpp"

namespace Graphics
{
	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture(const SurfacePtr& surface, const variant& node);
		OpenGLTexture(const SurfacePtr& surface, 
			TextureType type=TextureType::TEXTURE_2D, 
			int mipmap_levels=0);
		virtual ~OpenGLTexture();

		void Bind(int n=0);

		int Width() const { return width_; }
		int Height() const { return height_; }

		virtual void Init() override;

		void Update(int x, int y, int width, int height, int stride, void* pixels) override;
	private:
		void CreateTexture();

		int width_;
		int height_;
		GLuint texture_id_;

		GLenum format_;
		GLenum internal_format_;
		GLenum type_;
	};
}
