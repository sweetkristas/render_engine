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

#include "color.hpp"
#include "geometry.hpp"
#include "Material.hpp"
#include "util.hpp"
#include "vector_graphics.hpp"

namespace KRE
{
	class canvas;
	typedef std::shared_ptr<canvas> canvas_ptr;

	// A 2D canvas class for drawing on. Not in the renderable pipelines.
	// Canvas writes are done in the order in the code.
	// Intended for making things like UI's.
	class canvas
	{
	public:
		virtual ~canvas();

		void set_dimensions(unsigned w, unsigned h);

		unsigned width() const { return width_; }
		unsigned height() const { return height_; }

		// Blit's a texture from co-ordinates given in src to the screen co-ordinates dst
		virtual void blit_texture(const texture_ptr& tex, const rect& src, float rotation, const rect& dst, const color& color) = 0;

		// Blit's a material from internal co-ordinates to destination screen co-ordinates.
		virtual void blit_texture(const MaterialPtr& mat, float rotation, const rect& dst, const color& color) = 0;

		//void DrawVectorContext(const Vector::ContextPtr& context);
		static canvas_ptr get_instance();
	protected:
		canvas();
	private:
		DISALLOW_COPY_AND_ASSIGN(canvas);
		unsigned width_;
		unsigned height_;
		virtual void handle_dimensions_changed() = 0;
	};
}
