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

#include "Texture.hpp"
#include "variant.hpp"

namespace Graphics
{
	class OpenGLMaterial;

	class BlendMode
	{
	public:
		enum BlendModeConstants {
			BM_ZERO,
			BM_ONE,
			BM_SRC_COLOR,
			BM_ONE_MINUS_SRC_COLOR,
			BM_DST_COLOR,
			BM_ONE_MINUS_DST_COLOR,
			BM_SRC_ALPHA,
			BM_ONE_MINUS_SRC_ALPHA,
			BM_DST_ALPHA,
			BM_ONE_MINUS_DST_ALPHA,
			BM_CONSTANT_COLOR,
			BM_ONE_MINUS_CONSTANT_COLOR,
			BM_CONSTANT_ALPHA,
			BM_ONE_MINUS_CONSTANT_ALPHA,
		};
		BlendMode() : src_(BM_ONE), dst_(BM_ZERO) {}
		BlendMode(BlendModeConstants src, BlendModeConstants dst) : src_(src), dst_(dst) {}
		BlendModeConstants Source() const { return src_; }
		BlendModeConstants Destination() const { return dst_; }
		BlendModeConstants Src() const { return src_; }
		BlendModeConstants Dst() const { return dst_; }
		void Set(BlendModeConstants src, BlendModeConstants dst) {
			src_ = src;
			dst_ = dst;
		}
		void SetSource(BlendModeConstants src) {
			src_ = src;
		}
		void SetDestination(BlendModeConstants dst) {
			dst_ = dst;
		}
		void SetSrc(BlendModeConstants src) {
			src_ = src;
		}
		void SetDst(BlendModeConstants dst) {
			dst_ = dst;
		}
		void Set(const variant& node);
	private:
		BlendModeConstants src_;
		BlendModeConstants dst_;
	};

	class Material
	{
	public:
		Material(const variant& node);
		virtual ~Material();

		const std::vector<TexturePtr>& GetTexture() const { return tex_; }
		const std::string& Name() const { return name_; }
		bool UseFog() const { return use_fog_; }
		bool UseLighting() const { return use_lighting_; }
		bool DoDepthWrite() const { return do_depth_write_; }
		bool DoDepthCheck() const { return do_depth_check_; }
		const BlendMode& GetBlendMode() const { return blend_; }

		void SetTexture(const TexturePtr& tex);
		void EnableLighting(bool en=true);
		void EnableFog(bool en=true);
		void EnableDepthWrite(bool en=true);
		void EnableDepthCheck(bool en=true);
		void SetBlendMode(const BlendMode& bm);
		void SetBlendMode(BlendMode::BlendModeConstants src, BlendMode::BlendModeConstants dst);

		virtual void Apply() = 0;
	private:
		virtual TexturePtr CreateTexture(const variant& node) = 0;
		std::string name_;
		std::vector<TexturePtr> tex_;
		bool use_lighting_;
		bool use_fog_;
		bool do_depth_write_;
		bool do_depth_check_;
		BlendMode blend_;
	};

	typedef std::shared_ptr<Material> MaterialPtr;
}
