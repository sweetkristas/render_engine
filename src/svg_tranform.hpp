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
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "glm/glm.hpp"

namespace svg
{
	class transform;
	typedef std::shared_ptr<transform> transform_ptr;

	class transform
	{
	public:
		const glm::mat3& as_matrix() const { return mat_; }
		std::string as_string() const;
		static std::vector<transform_ptr> factory(const std::string& s);
	protected:
		enum TransformType {
			TRANSFORM_MATRIX,
			TRANSFORM_TRANSLATE,
			TRANSFORM_SCALE,
			TRANSFORM_ROTATE,
			TRANSFORM_SKEW_X,
			TRANSFORM_SKEW_Y,
		};
		transform(TransformType tt);
	private:
		TransformType type_;
		glm::mat3 mat_;
	};
}
