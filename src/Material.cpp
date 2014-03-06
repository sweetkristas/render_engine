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

#include "Material.hpp"
#include "Surface.hpp"
#include "WindowManager.hpp"

namespace Graphics
{
	Material::Material(const variant& node)
		: name_(node["name"].as_string()),
		blend_(BlendMode::BM_SRC_ALPHA, BlendMode::BM_ONE_MINUS_SRC_ALPHA)
	{
		// XXX: technically a material could have multiple technique's and passes -- ignoring for now.
		ASSERT_LOG(node.has_key("technique"), "PSYSTEM2: 'material' must have 'technique' attribute.");
		ASSERT_LOG(node["technique"].has_key("pass"), "PSYSTEM2: 'material' must have 'pass' attribute.");
		const variant& pass = node["technique"]["pass"];
		use_lighting_ = pass["lighting"].as_bool(false);
		use_fog_ = pass["fog_override"].as_bool(false);
		do_depth_write_ = pass["depth_write"].as_bool(true);
		do_depth_check_ = pass["depth_check"].as_bool(true);
		if(pass.has_key("scene_blend")) {
			blend_.Set(node["scene_blend"]);
		}
		if(pass.has_key("texture_unit")) {
			if(pass["texture_unit"].is_map()) {
				tex_.emplace_back(CreateTexture(pass["texture_unit"]));
			} else if(pass["texture_unit"].is_list()) {
				for(size_t n = 0; n != pass["texture_unit"].num_elements(); ++n) {
					tex_.emplace_back(CreateTexture(pass["texture_unit"][n]));
				}
			} else {
				ASSERT_LOG(false, "PSYSTEM2: 'texture_unit' attribute must be map or list ");
			}
		}
	}

	Material::~Material()
	{
	}

	void Material::SetTexture(const TexturePtr& tex)
	{
		tex_.emplace_back(tex);
	}

	void Material::EnableLighting(bool en)
	{
		use_lighting_ = en;
	}

	void Material::EnableFog(bool en)
	{
		use_fog_ = en;
	}

	void Material::EnableDepthWrite(bool en)
	{
		do_depth_write_ = en;
	}

	void Material::EnableDepthCheck(bool en)
	{
		do_depth_check_ = en;
	}

	void Material::SetBlendMode(const BlendMode& bm)
	{
		blend_ = bm;
	}

	void Material::SetBlendMode(BlendMode::BlendModeConstants src, BlendMode::BlendModeConstants dst)
	{
		blend_.Set(src, dst);
	}

	namespace
	{
		BlendMode::BlendModeConstants parse_blend_string(const std::string& s)
		{
			if(s == "zero") {
				return BlendMode::BM_ZERO;
			} else if(s == "one") {
				return BlendMode::BM_ONE;
			} else if(s == "src_color") {
				return BlendMode::BM_SRC_COLOR;
			} else if(s == "one_minus_src_color") {
				return BlendMode::BM_ONE_MINUS_SRC_COLOR;
			} else if(s == "dst_color") {
				return BlendMode::BM_DST_COLOR;
			} else if(s == "one_minus_dst_color") {
				return BlendMode::BM_ONE_MINUS_DST_COLOR;
			} else if(s == "src_alpha") {
				return BlendMode::BM_SRC_ALPHA;
			} else if(s == "one_minus_src_alpha") {
				return BlendMode::BM_ONE_MINUS_SRC_ALPHA;
			} else if(s == "dst_alpha") {
				return BlendMode::BM_DST_ALPHA;
			} else if(s == "one_minus_dst_alpha") {
				return BlendMode::BM_ONE_MINUS_DST_ALPHA;
			} else if(s == "const_color") {
				return BlendMode::BM_CONSTANT_COLOR;
			} else if(s == "one_minus_const_color") {
				return BlendMode::BM_ONE_MINUS_CONSTANT_COLOR;
			} else if(s == "const_alpha") {
				return BlendMode::BM_CONSTANT_ALPHA;
			} else if(s == "one_minus_const_alpha") {
				return BlendMode::BM_ONE_MINUS_CONSTANT_ALPHA;
			} else {
				ASSERT_LOG(false, "parse_blend_string: Unrecognised value: " << s);
			}
		}
	}

	void BlendMode::Set(const variant& node) 
	{
		if(node.is_string()) {
			const std::string& blend = node.as_string();
			if(blend == "add") {
				Set(BM_ONE, BM_ONE);
			} else if(blend == "alpha_blend") {
				Set(BM_SRC_ALPHA, BM_ONE_MINUS_SRC_ALPHA);
			} else if(blend == "colour_blend") {
				Set(BM_SRC_COLOR, BM_ONE_MINUS_SRC_COLOR);
			} else if(blend == "modulate") {
				Set(BM_DST_COLOR, BM_ZERO);
			} else if(blend == "src_colour one") {
				Set(BM_SRC_COLOR, BM_ONE);
			} else if(blend == "src_colour zero") {
				Set(BM_SRC_COLOR, BM_ZERO);
			} else if(blend == "src_colour dest_colour") {
				Set(BM_SRC_COLOR, BM_DST_COLOR);
			} else if(blend == "dest_colour one") {
				Set(BM_DST_COLOR, BM_ONE);
			} else if(blend == "dest_colour src_colour") {
				Set(BM_DST_COLOR, BM_SRC_COLOR);
			} else {
				ASSERT_LOG(false, "BlendMode: Unrecognised scene_blend mode " << blend);
			}
		} else if(node.is_list() && node.num_elements() >= 2) {
			ASSERT_LOG(node[0].is_string() && node[1].is_string(), 
				"BlendMode: Blend mode must be specified by a list of two strings.");
			Set(parse_blend_string(node[0].as_string()), parse_blend_string(node[1].as_string()));
		} else {
			ASSERT_LOG(false, "BlendMode: Setting blend requires either a string or a list of greater than two elements." << node.type_as_string());
		}
	}
}
