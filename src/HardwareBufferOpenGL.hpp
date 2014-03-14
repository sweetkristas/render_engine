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

#include <memory>
#include "asserts.hpp"
#include "HardwareBuffer.hpp"

namespace Graphics
{
	template<class T>
	class OpenGLHardwareBuffer : public HardwareBuffer<T>
	{
		OpenGLHardwareBuffer(size_t count = 0,
			bool dynamic = true, 
			bool shareable = true, 
			bool geometry_related = true)
			: HardwareBuffer(count, dynamic, shareable, geometry_related) {
			ASSERT_LOG(GLEW_ARB_vertex_buffer_object, "Hardware buffers (VBO) not supported.");
			glGenBuffers(1, &vbo_id_);
			if(dynamic) {
				usage_ = GL_DYNAMIC_DRAW;
			} else {
				usage_ = GL_STATIC_DRAW;
			}
		}
		virtual ~OpenGLHardwareBuffer() {
			glDeleteBuffers(1, &vbo_id_);
		}
		void Update(std::vector<T>* value) {
			glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);
			glBufferData(GL_ARRAY_BUFFER, sizeof(T)*value->size(), &(*value)[0], usage_);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		void* Value() { return NULL; }
	private:
		OpenGLHardwareBuffer(const OpenGLHardwareBuffer&);
		OpenGLHardwareBuffer& operator=(const OpenGLHardwareBuffer&);
		GLuint vbo_id_;
		GLenum usage_;
	};
}
