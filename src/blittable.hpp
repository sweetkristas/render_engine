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

#include "geometry.hpp"
#include "Material.hpp"
#include "scene_object.hpp"

namespace KRE
{
	// This is basically a helper class that lets you blit a 
	// texture to the screen in what should be a relatively 
	// optimised manner.
	class blittable : public scene_object
	{
	public:
		enum class Centre {
			MIDDLE,
			TOP_LEFT,
			TOP_RIGHT,
			BOTTOM_LEFT,
			BOTTOM_RIGHT,
			MANUAL,
		};
		blittable();
		explicit blittable(const TexturePtr& tex);
		explicit blittable(const MaterialPtr& mat);
		virtual ~blittable();
		void set_texture(const TexturePtr& tex);

		template<typename T>
		void set_draw_rect(const geometry::rect_t<T>& r) {
			draw_rect_ = r.template as_type<float>();
		}
		virtual void pre_render() override;

		Centre get_centre() const { return centre_; }
		void set_centre(Centre c);
		const pointf& get_centre_coords() const { return centre_offset_; }
		template<typename T>
		void set_centre_coords(const geometry::point_t<T>& p) {
			centre_offset_ = p;
			centre_ = Centre::MANUAL;
		}

		virtual DisplayDeviceDef Attach(const DisplayDevicePtr& dd);
	private:
		void init();
		std::shared_ptr<Attribute<vertex_texcoord>> attribs_;
		rectf draw_rect_;
		pointf centre_offset_;
		Centre centre_;
	};
}
