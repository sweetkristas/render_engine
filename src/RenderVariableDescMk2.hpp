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
	class RenderVariableDesc
	{
	public:
		virtual ~RenderVariableDesc() {}

		void SetDisplayData(const Graphics::DisplayDeviceDataPtr& dd) {
			display_data_ = dd;
		}
		const Graphics::DisplayDeviceDataPtr& GetDisplayData() const { return display_data_; }
	private:
		Graphics::DisplayDeviceDataPtr display_data_;
	};
    
	class AttributeRenderVariableDesc : public RenderVariableDesc
	{
	public:
		enum VertexType {
			UNKNOWN,
			POSITION,
			COLOR,
			TEXTURE,
			NORMAL,
		};
		enum VariableType {
			BOOL,
			HALF_FLOAT,
			FLOAT,
			DOUBLE,
			FIXED,
			SHORT,
			UNSIGNED_SHORT,
			BYTE,
			UNSIGNED_BYTE,
			INT,
			UNSIGNED_INT,
			INT_2_10_10_10,
			UNSIGNED_INT_2_10_10_10,
			UNSIGNED_INT_10F_11F_11F,
		};

		AttributeRenderVariableDesc(VertexType vertex_type, 
			unsigned num_elements, 
			VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset);
		AttributeRenderVariableDesc(const std::string& vertex_name, 
			unsigned num_elements, 
			VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset);
		virtual ~AttributeRenderVariableDesc() override {};

		VertexType GetVertexType() const { return vertex_type_; }
		const std::string& GetVertexTypeAsString() const { return vertex_name_; }
		VariableType GetVariableType() const { return var_type_; }
		unsigned NumElements() const { return num_elements_; }
		unsigned Stride() const { return stride_; }
		unsigned Offset() const { return offset_; }
		bool Normalised() const { return normalised_; }
	private:
		VertexType vertex_type_;
		std::string vertex_name_;
		unsigned num_elements_;
		VariableType var_type_;
		unsigned stride_; 
		unsigned offset_;
		bool normalised_;
	};
}
