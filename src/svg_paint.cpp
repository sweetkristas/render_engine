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
#include <map>

#include "asserts.hpp"
#include "svg_paint.hpp"

namespace svg
{
	namespace 
	{
		struct RGB
		{
			RGB() : r(0), g(0), b(0) {
			}
			RGB(uint8_t red, uint8_t green, uint8_t blue)
				: r(red), g(green), b(blue) {
			}
			uint8_t r;
			uint8_t g;
			uint8_t b;
		};
		typedef std::map<std::string, RGB> color_map;

		void init_color_map(color_map& res) {
            res["aliceblue"] = RGB(240, 248, 255);
            res["antiquewhite"] = RGB(250, 235, 215);
            res["aqua"] = RGB( 0, 255, 255);
            res["aquamarine"] = RGB(127, 255, 212);
            res["azure"] = RGB(240, 255, 255);
            res["beige"] = RGB(245, 245, 220);
            res["bisque"] = RGB(255, 228, 196);
            res["black"] = RGB( 0, 0, 0);
            res["blanchedalmond"] = RGB(255, 235, 205);
            res["blue"] = RGB( 0, 0, 255);
            res["blueviolet"] = RGB(138, 43, 226);
            res["brown"] = RGB(165, 42, 42);
            res["burlywood"] = RGB(222, 184, 135);
            res["cadetblue"] = RGB( 95, 158, 160);
            res["chartreuse"] = RGB(127, 255, 0);
            res["chocolate"] = RGB(210, 105, 30);
            res["coral"] = RGB(255, 127, 80);
            res["cornflowerblue"] = RGB(100, 149, 237);
            res["cornsilk"] = RGB(255, 248, 220);
            res["crimson"] = RGB(220, 20, 60);
            res["cyan"] = RGB( 0, 255, 255);
            res["darkblue"] = RGB( 0, 0, 139);
            res["darkcyan"] = RGB( 0, 139, 139);
            res["darkgoldenrod"] = RGB(184, 134, 11);
            res["darkgray"] = RGB(169, 169, 169);
            res["darkgreen"] = RGB( 0, 100, 0);
            res["darkgrey"] = RGB(169, 169, 169);
            res["darkkhaki"] = RGB(189, 183, 107);
            res["darkmagenta"] = RGB(139, 0, 139);
            res["darkolivegreen"] = RGB( 85, 107, 47);
            res["darkorange"] = RGB(255, 140, 0);
            res["darkorchid"] = RGB(153, 50, 204);
            res["darkred"] = RGB(139, 0, 0);
            res["darksalmon"] = RGB(233, 150, 122);
            res["darkseagreen"] = RGB(143, 188, 143);
            res["darkslateblue"] = RGB( 72, 61, 139);
            res["darkslategray"] = RGB( 47, 79, 79);
            res["darkslategrey"] = RGB( 47, 79, 79);
            res["darkturquoise"] = RGB( 0, 206, 209);
            res["darkviolet"] = RGB(148, 0, 211);
            res["deeppink"] = RGB(255, 20, 147);
            res["deepskyblue"] = RGB( 0, 191, 255);
            res["dimgray"] = RGB(105, 105, 105);
            res["dimgrey"] = RGB(105, 105, 105);
            res["dodgerblue"] = RGB( 30, 144, 255);
            res["firebrick"] = RGB(178, 34, 34);
            res["floralwhite"] = RGB(255, 250, 240);
            res["forestgreen"] = RGB( 34, 139, 34);
            res["fuchsia"] = RGB(255, 0, 255);
            res["gainsboro"] = RGB(220, 220, 220);
            res["ghostwhite"] = RGB(248, 248, 255);
            res["gold"] = RGB(255, 215, 0);
            res["goldenrod"] = RGB(218, 165, 32);
            res["gray"] = RGB(128, 128, 128);
            res["grey"] = RGB(128, 128, 128);
            res["green"] = RGB( 0, 128, 0);
            res["greenyellow"] = RGB(173, 255, 47);
            res["honeydew"] = RGB(240, 255, 240);
            res["hotpink"] = RGB(255, 105, 180);
            res["indianred"] = RGB(205, 92, 92);
            res["indigo"] = RGB( 75, 0, 130);
            res["ivory"] = RGB(255, 255, 240);
            res["khaki"] = RGB(240, 230, 140);
            res["lavender"] = RGB(230, 230, 250);
            res["lavenderblush"] = RGB(255, 240, 245);
            res["lawngreen"] = RGB(124, 252, 0);
            res["lemonchiffon"] = RGB(255, 250, 205);
            res["lightblue"] = RGB(173, 216, 230);
            res["lightcoral"] = RGB(240, 128, 128);
            res["lightcyan"] = RGB(224, 255, 255);
            res["lightgoldenrodyellow"] = RGB(250, 250, 210);
            res["lightgray"] = RGB(211, 211, 211);
            res["lightgreen"] = RGB(144, 238, 144);
            res["lightgrey"] = RGB(211, 211, 211);
            res["lightpink"] = RGB(255, 182, 193);
            res["lightsalmon"] = RGB(255, 160, 122);
            res["lightseagreen"] = RGB( 32, 178, 170);
            res["lightskyblue"] = RGB(135, 206, 250);
            res["lightslategray"] = RGB(119, 136, 153);
            res["lightslategrey"] = RGB(119, 136, 153);
            res["lightsteelblue"] = RGB(176, 196, 222);
            res["lightyellow"] = RGB(255, 255, 224);
            res["lime"] = RGB( 0, 255, 0);
            res["limegreen"] = RGB( 50, 205, 50);
            res["linen"] = RGB(250, 240, 230);
            res["magenta"] = RGB(255, 0, 255);
            res["maroon"] = RGB(128, 0, 0);
            res["mediumaquamarine"] = RGB(102, 205, 170);
            res["mediumblue"] = RGB( 0, 0, 205);
            res["mediumorchid"] = RGB(186, 85, 211);
            res["mediumpurple"] = RGB(147, 112, 219);
            res["mediumseagreen"] = RGB( 60, 179, 113);
            res["mediumslateblue"] = RGB(123, 104, 238);
            res["mediumspringgreen"] = RGB( 0, 250, 154);
            res["mediumturquoise"] = RGB( 72, 209, 204);
            res["mediumvioletred"] = RGB(199, 21, 133);
            res["midnightblue"] = RGB( 25, 25, 112);
            res["mintcream"] = RGB(245, 255, 250);
            res["mistyrose"] = RGB(255, 228, 225);
            res["moccasin"] = RGB(255, 228, 181);
            res["navajowhite"] = RGB(255, 222, 173);
            res["navy"] = RGB( 0, 0, 128);
            res["oldlace"] = RGB(253, 245, 230);
            res["olive"] = RGB(128, 128, 0);
            res["olivedrab"] = RGB(107, 142, 35);
            res["orange"] = RGB(255, 165, 0);
            res["orangered"] = RGB(255, 69, 0);
            res["orchid"] = RGB(218, 112, 214);
            res["palegoldenrod"] = RGB(238, 232, 170);
            res["palegreen"] = RGB(152, 251, 152);
            res["paleturquoise"] = RGB(175, 238, 238);
            res["palevioletred"] = RGB(219, 112, 147);
            res["papayawhip"] = RGB(255, 239, 213);
            res["peachpuff"] = RGB(255, 218, 185);
            res["peru"] = RGB(205, 133, 63);
            res["pink"] = RGB(255, 192, 203);
            res["plum"] = RGB(221, 160, 221);
            res["powderblue"] = RGB(176, 224, 230);
            res["purple"] = RGB(128, 0, 128);
            res["red"] = RGB(255, 0, 0);
            res["rosybrown"] = RGB(188, 143, 143);
            res["royalblue"] = RGB( 65, 105, 225);
            res["saddlebrown"] = RGB(139, 69, 19);
            res["salmon"] = RGB(250, 128, 114);
            res["sandybrown"] = RGB(244, 164, 96);
            res["seagreen"] = RGB( 46, 139, 87);
            res["seashell"] = RGB(255, 245, 238);
            res["sienna"] = RGB(160, 82, 45);
            res["silver"] = RGB(192, 192, 192);
            res["skyblue"] = RGB(135, 206, 235);
            res["slateblue"] = RGB(106, 90, 205);
            res["slategray"] = RGB(112, 128, 144);
            res["slategrey"] = RGB(112, 128, 144);
            res["snow"] = RGB(255, 250, 250);
            res["springgreen"] = RGB( 0, 255, 127);
            res["steelblue"] = RGB( 70, 130, 180);
            res["tan"] = RGB(210, 180, 140);
            res["teal"] = RGB( 0, 128, 128);
            res["thistle"] = RGB(216, 191, 216);
            res["tomato"] = RGB(255, 99, 71);
            res["turquoise"] = RGB( 64, 224, 208);
            res["violet"] = RGB(238, 130, 238);
            res["wheat"] = RGB(245, 222, 179);
            res["white"] = RGB(255, 255, 255);
            res["whitesmoke"] = RGB(245, 245, 245);
            res["yellow"] = RGB(255, 255, 0);
            res["yellowgreen"] = RGB(154, 205, 50);
		}

		color_map& get_color_map() {
			static color_map res;
			if(res.empty()) {
				init_color_map(res);
			}
			return res;
		}

		uint8_t convert_hex_digit(char d) {
			uint8_t value = 0;
			if(d >= 'A' && d <= 'F') {
				value = d - 'A' + 10;
			} else if(d >= 'a' && d <= 'f') {
				value = d - 'a' + 10;
			} else if(d >= '0' && d <= '9') {
				value = d - '0';
			} else {
				ASSERT_LOG(false, "Unrecognised hex digit: " << d);
			}
			return value;
		}
	}

	paint::paint(const std::string& s)
		: r_(0), g_(0), b_(0), a_(255), no_color_(false)
	{
		if(s == "none") {
			no_color_ = true;
		} else if(s == "currentColor") {
			ASSERT_LOG(false, "currentColor for paint is currently unsupported");
		} else if(s.length() > 1 && s[0] == '#') {
			ASSERT_LOG(s.length() == 4 || s.length() == 7, "Expected length of color definition to be 3 or 6 characters long, found: " << s.substr(1));
			if(s.length() == 4) {
				uint8_t r_hex = convert_hex_digit(s[1]);
				uint8_t g_hex = convert_hex_digit(s[2]);
				uint8_t b_hex = convert_hex_digit(s[3]);
				r_ |= (r_hex << 4) | r_hex;
				g_ |= (g_hex << 4) | g_hex;
				b_ |= (b_hex << 4) | b_hex;
			} else {
				r_ = (convert_hex_digit(s[1]) << 4) | convert_hex_digit(s[2]);
				g_ = (convert_hex_digit(s[3]) << 4) | convert_hex_digit(s[4]);
				b_ = (convert_hex_digit(s[5]) << 4) | convert_hex_digit(s[6]);
			}
		} else if(s.length() > 3 && s.substr(0,3) == "rgb") {
			boost::char_separator<char> seperators(" \n\t\r,()");
			boost::tokenizer<boost::char_separator<char>> tok(s.substr(3), seperators);
			int count = 0;
			for(auto it = tok.begin(); it != tok.end(); ++it) {
				char* end = NULL;
				long value = strtol(it->c_str(), &end, 10);
				uint8_t col_val = 0;
				if(value == 0 && end == it->c_str()) {
					ASSERT_LOG(false, "Unable to parse string as an integer: " << *it);
				}
				if(end != NULL && *end == '%') {
					ASSERT_LOG(value >= 0 && value <= 100, "Percentage values range from 0-100: " << value);
					col_val = uint8_t(value / 100.0 * 255);
				} else {
					ASSERT_LOG(value >= 0 && value <= 255, "Percentage values range from 0-255: " << value);
					col_val = uint8_t(value);
				}
				switch(count) {
					case 0: r_ = col_val; break;
					case 1: g_ = col_val; break;
					case 2: b_ = col_val; break;
					default:
						ASSERT_LOG(false, "Too many numbers in color value");
				}
			}
		} else {
			auto it = get_color_map().find(s);
			if(it != get_color_map().end()) {
				r_ = it->second.r;
				g_ = it->second.g;
				b_ = it->second.b;
			} else {
				ASSERT_LOG(false, "Unrecognised color keyword: " << s);
			}
		}
	}

	paint::~paint()
	{
	}
}
