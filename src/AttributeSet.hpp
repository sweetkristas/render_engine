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
#include <cstring>
#include <functional>
#include <memory>
#include <vector>
#include "asserts.hpp"
#include "DisplayDeviceFwd.hpp"
#include "Util.hpp"

namespace KRE
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
		explicit AttributeDesc(Type type, 
			unsigned num_elements,
			VariableType var_type,
			bool normalise=false,
			ptrdiff_t stride=0,
			ptrdiff_t offset=0,
			size_t divisor=1);
		explicit AttributeDesc(const std::string& type_name, 
			unsigned num_elements,
			VariableType var_type,
			bool normalise=false,
			ptrdiff_t stride=0,
			ptrdiff_t offset=0,
			size_t divisor=1);
		Type AttrType() const { return type_; }
		const std::string& AttrName() const { return type_name_; }
		VariableType VarType() const { return var_type_; }
		unsigned NumElements() const { return num_elements_; }
		bool Normalise() const { return normalise_; }
		ptrdiff_t Stride() const { return stride_; }
		ptrdiff_t Offset() const { return offset_; }
		size_t Divisor() const { return divisor_; }
		void SetDisplayData(const DisplayDeviceDataPtr& ddp) { display_data_ = ddp; }
		const DisplayDeviceDataPtr& GetDisplayData() const { return display_data_; }
	private:
		Type type_;
		std::string type_name_;
		VariableType var_type_;
		unsigned num_elements_;
		bool normalise_;
		ptrdiff_t stride_;
		ptrdiff_t offset_;
		size_t divisor_;
		DisplayDeviceDataPtr display_data_;
	};

	// This is ugly since the Update() method requires a void*
	// I've tried several things to try and make this nice,
	// but not found anything that really fits nicely.
	class Attribute
	{
	public:
		enum class AccessFreqHint {
			//! Data store modified once and used in-frequently
			STREAM,
			//! Data store modified once and used many times
			STATIC,
			//! Data store modified repeatedly and used many times.
			DYNAMIC,
		};
		enum class AccessTypeHint {
			//! Modified by application, used by display device for drawing.
			DRAW,
			//! Modified by display device, returned to application.
			READ,
			//! Data is modified by display device and used by display device for copying.
			COPY,
		};
		Attribute(AccessFreqHint freq, AccessTypeHint type) 
			: access_freq_(freq),
			access_type_(type),
			offs_(0) {
		}
		virtual ~Attribute() {}
		
		void AddAttributeDescription(const AttributeDesc& attrdesc) {
			desc_.emplace_back(attrdesc);
		}
		std::vector<AttributeDesc>& GetAttrDesc() { return desc_; }
		virtual void Update(const void* value, ptrdiff_t offset, size_t size) {
			if(mem_.empty()) {
				ASSERT_LOG(offset == 0, "Must have 0 offset upon initialisation");
				mem_.resize(size);
			}
			ASSERT_LOG(offset + size <= Size(), 
				"When buffering data offset+size exceeds data store size: " 
				<< offset+size 
				<< " > " 
				<< Size());
			std::memcpy(&mem_[offset], value, size);
		}
		virtual intptr_t Value() const { return reinterpret_cast<intptr_t>(&mem_[0]); }
		virtual size_t Size() const { return mem_.size(); }
		void SetOffset(ptrdiff_t offs) {
			offs_ = offs;
		}
		ptrdiff_t GetOffset() const { return offs_; } 
		virtual void Bind() {}
		virtual void Unbind() {}
		AccessFreqHint AccessFrequency() const { return access_freq_; }
		AccessTypeHint AccessType() const { return access_type_; }
	private:
		DISALLOW_COPY_ASSIGN_AND_DEFAULT(Attribute);
		AccessFreqHint access_freq_;
		AccessTypeHint access_type_;
		std::vector<AttributeDesc> desc_;
		ptrdiff_t offs_;
		std::vector<uint8_t> mem_;
	};
	typedef std::shared_ptr<Attribute> AttributePtr;

	struct AttributeBinder
	{
		AttributeBinder(AttributePtr ap) : ap_(ap)  {
			ap->Bind();
		}
		~AttributeBinder() {
			ap_->Unbind();
		}
		AttributePtr ap_;
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
		enum class IndexType {
			INDEX_NONE,
			INDEX_UCHAR,
			INDEX_USHORT,
			INDEX_ULONG,
		};
		explicit AttributeSet(bool indexed, bool instanced);
		virtual ~AttributeSet();

		virtual AttributePtr CreateAttribute(Attribute::AccessFreqHint freq=Attribute::AccessFreqHint::DYNAMIC, 
			Attribute::AccessTypeHint type=Attribute::AccessTypeHint::DRAW);
		
		void SetDrawMode(DrawMode dm);
		DrawMode GetDrawMode() { return draw_mode_; }

		bool IsIndexed() const { return indexed_draw_; }
		bool IsInstanced() const { return instanced_draw_; }
		IndexType GetIndexType() const { return index_type_; }
		virtual const void* GetIndexArray() const { 
			switch(index_type_) {
			case IndexType::INDEX_NONE:		break;
			case IndexType::INDEX_UCHAR:	return &index8_[0];
			case IndexType::INDEX_USHORT:	return &index16_[0];
			case IndexType::INDEX_ULONG:	return &index32_[0];
			}
			ASSERT_LOG(false, "Index type not set to valid value.");
		};
		void SetCount(size_t count) { count_= count; }
		size_t GetCount() const { return count_; }
		void SetInstanceCount(size_t instance_count) { instance_count_ = instance_count; }
		size_t GetInstanceCount() const { return instance_count_; }

		void UpdateIndicies(const std::vector<uint8_t>& value);
		void UpdateIndicies(const std::vector<uint16_t>& value);
		void UpdateIndicies(const std::vector<uint32_t>& value);
		void UpdateIndicies(std::vector<uint8_t>* value);
		void UpdateIndicies(std::vector<uint16_t>* value);
		void UpdateIndicies(std::vector<uint32_t>* value);

		virtual void BindIndex() {};
		virtual void UnbindIndex() {};

		void SetOffset(ptrdiff_t offset) { offset_ = offset; }
		ptrdiff_t GetOffset() const { return offset_; }

		std::vector<AttributePtr>& GetAttributes() { return attributes_; }
	private:
		DISALLOW_COPY_ASSIGN_AND_DEFAULT(AttributeSet);
		DrawMode draw_mode_;
		bool indexed_draw_;
		bool instanced_draw_;
		IndexType index_type_;
		size_t instance_count_;
		std::vector<uint8_t> index8_;
		std::vector<uint16_t> index16_;
		std::vector<uint32_t> index32_;
		std::vector<AttributePtr> attributes_;
		size_t count_;
		ptrdiff_t offset_;
	};
	typedef std::shared_ptr<AttributeSet> AttributeSetPtr;
}
