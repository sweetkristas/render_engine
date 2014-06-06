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

#include "blittable.hpp"
#include "display_device.hpp"
#include "Material.hpp"

namespace KRE
{
	blittable::blittable()
		: scene_object("blittable"),
		centre_(Centre::MIDDLE),
		centre_offset_()
	{
		init();
	}

	blittable::blittable(const texture_ptr& tex)
		: scene_object("blittable"),
		centre_(Centre::MIDDLE),
		centre_offset_()
	{
		set_texture(tex);
		init();
	}

	blittable::blittable(const MaterialPtr& mat)
		: scene_object("blittable"),
		centre_(Centre::MIDDLE),
		centre_offset_()
	{
		set_material(mat);
		init();
	}

	blittable::~blittable()
	{
	}

	void blittable::init()
	{
		auto as = display_device::create_attribute_set();
		attribs_.reset(new attribute<vertex_texcoord>(AccessFreqHint::DYNAMIC, AccessTypeHint::DRAW));
		attribs_->set_attr_desc(attribute_desc(attribute_desc::Type::POSITION, 2, attribute_desc::VariableType::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, vtx)));
		attribs_->set_attr_desc(attribute_desc(attribute_desc::Type::TEXTURE,  2, attribute_desc::VariableType::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, tc)));
		as->add_attribute(attribute_base_ptr(attribs_));
		as->set_draw_mode(attribute_set::DrawMode::TRIANGLE_STRIP);
		
		add_attribute_set(as);
	}

	void blittable::set_texture(const texture_ptr& tex)
	{
		set_material(display_device::create_material("blittable", std::vector<texture_ptr>(1,tex)));
	}

	display_device_def blittable::attach(const display_device_ptr& dd) 
	{
		display_device_def def(get_attribute_set()/*, get_uniform_set()*/);
		return def;
	}

	void blittable::pre_render()
	{
		float offs_x = 0.0f;
		float offs_y = 0.0f;
		switch(centre_) {
			case Centre::MIDDLE:		
				offs_x = -draw_rect_.w()/2.0f;
				offs_y = -draw_rect_.h()/2.0f;
				break;
			case Centre::TOP_LEFT: break;
			case Centre::TOP_RIGHT:
				offs_x = -draw_rect_.w();
				offs_y = 0;
				break;
			case Centre::BOTTOM_LEFT:
				offs_x = 0;
				offs_y = -draw_rect_.h();
				break;
			case Centre::BOTTOM_RIGHT:
				offs_x = -draw_rect_.w();
				offs_y = -draw_rect_.h();
				break;
			case Centre::MANUAL:
				offs_x = centre_offset_.x;
				offs_y = centre_offset_.y;
				break;
		}

		// XXX we should only do this if things changed.
		const float vx1 = draw_rect_.x() + offs_x;
		const float vy1 = draw_rect_.y() + offs_y;
		const float vx2 = draw_rect_.x2() + offs_x;
		const float vy2 = draw_rect_.y2() + offs_y;

		rectf r = material()->GetNormalisedtextureCoords(material()->Gettexture().begin());

		std::vector<vertex_texcoord> vertices;
		vertices.emplace_back(glm::vec2(vx1,vy1), glm::vec2(r.x(),r.y()));
		vertices.emplace_back(glm::vec2(vx2,vy1), glm::vec2(r.x2(),r.y()));
		vertices.emplace_back(glm::vec2(vx1,vy2), glm::vec2(r.x(),r.y2()));
		vertices.emplace_back(glm::vec2(vx2,vy2), glm::vec2(r.x2(),r.y2()));
		get_attribute_set().back()->set_count(vertices.size());
		attribs_->update(&vertices);
	}

	void blittable::set_centre(Centre c)
	{
		centre_  = c;
		centre_offset_ = pointf();
	}
}
