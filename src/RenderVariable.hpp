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

#include <memory>
#include <typeinfo>
#include <vector>

namespace Render
{
	class RenderVariable;
	typedef std::shared_ptr<RenderVariable> RenderVariablePtr;
	typedef std::vector<RenderVariablePtr> RenderVariableList;

	class RenderVariableDescription
	{
	public:
		enum VariableUsage {
			USAGE_VERTEX,
			USAGE_PRIMITIVE,
			USAGE_PRIMITIVE_GROUP,
			USAGE_GEOMETRY,
			USAGE_GLOBAL,
		};
		RenderVariableDescription(const std::string& name, 
			const std::string& type,
			VariableUsage use, 
			bool dynamic = true, 
			bool shareable = true, 
			bool geometry_related = true);
	private:
		std::string name_;
		VariableUsage usage_;
		bool dynamic_;
		bool shareable_;
		bool geometry_related_;
		std::string type_;
		RenderVariableDescription();
		RenderVariableDescription(const RenderVariableDescription&);
	};

	class RenderVariable
	{
	public:
		RenderVariable();
		virtual ~RenderVariable();

		static RenderVariablePtr factory(const RenderVariableDescription& desc);
		virtual RenderVariablePtr clone() = 0;
	private:
		RenderVariable(const RenderVariable&);
	};

	class RenderVariableVertex
	{
	public:
		RenderVariableVertex();
	};
}
