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

namespace RenderMk2
{
	class DrawBuffer
	{
	public:
        DrawBuffer() {}
		virtual ~DrawBuffer() {}

        // Update whole lot of data at once.
        void Update(const void *data, size_t size) {
            HandleUpdate(data, size);
        }
        // Update a range, starting at offset for size bytes.
        void Update(const void *data, size_t size, ptrdiff_t offset) {
            HandleUpdate(data, size, offset);
        }
        
		void SetDisplayData(const Graphics::DisplayDeviceDataPtr& dd) {
			display_data_ = dd;
		}
		const Graphics::DisplayDeviceDataPtr& GetDisplayData() const { return display_data_; }
	private:
        DISALLOW_COPY_AND_ASSIGN(DrawBuffer);

        // Update whole lot of data at once.
        virtual void HandleUpdate(const void *data, size_t size) = 0;
        // Update a range, starting at offset for size bytes.
        virtual void HandleUpdate(const void *data, size_t size, ptrdiff_t offset) = 0;
        
		Graphics::DisplayDeviceDataPtr display_data_;
	};
    
    class IndexBuffer : public DrawBuffer
    {
    public:
    private:
        DISALLOW_COPY_ASSIGN_AND_DEFAULT(IndexBuffer);
    };
    
    class UniformValue
    {
    public:
		enum class UniformType {
			UNKOWN,
			COLOR,
			POINT_SIZE,
			MODELVIEW_MATRIX,
            PROJECTION_MATRIX,
            MVP_MATRIX,
		};
		UniformValue(UniformType uniform_type);
		UniformValue(const std::string& uniform_name);
		virtual ~UniformValue();
        
        const void* GetValue() const {
            return Value(); 
        }
    private:
        const void* Value() const = 0;
        
		std::string uniform_name_;
		UniformType uniform_type_;       
    };
    typedef std::shared_ptr<UniformValue> UniformValuePtr;
   
    template<typename T>
    class TypedUniformValue
    {
    public:
        TypedUniformValue(UniformType uniform_type) : UniformValue(uniform_type) {}
        TypedUniformValue(const std::string& uniform_name) : UniformValue(uniform_name) {}
        virtual ~TypedUniformValue() {}
        void Update(const T& value) {
            value_ = value;
        }        
		const std::string& GetUniformTypeAsString() const { return uniform_name_; }
    private:
        DISALLOW_COPY_ASSIGN_AND_DEFAULT(UniformValue);
        
        const void* Value() {
            return value_;
        }
        T value_;
    };
    
    class UniformBuffer : public DrawBuffer
    {
    public:
        enum class UniformValueType {
			FLOAT,
			FLOAT_VEC2,
			FLOAT_VEC3,
			FLOAT_VEC4,
			INT,
			INT_VEC2,
			INT_VEC3,
			INT_VEC4,
			BOOL,
			BOOL_VEC2,
			BOOL_VEC3,
			BOOL_VEC4,
			FLOAT_MAT2,
			FLOAT_MAT3,
			FLOAT_MAT4,
			SAMPLER_2D, 
			SAMPLER_CUBE,
        };
        UniformBuffer(const std::string& buffer_name);
        virtual ~UniformBuffer();
        void AddUniformDesc(const std::string& name, UniformValueType type, unsigned num_elements);
        // If this is a software buffer then we need to manually supply the description.
        // If none is supplied and we try updating the buffer an error will be thrown.
        bool RequiresDescription() const;
        UniformValuePtr GetUniformFromBuffer
    private:
        DISALLOW_COPY_ASSIGN_AND_DEFAULT(UniformBuffer);

        // Update whole lot of data at once.
        virtual void HandleUpdate(const void *data, size_t size) = 0;
        // Update a range, starting at offset for size bytes.
        virtual void HandleUpdate(const void *data, size_t size, ptrdiff_t offset) = 0;
    };
    
    /*
        // in GLSL
        uniform mat4 mvp_matrix;
        
        // in C++
        FloatUniform<glm::mat4> mvp = std::make_shared("mvp_matrix");
        mvp->Update(glm::mat4(1.0f));
        AddUniform(mvp);
        
        // in GLSL
        layout (std140) uniform some_name { mat4 mvp_matrix; vec4 color; }

        // in C++
        UniformBufferPtr ub = DisplayDevice::GetUniformBuffer("some_name");
        auto mvp = ub->GetUniformFromBuffer("mvp_matrix");
        mvp->Update(glm::value_ptr(glm::mat4(1.0f)));
        AddUniformBuffer(ub);
        
        or.
        struct s_uniforms {
            glm::mat4 mvp;
            glm::vec4 color;
        } uniforms;
        uniforms.mvp = glm::mat4(1.0f);
        uniforms.color = glm::vec3(1.0f,0.0f,0.0f,1.0f);
        ub->Update(uniforms, sizeof(uniforms));
    */
    
	class AttributeBuffer : public DrawBuffer
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

		AttributeBuffer(VertexType vertex_type, 
			unsigned num_elements, 
			VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset);
		AttributeBuffer(const std::string& vertex_name, 
			unsigned num_elements, 
			VariableType var_type, 
			bool normalised, 
			unsigned stride, 
			unsigned offset);
		virtual ~AttributeBuffer() override {};

		VertexType GetVertexType() const { return vertex_type_; }
		const std::string& GetVertexTypeAsString() const { return vertex_name_; }
		VariableType GetVariableType() const { return var_type_; }
		unsigned NumElements() const { return num_elements_; }
		unsigned Stride() const { return stride_; }
		unsigned Offset() const { return offset_; }
		bool Normalised() const { return normalised_; }
	private:
        DISALLOW_COPY_ASSIGN_AND_DEFAULT(AttributeBuffer);
        
		VertexType vertex_type_;
		std::string vertex_name_;
		unsigned num_elements_;
		VariableType var_type_;
		unsigned stride_; 
		unsigned offset_;
		bool normalised_;
	};
}
