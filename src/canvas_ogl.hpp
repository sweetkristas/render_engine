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

#include "canvas.hpp"

namespace KRE
{
	class canvas_ogl : public canvas
	{
	public:
		canvas_ogl();
		virtual ~canvas_ogl();

		void blit_texture(const texture_ptr& tex, const rect& src, float rotation, const rect& dst, const color& color) override;
		void blit_texture(const MaterialPtr& mat, float rotation, const rect& dst, const color& color) override;

		static canvas_ptr get_instance();
	private:
		void handle_dimensions_changed() override;
		glm::mat4 mvp_;
	};
}
