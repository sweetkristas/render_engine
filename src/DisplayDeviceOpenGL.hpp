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

#include "DisplayDevice.hpp"
#include "Material.hpp"
#include "ShadersOpenGL.hpp"

namespace Graphics
{
	class DisplayDeviceOpenGL : public DisplayDevice
	{
	public:
		DisplayDeviceOpenGL();
		~DisplayDeviceOpenGL();

		DisplayDeviceId ID() const override { return DISPLAY_DEVICE_OPENGL; }

		void Swap() override;
		void Clear(uint32_t clr) override;

		void SetClearColor(float r, float g, float b, float a) override;
		void SetClearColor(const Color& color) override;

		void Render(const Render::RenderablePtr& r) override;

		TexturePtr CreateTexture(const SurfacePtr& surface, const variant& node) override;
		TexturePtr CreateTexture(const SurfacePtr& surface, Texture::TextureType type, int mipmap_levels) override;
		TexturePtr CreateTexture(unsigned width, PixelFormat::PixelFormatConstant fmt) override;
		TexturePtr CreateTexture(unsigned width, unsigned height, PixelFormat::PixelFormatConstant fmt, Texture::TextureType type=Texture::TEXTURE_2D) override;
		TexturePtr CreateTexture(unsigned width, unsigned height, unsigned depth, PixelFormat::PixelFormatConstant fmt) override;
		void BlitTexture(const TexturePtr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch) override;

		MaterialPtr CreateMaterial(const variant& node) override;

		RenderTargetPtr CreateRenderTarget(size_t width, size_t height, 
			size_t color_plane_count=1, 
			bool depth=false, 
			bool stencil=false, 
			bool use_multi_sampling=false, 
			size_t multi_samples=0) override;
		RenderTargetPtr CreateRenderTarget(const variant& node) override;

		void Init(size_t width, size_t height) override;
		void PrintDeviceInfo() override;

		virtual DisplayDeviceDataPtr CreateDisplayDeviceData(const DisplayDeviceDef& def) override;

	private:
		DisplayDeviceOpenGL(const DisplayDeviceOpenGL&);
	};
}
