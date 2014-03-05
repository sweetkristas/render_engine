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

#ifdef _MSC_VER
#	define _USE_MATH_DEFINES 1
#endif
#include <cmath>
#include <glm/ext.hpp>
#if defined(_MSC_VER)
#include <boost/math/special_functions/round.hpp>
#define bmround	boost::math::round
#else
#define bmround	round
#endif

#include "asserts.hpp"
#include "CameraObject.hpp"
#include "DisplayDevice.hpp"
#include "WindowManager.hpp"

namespace Scene
{
	namespace 
	{
		const float default_fov					= 45.0f;
		const float default_horizontal_angle	= float(M_PI);
		const float default_vertical_angle		= 0.0f;
		const float default_speed				= 0.1f;
		const float default_mouse_speed			= 0.005f;
		const float default_near_clip			= 0.1f;
		const float default_far_clip			= 300.0f;
	}

	Camera::Camera(const std::string& name, const Graphics::WindowManagerPtr& wnd)
		: SceneObject(name), 
		fov_(default_fov), 
		horizontal_angle_(default_horizontal_angle), 
		vertical_angle_(default_vertical_angle), 
		speed_(default_speed), 
		mouse_speed_(default_mouse_speed), 
		near_clip_(default_near_clip), 
		far_clip_(default_far_clip),
		type_(CAMERA_PERSPECTIVE), 
		ortho_left_(0), 
		ortho_bottom_(0),
		ortho_top_(wnd->logical_height()), 
		ortho_right_(wnd->logical_width()),
		clip_planes_set_(false),
		view_mode_(VIEW_MODE_AUTO)
	{
		up_ = glm::vec3(0.0f, 1.0f, 0.0f);
		position_ = glm::vec3(0.0f, 0.0f, 0.7f); 
		aspect_ = float(wnd->logical_width())/float(wnd->logical_height());
	
		ComputeView();
		ComputeProjection();
	}

	/*Camera::Camera(const variant& node, const Graphics::WindowManagerPtr& wnd)
		: SceneObject(node["name"].as_string()), 
		fov_(default_fov), 
		horizontal_angle_(default_horizontal_angle), 
		vertical_angle_(default_vertical_angle), 
		speed_(default_speed), 
		mouse_speed_(default_mouse_speed), 
		near_clip_(default_near_clip), 
		far_clip_(default_far_clip),
		type_(CAMERA_PERSPECTIVE), 
		ortho_left_(0), 
		ortho_bottom_(0),
		ortho_top_(wnd->logical_height()), 
		ortho_right_(wnd->logical_width()),
		clip_planes_set_(false)
	{
		position_ = glm::vec3(0.0f, 0.0f, 10.0f); 
		if(node.has_key("fov")) {
			fov_ = std::min(90.0f, std::max(15.0f, float(node["fov"].as_decimal().as_float())));
		}
		if(node.has_key("horizontal_angle")) {
			horizontal_angle_ = float(node["horizontal_angle"].as_decimal().as_float());
		}
		if(node.has_key("vertical_angle")) {
			vertical_angle_ = float(node["vertical_angle"].as_decimal().as_float());
		}
		if(node.has_key("speed")) {
			speed_ = float(node["speed"].as_decimal().as_float());
		}
		if(node.has_key("mouse_speed")) {
			mouse_speed_ = float(node["mouse_speed"].as_decimal().as_float());
		}
		if(node.has_key("aspect")) {
			aspect_ = float(node["aspect"].as_decimal().as_float());
		} else {
			aspect_ = float(preferences::actual_screen_width())/float(preferences::actual_screen_height());
		}

		if(node.has_key("position")) {
			ASSERT_LOG(node["position"].is_list() && node["position"].num_elements() == 3, 
				"position must be a list of 3 decimals.");
			position_ = glm::vec3(float(node["position"][0].as_decimal().as_float()),
				float(node["position"][1].as_decimal().as_float()),
				float(node["position"][2].as_decimal().as_float()));
		}

		if(node.has_key("type")) {
			if(node["type"].as_string() == "orthogonal") {
				type_ = CAMERA_ORTHOGONAL;
			}
		}
		if(node.has_key("ortho_window")) {
			ASSERT_LOG(node["ortho_window"].is_list() && node["ortho_window"].num_elements() == 4, "Attribute 'ortho_window' must be a 4 element list. left,right,top,bottom");
			ortho_left_ = node["ortho_window"][0].as_int();
			ortho_right_ = node["ortho_window"][1].as_int();
			ortho_top_ = node["ortho_window"][2].as_int();
			ortho_bottom_ = node["ortho_window"][3].as_int();
		}

		// If lookat key is specified it overrides the normal compute.
		if(node.has_key("lookat")) {
			const variant& la = node["lookat"];
			ASSERT_LOG(la.has_key("position") && la.has_key("target") && la.has_key("up"),
				"lookat must be a map having 'position', 'target' and 'up' as tuples");
			glm::vec3 position(la["position"][0].as_decimal().as_float(), 
				la["position"][1].as_decimal().as_float(), 
				la["position"][2].as_decimal().as_float());
			glm::vec3 target(la["target"][0].as_decimal().as_float(), 
				la["target"][1].as_decimal().as_float(), 
				la["target"][2].as_decimal().as_float());
			glm::vec3 up(la["up"][0].as_decimal().as_float(), 
				la["up"][1].as_decimal().as_float(), 
				la["up"][2].as_decimal().as_float());
			look_at(position, target, up);
			view_mode_ = VIEW_MODE_MANUAL;
		} else {
			ComputeView();
		}
		ComputeProjection();
	}*/

	Camera::Camera(const std::string& name, int left, int right, int top, int bottom)
		: SceneObject(name), 
		fov_(default_fov), 
		horizontal_angle_(default_horizontal_angle), 
		vertical_angle_(default_vertical_angle), 
		speed_(default_speed), 
		mouse_speed_(default_mouse_speed), 
		near_clip_(default_near_clip), 
		far_clip_(default_far_clip),
		type_(CAMERA_ORTHOGONAL), 
		ortho_left_(left), 
		ortho_bottom_(bottom),
		ortho_top_(top), 
		ortho_right_(right), 
		clip_planes_set_(false), 
		view_(1.0f),
		view_mode_(VIEW_MODE_AUTO)
	{
		up_ = glm::vec3(0.0f, 1.0f, 0.0f);
		position_ = glm::vec3(0.0f, 0.0f, 0.70f); 
		aspect_ = float(right - left)/float(top - bottom);
	
		ComputeProjection();
	}

	Camera::Camera(const std::string& name, const Graphics::WindowManagerPtr& wnd, float fov, float aspect, float near_clip, float far_clip)
		: SceneObject(name), 
		fov_(fov), 
		horizontal_angle_(default_horizontal_angle), 
		vertical_angle_(default_vertical_angle), 
		speed_(default_speed), 
		mouse_speed_(default_mouse_speed), 
		near_clip_(near_clip), 
		far_clip_(far_clip),
		aspect_(aspect), 
		type_(CAMERA_PERSPECTIVE), 
		ortho_left_(0), 
		ortho_bottom_(0),
		ortho_top_(wnd->logical_height()), 
		ortho_right_(wnd->logical_width()),
		clip_planes_set_(true),
		view_mode_(VIEW_MODE_AUTO)
	{
		up_ = glm::vec3(0.0f, 1.0f, 0.0f);
		position_ = glm::vec3(0.0f, 0.0f, 10.0f); 

		ComputeView();
		ComputeProjection();
	}


	Camera::~Camera()
	{
	}

	/*variant Camera::write()
	{
		variant_builder res;
		if(type_ == CAMERA_PERSPECTIVE) {
			if(fov_ != default_fov) {
				res.add("fov", fov_);
			}
			if(horizontal_angle_ != default_horizontal_angle) {
				res.add("horizontal_angle", horizontal_angle_);
			}
			if(vertical_angle_ != default_vertical_angle) {
				res.add("vertical_angle", vertical_angle_);
			}
			if(speed_ != default_speed) {
				res.add("speed", speed_);
			}
			if(mouse_speed_ != default_mouse_speed) {
				res.add("mouse_speed", mouse_speed_);
			}
			res.add("position", position_.x);
			res.add("position", position_.y);
			res.add("position", position_.z);
		} else {
			res.add("type", "orthogonal");
			res.add("ortho_window", ortho_left_);
			res.add("ortho_window", ortho_right_);
			res.add("ortho_window", ortho_top_);
			res.add("ortho_window", ortho_bottom_);
		}
		if(view_mode_ == VIEW_MODE_MANUAL) {
			variant_builder la_map;
			la_map.add("position", position_.x);
			la_map.add("position", position_.y);
			la_map.add("position", position_.z);
			la_map.add("target", target_.x);
			la_map.add("target", target_.y);
			la_map.add("target", target_.z);
			la_map.add("up", up_.x);
			la_map.add("up", up_.y);
			la_map.add("up", up_.z);
			res.add("lookat", la_map.build());
		}
		return res.build();
	}*/

	void Camera::ComputeView()
	{
		view_mode_ = VIEW_MODE_AUTO;
		direction_ = glm::vec3(
			cos(vertical_angle_) * sin(horizontal_angle_), 
			sin(vertical_angle_),
			cos(vertical_angle_) * cos(horizontal_angle_)
		);
		right_ = glm::vec3(
			sin(horizontal_angle_ - float(M_PI)/2.0f), 
			0,
			cos(horizontal_angle_ - float(M_PI)/2.0f)
		);
	
		// Up vector
		up_ = glm::cross(right_, direction_);
		target_ = position_ + direction_;

		view_ = glm::lookAt(position_, target_, up_);
		if(frustum_) {
			frustum_->UpdateMatrices(projection_, view_);
		}
	}

	void Camera::SetType(CameraType type)
	{
		type_ = type;
		ComputeProjection();
	}

	void Camera::SetOrthoWindow(int left, int right, int top, int bottom)
	{
		ortho_left_ = left;
		ortho_right_ = right;
		ortho_top_ = top;
		ortho_bottom_ = bottom;
	
		if(type_ == CAMERA_ORTHOGONAL) {
			ComputeProjection();
		}
	}

	/*BEGIN_DEFINE_CALLABLE_NOBASE(Camera)
	BEGIN_DEFINE_FN(screen_to_world, "(int,int,int=0,int=0) -> [decimal,decimal,decimal]")
		int wx = preferences::actual_screen_width();
		int wy = preferences::actual_screen_height();
		if(NUM_FN_ARGS > 2) {
			wx = FN_ARG(2).as_int();
			if(NUM_FN_ARGS > 3) {
				wy = FN_ARG(3).as_int();
			}
		}
		return vec3_to_variant(obj.screen_to_world(FN_ARG(0).as_int(), FN_ARG(1).as_int(),wx,wy));
	END_DEFINE_FN

	DEFINE_FIELD(position, "[decimal,decimal,decimal]")
		std::vector<variant> v;
		v.push_back(variant(obj.position().x));
		v.push_back(variant(obj.position().y));
		v.push_back(variant(obj.position().z));
		return variant(&v);
	DEFINE_SET_FIELD
		ASSERT_LOG(value.is_list() && value.num_elements() == 3, "position must be a list of 3 elements");
		obj.set_position(glm::vec3(float(value[0].as_decimal().as_float()),
			float(value[1].as_decimal().as_float()),
			float(value[2].as_decimal().as_float())));
		if(obj.mode_ == MODE_MANUAL) {
			obj.view_ = glm::lookAt(obj.position_, obj.target_, obj.up_);
			obj.frustum_.update_matrices(obj.projection_, obj.view_);
		} else {
			obj.compute_view();
		}

	DEFINE_FIELD(speed, "decimal")
		return variant(obj.speed());
	DEFINE_SET_FIELD
		obj.set_speed(value.as_decimal().as_float());

	DEFINE_FIELD(right, "[decimal,decimal,decimal]")
		std::vector<variant> v;
		v.push_back(variant(obj.right().x));
		v.push_back(variant(obj.right().y));
		v.push_back(variant(obj.right().z));
		return variant(&v);

	DEFINE_FIELD(direction, "[decimal,decimal,decimal]")
		std::vector<variant> v;
		v.push_back(variant(obj.direction().x));
		v.push_back(variant(obj.direction().y));
		v.push_back(variant(obj.direction().z));
		return variant(&v);

	DEFINE_FIELD(horizontal_angle, "decimal")
		return variant(obj.hangle());
	DEFINE_SET_FIELD
		obj.set_hangle(value.as_decimal().as_float());
		obj.compute_view();

	DEFINE_FIELD(hangle, "decimal")
		return variant(obj.hangle());
	DEFINE_SET_FIELD
		obj.set_hangle(value.as_decimal().as_float());
		obj.compute_view();

	DEFINE_FIELD(vertical_angle, "decimal")
		return variant(obj.vangle());
	DEFINE_SET_FIELD
		obj.set_vangle(value.as_decimal().as_float());
		obj.compute_view();

	DEFINE_FIELD(vangle, "decimal")
		return variant(obj.vangle());
	DEFINE_SET_FIELD
		obj.set_vangle(value.as_decimal().as_float());
		obj.compute_view();

	DEFINE_FIELD(mouse_speed, "decimal")
		return variant(obj.mousespeed());
	DEFINE_SET_FIELD
		obj.set_mousespeed(value.as_decimal().as_float());

	DEFINE_FIELD(target, "[decimal,decimal,decimal]")
		std::vector<variant> v;
		v.push_back(variant(obj.target_.x));
		v.push_back(variant(obj.target_.y));
		v.push_back(variant(obj.target_.z));
		return variant(&v);

	DEFINE_FIELD(up, "[decimal,decimal,decimal]")
		std::vector<variant> v;
		v.push_back(variant(obj.up_.x));
		v.push_back(variant(obj.up_.y));
		v.push_back(variant(obj.up_.z));
		return variant(&v);

	DEFINE_FIELD(fov, "decimal")
		return variant(obj.fov());
	DEFINE_SET_FIELD
		obj.set_fov(value.as_decimal().as_float());

	DEFINE_FIELD(aspect, "decimal")
		return variant(obj.aspect());
	DEFINE_SET_FIELD
		obj.set_aspect(value.as_decimal().as_float());

	DEFINE_FIELD(clip_planes, "[decimal,decimal]")
		std::vector<variant> v;
		v.push_back(variant(obj.near_clip_));
		v.push_back(variant(obj.far_clip_));
		return variant(&v);
	DEFINE_SET_FIELD
		ASSERT_LOG(value.is_list() && value.num_elements() == 2, "clip_planes takes a tuple of two decimals");
		obj.set_clip_planes(value[0].as_decimal().as_float(), value[1].as_decimal().as_float());

	DEFINE_FIELD(type, "string")
		if(obj.type() == ORTHOGONAL_CAMERA) {
			return variant("orthogonal");
		} 
		return variant("perspective");
	DEFINE_SET_FIELD
		if(value.as_string() == "orthogonal") {
			obj.set_type(ORTHOGONAL_CAMERA);
		} else {
			obj.set_type(PERSPECTIVE_CAMERA);
		}
	
	DEFINE_FIELD(ortho_window, "[int,int,int,int]")
		std::vector<variant> v;
		v.push_back(variant(obj.ortho_left()));
		v.push_back(variant(obj.ortho_right()));
		v.push_back(variant(obj.ortho_top()));
		v.push_back(variant(obj.ortho_bottom()));
		return variant(&v);
	DEFINE_SET_FIELD
		ASSERT_LOG(value.is_list() && value.num_elements() == 4, "Attribute 'ortho_window' must be a 4 element list. left,right,top,bottom");
		obj.set_ortho_window(value[0].as_int(), value[1].as_int(), value[2].as_int(), obj.ortho_bottom_ = value[3].as_int());

	END_DEFINE_CALLABLE(Camera)
	*/

	void Camera::LookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
	{
		view_mode_ = VIEW_MODE_MANUAL;
		position_ = position;
		target_ = target;
		up_ = up;
		direction_ = target_ - position_;
		view_ = glm::lookAt(position_, target_, up_);
		if(frustum_) {
			frustum_->UpdateMatrices(projection_, view_);
		}
	}


	void Camera::SetFov(float fov)
	{
		fov_ = fov;
		ComputeProjection();
	}

	void Camera::SetClipPlanes(float z_near, float z_far)
	{
		near_clip_ = z_near;
		far_clip_ = z_far;
		clip_planes_set_ = true;
		ComputeProjection();
	}

	void Camera::SetAspect(float aspect)
	{
		aspect_ = aspect;
		ComputeProjection();
	}

	void Camera::AttachFrustum(const FrustumPtr& frustum)
	{
		frustum_ = frustum;
		if(frustum_) {
			frustum_->UpdateMatrices(projection_, view_);
		}
	}

	void Camera::ComputeProjection()
	{
		if(type_ == CAMERA_ORTHOGONAL) {
			if(clip_planes_set_) {
				projection_ = glm::frustum(float(ortho_left_), float(ortho_right_), float(ortho_bottom_), float(ortho_top_), NearClip(), FarClip());
			} else {
				projection_ = glm::ortho(float(ortho_left_), float(ortho_right_), float(ortho_bottom_), float(ortho_top_));
			}
		} else {
			projection_ = glm::perspective(Fov(), aspect_, NearClip(), FarClip());
		}
		if(frustum_) {
			frustum_->UpdateMatrices(projection_, view_);
		}
	}

	Graphics::DisplayDeviceDef Camera::Attach(const Graphics::DisplayDevicePtr& dd)
	{
		Graphics::DisplayDeviceDef def(AttributeRenderVariables(), UniformRenderVariables());
		// XXX
		return def;
	}

	// Convert from a screen position (assume +ve x to right, +ve y down) to world space.
	// Assumes the depth buffer was enabled.
	glm::vec3 Camera::ScreenToWorld(int x, int y, int wx, int wy) const
	{
		glm::vec4 view_port(0, 0, wx, wy);

		//GLfloat depth;
		//glReadPixels(x, wy - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		//glm::vec3 screen(x, wy - y, depth);

		//return glm::unProject(screen, view_, projection_, view_port);
		ASSERT_LOG(false, "Camera::ScreenToWorld: fixme");
		return glm::vec3();
	}


	namespace
	{
		float dti(float val) 
		{
			return abs(val - bmround(val));
		}
	}

	glm::ivec3 Camera::GetFacing(const glm::vec3& coords) const
	{
		if(dti(coords.x) < dti(coords.y)) {
			if(dti(coords.x) < dti(coords.z)) {
				if(direction_.x > 0) {
					return glm::ivec3(-1,0,0);
				} else {
					return glm::ivec3(1,0,0);
				}
			} else {
				if(direction_.z > 0) {
					return glm::ivec3(0,0,-1);
				} else {
					return glm::ivec3(0,0,1);
				}
			}
		} else {
			if(dti(coords.y) < dti(coords.z)) {
				if(direction_.y > 0) {
					return glm::ivec3(0,-1,0);
				} else {
					return glm::ivec3(0,1,0);
				}
			} else {
				if(direction_.z > 0) {
					return glm::ivec3(0,0,-1);
				} else {
					return glm::ivec3(0,0,1);
				}
			}
		}
	}
}
