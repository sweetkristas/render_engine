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

#include "frustum.hpp"
#include "scene_object.hpp"
#include "util.hpp"
#include "window_manager_fwd.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace KRE
{
	class camera : public scene_object
	{
	public:
		enum class CameraType { CAMERA_PERSPECTIVE, CAMERA_ORTHOGONAL };
		camera(const std::string& name, const window_manager_ptr& wnd);
		explicit camera(const std::string& name, int left, int right, int top, int bottom);
		explicit camera(const std::string& name, const window_manager_ptr& wnd, float fov, float aspect, float near_clip, float far_clip);
		virtual ~camera();

		void set_mouse_speed(float ms) { mouse_speed_ = ms; }
		void set_speed(float spd) { speed_ = spd; }
		void set_hangle(float ha) { horizontal_angle_ = ha; }
		void set_vangle(float va) { vertical_angle_ = va; }
		void set_fov(float fov);
		void set_aspect(float aspect);
		void set_clip_planes(float z_near, float z_far);
		void set_type(CameraType type);
		void set_ortho_window(int left, int right, int top, int bottom);
		float mousespeed() const { return mouse_speed_; }
		float speed() const { return speed_; }
		float hangle() const { return horizontal_angle_; }
		float vangle() const { return vertical_angle_; }
		float fov() const { return fov_; }
		float aspect() const { return aspect_; }
		float near_clip() const { return near_clip_; }
		float far_clip() const { return far_clip_; }
		CameraType type() const { return type_; }
		int ortho_left() const { return ortho_left_; }
		int ortho_right() const { return ortho_right_; }
		int ortho_top() const { return ortho_top_; }
		int ortho_bottom() const { return ortho_bottom_; }
		const glm::vec3& position() const { return position_; }
		const glm::vec3& right() const { return right_; }
		const glm::vec3& direction() const { return direction_; }
		const glm::vec3& target() const { return target_; }
		const glm::vec3& up() const { return up_; }
		void set_position(const glm::vec3& position) { position_ = position; }

		void look_at(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

		const float* projection() const { return glm::value_ptr(projection_); }
		const float* view() const { return glm::value_ptr(view_); }
		const glm::mat4& view_mat() const { return view_; }
		const glm::mat4& projection_mat() const { return projection_; }

		const frustum_ptr& get_frustum() const { return frustum_; }
		void attach_frustum(const frustum_ptr& frustum);

		glm::vec3 screen_to_world(int x, int y, int wx, int wy) const;
		glm::ivec3 get_facing(const glm::vec3& coords) const;

		//variant write();

		virtual DisplayDeviceDef Attach(const DisplayDevicePtr& dd) override;
	private:
		DISALLOW_COPY_AND_ASSIGN(camera);

		void compute_view();
		void compute_projection();

		// special case handling if LookAt function is called.
		// Since we then are specifying the position/target/up
		// vectors directly rather than being calculated.
		enum {
			VIEW_MODE_MANUAL,
			VIEW_MODE_AUTO,
		} view_mode_;

		CameraType type_;

		float fov_;
		float horizontal_angle_;
		float vertical_angle_;
		glm::vec3 position_;
		glm::vec3 target_;
		glm::vec3 up_;
		glm::vec3 right_;
		glm::vec3 direction_;
		float speed_;
		float mouse_speed_;

		float near_clip_;
		float far_clip_;
		bool clip_planes_set_;

		float aspect_;

		frustum_ptr frustum_;

		int ortho_left_;
		int ortho_right_;
		int ortho_top_;
		int ortho_bottom_;

		glm::mat4 projection_;
		glm::mat4 view_;
	};
}