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

#include <memory>
#include <string>
#include <unordered_map>

#include "RenderFwd.hpp"

#include "glm/glm.hpp"

namespace KRE
{
	class Light;
	typedef std::shared_ptr<Light> LightPtr;
	typedef std::unordered_map<size_t, LightPtr> LightPtrList;
	class camera;
	typedef std::shared_ptr<camera> camera_ptr;
	class Parameter;
	typedef std::shared_ptr<Parameter> ParameterPtr;
	class scene_object;
	typedef std::shared_ptr<scene_object> scene_object_ptr;
	class scene_node;
	typedef std::shared_ptr<scene_node> scene_node_ptr;
	class scene_graph;
	typedef std::shared_ptr<scene_graph> scene_graph_ptr;

	struct scene_node_params
	{
		camera_ptr camera;
		LightPtrList lights;
		RenderTargetPtr render_target;
	};

	class blittable;

	struct vertex_texcoord
	{
		vertex_texcoord() : vtx(0.0f), tc(0.0f) {}
		vertex_texcoord(const glm::vec2& v, const glm::vec2& c) : vtx(v), tc(c) {}
		glm::vec2 vtx;
		glm::vec2 tc;
	};
}
