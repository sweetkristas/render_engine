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

#include "asserts.hpp"
#include "RenderManager.hpp"
#include "SceneGraph.hpp"
#include "SceneNode.hpp"
#include "SceneObject.hpp"

namespace Scene
{
	SceneNode::SceneNode(SceneGraph* sg)
		: scene_graph_(sg)
	{
		ASSERT_LOG(scene_graph_ != NULL, "scene_graph_ was null.");
	}

	SceneNode::~SceneNode()
	{
	}

	void SceneNode::AttachNode(const SceneNodePtr& node)
	{
		ASSERT_LOG(scene_graph_ != NULL, "scene_graph_ was null.");
		scene_graph_->AttachNode(SceneNodePtr(this), node);
	}

	void SceneNode::AttachObject(const SceneObjectPtr& obj)
	{
		objects_.emplace_back(obj);
	}

	void SceneNode::AttachLight(size_t ref, const LightPtr& obj)
	{
		auto it = lights_.find(ref);
		if(it != lights_.end()) {
			lights_.erase(it);
		}
		lights_.emplace(ref,obj);
	}

	void SceneNode::AttachCamera(const CameraPtr& obj)
	{
		camera_ = obj;
	}

	void SceneNode::RenderNode(const Render::RenderManagerPtr& renderer, CameraPtr& camera, LightPtrList& lights)
	{
		for(auto l : lights_) {
			lights[l.first] = l.second;
			if(camera_) {
				camera = camera_;
			}
		}
		for(auto o : objects_) {
			renderer->AddRenderableToQueue(o->Queue(), o->Order(), o);
		}
	}
}
