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

#include <map>
#include <memory>
#include <string>

#include "Color.hpp"
#include "DisplayDeviceFwd.hpp"
#include "Material.hpp"
#include "Renderable.hpp"
#include "RenderTarget.hpp"
#include "variant.hpp"

namespace Graphics
{
	typedef std::vector<std::string> HintList;
	typedef std::map<std::string,HintList> HintMap;
	class DisplayDeviceDef
	{
	public:
		DisplayDeviceDef(const Render::RenderVariableList& arv, const Render::RenderVariableList& urv);
		~DisplayDeviceDef();

		const Render::RenderVariableList& GetAttributeRenderVars() const { return attrib_render_vars_; }
		const Render::RenderVariableList& GetUniformRenderVars() const { return uniform_render_vars_; }

		void SetHint(const std::string& hint_name, const std::string& hint);
		void SetHint(const std::string& hint_name, const HintList& hint);
		HintMap GetHints() const { return hints_; }
	private:
		HintMap hints_;
		const Render::RenderVariableList& attrib_render_vars_;
		const Render::RenderVariableList& uniform_render_vars_;
	};

	class DisplayDeviceData
	{
	public:
		DisplayDeviceData();
		virtual ~DisplayDeviceData();
	private:
		DisplayDeviceData(const DisplayDeviceData&);
	};

	class DisplayDevice
	{
	public:
		enum DisplayDeviceId {
			// Display device is OpenGL 2.1 compatible, using shaders.
			DISPLAY_DEVICE_OPENGL,
			// Display device is OpenGLES 2.0, using shaders
			DISPLAY_DEVICE_OPENGLES,
			// Display device is OpenGL 1.1, fixed function pipeline
			DISPLAY_DEVICE_OPENGL_FIXED,
			// Display device is whatever SDL wants to use
			DISPLAY_DEVICE_SDL,
			// Display device is Direct3D
			DISPLAY_DEVICE_D3D,
		};
		enum ClearFlags {
			DISPLAY_CLEAR_COLOR		= 1,
			DISPLAY_CLEAR_DEPTH		= 2,
			DISPLAY_CLEAR_STENCIL	= 4,
			DISPLAY_CLEAR_ALL		= 0xffffffff,
		};

		DisplayDevice();
		virtual ~DisplayDevice();

		virtual DisplayDeviceId ID() const = 0;

		virtual void SetClearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void SetClearColor(const Color& color) = 0;

		virtual void Clear(uint32_t clr) = 0;
		virtual void Swap() = 0;

		virtual void Init(size_t width, size_t height) = 0;
		virtual void PrintDeviceInfo() = 0;

		virtual void Render(const Render::RenderablePtr& r) = 0;

		virtual TexturePtr CreateTexture(const SurfacePtr& surface, const variant& node) = 0;
		virtual TexturePtr CreateTexture(const SurfacePtr& surface, 
			Texture::TextureType type=Texture::TextureType::TEXTURE_2D, 
			int mipmap_levels=0) = 0;
		virtual TexturePtr CreateTexture(unsigned width, PixelFormat::PixelFormatConstant fmt) = 0;
		virtual TexturePtr CreateTexture(unsigned width, unsigned height, PixelFormat::PixelFormatConstant fmt, Texture::TextureType type=Texture::TEXTURE_2D) = 0;
		virtual TexturePtr CreateTexture(unsigned width, unsigned height, unsigned depth, PixelFormat::PixelFormatConstant fmt) = 0;
		virtual void BlitTexture(const TexturePtr& tex, int dstx, int dsty, int dstw, int dsth, float rotation, int srcx, int srcy, int srcw, int srch) = 0;

		virtual MaterialPtr CreateMaterial(const variant& node) = 0;

		virtual RenderTargetPtr CreateRenderTarget(size_t width, size_t height, 
			size_t color_plane_count=1, 
			bool depth=false, 
			bool stencil=false, 
			bool use_multi_sampling=false, 
			size_t multi_samples=0) = 0;
		virtual RenderTargetPtr CreateRenderTarget(const variant& node) = 0;

		virtual DisplayDeviceDataPtr CreateDisplayDeviceData(const DisplayDeviceDef& def) = 0;

		static DisplayDevicePtr Factory(const std::string& type);

		static void RegisterFactoryFunction(const std::string& type, std::function<DisplayDevicePtr()>);
	private:
		DisplayDevice(const DisplayDevice&);
	};

	template<class T>
	struct DisplayDeviceRegistrar
	{
		DisplayDeviceRegistrar(const std::string& type)
		{
			// register the class factory function 
			DisplayDevice::RegisterFactoryFunction(type, []() -> DisplayDevicePtr { return DisplayDevicePtr(new T());});
		}
	};
}
