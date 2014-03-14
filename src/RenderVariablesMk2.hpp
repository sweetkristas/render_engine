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

#include <functional>
#include <memory>

namespace RenderMk2
{
    class RenderVariable;
    typedef std::shared_ptr<RenderVariable> RenderVariablePtr;

	template<class T>
	struct RenderVariableRegistrar
	{
        enum class Type {
            UNIFORM,
            ATTRIBUTE,
            INDEX,            
        };
		RenderVariableRegistrar(const Type& type, bool hardware)
		{
			// register the class factory function 
			DisplayDevice::RegisterFactoryFunction(type, hardware, []() -> RenderVariablePtr { return RenderVariablePtr(new T());});
		}
	};
    
    class RenderVariable
    {
        public:
            RenderVariable();
            virtual ~RenderVariable();
            static RenderVariablePtr Factory(RenderVariableRegistrar::Type type, bool hardware_backed);
            static RegisterFactoryFunction(RenderVariableRegistrar::Type, bool hardware, std:function<RenderVariablePtr()> fn);
        private:
            RenderVariable(const RenderVariable&);
    };
    
}
