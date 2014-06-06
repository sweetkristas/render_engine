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

#include "attribute_set.hpp"
#include "display_device.hpp"

namespace KRE
{
	attribute_set::attribute_set(bool indexed, bool instanced)
		: draw_mode_(DrawMode::TRIANGLES),
		indexed_draw_(indexed),
		instanced_draw_(instanced),
		index_type_(IndexType::INDEX_NONE),
		instance_count_(0),
		offset_(0),
		count_(0)
	{
	}

	attribute_set::~attribute_set()
	{
	}

	void attribute_set::set_draw_mode(DrawMode dm)
	{
		draw_mode_ = dm;
	}

	void attribute_set::update_indicies(const std::vector<uint8_t>& value) 
	{
		index_type_ = IndexType::INDEX_UCHAR;
		index8_ = value;
		count_ = index8_.size();
		handle_index_update();
	}

	void attribute_set::update_indicies(const std::vector<uint16_t>& value) 
	{
		index_type_ = IndexType::INDEX_USHORT;
		index16_ = value;
		count_ = index16_.size();
		handle_index_update();
	}

	void attribute_set::update_indicies(const std::vector<uint32_t>& value) 
	{
		index_type_ = IndexType::INDEX_ULONG;
		index32_ = value;
		count_ = index32_.size();
		handle_index_update();
	}

	void attribute_set::update_indicies(std::vector<uint8_t>* value)
	{
		index_type_ = IndexType::INDEX_UCHAR;
		index8_.swap(*value);
		handle_index_update();
	}

	void attribute_set::update_indicies(std::vector<uint16_t>* value)
	{
		index_type_ = IndexType::INDEX_USHORT;
		index16_.swap(*value);
		handle_index_update();
	}

	void attribute_set::update_indicies(std::vector<uint32_t>* value)
	{
		index_type_ = IndexType::INDEX_ULONG;
		index32_.swap(*value);
		handle_index_update();
	}

	void attribute_set::add_attribute(const attribute_base_ptr& attrib) 
	{
		attributes_.emplace_back(attrib);
		auto hwbuffer = display_device::create_attribute_buffer(is_hardware_backed(), attrib.get());
		attrib->set_device_buffer_data(hwbuffer);
	}


	attribute_desc::attribute_desc(Type type, 
		unsigned num_elements,
		VariableType var_type,
		bool normalise,
		ptrdiff_t stride,
		ptrdiff_t offset,
		size_t divisor)
		: type_(type),
		num_elements_(num_elements),
		var_type_(var_type),
		normalise_(normalise),
		stride_(stride),
		offset_(offset),
		divisor_(divisor)
	{
		switch(type_) {
		case Type::POSITION:	type_name_ = "position"; break;
		case Type::COLOR:		type_name_ = "color"; break;
		case Type::TEXTURE:		type_name_ = "texcoord"; break;
		case Type::NORMAL:		type_name_ = "normal"; break;
		default:
			ASSERT_LOG(false, "Unknown type used.");
		}
	}

	attribute_desc::attribute_desc(const std::string& type_name, 
		unsigned num_elements,
		VariableType var_type,
		bool normalise,
		ptrdiff_t stride,
		ptrdiff_t offset,
		size_t divisor)
		: type_(Type::UNKOWN),
		type_name_(type_name),
		num_elements_(num_elements),
		var_type_(var_type),
		normalise_(normalise),
		stride_(stride),
		offset_(offset),
		divisor_(divisor)
	{
	}
}