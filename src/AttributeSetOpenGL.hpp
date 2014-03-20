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

#include "AttributeSet.hpp"

namespace KRE
{
	class AttributeOGL : public Attribute
	{
	public:
		AttributeOGL(AccessFreqHint freq, AccessTypeHint type);
		virtual ~AttributeOGL();
		void Update(const void* value, ptrdiff_t offset, size_t size) override;
		const void* Value() const override;
		size_t Size() const override { return size_; }
		void Bind() override;
		void Unbind() override;
	private:
		DISALLOW_COPY_AND_ASSIGN(AttributeOGL);
		GLuint buffer_id_;
		size_t size_;
		GLenum access_pattern_;
	};

	class AttributeSetOGL : public AttributeSet
	{
	public:
		enum class DrawMode {
			POINTS,
			LINE_STRIP,
			LINE_LOOP,
			LINES,
			TRIANGLE_STRIP,
			TRIANGLE_FAN,
			TRIANGLES,
			QUAD_STRIP,
			QUADS,
			POLYGON,		
		};
	
		explicit AttributeSetOGL(bool indexed, bool instanced);
		virtual ~AttributeSetOGL();	
		AttributePtr CreateAttribute(Attribute::AccessFreqHint freq, Attribute::AccessTypeHint type) override;
	private:
		DISALLOW_COPY_ASSIGN_AND_DEFAULT(AttributeSetOGL);
		GLuint index_buffer_id_;
	};
	typedef std::shared_ptr<AttributeSet> AttributeSetPtr;
}
