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

#include "MaterialOpenGL.hpp"
#include "TextureOpenGL.hpp"

namespace Graphics
{

	OpenGLMaterial::OpenGLMaterial(const variant& node) 
		: Material(node) 
	{
	}

	OpenGLMaterial::~OpenGLMaterial()
	{
	}

	void OpenGLMaterial::Apply() 
	{
		ASSERT_LOG(false, "XXX: OpenGLMaterial::Apply");
	}

	TexturePtr OpenGLMaterial::CreateTexture(const variant& node)
	{
		ASSERT_LOG(node.has_key("image") || node.has_key("texture"), "Must have either 'image' or 'texture' attribute.");
		const std::string image_name = node.has_key("image") ? node["image"].as_string() : node["texture"].as_string();
		auto surface = Surface::Create(image_name);
		return TexturePtr(new OpenGLTexture(surface, node));
	}
}
