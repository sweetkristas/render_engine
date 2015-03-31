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

#include "SDL.h"
#include "SDL_image.h"

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
		  tile_sets_()
	{
	}

	MapPtr Map::create()
	{
		return std::make_shared<Map>();
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
		  terrain_types_()
	{
	}

	void TileSet::setImage(const TileImage& tile_image)
	{
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

	Tile::Tile(uint32_t local_id)
		: local_id_(local_id),
		  terrain_(),
		  probability_(1.0f),
		  properties_(),
		  object_group_()
	{
		terrain_[0] = terrain_[1] = terrain_[2] = terrain_[3] = -1;
	}
	
	void Tile::addImage(const TileImage& image)
	{
		ASSERT_LOG(false, "XXX");
	}

	Layer::Layer(const std::string& name)
		: name_(name),
		  properties_(),
		  tile_data_(),
		  opacity_(1.0f),
		  is_visible_(true)
	{
	}
}
