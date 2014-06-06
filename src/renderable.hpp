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

#include <glm/gtx/quaternion.hpp>

#include "attribute_set.hpp"
#include "Material.hpp"
#include "renderQueue.hpp"
#include "scene_fwd.hpp"
#include "util.hpp"

namespace KRE
{
	class renderable
	{
	public:
		renderable();
		renderable(size_t order);
		virtual ~renderable();

		void set_position(const glm::vec3& position);
		void set_position(float x, float y, float z=0.0f);
		void set_position(int x, int y, int z=0);
		const glm::vec3& position() const { return position_; }

		void set_rotation(float angle, const glm::vec3& axis);
		void set_rotation(const glm::quat& rot);
		const glm::quat& rotation() const { return rotation_; }

		void set_scale(float xs, float ys, float zs=1.0f);
		void set_scale(const glm::vec3& scale);
		const glm::vec3& scale() const { return scale_; }

		glm::mat4 model_matrix() const;

		void set_color(float r, float g, float b, float a=1.0);
		void set_color(int r, int g, int b, int a=255);
		void set_color(const color& color);
		const color& get_color() const { return color_; }
		bool is_color_set() const { return color_set_; }

		size_t order() const { return order_; }
		void set_order(size_t o) { order_ = o; }

		const camera_ptr& camera() const { return camera_; }
		void set_camera(const camera_ptr& camera);

		const LightPtrList& lights() const { return lights_; }
		void set_lights(const LightPtrList& lights);

		const MaterialPtr& material() const { return material_; }
		void set_material(const MaterialPtr& material);

		const render_target_ptr& get_render_target() const { return render_target_; }
		void set_render_target(const render_target_ptr& rt);

		void set_display_data(const display_device_ptr& dd, const display_device_def& def);
		const display_device_data_ptr& get_display_data() const { return display_data_; }

		void add_attribute_set(const attribute_set_ptr& attrset);
		//void add_uniform_set(const UniformSetPtr& uniset);
		const std::vector<attribute_set_ptr>& get_attribute_set() const { return attributes_; }
		//const std::vector<UniformSetPtr>& get_uniform_set() const { return uniforms_; }

		virtual void pre_render() {}

		// Called just before rendering this item, after shaders and other variables
		// have been set-up
		virtual void render_begin() {}
		// Called after draw commands have been sent before anything is torn down.
		virtual void render_end() {}
	private:
		DISALLOW_COPY_AND_ASSIGN(renderable);

		size_t order_;
		glm::vec3 position_;
		glm::quat rotation_;
		glm::vec3 scale_;
		camera_ptr camera_;
		LightPtrList lights_;
		MaterialPtr material_;
		render_target_ptr render_target_;
		color color_;
		bool color_set_;

		std::vector<attribute_set_ptr> attributes_;
		//std::vector<UniformSetPtr> uniforms_;

		display_device_data_ptr display_data_;
	};
}
