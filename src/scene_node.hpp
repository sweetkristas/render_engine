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

#include <unordered_map>
#include <vector>

#include "RenderFwd.hpp"
#include "scene_fwd.hpp"

namespace KRE
{
	class scene_node
	{
	public:
		explicit scene_node(scene_graph* sg);
		virtual ~scene_node();
		void attach_node(const scene_node_ptr& node);
		void attach_light(size_t ref, const LightPtr& obj);
		void attach_camera(const camera_ptr& obj);
		void attach_object(const scene_object_ptr& obj);
		void attach_render_target(const RenderTargetPtr& obj);
		const camera_ptr& camera() const { return camera_; }
		const LightPtrList& lights() const { return lights_; }
		const RenderTargetPtr get_render_target() const { return render_target_; }
		void render_node(const RenderManagerPtr& renderer, scene_node_params* rp);
		scene_graph* parent_graph() { return scene_graph_; }
		virtual void process(double);
		virtual void node_attached();
		void set_node_name(const std::string& s) { name_ = s; }
		const std::string& node_name() const { return name_; }
	private:
		std::string name_;
		scene_graph* scene_graph_;
		std::vector<scene_object_ptr> objects_;
		LightPtrList lights_;
		camera_ptr camera_;
		RenderTargetPtr render_target_;
		scene_node();
		scene_node(const scene_node&);
		scene_node& operator=(const scene_node&);

		friend std::ostream& operator<<(std::ostream& os, const scene_node& node);
	};

	std::ostream& operator<<(std::ostream& os, const scene_node& node);
}
