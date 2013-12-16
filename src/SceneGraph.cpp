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
#include "SceneGraph.hpp"
#include "SceneNode.hpp"
#include "SceneObject.hpp"

namespace Scene
{
	namespace 
	{
		typedef std::map<std::string, ObjectTypeFunction> SceneObjectFactoryLookupTable;
		SceneObjectFactoryLookupTable& get_object_factory()
		{
			static SceneObjectFactoryLookupTable res;
			return res;
		}
	}
		
	SceneGraph::SceneGraph(const std::string& name) 
		: name_(name) 
	{
	}

	SceneGraph::~SceneGraph() 
	{
	}

	SceneNodePtr SceneGraph::RootNode()
	{
		return *graph_.begin();
	}

	void SceneGraph::AttachNode(const SceneNodePtr& parent, SceneNodePtr node) 
	{
		if(parent == NULL) {
			graph_.insert(graph_.end_child(), node);
			return;
		}
		the::tree<SceneNodePtr>::const_pre_iterator it = graph_.begin();
		for(; it != graph_.end(); ++it) {
			if(*it == parent) {
				//graph_.insert(it, node);
				return;
			}
		}
		ASSERT_LOG(false, "parent node not found when attaching a child node");
	}

	SceneGraphPtr SceneGraph::Create(const std::string& name) 
	{
		// Create graph then insert a root node into the tree.
		auto sg = std::make_shared<SceneGraph>(name);
		sg->graph_.insert(sg->graph_.end(), sg->CreateNode());
		return sg;
	}
	
	SceneNodePtr SceneGraph::CreateNode()
	{
		return std::make_shared<SceneNode>(this);
	}

	/*SceneObjectPtr SceneGraph::CreateObject(const std::string& type, const std::string& name) 
	{
		auto it = get_object_factory().find(type);
		ASSERT_LOG(it == get_object_factory().end(), "Couldn't find a way to create the following type of object: " << type);
		return it->second(name);
	}*/

	void SceneGraph::RegisterObjectType(const std::string& type, ObjectTypeFunction fn)
	{
		auto it = get_object_factory().find(type);
		ASSERT_LOG(it != get_object_factory().end(), "Type(" << type << ") already registered");
		get_object_factory()[type] = fn;
	}
}
