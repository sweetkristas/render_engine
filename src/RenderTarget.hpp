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
#include "blittable.hpp"

namespace KRE
{
	class renderTarget : public blittable
	{
	public:
		explicit renderTarget(unsigned width, unsigned height, 
			unsigned color_plane_count=1, 
			bool depth=false, 
			bool stencil=false, 
			bool use_multi_sampling=false, 
			unsigned multi_samples=0);
		explicit renderTarget(const variant& node);
		virtual ~renderTarget();

		void Create();
		void Apply();
		void Unapply();
		void clear();

		unsigned width() const { return width_; }
		unsigned height() const { return height_; }
		unsigned colorPlanes() const { return color_attachments_; }
		bool depthPlane() const { return depth_attachment_; }
		bool StencilPlane() const { return stencil_attachment_; }
		bool UsesMultiSampling() const { return multi_sampling_; }
		unsigned MultiSamples() const { return multi_samples_; }

		void set_clear_color(int r, int g, int b, int a=255);
		void set_clear_color(float r, float g, float b, float a=1.0f);
		void set_clear_color(const color& color);
		const color& Getclearcolor() const { return clear_color_; }
	private:
		virtual void HandleCreate() = 0;
		virtual void HandleApply() = 0;
		virtual void HandleUnapply() = 0;
		virtual void Handleclear() = 0;

		unsigned width_;
		unsigned height_;
		unsigned color_attachments_;
		bool depth_attachment_;
		bool stencil_attachment_;
		bool multi_sampling_;
		unsigned multi_samples_;

		color clear_color_;

		renderTarget();
		renderTarget(const renderTarget&);
	};

	typedef std::shared_ptr<renderTarget> render_target_ptr;
}
