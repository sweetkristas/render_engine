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

#include"AttributeSet.hpp"
#include "Material.hpp"
#include "RenderQueue.hpp"
#include "RenderVariable.hpp"
#include "SceneFwd.hpp"

namespace KRE
{
	class Renderable
	{
	public:
		Renderable();
		Renderable(size_t order);
		virtual ~Renderable();

		void SetPosition(const glm::vec3& position);
		void SetPosition(float x, float y, float z=0.0f);
		void SetPosition(int x, int y, int z=0);
		const glm::vec3& Position() const { return position_; }

		void SetRotation(float angle, const glm::vec3& axis);
		void SetRotation(const glm::quat& rot);
		const glm::quat& Rotation() const { return rotation_; }

		void SetScale(float xs, float ys, float zs=1.0f);
		void SetScale(const glm::vec3& scale);
		const glm::vec3& Scale() const { return scale_; }

		glm::mat4 ModelMatrix() const;

		void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255);
		void SetColor(const Color& color);
		const Color& GetColor() const { return color_; }
		bool IsColorSet() const { return color_set_; }

		size_t Order() const { return order_; }
		void SetOrder(size_t o) { order_ = o; }

		const CameraPtr& Camera() const { return camera_; }
		void SetCamera(const CameraPtr& camera);

		const LightPtrList& Lights() const { return lights_; }
		void SetLights(const LightPtrList& lights);

		const MaterialPtr& Material() const { return material_; }
		void SetMaterial(const MaterialPtr& material);

		const RenderTargetPtr& GetRenderTarget() const { return render_target_; }
		void SetRenderTarget(const RenderTargetPtr& rt);

		void SetDisplayData(const DisplayDevicePtr& dd, const DisplayDeviceDef& def);
		const DisplayDeviceDataPtr& GetDisplayData() const { return display_data_; }

		void AddAttributeRenderVariable(const RenderVariablePtr& rv);
		void AddUniformRenderVariable(const RenderVariablePtr& rv);
		const RenderVariableList& AttributeRenderVariables() const { return attribute_render_vars_; }
		const RenderVariableList& UniformRenderVariables() const { return uniform_render_vars_; }

		void AddAttributeSet(const AttributeSetPtr& attrset);

		virtual void PreRender() {}
	private:
		size_t order_;
		glm::vec3 position_;
		glm::quat rotation_;
		glm::vec3 scale_;
		CameraPtr camera_;
		LightPtrList lights_;
		MaterialPtr material_;
		RenderTargetPtr render_target_;
		Color color_;
		bool color_set_;

		RenderVariableList attribute_render_vars_;
		RenderVariableList uniform_render_vars_;

		DisplayDeviceDataPtr display_data_;
		Renderable(const Renderable&);
	};
}
