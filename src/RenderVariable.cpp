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

#include <algorithm>
#include <iterator>
#include <vector>
#include <glm/glm.hpp>

#include "asserts.hpp"
#include "RenderVariable.hpp"

namespace Render
{
	RenderVariableDesc::RenderVariableDesc(VertexType vertex_type, 
		unsigned num_elements, 
		VariableType var_type, 
		bool normalised, 
		unsigned stride, 
		unsigned offset)
		: desc_type_(DESC_ATTRIB),
		vertex_type_(vertex_type), 
		num_elements_(num_elements),
		var_type_(var_type),
		normalised_(normalised),
		stride_(stride),
		offset_(offset)
	{
		switch(vertex_type_) {
			case VERTEX_POSITION:	vertex_name_ = "position"; break;
			case VERTEX_COLOR:		vertex_name_ = "color"; break;
			case VERTEX_TEXTURE:	vertex_name_ = "texture"; break;
			case VERTEX_NORMAL:		vertex_name_ = "normal"; break;
			default:
				ASSERT_LOG(false, "vertex_type has an unknown value: " << vertex_type_);
		}
	}

	RenderVariableDesc::RenderVariableDesc(const std::string& vertex_name, 
		unsigned num_elements, 
		VariableType var_type, 
		bool normalised, 
		unsigned stride, 
		unsigned offset)
		: desc_type_(DESC_ATTRIB),
		vertex_type_(VERTEX_UNKNOWN), 
		vertex_name_(vertex_name),
		num_elements_(num_elements),
		var_type_(var_type),
		normalised_(normalised),
		stride_(stride),
		offset_(offset)
	{
	}

	RenderVariableDesc::RenderVariableDesc(UniformType uniform_type,
		unsigned num_elements,
		UniformVariableType uniform_var_type)
		: desc_type_(DESC_UNIFORM),
		uniform_type_(uniform_type),
		num_elements_(num_elements),
		uniform_var_type_(uniform_var_type)
	{
		switch(uniform_type_) {
			case UNIFORM_COLOR:			uniform_name_ = "color"; break;
			case UNIFORM_POINT_SIZE:	uniform_name_ = "point_size"; break;
			case UNIFORM_MODEL:			uniform_name_ = "model_matrix"; break;
			default:
				ASSERT_LOG(false, "uniform_type has an unknown value: " << uniform_type_);
		}
	}

	RenderVariableDesc::RenderVariableDesc(const std::string& uniform_name,
		unsigned num_elements,
		UniformVariableType uniform_var_type)
		: desc_type_(DESC_UNIFORM),
		uniform_type_(UNIFORM_UNKOWN),
		uniform_name_(uniform_name),
		num_elements_(num_elements),
		uniform_var_type_(uniform_var_type)
	{
	}

	RenderVariableDesc::~RenderVariableDesc()
	{
	}


	void RenderVariable::AddVariableDescription(RenderVariableDesc::VertexType vertex_type, 
		unsigned num_elements, 
		RenderVariableDesc::VariableType var_type, 
		bool normalised, 
		unsigned stride, 
		unsigned offset)
	{
		desc_list_.push_back(RenderVariableDesc(vertex_type, num_elements, var_type, normalised, stride, offset));
	}

	void RenderVariable::AddVariableDescription(const std::string& vertex_type, 
		unsigned num_elements, 
		RenderVariableDesc::VariableType var_type, 
		bool normalised, 
		unsigned stride, 
		unsigned offset)
	{
		desc_list_.push_back(RenderVariableDesc(vertex_type, num_elements, var_type, normalised, stride, offset));
	}

	void RenderVariable::AddVariableDescription(const std::string& uniform_name, 
		unsigned num_elements,
		RenderVariableDesc::UniformVariableType uniform_var_type)
	{
		desc_list_.push_back(RenderVariableDesc(uniform_name, num_elements, uniform_var_type));
	}

	void RenderVariable::AddVariableDescription(RenderVariableDesc::UniformType uniform_type,
		unsigned num_elements,
		RenderVariableDesc::UniformVariableType uniform_var_type)
	{
		desc_list_.push_back(RenderVariableDesc(uniform_type, num_elements, uniform_var_type));
	}
}