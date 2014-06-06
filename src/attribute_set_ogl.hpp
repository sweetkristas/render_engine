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

#include "attribute_set.hpp"

namespace KRE
{
	class hardware_attribute_ogl : public hardware_attribute
	{
	public:
		hardware_attribute_ogl(attribute_base* parent);
		virtual ~hardware_attribute_ogl();
		void update(const void* value, ptrdiff_t offset, size_t size) override;
		void bind() override;
		void unbind() override;
		intptr_t value() override { return 0; }
	private:
		GLuint buffer_id_;
		GLenum access_pattern_;
		size_t size_;
	};


	class attribute_set_ogl : public attribute_set
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
	
		explicit attribute_set_ogl(bool indexed, bool instanced);
		virtual ~attribute_set_ogl();	
		const void* get_index_array() const override { return NULL; }
		void bind_index() override;
		void unbind_index() override;
		bool is_hardware_backed() const override { return true; }
	private:
		//DISALLOW_COPY_ASSIGN_AND_DEFAULT(attribute_set_ogl);
		attribute_set_ogl();
		attribute_set_ogl(const attribute_set_ogl&);
		void operator=(const attribute_set_ogl&);

		void handle_index_update() override;
		GLuint index_buffer_id_;
	};
	typedef std::shared_ptr<attribute_set> attribute_set_ptr;
}
