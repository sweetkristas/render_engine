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
#include "scene_graph.hpp"
#include "scene_node.hpp"
#include "scene_object.hpp"

namespace KRE
{
	scene_node::scene_node(scene_graph* sg)
		: scene_graph_(sg)
	{
		ASSERT_LOG(scene_graph_ != NULL, "scene_graph_ was null.");
	}

	scene_node::~scene_node()
	{
	}

	void scene_node::AttachNode(const scene_node_ptr& node)
	{
		ASSERT_LOG(scene_graph_ != NULL, "scene_graph_ was null.");
		scene_graph_->AttachNode(this, node);
	}

	void scene_node::AttachObject(const scene_object_ptr& obj)
	{
		ASSERT_LOG(scene_graph_ != NULL, "scene_graph_ was null.");
		auto dd = DisplayDevice::GetCurrent();
		ASSERT_LOG(dd != NULL, "DisplayDevice was null.");
		objects_.emplace_back(obj);
		obj->SetDisplayData(dd, obj->Attach(dd));
	}

	void scene_node::AttachLight(size_t ref, const LightPtr& obj)
	{
		auto it = lights_.find(ref);
		if(it != lights_.end()) {
			lights_.erase(it);
		}
		lights_.emplace(ref,obj);
		auto dd = DisplayDevice::GetCurrent();
		ASSERT_LOG(dd != NULL, "DisplayDevice was null.");
		obj->SetDisplayData(dd, obj->Attach(dd));		
	}

	void scene_node::AttachCamera(const CameraPtr& obj)
	{
		camera_ = obj;
		auto dd = DisplayDevice::GetCurrent();
		ASSERT_LOG(dd != NULL, "DisplayDevice was null.");
		obj->SetDisplayData(dd, obj->Attach(dd));		
	}

	void scene_node::AttachRenderTarget(const RenderTargetPtr& obj)
	{
		render_target_ = obj;
		auto dd = DisplayDevice::GetCurrent();
		ASSERT_LOG(dd != NULL, "DisplayDevice was null.");
		obj->SetDisplayData(dd, obj->Attach(dd));		
	}

	void scene_node::RenderNode(const RenderManagerPtr& renderer, scene_node_params* rp)
	{
		if(camera_) {
			rp->camera = camera_;
		}
		for(auto l : lights_) {
			rp->lights[l.first] = l.second;
		}
		if(render_target_) {
			rp->render_target = render_target_;
			render_target_->Clear();
		}
		
		for(auto o : objects_) {
			o->SetCamera(rp->camera);
			o->SetLights(rp->lights);
			o->SetRenderTarget(rp->render_target);
			renderer->AddRenderableToQueue(o->Queue(), o->Order(), o);
		}
	}

	void scene_node::NodeAttached()
	{
		// nothing need be done as default
	}

	void scene_node::Process(double)
	{
		// nothing need be done as default
	}

	std::ostream& operator<<(std::ostream& os, const scene_node& node)
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
