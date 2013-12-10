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

	template<typename T>
	RenderVariable<T>::RenderVariable(const RenderVariableDescription& desc)
		: desc_(desc)
	{
	}

	template<typename T>
	void RenderVariable<T>::update(size_t offset, const std::vector<T>& values, size_t count)
	{
		if(values_.size() < offset + count) {
			values_.resize(values_.size() + offset + count);
		}
		std::copy(values.begin(), values.end(), values_.begin() + offset);
	}

	template<typename T>
	void RenderVariable<T>::update(std::vector<T>* values)
	{
		values_.swap(*values);
	}
}