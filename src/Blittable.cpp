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

#include "Blittable.hpp"
#include "DisplayDevice.hpp"
#include "Material.hpp"

namespace KRE
{
	Blittable::Blittable()
		: SceneObject("blittable")
	{
		Init();
	}

	Blittable::Blittable(const TexturePtr& tex)
		: SceneObject("blittable")
	{
		SetTexture(tex);
		Init();
	}

	Blittable::Blittable(const MaterialPtr& mat)
		: SceneObject("blittable")
	{
		SetMaterial(mat);
		Init();
	}

	Blittable::~Blittable()
	{
	}

	void Blittable::Init()
	{
		auto as = DisplayDevice::CreateAttributeSet();
		attribs_.reset(new Attribute<vertex_texcoord>(AccessFreqHint::DYNAMIC, AccessTypeHint::DRAW));
		attribs_->AddAttributeDescription(AttributeDesc(AttributeDesc::Type::POSITION, 2, AttributeDesc::VariableType::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, vtx)));
		attribs_->AddAttributeDescription(AttributeDesc(AttributeDesc::Type::TEXTURE,  2, AttributeDesc::VariableType::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, tc)));
		as->AddAttribute(AttributeBasePtr(attribs_));
		as->SetDrawMode(AttributeSet::DrawMode::TRIANGLE_STRIP);
		
		AddAttributeSet(as);
	}

	void Blittable::SetTexture(const TexturePtr& tex)
	{
		SetMaterial(DisplayDevice::CreateMaterial("blittable", std::vector<TexturePtr>(1,tex)));
	}

	DisplayDeviceDef Blittable::Attach(const DisplayDevicePtr& dd) 
	{
		DisplayDeviceDef def(GetAttributeSet()/*, GetUniformSet()*/);
		return def;
	}

	void Blittable::PreRender()
	{
		// XXX we should only do this if things changed.
		const float vx1 = draw_rect_.x();
		const float vy1 = draw_rect_.y();
		const float vx2 = draw_rect_.x2();
		const float vy2 = draw_rect_.y2();

		rectf r = Material()->GetNormalisedTextureCoords(Material()->GetTexture().begin());

		std::vector<vertex_texcoord> vertices;
		vertices.emplace_back(glm::vec2(vx1,vy1), glm::vec2(r.x(),r.y()));
		vertices.emplace_back(glm::vec2(vx2,vy1), glm::vec2(r.x2(),r.y()));
		vertices.emplace_back(glm::vec2(vx1,vy2), glm::vec2(r.x(),r.y2()));
		vertices.emplace_back(glm::vec2(vx2,vy2), glm::vec2(r.x2(),r.y2()));
		GetAttributeSet().back()->SetCount(vertices.size());
		attribs_->Update(&vertices);
	}
}
