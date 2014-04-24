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
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#include<boost/tokenizer.hpp>

#include "asserts.hpp"
#include "svg_tranform.hpp"

namespace svg
{
	transform::transform(TransformType tt)
		: type_(tt), mat_(0.0f)
	{
	}

	std::vector<transform_ptr> transform::factory(const std::string& s)
	{
		std::vector<transform_ptr> results;
		enum {
			STATE_TYPE,
			STATE_NUMBER,
		} state = STATE_TYPE;
		
		transform* trf = NULL;
		std::vector<float> parameters;

		boost::char_separator<char> seperators(" \n\t\r,", "()");
		boost::tokenizer<boost::char_separator<char>> tok(s, seperators);
		for(auto it = tok.begin(); it != tok.end(); ++it) {
			if(state == STATE_TYPE) {
				if(*it == "matrix") {
					trf = new transform(TRANSFORM_MATRIX);
				} else if(*it == "translate") {
					trf = new transform(TRANSFORM_TRANSLATE);
				} else if(*it == "scale") {
					trf = new transform(TRANSFORM_SCALE);
				} else if(*it == "rotate") {
					trf = new transform(TRANSFORM_ROTATE);
				} else if(*it == "skewX") {
					trf = new transform(TRANSFORM_SKEW_X);
				} else if(*it == "skewY") {
					trf = new transform(TRANSFORM_SKEW_Y);
				} else if(*it == "(") {
					parameters.clear();
					state = STATE_NUMBER;
				} else {
					ASSERT_LOG(false, "Unexpected token while looking for a type: " << *it << " : " << s);
				}
			} else if(state == STATE_NUMBER) {
				if(*it == ")") {
					ASSERT_LOG(trf != NULL, "trf is null parsing transform: " << s);
					switch(trf->type_) {
						case TRANSFORM_MATRIX:
							ASSERT_LOG(parameters.size() == 6, "Parsing transform:matrix found " << parameters.size() << " parameter(s), expected 6");
							trf->mat_ = glm::mat3(parameters[0], parameters[2], parameters[4], 
								parameters[1], parameters[3], parameters[5], 
								0.0f, 0.0f, 1.0f);
							break;
						case TRANSFORM_TRANSLATE: {
							ASSERT_LOG(parameters.size() == 1 || parameters.size() == 2, "Parsing transform:translate found " << parameters.size() << " parameter(s), expected 1 or 2");
							float tx = parameters[0];
							float ty = parameters.size() == 2 ? parameters[1] : 0.0f;
							trf->mat_ = glm::mat3(
								1.0f, 0.0f, tx, 
								0.0f, 1.0f, ty, 
								0.0f, 0.0f, 1.0f);
							break;
						}
						case TRANSFORM_SCALE: {
							ASSERT_LOG(parameters.size() == 1 || parameters.size() == 2, "Parsing transform:scale found " << parameters.size() << " parameter(s), expected 1 or 2");
							float sx = parameters[0];
							float sy = parameters.size() == 2 ? parameters[1] : sx;
							trf->mat_ = glm::mat3(
								sx,   0.0f, 0.0f, 
								0.0f, sy,   0.0f, 
								0.0f, 0.0f, 1.0f);
							break;
						}
						case TRANSFORM_ROTATE: {
							ASSERT_LOG(parameters.size() == 1 || parameters.size() == 3, "Parsing transform:rotate found " << parameters.size() << " parameter(s), expected 1 or 3");
							float angle = parameters[0];
							float cx = parameters.size() == 3 ? parameters[1] : 0.0f;
							float cy = parameters.size() == 3 ? parameters[2] : 0.0f;
							float sa = sin(angle);
							float ca = cos(angle);
							glm::mat3 txns1(
								1.0f, 0.0f, cx, 
								0.0f, 1.0f, cy, 
								0.0f, 0.0f, 1.0f);
							glm::mat3 txns2(
								1.0f, 0.0f, -cx, 
								0.0f, 1.0f, -cy, 
								0.0f, 0.0f, 1.0f);
							trf->mat_ = txns1 * glm::mat3(
								  ca,   sa, 0.0f, 
								 -sa,   ca, 0.0f, 
								0.0f, 0.0f, 1.0f) * txns2;
							break;
						}
						case TRANSFORM_SKEW_X: {
							ASSERT_LOG(parameters.size() == 1, "Parsing transform:skewX found " << parameters.size() << " parameter(s), expected 1");
							float sa = tan(parameters[0]);
							trf->mat_ = glm::mat3(
								1.0f,   sa, 0.0f, 
								0.0f, 1.0f, 0.0f, 
								0.0f, 0.0f, 1.0f);
							break;
						}
						case TRANSFORM_SKEW_Y: {
							ASSERT_LOG(parameters.size() == 1, "Parsing transform:skewY found " << parameters.size() << " parameter(s), expected 1");
							float sa = tan(parameters[0]);
							trf->mat_ = glm::mat3(
								1.0f, 0.0f, 0.0f, 
								  sa, 1.0f, 0.0f, 
								0.0f, 0.0f, 1.0f);
							break;
						}
					}
					state = STATE_TYPE;					
					// emit transform
					results.emplace_back(trf);
					trf = NULL;
				} else {
					char* end = NULL;
					double value = strtod(it->c_str(), &end);
					if(value == 0 && it->c_str() == end) {
						ASSERT_LOG(false, "Invalid number value: " << *it);
					}
					ASSERT_LOG(errno != ERANGE, "Parsed numeric value out-of-range: " << *it);					
					parameters.push_back(float(value));
				}
			}
		}

		//std::cerr << "Transforms: ";
		//for(auto& res : results) {
		//	std::cerr << res->as_string() << " ";
		//}
		//std::cerr << std::endl;

		return results;
	}

	std::string transform::as_string() const
	{
		switch(type_) {
			case TRANSFORM_MATRIX:
				return "matrix";
			case TRANSFORM_TRANSLATE:
				return "translate";
			case TRANSFORM_SCALE:
				return "scale";
			case TRANSFORM_ROTATE:
				return "rotate";
			case TRANSFORM_SKEW_X:
				return "skewX";
			case TRANSFORM_SKEW_Y:
				return "skewY";
		}
		ASSERT_LOG(false, "Unrecognised transform value: " << type_);
		return "";
	}

}
