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

#include <algorithm>
#include <iterator>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include "asserts.hpp"
#include "display_device_fwd.hpp"
#include "util.hpp"

namespace KRE
{
	class attribute_base;
	typedef std::shared_ptr<attribute_base> attribute_base_ptr;

	// abstract base class for Hardware-buffered attributes.
	class hardware_attribute
	{
	public:
		hardware_attribute(attribute_base* parent) : parent_(parent) {}
		virtual ~hardware_attribute() {}
		virtual void update(const void* value, ptrdiff_t offset, size_t size) = 0;
		virtual void bind() {}
		virtual void unbind() {}
		virtual intptr_t value() = 0;		
	private:
		attribute_base* parent_;
	};
	typedef std::shared_ptr<hardware_attribute> hardware_attribute_ptr;

	class hardware_attribute_impl : public hardware_attribute
	{
	public:
		hardware_attribute_impl(attribute_base* parent) : hardware_attribute(parent) {}
		virtual ~hardware_attribute_impl() {}
		void update(const void* value, ptrdiff_t offset, size_t size) {
			if(offset == 0) {
				value_ = reinterpret_cast<intptr_t>(value);
			}
		}
		void bind() {}
		void unbind() {}
		intptr_t value() override { return value_; }
	private:
		intptr_t value_;
	};

	class attribute_desc
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
		explicit attribute_desc(Type type, 
			unsigned num_elements,
			VariableType var_type,
			bool normalise=false,
			ptrdiff_t stride=0,
			ptrdiff_t offset=0,
			size_t divisor=1);
		explicit attribute_desc(const std::string& type_name, 
			unsigned num_elements,
			VariableType var_type,
			bool normalise=false,
			ptrdiff_t stride=0,
			ptrdiff_t offset=0,
			size_t divisor=1);
		Type attr_type() const { return type_; }
		const std::string& attr_name() const { return type_name_; }
		VariableType var_type() const { return var_type_; }
		unsigned num_elements() const { return num_elements_; }
		bool normalise() const { return normalise_; }
		ptrdiff_t stride() const { return stride_; }
		ptrdiff_t offset() const { return offset_; }
		size_t divisor() const { return divisor_; }
		void set_display_data(const display_device_data_ptr& ddp) { display_data_ = ddp; }
		const display_device_data_ptr& get_display_data() const { return display_data_; }
	private:
		Type type_;
		std::string type_name_;
		VariableType var_type_;
		unsigned num_elements_;
		bool normalise_;
		ptrdiff_t stride_;
		ptrdiff_t offset_;
		size_t divisor_;
		display_device_data_ptr display_data_;
	};

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

	class attribute_base
	{
	public:
		attribute_base(AccessFreqHint freq, AccessTypeHint type)
			: access_freq_(freq),
			access_type_(type),
			offs_(0) {
		}
		virtual ~attribute_base() {}
		void set_attr_desc(const attribute_desc& attrdesc) {
			desc_.emplace_back(attrdesc);
		}
		std::vector<attribute_desc>& get_attr_desc() { return desc_; }
		void set_offset(ptrdiff_t offs) {
			offs_ = offs;
		}
		ptrdiff_t get_offset() const { return offs_; } 
		AccessFreqHint access_frequency() const { return access_freq_; }
		AccessTypeHint access_type() const { return access_type_; }
		hardware_attribute_ptr get_device_buffer_data() { return hardware_; }
		void set_device_buffer_data(const hardware_attribute_ptr& hardware) { 
			hardware_ = hardware; 
			handle_attach_hardware_buffer();
		}
	private:
		virtual void handle_attach_hardware_buffer() = 0;
		AccessFreqHint access_freq_;
		AccessTypeHint access_type_;
		ptrdiff_t offs_;
		std::vector<attribute_desc> desc_;
		hardware_attribute_ptr hardware_;
		bool hardware_buffer_;
	};

	/* Templated attribute buffer. Is sub-optimal in that we double buffer attributes
		if there is a real hardware buffer attached. But mitigating that it is easy
		for us to generate a new hardware buffer from existing data in the case of
		a context tear down.
	*/
	template<typename T, 
		template<typename E, 
		         typename = std::allocator<E>> 
		class Container = std::vector>
	class attribute : public attribute_base
	{
	public:
		typedef typename Container<T>::reference reference;
		typedef typename Container<T>::const_reference const_reference;
		typedef typename Container<T>::iterator iterator;
		typedef typename Container<T>::const_iterator const_iterator;
		typedef typename Container<T>::size_type size_type;
		typedef T value_type;

		explicit attribute(AccessFreqHint freq, AccessTypeHint type=AccessTypeHint::DRAW) 
			:  attribute_base(freq, type) {
		}
		virtual ~attribute() {}
		
		void update(const Container<T>& values) {
			elements_ = values;
			if(get_device_buffer_data()) {
				get_device_buffer_data()->update(&elements_[0], 0, elements_.size() * sizeof(T));
			}
		}
		void update(const Container<T>& src, iterator& dst) {
			std::copy(src.begin(), src.end(), dst);
			if(get_device_buffer_data()) {
				get_device_buffer_data()->update(&elements_[0], 
					std::distance(elements_.begin(), dst), 
					std::distance(src.begin(), src.end()) * sizeof(T));
			}
		}
		void update(Container<T>* values) {
			elements_.swap(*values);
			if(get_device_buffer_data()) {
				get_device_buffer_data()->update(&elements_[0], 0, elements_.size() * sizeof(T));
			}
		}
		size_t size() const { 
			return elements_.size();
		}
		void bind() {
			ASSERT_LOG(get_device_buffer_data() != NULL, "bind call on null hardware attribute buffer.");
			get_device_buffer_data()->bind();
		}		
		void unbind() {
			ASSERT_LOG(get_device_buffer_data() != NULL, "bind call on null hardware attribute buffer.");
			get_device_buffer_data()->unbind();
		}
		const_iterator begin() const {
			return elements_.begin();
		}
		const_iterator end() const {
			return elements_.end();
		}
		const_iterator cbegin() const {
			return elements_.cbegin();
		}
		const_iterator cend() const {
			return elements_.cend();
		}
		iterator begin() {
			return elements_.begin();
		}
		iterator end() {
			return elements_.end();
		}
		void set_offset(const_iterator& it) {
			set_offset(std::distance(elements_.begin(), it));
		}
	private:
		attribute();
		attribute(const attribute&);
		void operator=(const attribute&);
		void handle_attach_hardware_buffer() override {
			// This just makes sure that if we add any elements
			// before an attach then they are all updated correctly.
			if(elements_.size() > 0) {
				get_device_buffer_data()->update(&elements_[0], 0, elements_.size() * sizeof(T));
			}
		}
		Container<T> elements_;
	};

	class attribute_set
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
		explicit attribute_set(bool indexed, bool instanced);
		virtual ~attribute_set();

		void set_draw_mode(DrawMode dm);
		DrawMode get_draw_mode() { return draw_mode_; }

		bool is_indexed() const { return indexed_draw_; }
		bool is_instanced() const { return instanced_draw_; }
		IndexType get_index_type() const { return index_type_; }
		virtual const void* get_index_array() const { 
			switch(index_type_) {
			case IndexType::INDEX_NONE:		break;
			case IndexType::INDEX_UCHAR:	return &index8_[0];
			case IndexType::INDEX_USHORT:	return &index16_[0];
			case IndexType::INDEX_ULONG:	return &index32_[0];
			}
			ASSERT_LOG(false, "Index type not set to valid value.");
		};
		size_t get_total_array_size() const {
			switch(index_type_) {
			case IndexType::INDEX_NONE:		break;
			case IndexType::INDEX_UCHAR:	return index8_.size() * sizeof(uint8_t);
			case IndexType::INDEX_USHORT:	return index16_.size() * sizeof(uint16_t);
			case IndexType::INDEX_ULONG:	return index32_.size() * sizeof(uint32_t);
			}
			ASSERT_LOG(false, "Index type not set to valid value.");
		}
		void set_count(size_t count) { count_= count; }
		size_t get_count() const { return count_; }
		void set_instance_count(size_t instance_count) { instance_count_ = instance_count; }
		size_t get_instance_count() const { return instance_count_; }

		void update_indicies(const std::vector<uint8_t>& value);
		void update_indicies(const std::vector<uint16_t>& value);
		void update_indicies(const std::vector<uint32_t>& value);
		void update_indicies(std::vector<uint8_t>* value);
		void update_indicies(std::vector<uint16_t>* value);
		void update_indicies(std::vector<uint32_t>* value);

		void add_attribute(const attribute_base_ptr& attrib);

		virtual void bind_index() {};
		virtual void unbind_index() {};

		void set_offset(ptrdiff_t offset) { offset_ = offset; }
		ptrdiff_t get_offset() const { return offset_; }

		virtual bool is_hardware_backed() const { return false; }

		std::vector<attribute_base_ptr>& get_attributes() { return attributes_; }
	protected:
		const void* get_index_data() const { 
			switch(index_type_) {
			case IndexType::INDEX_NONE:		break;
			case IndexType::INDEX_UCHAR:	return &index8_[0];
			case IndexType::INDEX_USHORT:	return &index16_[0];
			case IndexType::INDEX_ULONG:	return &index32_[0];
			}
			ASSERT_LOG(false, "Index type not set to valid value.");
		};
	private:
		//DISALLOW_COPY_ASSIGN_AND_DEFAULT(attribute_set);
		attribute_set();
		attribute_set(const attribute_set&);
		void operator=(const attribute_set&);

		virtual void handle_index_update() {}
		DrawMode draw_mode_;
		bool indexed_draw_;
		bool instanced_draw_;
		IndexType index_type_;
		size_t instance_count_;
		std::vector<uint8_t> index8_;
		std::vector<uint16_t> index16_;
		std::vector<uint32_t> index32_;
		std::vector<attribute_base_ptr> attributes_;
		size_t count_;
		ptrdiff_t offset_;
	};
	typedef std::shared_ptr<attribute_set> attribute_set_ptr;
}
