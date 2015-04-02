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
#include "WindowManager.hpp"

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

	void Map::draw(const KRE::WindowPtr& wnd) const
	{
		std::unique_ptr<KRE::Canvas::ModelManager> mm_ptr;
		if(orientation_ == Orientation::ISOMETRIC || orientation_ == Orientation::STAGGERED) {
			// shift half a screen to center in isometric mode.
			//mm_ptr = std::make_unique<KRE::Canvas::ModelManager>(wnd->width()/2, 0);
			mm_ptr = std::unique_ptr<KRE::Canvas::ModelManager>(new KRE::Canvas::ModelManager(wnd->width()/2, 0));
		}
		for(const auto& layer : layers_) {
			layer.draw(orientation_, render_order_);
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
		return p;
	}

	TilePtr Map::createTileInstance(int x, int y, int tile_gid)
	{
		for(auto it = tile_sets_.rbegin(); it != tile_sets_.rend(); ++it) {
			if(it->getFirstId() <= tile_gid) {
				const int local_id = tile_gid - it->getFirstId();
				const TileDefinition* td = it->getTileDefinition(local_id);
				auto p = getPixelPos(x, y) + point(it->getTileOffsetX(), it->getTileOffsetY());
				auto t = std::make_shared<Tile>(tile_gid, td != nullptr && td->getTexture() != nullptr ? td->getTexture() : it->getTexture());
				t->setDestRect(rect(p.x, p.y, it->getTileWidth(), it->getTileHeight()));
				// XXX if the TileDefinition(td) has it's on texture we use those source co-ords.
				t->setSrcRect(it->getImageRect(local_id));
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
		return rect(((local_id + spacing_) * tile_width_) % image_width_, (((local_id + spacing_) * tile_width_) / image_width_) * tile_height_, tile_width_, tile_height_);
	}

	const TileDefinition* TileSet::getTileDefinition(int local_id) const
	{
		for(auto& td : tiles_) {
			if(td.getLocalId() == local_id) {
				return &td;
			}
		}
		return nullptr;
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

	Layer::Layer(const std::string& name, int w, int h)
		: name_(name),
		  width_(w),
		  height_(h),
		  properties_(),
		  tiles_(),
		  opacity_(1.0f),
		  is_visible_(true),
		  add_x_(0),
		  add_y_(0)
	{
		tiles_.resize(height_);
		for(auto& r : tiles_) {
			r.resize(width_);
		}
	}

	void Layer::addTile(TilePtr t)
	{
		ASSERT_LOG(add_x_ < width_, "add_x_ >= width_: " << add_x_ << " : " << width_);
		ASSERT_LOG(add_y_ < height_, "add_y_ >= height_: " << add_y_ << " : " << height_);
		tiles_[add_y_][add_x_] = t;
		if(++add_x_ >= width_) {
			++add_y_;
			add_x_ = 0;
		}
	}

	void Layer::drawIsometic(RenderOrder render_order) const
	{
		int limit_x = width_ - 1;
		int limit_y = height_ - 1;
		int xend = 0;
		int ystart = 0;
		int xstart = 0;
		int yend = 0;
		while(true) {
			for(int x = xstart, y = ystart; x <= xend; ++x) {
				tiles_[y][x]->draw();
				//LOG_DEBUG("draw tile at: (" << x << "," << y << "), id: " << tiles_[y][x]->gid() << ", src_rect=" << tiles_[y][x]->getSrcRect());
				if(--y < yend) {
					y = yend;
				}
			}
			
			if(xstart == limit_x && ystart == limit_y) {
				break;
			}

			if(++xend > limit_x) {
				xend = limit_x;
				++xstart;
			}
			if(++ystart > limit_y) {
				ystart = limit_y;
				++yend;
			}
		}
	}

	void Layer::drawStaggered(RenderOrder render_order) const
	{
		ASSERT_LOG(false, "XXX Layer::drawHexagonal");
	}

	void Layer::drawOrthogonal(RenderOrder render_order) const
	{
		switch(render_order) {
			case RenderOrder::RIGHT_DOWN: {
				for(auto& r : tiles_) {
					for(auto c = r.rbegin(); c != r.rend(); ++c) {
						(*c)->draw();
					}
				}
				break;
			}
			case RenderOrder::RIGHT_UP: {
				for(auto r = tiles_.rbegin(); r != tiles_.rend(); ++r) {
					for(auto c = r->rbegin(); c != r->rend(); ++c) {
						(*c)->draw();
					}
				}
				break;
			}
			case RenderOrder::LEFT_DOWN: {
				for(auto& r : tiles_) {
					for(auto& c : r) {
						c->draw();
					}
				}
				break;
			}
			case RenderOrder::LEFT_UP: {
				for(auto r = tiles_.rbegin(); r != tiles_.rend(); ++r) {
					for(auto& c : *r) {
						c->draw();
					}
				}
				break;
			}
		}
	}

	void Layer::drawHexagonal(RenderOrder render_order) const
	{
		ASSERT_LOG(false, "XXX Layer::drawHexagonal");
	}

	void Layer::draw(Orientation orientation, RenderOrder render_order) const
	{
		if(!is_visible_) {
			return;
		}

		// XXX apply opacity change here.
		switch(orientation) {
			case Orientation::ORTHOGONAL:	drawOrthogonal(render_order); break;
			case Orientation::ISOMETRIC:	drawIsometic(render_order); break;
			case Orientation::STAGGERED:	drawStaggered(render_order); break;
			case Orientation::HEXAGONAL:	drawHexagonal(render_order); break;
		}
	}

	Tile::Tile(int gid, KRE::TexturePtr tex)
		: global_id_(gid),
		  dest_rect_(),
		  texture_(tex),
		  src_rect_(),
		  flipped_horizontally_(false),
		  flipped_vertically_(false),
		  flipped_diagonally_(false)
	{
	}

	void Tile::draw() const
	{
		auto canvas = KRE::Canvas::getInstance();
		ASSERT_LOG(texture_ != nullptr, "texture was nullptr");
		canvas->blitTexture(texture_, src_rect_, 0, dest_rect_);
	}
}
