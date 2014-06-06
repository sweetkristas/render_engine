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

#include "renderFwd.hpp"
#include "scene_fwd.hpp"
#include "treetree/tree.hpp"
#include "variant.hpp"
#include "util.hpp"

namespace KRE
{
	typedef std::function<scene_object_ptr(const std::string&)> ObjectTypeFunction;

	class scene_graph
	{
	public:
		scene_graph(const std::string& name);
		~scene_graph();
		void attach_node(scene_node* parent, scene_node_ptr node);
		static scene_graph_ptr create(const std::string& name);
		scene_node_ptr create_node(const std::string& node_type=std::string(), const variant& node=variant());
		static void register_object_type(const std::string& type, ObjectTypeFunction fn);
		scene_node_ptr root_node();
		void render_scene(const renderManagerPtr& renderer);
		void render_scene_helper(const renderManagerPtr& renderer, the::tree<scene_node_ptr>::pre_iterator& it, scene_node_params* snp);
	
		void process(double);

		static void register_factory_function(const std::string& type, std::function<scene_node_ptr(scene_graph*,const variant&)>);
	private:
		DISALLOW_COPY_AND_ASSIGN(scene_graph);

		std::string name_;
		the::tree<scene_node_ptr> graph_;

		friend std::ostream& operator<<(std::ostream& s, const scene_graph& sg);
	};

	std::ostream& operator<<(std::ostream& s, const scene_graph& sg);

	template<class T>
	struct scene_node_registrar
	{
		scene_node_registrar(const std::string& type)
		{
			// register the class factory function 
			scene_graph::register_factory_function(type, [](scene_graph* sg, const variant& node) -> scene_node_ptr { return scene_node_ptr(new T(sg, node));});
		}
	};
}
