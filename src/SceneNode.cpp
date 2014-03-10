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
#include "CameraObject.hpp"
#include "LightObject.hpp"
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
		scene_graph_->AttachNode(this, node);
	}

	void SceneNode::AttachObject(const SceneObjectPtr& obj)
	{
		ASSERT_LOG(scene_graph_ != NULL, "scene_graph_ was null.");
		auto dd = scene_graph_->DisplayDevice();
		ASSERT_LOG(dd != NULL, "DisplayDevice was null.");
		objects_.emplace_back(obj);
		obj->SetDisplayData(dd, obj->Attach(dd));
	}

	void SceneNode::AttachLight(size_t ref, const LightPtr& obj)
	{
		auto it = lights_.find(ref);
		if(it != lights_.end()) {
			lights_.erase(it);
		}
		lights_.emplace(ref,obj);
		auto dd = scene_graph_->DisplayDevice();
		ASSERT_LOG(dd != NULL, "DisplayDevice was null.");
		obj->SetDisplayData(dd, obj->Attach(dd));		
	}

	void SceneNode::AttachCamera(const CameraPtr& obj)
	{
		camera_ = obj;
		auto dd = scene_graph_->DisplayDevice();
		ASSERT_LOG(dd != NULL, "DisplayDevice was null.");
		obj->SetDisplayData(dd, obj->Attach(dd));		
	}

	void SceneNode::RenderNode(const Render::RenderManagerPtr& renderer, CameraPtr& camera, LightPtrList& lights)
	{
		if(camera_) {
			camera = camera_;
		}
		for(auto l : lights_) {
			lights[l.first] = l.second;
		}
		for(auto o : objects_) {
			o->SetCamera(camera);
			o->SetLights(lights);
			renderer->AddRenderableToQueue(o->Queue(), o->Order(), o);
		}
	}

	void SceneNode::NodeAttached()
	{
		// nothing need be done as default
	}

	void SceneNode::Process(double)
	{
		// nothing need be done as default
	}

	std::ostream& operator<<(std::ostream& os, const SceneNode& node)
	{
		os  << "NODE(" 
			<< node.NodeName() << " : "
			<< (node.camera_ ? "1 camera, " : "") 
			<< node.lights_.size() << " light" << (node.lights_.size() != 1 ? "s" : "") << ", "
			<< node.objects_.size() << " object" << (node.objects_.size() != 1 ? "s (" : " (");
		for(auto o : node.objects_) {
			os << ", \"" << o->ObjectName() << "\"";
		}
		os << "))";
		return os;
	}
}
