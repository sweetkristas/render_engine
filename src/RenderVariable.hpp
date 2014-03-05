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
		virtual ~RenderVariableDesc() {}

		void SetDisplayData(const Graphics::DisplayDeviceDataPtr& dd) {
			display_data_ = dd;
		}
		const Graphics::DisplayDeviceDataPtr& GetDisplayData() const { return display_data_; }
	private:
		Graphics::DisplayDeviceDataPtr display_data_;
	};

	typedef std::shared_ptr<RenderVariableDesc> RenderVariableDescPtr;

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

	class UniformRenderVariableDesc : public RenderVariableDesc
	{
	public:
		enum UniformType {
			UNKOWN,
			COLOR,
			POINT_SIZE,
			MODEL,
		};
		enum UniformVariableType {
			IS_FLOAT		= 0x40000000,
			FLOAT			= 0x40000001,
			FLOAT_VEC2		= 0x40000002,
			FLOAT_VEC3		= 0x40000003,
			FLOAT_VEC4		= 0x40000004,
			INT				= 0x00000001,
			INT_VEC2		= 0x00000002,
			INT_VEC3		= 0x00000003,
			INT_VEC4		= 0x00000004,
			BOOL			= 0x00100001,
			BOOL_VEC2		= 0x00100002,
			BOOL_VEC3		= 0x00100003,
			BOOL_VEC4		= 0x00100004,
			FLOAT_MAT2		= 0x40100002,
			FLOAT_MAT3		= 0x40100003,
			FLOAT_MAT4		= 0x40100004,
			SAMPLER_2D		= 0x00200001, 
			SAMPLER_CUBE	= 0x00200002,
		};

		UniformRenderVariableDesc(UniformType uniform_type,
			UniformVariableType uniform_var_type);
		UniformRenderVariableDesc(const std::string& uniform_name,
			UniformVariableType uniform_var_type);
		virtual ~UniformRenderVariableDesc() override {};

		const std::string& GetUniformTypeAsString() const { return uniform_name_; }
		UniformVariableType GetUniformVariableType() const { return uniform_var_type_; }
	private:
		std::string uniform_name_;
		UniformType uniform_type_;
		UniformVariableType uniform_var_type_;
	};

	typedef std::vector<RenderVariableDescPtr> RenderVariableDescList;

	class RenderVariable
	{
	public:
		virtual ~RenderVariable() {}

		void SetCount(size_t cnt) { count_ = cnt; }
		size_t Count() const { return count_; }

		RenderVariableDescList& VariableDescritionList() { return desc_list_; }

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

		void SetIndexedDraw(bool indexed_draw) { indexed_draw_ = indexed_draw; }
		bool IsIndexedDraw() const { return indexed_draw_; }

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
	protected:
		void AddVariableDesc(const RenderVariableDescPtr& desc);
	private:
		// Estimated count of number of items.
		size_t count_;
		// Whether the data is reloaded often or is static.
		bool dynamic_;
		// Whether the data is shareable or a deep copy need be made.
		bool shareable_;
		// Whether the data is geometry related. e.g. Vertex positions.
		bool geometry_related_;

		RenderVariableDescList desc_list_;

		bool indexed_draw_;
		DrawMode draw_mode_;

		RenderVariable();
		RenderVariable(const RenderVariable&);
	};

	template<class T>
	class AttributeRenderVariable : public RenderVariable
	{
	public:
		AttributeRenderVariable(size_t count = 0,
			bool dynamic = true, 
			bool shareable = true, 
			bool geometry_related = true) 
			: RenderVariable(count, dynamic, shareable, geometry_related) {}
		virtual ~AttributeRenderVariable() {}

		void AddVariableDescription(AttributeRenderVariableDesc::VertexType vertex_type, 
			unsigned num_elements, 
			AttributeRenderVariableDesc::VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset) {
			AddVariableDesc(RenderVariableDescPtr(new AttributeRenderVariableDesc(vertex_type, num_elements, var_type, normalised, stride, offset)));
		}
		void AddVariableDescription(const std::string& vertex_type, 
			unsigned num_elements, 
			AttributeRenderVariableDesc::VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset) {
			AddVariableDesc(RenderVariableDescPtr(new AttributeRenderVariableDesc(vertex_type, num_elements, var_type, normalised, stride, offset)));
		}

		void Update(const std::vector<T>& value) {
			value_ = value;
			SetCount(value.size());
		}
		void* Value() override { return &value_[0]; }
	private:
		std::vector<T> value_;
		AttributeRenderVariable(const AttributeRenderVariable&);
	};

	template<class T>
	class UniformRenderVariable : public RenderVariable
	{
	public:
		UniformRenderVariable() : RenderVariable(0, false, true, false) {}
		virtual ~UniformRenderVariable() {}

		void AddVariableDescription(const std::string& uniform_type, 
			UniformRenderVariableDesc::UniformVariableType uniform_var_type) {
			AddVariableDesc(RenderVariableDescPtr(new UniformRenderVariableDesc(uniform_type, uniform_var_type)));
		}
		void AddVariableDescription(UniformRenderVariableDesc::UniformType uniform_type,
			UniformRenderVariableDesc::UniformVariableType uniform_var_type) {
			AddVariableDesc(RenderVariableDescPtr(new UniformRenderVariableDesc(uniform_type, uniform_var_type)));
		}

		void Update(const T& value) {
			value_ = value;
		}
		void* Value() override { return &value_; }
	private:
		T value_;
		UniformRenderVariable(const UniformRenderVariable&);
	};
}
