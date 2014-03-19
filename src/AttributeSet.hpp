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

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include "Util.hpp"

namespace Render
{
	class AttributeDesc
	{
	public:
		enum class Type {
			UNKOWN,
			POSITION,
			COLOR, 
			TEXTURE,
			NORMAL,
		};
		enum class VariableType {
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
			INT_2_10_10_10_REV,
			UNSIGNED_INT_2_10_10_10_REV,
			UNSIGNED_INT_10F_11F_11F_REV,
		};
		explicit AttributeDesc(Type type, VariableType var_type);
	private:
		Type type_;
		VariableType var_type_;
	};

	template<typename T>
	class Attribute
	{
	public:
		explicit Attribute(AttributeDesc::Type type, 
			AttributeDesc::VariableType var_type, 
			unsigned num_elements, 
			bool normalise, 
			ptrdiff_t stride, 
			ptrdiff_t offset, 
			size_t divisor)
			: desc_(type, var_type),
			num_elements_(num_elements),
			normalise_(normalise),
			stride_(stride),
			offset(offset_),
			divisor_(divisor) {}
		virtual ~Attribute() {}
		
		virtual void Update(std::vector<T>* value) = 0;
		virtual void Update(std::vector<T>&& value)  = 0;
		void SetOffset(ptrdiff_t offs);
	private:
		DISALLOW_COPY_ASSIGN_AND_DEFAULT(Attribute);
		AttributeDesc desc_;
		unsigned num_elements_;
		bool normalise_;
		ptrdiff_t stride_;
		ptrdiff_t offset_;
		size_t divisor_;
	};
	template<typename T>
	typedef std::shared_ptr<Attribute<T>> AttributePtr<T>;

	template<typename T>
	class AttributeImpl
	{
	public:
		explicit AttributeImpl(AttributeDesc::Type type, 
			AttributeDesc::VariableType var_type, 
			unsigned num_elements, 
			bool normalise, 
			ptrdiff_t stride, 
			ptrdiff_t offset, 
			size_t divisor)
			: Attribute<T>(type, var_type, num_elements, normalise, stride, offset, divisor) {}
		virtual ~AttributeImpl() {}

		virtual void Update(std::vector<T>* value) override {
			value_.swap(*value);
		}
		virtual void Update(std::vector<T>&& value) override {
			value_ = value;
		}
	private:
		DISALLOW_COPY_ASSIGN_AND_DEFAULT(AttributeImpl);
		std::vector<T> value_;
	};


	class AttributeSet
	{
	public:
		enum class DrawMode {
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
	
		explicit AttributeSet(bool hardware_hint=false, bool indexed=false, bool instanced=false);
		virtual ~AttributeSet();
		template<typename T>
		AttributePtr<T> AddAttributeDescription<T>(AttributeDesc::Type type, AttributeDesc::VariableType var_type, unsigned num_elements, bool normalise, ptrdiff_t stride, ptrdiff_t offset, size_t divisor=1);
		void SetDrawMode(DrawMode dm);
		DrawMode GetDrawMode() { return draw_mode_; }
		virtual void UpdateIndicies(const std::vector<uint8_t>&) = 0;
		virtual void UpdateIndicies(const std::vector<uint16_t>&) = 0;
		virtual void UpdateIndicies(const std::vector<uint32_t>&) = 0;
	private:
		DISALLOW_COPY_ASSIGN_AND_DEFAULT(AttributeSet);
		DrawMode draw_mode_;
	};
}
