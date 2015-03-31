/*
	Copyright (C) 2013-2014 by Kristina Simpson <sweet.kristas@gmail.com>
	
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

#include <array>
#include <cstdint>
#include <memory>

#include "Color.hpp"

namespace tiled
{
	enum class Orientation {
		ORTHOGONAL,
		ISOMETRIC,
		STAGGERED,
		HEXAGONAL
	};

	enum class RenderOrder {
		RIGHT_DOWN,
		RIGHT_UP,
		LEFT_DOWN,
		LEFT_UP,
	};

	enum class StaggerIndex {
		EVEN,
		ODD,
	};

	enum class StaggerDirection {
		ROWS,
		COLS,
	};

	enum class ImageFormat {
		NONE,
		PNG,
		GIF,
		BMP,
		JPEG
	};

	class Map;
	typedef std::shared_ptr<Map> MapPtr;

	struct Property
	{
		explicit Property(const std::string& n, const std::string& v) : name(n), value(v) {}
		std::string name;
		std::string value;
	};
	
	struct Terrain
	{
		explicit Terrain(const std::string& n, uint32_t id) : name(n), tile_id(id) {}
		std::string name;
		uint32_t tile_id;
	};


	class ObjectGroup
	{
	public:
		ObjectGroup();
	private:
	};

	class Layer
	{
	public:
		explicit Layer(const std::string& name);
		void setProperties(std::vector<Property>* props) { properties_.swap(*props); }
		void setData(std::vector<uint32_t>* data) { tile_data_.swap(*data); }
		void setOpacity(float o) { opacity_ = o; }
		void setVisibility(bool visible) { is_visible_ = visible; }
	private:
		std::string name_;
		std::vector<Property> properties_;
		std::vector<uint32_t> tile_data_;
		float opacity_;
		bool is_visible_;
	};

	class TileImage
	{
	public:
		TileImage();
		void setSource(const std::string& source) { source_ = source; }
		void setImageData(ImageFormat fmt, const std::vector<char>& data) { format_ = fmt; data_ = data; }
		explicit TileImage(const std::string& source);
		void setTransparentColor(const KRE::Color& color) { transparent_color_ = color; has_transparent_color_set_ = true; }
		void setWidth(int w) { width_ = w; }
		void setHeight(int h) { height_ = h; }
	private:
		ImageFormat format_;
		std::vector<char> data_;
		std::string source_;
		bool has_transparent_color_set_;
		KRE::Color transparent_color_;
		int width_;
		int height_;
	};

	class Tile
	{
	public:
		explicit Tile(uint32_t local_id);
		void addImage(const TileImage& image);
		void setProperties(std::vector<Property>* props) { properties_.swap(*props); }
		void setProbability(float p) { probability_ = p; }
		void setTerrain(const std::array<int, 4>& t) { terrain_ = t; }
	private:
		Tile() = delete;
		uint32_t local_id_;
		std::array<int, 4> terrain_;
		float probability_;
		std::vector<Property> properties_;
		std::vector<ObjectGroup> object_group_;
	};

	class TileSet
	{
	public:
		explicit TileSet(int first_gid);
		
		void setName(const std::string& name) { name_ = name; }
		void setTileDimensions(int width, int height) { tile_width_ = width; tile_height_ = height; }
		void setSpacing(int spacing) { spacing_ = spacing; }
		void setMargin(int margin) { margin_ = margin; }
		void setTileOffset(int x, int y) { tile_offset_x_ = x; tile_offset_y_ = y; }
		void addImage(const TileImage& image);
		void setTerrainTypes(const std::vector<Terrain>& tt) { terrain_types_ = tt; }
		void setProperties(std::vector<Property>* props) { properties_.swap(*props); }
		void addTile(const Tile& t) { tiles_.emplace_back(t); }
	private:
		TileSet() = delete;

		int first_gid_;
		std::string name_;
		int tile_width_;
		int tile_height_;
		int spacing_;
		int margin_;
		int tile_offset_x_;
		int tile_offset_y_;
		std::vector<Property> properties_;
		std::vector<Terrain> terrain_types_;
		std::vector<Tile> tiles_;
	};

	class Map
	{
	public:
		Map();
		static MapPtr create();

		void setDimensions(int w, int h) { width_ = w; height_ = h; }
		void setTileDimensions(int w, int h) { tile_width_ = w; tile_height_ = h; }
		void setOrientation(Orientation o) { orientation_ = o; }
		void setRenderOrder(RenderOrder ro) { render_order_ = ro; }
		void setStaggerIndex(StaggerIndex si) { stagger_index_ = si; }
		void setStaggerDirection(StaggerDirection sd) { stagger_direction_ = sd; }
		void setHexsideLength(int length) { hexside_length_ = length; }
		void setBackgroundColor(const KRE::Color& color) { background_color_ = color; }
		void setProperties(std::vector<Property>* props) { properties_.swap(*props); }
		void addLayer(const Layer& layer) { layers_.emplace_back(layer); }
	private:
		int width_;
		int height_;
		int tile_width_;
		int tile_height_;
		Orientation orientation_;
		RenderOrder render_order_;
		StaggerIndex stagger_index_;
		StaggerDirection stagger_direction_;
		int hexside_length_;
		KRE::Color background_color_;

		std::vector<TileSet> tile_sets_;
		std::vector<Property> properties_;
		std::vector<Layer> layers_;
	};
}
