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

#include "AttributeSetOpenGL.hpp"

namespace KRE
{
	namespace
	{
		GLenum convert_access_type_and_frequency(Attribute::AccessFreqHint f, Attribute::AccessTypeHint t)
		{
			switch(f) {
			case Attribute::AccessFreqHint::STATIC:
				switch(t) {
				case Attribute::AccessTypeHint::DRAW: return GL_STATIC_DRAW;
				case Attribute::AccessTypeHint::READ: return GL_STATIC_READ;
				case Attribute::AccessTypeHint::COPY: return GL_STATIC_COPY;
				}
				break;
			case Attribute::AccessFreqHint::STREAM:
				switch(t) {
				case Attribute::AccessTypeHint::DRAW: return GL_STREAM_DRAW;
				case Attribute::AccessTypeHint::READ: return GL_STREAM_READ;
				case Attribute::AccessTypeHint::COPY: return GL_STREAM_COPY;
				}
				break;
			case Attribute::AccessFreqHint::DYNAMIC:
				switch(t) {
				case Attribute::AccessTypeHint::DRAW: return GL_DYNAMIC_DRAW;
				case Attribute::AccessTypeHint::READ: return GL_DYNAMIC_READ;
				case Attribute::AccessTypeHint::COPY: return GL_DYNAMIC_COPY;
				}
				break;
			}
			ASSERT_LOG(false, "Not a valid combination of Access Frequency and Access Type.");
			return GL_NONE;
		}
	}

	AttributeOGL::AttributeOGL(AccessFreqHint freq, AccessTypeHint type)
		: Attribute(freq, type),
		access_pattern_(convert_access_type_and_frequency(freq, type))
	{
		glGenBuffers(1, &buffer_id_);
	}

	AttributeOGL::~AttributeOGL()
	{
		glDeleteBuffers(1, &buffer_id_);
	}

	void AttributeOGL::Update(const void* value, ptrdiff_t offset, size_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id_);
		if(offset == 0) {
			// this is a minor optimisation.
			glBufferData(GL_ARRAY_BUFFER, size, 0, access_pattern_);
			glBufferSubData(GL_ARRAY_BUFFER, 0, size, value);
			size_ = size;
		} else {
			ASSERT_LOG(Size() != 0, "No buffer previously bound.");
			ASSERT_LOG(offset + size <= Size(), 
				"When buffering data offset+size exceeds data store size: " 
				<< offset+size 
				<< " > " 
				<< Size());
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, value);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	intptr_t AttributeOGL::Value() const
	{
		return 0;
	}

	void AttributeOGL::Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id_);
	}

	void AttributeOGL::Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	AttributeSetOGL::AttributeSetOGL(bool indexed, bool instanced)
		: AttributeSet(indexed, instanced)
	{
		if(indexed) {
			glGenBuffers(1, &index_buffer_id_);
		}
	}

	AttributeSetOGL::~AttributeSetOGL()
	{
		if(IsIndexed()) {
			glDeleteBuffers(1, &index_buffer_id_);
		}
	}

	AttributePtr AttributeSetOGL::CreateAttribute(Attribute::AccessFreqHint freq, Attribute::AccessTypeHint type)
	{
		GetAttributes().emplace_back(new AttributeOGL(freq, type));
		return GetAttributes().back();
	}

	void AttributeSetOGL::BindIndex()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id_);
	}

	void AttributeSetOGL::UnbindIndex()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}
