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
#include "variant.hpp"

namespace Graphics
{
	//class ApplyRenderTarget
	//{
	//
	//};

	class RenderTarget
	{
	public:
		explicit RenderTarget(size_t width, size_t height, 
			size_t color_plane_count=1, 
			bool depth=false, 
			bool stencil=false, 
			bool use_multi_sampling=false, 
			size_t multi_samples=0);
		explicit RenderTarget(const variant& node);
		virtual ~RenderTarget();
		virtual void Render() = 0;

		void Create();
		void Apply();
		void Unappy();

		size_t Width() const { return width_; }
		size_t Height() const { return height_; }
		size_t ColorPlanes() const { return color_attachments_; }
		bool DepthPlane() const { return depth_attachment_; }
		bool StencilPlane() const { return stencil_attachment_; }
		bool UsesMultiSampling() const { return multi_sampling_; }
		size_t MultiSamples() const { return multi_samples_; }
	private:
		virtual void HandleCreate() = 0;
		virtual void SetAsRenderTarget() = 0;
		virtual void HandleApply() = 0;
		virtual void HandleUnapply() = 0;

		size_t width_;
		size_t height_;
		size_t color_attachments_;
		bool depth_attachment_;
		bool stencil_attachment_;
		bool multi_sampling_;
		size_t multi_samples_;

		RenderTarget();
		RenderTarget(const RenderTarget&);
	};

	typedef std::shared_ptr<RenderTarget> RenderTargetPtr;
}
