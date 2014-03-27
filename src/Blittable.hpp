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

#include "Geometry.hpp"
#include "Material.hpp"
#include "SceneObject.hpp"

namespace KRE
{
	struct vertex_texcoord
	{
		vertex_texcoord() : vtx(0.0f), tc(0.0f) {}
		vertex_texcoord(const glm::vec2& v, const glm::vec2& c) : vtx(v), tc(c) {}
		glm::vec2 vtx;
		glm::vec2 tc;
	};

	// This is basically a helper class that lets you blit a 
	// texture to the screen in what should be a relatively 
	// optimised manner.
	class Blittable : public SceneObject
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
		Blittable();
		explicit Blittable(const TexturePtr& tex);
		explicit Blittable(const MaterialPtr& mat);
		virtual ~Blittable();
		void SetTexture(const TexturePtr& tex);

		template<typename T>
		void SetDrawRect(const Geometry::Rect<T>& r) {
			draw_rect_ = r.as_type<float>();
		}
		virtual void PreRender() override;

		Centre GetCentre() const { return centre_; }
		void SetCentre(Centre c);
		const pointf& GetCentreCoords() const { return centre_offset_; }
		template<typename T>
		void SetCentreCoords(const Geometry::Point<T>& p) {
			centre_offset_ = p;
			centre_ = Centre::MANUAL;
		}

		virtual DisplayDeviceDef Attach(const DisplayDevicePtr& dd);
	private:
		void Init();
		std::shared_ptr<Attribute<vertex_texcoord>> attribs_;
		rectf draw_rect_;
		pointf centre_offset_;
		Centre centre_;
	};
}
