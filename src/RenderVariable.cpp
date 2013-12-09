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

#include "RenderVariable.hpp"

namespace Render
{
	namespace 
	{
		typedef std::vector<std::function<RenderVariablePtr(const RenderVariableDescription&)>> factory_worker_list;
		factory_worker_list& get_factory_workers()
		{
			static factory_worker_list res;
			return res;
		}
	}

	RenderVariableDescription::RenderVariableDescription(const std::string& name, 
		VariableType type,
		VariableUsage use, 
		size_t count,
		bool dynamic, 
		bool shareable, 
		bool geometry_related)
		: name_(name), 
		type_(type), 
		count_(count), 
		usage_(use), 
		dynamic_(dynamic), 
		shareable_(shareable), 
		geometry_related_(geometry_related)
	{
	}

	RenderVariable::RenderVariable(const RenderVariableDescription& desc)
		: desc_(desc)
	{}

	RenderVariable::~RenderVariable()
	{}

	void RenderVariable::register_factory_worker(std::function<RenderVariablePtr(const RenderVariableDescription&)> fn)
	{
		get_factory_workers().push_back(fn);
	}

	RenderVariablePtr RenderVariable::factory(const RenderVariableDescription& desc)
	{
		for(auto fn : get_factory_workers()) {
			auto ret = fn(desc);
			if(ret != NULL) {
				return ret;
			}
		}
		switch(desc.type()) {
			case RenderVariableDescription::TYPE_FLOAT: 
				return RenderVariablePtr(new LocalRenderVariable<float>(desc));
			case RenderVariableDescription::TYPE_FLOAT_VEC2:
				return RenderVariablePtr(new LocalRenderVariable<glm::vec2>(desc));
			case RenderVariableDescription::TYPE_FLOAT_VEC3:
				return RenderVariablePtr(new LocalRenderVariable<glm::vec3>(desc));
			case RenderVariableDescription::TYPE_FLOAT_VEC4:
				return RenderVariablePtr(new LocalRenderVariable<glm::vec4>(desc));
			case RenderVariableDescription::TYPE_FLOAT_MAT2:
				return RenderVariablePtr(new LocalRenderVariable<glm::mat2>(desc));
			case RenderVariableDescription::TYPE_FLOAT_MAT3:
				return RenderVariablePtr(new LocalRenderVariable<glm::mat3>(desc));
			case RenderVariableDescription::TYPE_FLOAT_MAT4:
				return RenderVariablePtr(new LocalRenderVariable<glm::mat4>(desc));
			case RenderVariableDescription::TYPE_INT:
				return RenderVariablePtr(new LocalRenderVariable<int32_t>(desc));
			case RenderVariableDescription::TYPE_INT_VEC2:
				return RenderVariablePtr(new LocalRenderVariable<glm::ivec2>(desc));
			case RenderVariableDescription::TYPE_INT_VEC3:
				return RenderVariablePtr(new LocalRenderVariable<glm::ivec3>(desc));
			case RenderVariableDescription::TYPE_INT_VEC4:
				return RenderVariablePtr(new LocalRenderVariable<glm::ivec4>(desc));
			case RenderVariableDescription::TYPE_USIGNED_INT:
				return RenderVariablePtr(new LocalRenderVariable<uint32_t>(desc));
			case RenderVariableDescription::TYPE_USIGNED_INT_VEC2:
				return RenderVariablePtr(new LocalRenderVariable<glm::uvec2>(desc));
			case RenderVariableDescription::TYPE_USIGNED_INT_VEC3:
				return RenderVariablePtr(new LocalRenderVariable<glm::uvec3>(desc));
			case RenderVariableDescription::TYPE_USIGNED_INT_VEC4:
				return RenderVariablePtr(new LocalRenderVariable<glm::uvec4>(desc));
			case RenderVariableDescription::TYPE_BOOL:
				return RenderVariablePtr(new LocalRenderVariable<bool>(desc));
			case RenderVariableDescription::TYPE_BOOL_VEC2:
				return RenderVariablePtr(new LocalRenderVariable<glm::bvec2>(desc));
			case RenderVariableDescription::TYPE_BOOL_VEC3:
				return RenderVariablePtr(new LocalRenderVariable<glm::bvec3>(desc));
			case RenderVariableDescription::TYPE_BOOL_VEC4:
				return RenderVariablePtr(new LocalRenderVariable<glm::bvec4>(desc));
			case RenderVariableDescription::TYPE_SAMPLER_1D:
			case RenderVariableDescription::TYPE_SAMPLER_2D:
			case RenderVariableDescription::TYPE_SAMPLER_3D:
			case RenderVariableDescription::TYPE_SAMPLER_CUBE:
				return RenderVariablePtr(new LocalRenderVariable<int32_t>(desc));
			case RenderVariableDescription::TYPE_FLOAT_MAT2x3:
				return RenderVariablePtr(new LocalRenderVariable<glm::mat2x3>(desc));
			case RenderVariableDescription::TYPE_FLOAT_MAT2x4:
				return RenderVariablePtr(new LocalRenderVariable<glm::mat2x4>(desc));
			case RenderVariableDescription::TYPE_FLOAT_MAT3x2:
				return RenderVariablePtr(new LocalRenderVariable<glm::mat3x2>(desc));
			case RenderVariableDescription::TYPE_FLOAT_MAT3x4:
				return RenderVariablePtr(new LocalRenderVariable<glm::mat3x4>(desc));
			case RenderVariableDescription::TYPE_FLOAT_MAT4x2:
				return RenderVariablePtr(new LocalRenderVariable<glm::mat4x2>(desc));
			case RenderVariableDescription::TYPE_FLOAT_MAT4x3:
				return RenderVariablePtr(new LocalRenderVariable<glm::mat4x3>(desc));
		}
	}

	template<typename T>
	LocalRenderVariable<T>::LocalRenderVariable(const RenderVariableDescription& desc)
		: RenderVariable(desc)
	{
	}

	template<typename T>
	LocalRenderVariable<T>::~LocalRenderVariable()
	{
	}

	template<typename T>
	RenderVariablePtr LocalRenderVariable<T>::clone()
	{
		// XXX
		if(desc().is_shareable()) {
			return RenderVariablePtr(this);
		}
	}

	template<typename T>
	void LocalRenderVariable<T>::update(const std::vector<T>& values)
	{
		value_.clear();
		std::copy(values.begin(), values.end(), std::back_inserter(value_));
	}

	template<typename T>
	void LocalRenderVariable<T>::update(std::vector<T>* values)
	{
		value_.swap(*values);
	}
}