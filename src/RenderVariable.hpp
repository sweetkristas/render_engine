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

namespace Render
{
	class RenderVariable;
	typedef std::shared_ptr<RenderVariable> RenderVariablePtr;
	typedef std::vector<RenderVariablePtr> RenderVariableList;

	class RenderVariableDescription
	{
	public:
		enum VariableUsage {
			// Per vertex data, e.g. normals, vertex position, color
			USAGE_VERTEX,
			// Per primitive data
			USAGE_PRIMITIVE,
			// Per primitive group data, e.g. material
			USAGE_PRIMITIVE_GROUP,
			// Per geometry data, e.g. cameras, lighting
			USAGE_GEOMETRY,
			// Global objects, e.g. matrices
			USAGE_GLOBAL,
		};
		enum VariableType {
			TYPE_FLOAT,
			TYPE_FLOAT_VEC2,
			TYPE_FLOAT_VEC3,
			TYPE_FLOAT_VEC4,
			TYPE_FLOAT_MAT2,
			TYPE_FLOAT_MAT3,
			TYPE_FLOAT_MAT4,
			TYPE_INT,
			TYPE_INT_VEC2,
			TYPE_INT_VEC3,
			TYPE_INT_VEC4,
			TYPE_USIGNED_INT,
			TYPE_USIGNED_INT_VEC2,
			TYPE_USIGNED_INT_VEC3,
			TYPE_USIGNED_INT_VEC4,
			TYPE_BOOL,
			TYPE_BOOL_VEC2,
			TYPE_BOOL_VEC3,
			TYPE_BOOL_VEC4,
			TYPE_SAMPLER_1D,
			TYPE_SAMPLER_2D,
			TYPE_SAMPLER_3D,
			TYPE_SAMPLER_CUBE,
			TYPE_FLOAT_MAT2x3,
			TYPE_FLOAT_MAT2x4,
			TYPE_FLOAT_MAT3x2,
			TYPE_FLOAT_MAT3x4,
			TYPE_FLOAT_MAT4x2,
			TYPE_FLOAT_MAT4x3,
		};
		RenderVariableDescription(const std::string& name, 
			VariableType type,
			VariableUsage use, 
			size_t count = 0,
			bool dynamic = true, 
			bool shareable = true, 
			bool geometry_related = true);
		const std::string& name() const { return name_; }
		VariableUsage usage() const {return usage_; }
		bool is_dynamic() const { return dynamic_; }
		bool is_static() const { return !dynamic_; }
		bool is_shareable() const { return shareable_; }
		bool is_geometry_related() const { return geometry_related_; }
		VariableType type() const { return type_; }
	private:
		// Name of the variable.
		std::string name_;
		// usage pattern as defined above.
		VariableUsage usage_;
		// Estimated count of number of items.
		size_t count_;
		// Whether the data is reloaded often or is static.
		bool dynamic_;
		// Whether the data is shareable or a deep copy need be made.
		bool shareable_;
		// Whether the data is geometry related. e.g. Vertex positions.
		bool geometry_related_;
		// data type
		VariableType type_;
	};

	struct RenderVariableVisitor
	{
		virtual void Visit(const std::vector<glm::vec2>&) = 0;
		virtual void Visit(const glm::vec4&) = 0;
	};

	typedef std::shared_ptr<RenderVariableVisitor> RenderVariableVisitorPtr;

	template<typename T>
	class UpdateImpl : RenderVariableVisitor
	{
	public:
		UpdateImpl(T& data) : data_(data) {}
		void Visit(const std::vector<glm::vec2>& value) { data_.Update(value); }
		void Visit(const glm::vec4& value) { data_.Update(value); }
	private:
		T& data_;
	};

	class RenderVariable
	{
	public:
		virtual ~RenderVariable();
		RenderVariableVisitorPtr update;
	protected:
		RenderVariable(const RenderVariableDescription& desc, RenderVariableVisitorPtr visitor) 
			: desc_(desc), 
			update(visitor) {
		}
	private:
		RenderVariableDescription desc_;
		RenderVariable();
		RenderVariable(const RenderVariable&);
	};

	template<typename T>
	class TypedRenderVariable : public RenderVariable
	{
	public:
		TypedRenderVariable<T>(const RenderVariableDescription& desc) 
			: RenderVariable(desc, RenderVariableVisitorPtr(new UpdateImpl<TypedRenderVariable<T>>(*this))) {
		}
		virtual ~TypedRenderVariable<T>() {}
		void Update(const T& value) {
			value_ = value;
		}
	private:
		T value_;
	};
}
