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
#include <typeinfo>
#include <vector>

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "DisplayDeviceFwd.hpp"

namespace Render
{
	class RenderVariable;
	typedef std::shared_ptr<RenderVariable> RenderVariablePtr;
	typedef std::vector<RenderVariablePtr> RenderVariableList;

	class RenderVariableDesc
	{
	public:
		enum VertexType {
			VERTEX_UNKNOWN,
			VERTEX_POSITION,
			VERTEX_COLOR,
			VERTEX_TEXTURE,
			VERTEX_NORMAL,
		};
		enum VariableType {
			TYPE_BOOL,
			TYPE_HALF_FLOAT,
			TYPE_FLOAT,
			TYPE_DOUBLE,
			TYPE_FIXED,
			TYPE_SHORT,
			TYPE_UNSIGNED_SHORT,
			TYPE_BYTE,
			TYPE_UNSIGNED_BYTE,
			TYPE_INT,
			TYPE_UNSIGNED_INT,
			TYPE_INT_2_10_10_10,
			TYPE_UNSIGNED_INT_2_10_10_10,
			TYPE_UNSIGNED_INT_10F_11F_11F,
		};
		enum UniformType {
			UNIFORM_UNKOWN,
			UNIFORM_COLOR,
			UNIFORM_POINT_SIZE,
			UNIFORM_MODEL,
		};
		enum UniformVariableType {
			UNIFORM_TYPE_FLOAT,
			UNIFORM_TYPE_FLOAT_VEC2,
			UNIFORM_TYPE_FLOAT_VEC3,
			UNIFORM_TYPE_FLOAT_VEC4,
			UNIFORM_TYPE_INT,
			UNIFORM_TYPE_INT_VEC2,
			UNIFORM_TYPE_INT_VEC3,
			UNIFORM_TYPE_INT_VEC4,
			UNIFORM_TYPE_BOOL,
			UNIFORM_TYPE_BOOL_VEC2,
			UNIFORM_TYPE_BOOL_VEC3,
			UNIFORM_TYPE_BOOL_VEC4,
			UNIFORM_TYPE_FLOAT_MAT2,
			UNIFORM_TYPE_FLOAT_MAT3,
			UNIFORM_TYPE_FLOAT_MAT4,
			UNIFORM_TYPE_SAMPLER_2D, 
			UNIFORM_TYPE_SAMPLER_CUBE,
		};
		enum DescriptionType {
			DESC_ATTRIB,
			DESC_UNIFORM,
		};

		RenderVariableDesc(VertexType vertex_type, 
			unsigned num_elements, 
			VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset);
		RenderVariableDesc(const std::string& vertex_name, 
			unsigned num_elements, 
			VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset);
		RenderVariableDesc(UniformType uniform_type,
			unsigned num_elements,
			UniformVariableType uniform_var_type);
		RenderVariableDesc(const std::string& uniform_name,
			unsigned num_elements,
			UniformVariableType uniform_var_type);

		~RenderVariableDesc();

		DescriptionType GetDescription() const { return desc_type_; }

		VertexType GetVertexType() const { return vertex_type_; }
		const std::string& GetVertexTypeAsString() const { return vertex_name_; }
		VariableType GetVariableType() const { return var_type_; }
		unsigned NumElements() const { return num_elements_; }
		unsigned Stride() const { return stride_; }
		unsigned Offset() const { return offset_; }
		bool Normalised() const { return normalised_; }

		void SetDisplayData(const Graphics::DisplayDeviceDataPtr& dd) {
			display_data_ = dd;
		}
		const Graphics::DisplayDeviceDataPtr& GetDisplayData() const { return display_data_; }
		const std::string& GetUniformTypeAsString() const { return uniform_name_; }
	private:
		DescriptionType desc_type_;
		VertexType vertex_type_;
		std::string vertex_name_;
		unsigned num_elements_;
		VariableType var_type_;
		unsigned stride_; 
		unsigned offset_;
		bool normalised_;

		std::string uniform_name_;
		UniformType uniform_type_;
		UniformVariableType uniform_var_type_;
		Graphics::DisplayDeviceDataPtr display_data_;
	};

	typedef std::vector<RenderVariableDesc> RenderVariableDescList;

	class RenderVariable
	{
	public:
		virtual ~RenderVariable() {}

		void AddVariableDescription(RenderVariableDesc::VertexType vertex_type, 
			unsigned num_elements, 
			RenderVariableDesc::VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset);
		void AddVariableDescription(const std::string& vertex_type, 
			unsigned num_elements, 
			RenderVariableDesc::VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset);
		void AddVariableDescription(const std::string& uniform_type, 
			unsigned num_elements,
			RenderVariableDesc::UniformVariableType uniform_var_type);
		void AddVariableDescription(RenderVariableDesc::UniformType uniform_type,
			unsigned num_elements,
			RenderVariableDesc::UniformVariableType uniform_var_type);
		
		void SetIndexedDraw(bool indexed_draw) { indexed_draw_ = indexed_draw; }
		bool IsIndexedDraw() const { return indexed_draw_; }

		void SetCount(size_t cnt) { count_ = cnt; }
		size_t Count() const { return count_; }

		enum DrawMode {
			POINTS,
			LINE_STRIP,
			LINE_LOOP,
			LINES,
			TRIANGLE_STRIP,
			TRIANGLE_FAN,
			TRIANGLES,
			QUAD_STRIP,
			QUADS,
			POLYGON,
		};
		void SetDrawMode(DrawMode draw_mode) { draw_mode_ = draw_mode; }
		DrawMode GetDrawMode() const { return draw_mode_; }

		RenderVariableDescList& VariableDescritionList() { return desc_list_; }

		virtual void* Value() = 0;
	protected:
		RenderVariable(size_t count,
			bool dynamic, 
			bool shareable, 
			bool geometry_related) 
			: count_(count),
			dynamic_(dynamic),
			shareable_(shareable),
			geometry_related_(geometry_related),
			draw_mode_(TRIANGLE_STRIP),
			indexed_draw_(false)
		{
		}
	private:
		//VertexType vertex_type_;
		//const std::string vertex_type_str_;
		//unsigned num_elements_;
		//VariableType var_type_;
		//unsigned stride_;
		//unsigned offset_;
		bool indexed_draw_;
		DrawMode draw_mode_;
		// Estimated count of number of items.
		size_t count_;
		// Whether the data is reloaded often or is static.
		bool dynamic_;
		// Whether the data is shareable or a deep copy need be made.
		bool shareable_;
		// Whether the data is geometry related. e.g. Vertex positions.
		bool geometry_related_;

		RenderVariableDescList desc_list_;

		RenderVariable();
		RenderVariable(const RenderVariable&);
	};

	template<typename T>
	class TypedRenderVariable : public RenderVariable
	{
	public:
		TypedRenderVariable<T>(size_t count = 0,
			bool dynamic = true, 
			bool shareable = true, 
			bool geometry_related = true) 
			: RenderVariable(count, dynamic, shareable, geometry_related) {
		}
		virtual ~TypedRenderVariable<T>() {}
		void Update(const std::vector<T>& value) {
			value_ = value;
		}
		void* Value() override { return &value_[0]; }
	private:
		std::vector<T> value_;
	};

	template<typename T>
	class UniformRenderVariable : public RenderVariable
	{
	public:
		UniformRenderVariable<T>() 
			: RenderVariable(0, false, true, false) {
		}
		virtual ~UniformRenderVariable<T>() {}
		void Update(const T& value) {
			value_ = value;
		}
		void* Value() override { return &value_; }
	private:
		T value_;
	};
}
