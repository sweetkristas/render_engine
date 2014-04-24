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

#include <functional>
#include <map>

#include "asserts.hpp"
#include "scene_graph.hpp"
#include "scene_node.hpp"
#include "scene_object.hpp"

namespace KRE
{
	namespace 
	{
		typedef std::map<std::string, ObjectTypeFunction> scene_objectFactoryLookupTable;
		scene_objectFactoryLookupTable& get_object_factory()
		{
			static scene_objectFactoryLookupTable res;
			return res;
		}

		typedef std::map<std::string, std::function<scene_node_ptr(scene_graph* sg, const variant&)>> scene_nodeRegistry;
		scene_nodeRegistry& get_scene_node_registry()
		{
			static scene_nodeRegistry res;
			return res;
		}
	}
		
	scene_graph::scene_graph(const std::string& name) 
		: name_(name) 
	{
	}

	scene_graph::~scene_graph() 
	{
	}

	scene_node_ptr scene_graph::RootNode()
	{
		return *graph_.begin();
	}

	void scene_graph::AttachNode(scene_node* parent, scene_node_ptr node) 
	{
		if(parent == NULL) {
			graph_.insert(graph_.end_child(), node);
			node->NodeAttached();
			return;
		}
		the::tree<scene_node_ptr>::pre_iterator it = graph_.begin();
		for(; it != graph_.end(); ++it) {
			if(it->get() == parent) {
				//graph_.insert(it, node);
				graph_.insert_below(it, node);
				node->NodeAttached();
				return;
			}
		}
		ASSERT_LOG(false, "parent node not found when attaching a child node");
	}

	scene_graph_ptr scene_graph::Create(const std::string& name) 
	{
		// Create graph then insert a root node into the tree.
		auto sg = std::make_shared<scene_graph>(name);
		sg->graph_.insert(sg->graph_.end(), sg->CreateNode());
		return sg;
	}
	
	scene_node_ptr scene_graph::CreateNode(const std::string& node_type, const variant& node)
	{
		auto it = get_scene_node_registry().find(node_type);
		if(node_type.empty()) {
			return std::make_shared<scene_node>(this);
		}
		ASSERT_LOG(it != get_scene_node_registry().end(), "Couldn't find a node with name '" << node_type << "' to create.");
		return it->second(this, node);
	}

	void scene_graph::RegisterFactoryFunction(const std::string& type, std::function<scene_node_ptr(scene_graph*,const variant&)> create_fn)
	{
		auto it = get_scene_node_registry().find(type);
		if(it != get_scene_node_registry().end()) {
			LOG_WARN("Overwriting the Scene Node Function: " << type);
		}
		get_scene_node_registry()[type] = create_fn;
		
	}

	/*scene_object_ptr scene_graph::CreateObject(const std::string& type, const std::string& name) 
	{
		auto it = get_object_factory().find(type);
		ASSERT_LOG(it == get_object_factory().end(), "Couldn't find a way to create the following type of object: " << type);
		return it->second(name);
	}*/

	void scene_graph::RegisterObjectType(const std::string& type, ObjectTypeFunction fn)
	{
		auto it = get_object_factory().find(type);
		ASSERT_LOG(it != get_object_factory().end(), "Type(" << type << ") already registered");
		get_object_factory()[type] = fn;
	}

	void scene_graph::RenderSceneHelper(const RenderManagerPtr& renderer, 
		the::tree<scene_node_ptr>::pre_iterator& it, 
		scene_node_params* snp)
	{
		if(it == graph_.end()) {
			return;
		}
		// XXX the logic isn't quite right here, snp needs to be cleared at some point.
		(*it)->RenderNode(renderer, snp);
		RenderSceneHelper(renderer, ++it, snp);
	}

	void scene_graph::RenderScene(const RenderManagerPtr& renderer)
	{
		the::tree<scene_node_ptr>::pre_iterator it = graph_.begin();
		//LOG_DEBUG("RenderScene: " << (*it)->NodeName());
		scene_node_params snp;
		RenderSceneHelper(renderer, it, &snp);
	}

	void scene_graph::Process(double elapsed_time)
	{
		the::tree<scene_node_ptr>::pre_iterator it = graph_.begin();
		for(; it != graph_.end(); ++it) {
			(*it)->Process(elapsed_time);
		}
	}

	std::ostream& operator<<(std::ostream& os, const scene_graph& sg)
	{
		os << "SCENEGRAPH(";
		the::tree<scene_node_ptr>::const_pre_iterator it = sg.graph_.begin();
		for(; it != sg.graph_.end(); ++it) {
			os << *(*it);
		}
		os << ")\n";
		return os;
	}
}
