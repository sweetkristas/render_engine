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

#include "RenderQueue.hpp"
#include "RenderVariable.hpp"

namespace Render
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

		const glm::mat4& ModelMatrix() const;

		size_t Order() const { return order_; }
		void SetOrder(size_t o) { order_ = o; }

		void draw(RenderQueuePtr queue) const;
	protected:
		RenderVariableList render_vars_;
	private:
		size_t order_;
		glm::vec3 position_;
		glm::quat rotation_;
		glm::vec3 scale_;
		Renderable(const Renderable&);
	};
}
