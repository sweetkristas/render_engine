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

#define HAVE_M_PI
#include "SDL.h"
#include "SDL_image.h"

#include "Canvas.hpp"

#include "asserts.hpp"
#include "tiled.hpp"

namespace tiled
{
	Map::Map()
		: width_(-1),
		  height_(-1),
		  tile_width_(-1),
		  tile_height_(-1),
		  orientation_(Orientation::ORTHOGONAL),
		  render_order_(RenderOrder::RIGHT_DOWN),
		  stagger_index_(StaggerIndex::EVEN),
		  stagger_direction_(StaggerDirection::ROWS),
		  hexside_length_(-1),
		  background_color_(128, 128, 128),
		  tile_sets_(),
		  properties_(),
		  layers_()
	{
	}

	MapPtr Map::create()
	{
		return std::make_shared<Map>();
	}

	void Map::draw() const
	{
		for(const auto& layer : layers_) {
			layer.draw();
		}
	}

	point Map::getPixelPos(int x, int y) const
	{
		point p;
		switch(orientation_) {
		case Orientation::ORTHOGONAL:
			p = point(tile_width_ * x, tile_height_ * y);
			break;
		case Orientation::ISOMETRIC:
			p = point((x - y) * tile_width_/2, (x + y) * tile_height_/2);
			break;
		case Orientation::STAGGERED:
			if(stagger_index_ == StaggerIndex::ODD) {
				p = point((x + y % 2) * tile_width_, y * tile_height_);
			} else {
				p = point((x + 1 - (y % 2)) * tile_width_, y * tile_height_);
			};
			break;
		case Orientation::HEXAGONAL: {
			const int length_one_and_a_half = (3 * hexside_length_) / 2;
			const int even_length = hexside_length_ * (2 * x + y % 2);
			const int odd_length = hexside_length_ * (2 * x + 1 - (y % 2));
			if(stagger_index_ == StaggerIndex::ODD) {
				if(stagger_direction_ == StaggerDirection::ROWS) {
					// odd-r
					p = point(even_length, length_one_and_a_half);
				} else {
					// odd-q
					p = point(odd_length, length_one_and_a_half);
				}
			} else {
				if(stagger_direction_ == StaggerDirection::ROWS) {
					// even-r
					p = point(length_one_and_a_half, even_length);
				} else {
					// even-q
					p = point(length_one_and_a_half, odd_length);
				}
			}
			break;
		}
		default:
			ASSERT_LOG(false, "Invalid case for orientation: " << static_cast<int>(orientation_));
			break;
		}
		LOG_DEBUG("pixel pos for (" << x << "," << y << "): " << p);
		return p;
	}

	TilePtr Map::createTileInstance(int x, int y, int tile_gid)
	{
		for(auto it = tile_sets_.rbegin(); it != tile_sets_.rend(); ++it) {
			if(it->getFirstId() <= tile_gid) {
				auto& td = it->getTileDefinition(tile_gid - it->getFirstId());
				auto p = getPixelPos(x, y) + point(it->getTileOffsetX(), it->getTileOffsetY());
				auto t = std::make_shared<Tile>(td);
				t->setDestRect(rect(p.x, p.y, it->getTileWidth(), it->getTileHeight()));
				// XXX if the TileDefinition(td) has it's on texture we use those source co-ords.
				t->setSrcRect(it->getImageRect(td.getLocalId()));
				return t;
			}
		}
		ASSERT_LOG(false, "Unable to match a tile with gid of: " << tile_gid);
		return nullptr;
	}

	TileSet::TileSet(int first_gid)
		: first_gid_(first_gid),
		  name_(),
		  tile_width_(-1),
		  tile_height_(-1),
		  spacing_(0),
		  margin_(0),
		  tile_offset_x_(0),
		  tile_offset_y_(0),
		  properties_(),
		  terrain_types_(),
		  texture_(),
		  image_width_(-1),
		  image_height_(-1)
	{
	}

	rect TileSet::getImageRect(int local_id) const
	{		
		// XXX we need to check use of margin and spacing for correctness
		return rect(((local_id + spacing_) * tile_width_) % image_width_, ((local_id + spacing_) * tile_width_) / image_width_, tile_width_, tile_height_);
	}

	const TileDefinition& TileSet::getTileDefinition(int local_id) const
	{
		for(auto& td : tiles_) {
			if(td.getLocalId() == local_id) {
				return td;
			}
		}
		ASSERT_LOG(false, "No tile definition found for local id of: " << local_id << " in tile_set: " << name_ << ", gid: " << first_gid_);
		static TileDefinition dummy(0);
		return dummy;
	}

	void TileSet::setImage(const TileImage& tile_image)
	{
		image_width_ = tile_image.getWidth();
		image_height_ = tile_image.getHeight();
		texture_ = tile_image.getTexture();
	}

	TileImage::TileImage()
		: format_(ImageFormat::NONE),
		  data_(),
		  source_(),
		  has_transparent_color_set_(false),
		  transparent_color_(),
		  width_(-1),
		  height_(-1)
	{
	}

	KRE::TexturePtr TileImage::getTexture() const
	{
		if(!source_.empty()) {
			// is a file
			return KRE::Texture::createTexture(source_);
		}
		// is raw data.
		SDL_Surface* image = IMG_Load_RW(SDL_RWFromConstMem(&data_[0], static_cast<int>(data_.size())), 1);
		ASSERT_LOG(image != nullptr, "Unable to create a surface from suplied data: " << SDL_GetError());
		ASSERT_LOG(image->format != nullptr, "No format attached to the surface.");
		int bpp = image->format->BytesPerPixel;
		Uint32 rmask = image->format->Rmask;
		Uint32 gmask = image->format->Gmask;
		Uint32 bmask = image->format->Bmask;
		Uint32 amask = image->format->Amask;
		KRE::SurfacePtr surf = KRE::Surface::create(image->w, image->h, bpp, image->pitch, rmask, gmask, bmask, amask, image->pixels);		
		SDL_FreeSurface(image);
		return KRE::Texture::createTexture(surf);
	}

	TileDefinition::TileDefinition(uint32_t local_id)
		: local_id_(local_id),
		  terrain_(),
		  probability_(1.0f),
		  properties_(),
		  object_group_(),
		  texture_()
	{
		terrain_[0] = terrain_[1] = terrain_[2] = terrain_[3] = -1;
	}
	
	void TileDefinition::addImage(const TileImage& image)
	{
		texture_ = image.getTexture();
	}

	Layer::Layer(const std::string& name)
		: name_(name),
		  properties_(),
		  tiles_(),
		  opacity_(1.0f),
		  is_visible_(true)
	{
	}

	void Layer::draw() const
	{
		if(!is_visible_) {
			return;
		}

		// XXX apply opacity change here.
		for(const auto& t : tiles_) {
			t->draw();
		}
	}

	Tile::Tile(const TileDefinition& td)
		: dest_rect_(),
		  texture_(),
		  src_rect_(),
		  flipped_horizontally_(false),
		  flipped_vertically_(false),
		  flipped_diagonally_(false),
		  tile_def_(td)
	{
	}

	void Tile::draw() const
	{
		// XXX
		auto canvas = KRE::Canvas::getInstance();
		ASSERT_LOG(tile_def_.getTexture() != nullptr, "texture was nullptr");
		canvas->blitTexture(tile_def_.getTexture(), dest_rect_, 0, src_rect_);
	}
}
