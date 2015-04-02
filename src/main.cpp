#include <cstddef>
#include <deque>
#include <list>
#include <vector>

#include <glm/gtc/type_precision.hpp>

#include "AttributeSet.hpp"
#include "json.hpp"
#include "profile_timer.hpp"
#include "SDLWrapper.hpp"
#include "CameraObject.hpp"
#include "Canvas.hpp"
#include "Font.hpp"
#include "LightObject.hpp"
#include "ParticleSystem.hpp"
#include "Renderable.hpp"
#include "RenderManager.hpp"
#include "RenderQueue.hpp"
#include "RenderTarget.hpp"
#include "SceneGraph.hpp"
#include "SceneNode.hpp"
#include "Shaders.hpp"
#include "Surface.hpp"
#include "TexPack.hpp"
#include "UniformBuffer.hpp"
#include "WindowManager.hpp"
#include "VGraph.hpp"

#include "tmx_reader.hpp"

#include "variant_utils.hpp"

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace
{	
	struct vertex_color
	{
		vertex_color() : vertex(0.0f), color(0) {}
		vertex_color(const glm::vec2& v, const glm::u8vec4& c) : vertex(v), color(c) {}
		glm::vec2 vertex;
		glm::u8vec4 color;
	};

	class SquareRenderable : public KRE::SceneObject
	{
	public:
		SquareRenderable() : KRE::SceneObject("square") {
			using namespace KRE;

			setShader(ShaderProgram::getProgram("attr_color_shader"));

			auto ab = DisplayDevice::createAttributeSet(false, false, false);
			auto pc = new Attribute<vertex_color>(AccessFreqHint::DYNAMIC, AccessTypeHint::DRAW);
			pc->addAttributeDesc(AttributeDesc(AttrType::POSITION, 2, AttrFormat::FLOAT, false, sizeof(vertex_color), offsetof(vertex_color, vertex)));
			pc->addAttributeDesc(AttributeDesc(AttrType::COLOR, 4, AttrFormat::UNSIGNED_BYTE, true, sizeof(vertex_color), offsetof(vertex_color, color)));
			ab->addAttribute(AttributeBasePtr(pc));
			ab->setDrawMode(DrawMode::TRIANGLE_STRIP);
			addAttributeSet(ab);

			std::vector<vertex_color> vertices;
			vertices.emplace_back(glm::vec2(0.0f,0.0f), glm::u8vec4(255,0,0,255));
			vertices.emplace_back(glm::vec2(0.0f,100.0f), glm::u8vec4(0,255,0,255));
			vertices.emplace_back(glm::vec2(100.0f,0.0f), glm::u8vec4(0,0,255,255));
			vertices.emplace_back(glm::vec2(100.0f,100.0f), glm::u8vec4(255,0,0,255));
			ab->setCount(vertices.size());
			pc->update(&vertices, pc->end());

			//std::vector<uint8_t> indices;
			//indices.emplace_back(0);
			//indices.emplace_back(1);
			//indices.emplace_back(2);
			//indices.emplace_back(3);
			//ab->UpdateIndicies(indices);

			//pc->Update(vertices, pc->begin()+5);

			/*
			// first parameter is a hint to wether the buffer should be hardware backed.
			// second parameter is whether we are using indexed drawing.
			// third parameter is wether this is instanced.
			auto ab = DisplayDevice::CreateAttributeSet(false, true, false);
			// If these were instanced then there is an extra parameter on the end, which defaults to 1.
			// A 0 indictes that there is no advancement per instance
			auto pos = ab->CreateAttribute();
			pos->AddAttributeDescription(AttributeDesc(AttributeDesc::Type::POSITION, 2, AttributeDesc::VariableType::FLOAT, false));
			auto col = ab->CreateAttribute();
			col->AddAttributeDescription(AttributeDesc(AttributeDesc::Type::COLOR, 4, AttributeDesc::VariableType::UNSIGNED_BYTE, true));
			ab->SetDrawMode(AttributeSet::DrawMode::TRIANGLES);
			AddAttributeSet(ab);

			std::vector<glm::vec2> vertices;
			vertices.emplace_back(0.0f, 0.0f);
			vertices.emplace_back(0.0f, 1.0f);
			vertices.emplace_back(1.0f, 0.0f);
			vertices.emplace_back(1.0f, 1.0f);
			//ab->SetCount(vertices.size());
			pos->Update(&vertices[0], 0, vertices.size() * sizeof(glm::vec2));

			std::vector<glm::u8vec4> colors;
			colors.emplace_back(255,0,0,255);
			colors.emplace_back(0,255,0,255);
			colors.emplace_back(0,0,255,255);
			colors.emplace_back(255,0,0,255);			
			col->Update(&colors[0], 0, colors.size() * sizeof(glm::u8vec4));			

			glm::u8vec4 x;

			std::vector<uint8_t> indices;
			indices.emplace_back(0);
			indices.emplace_back(1);
			indices.emplace_back(2);
			indices.emplace_back(2);
			indices.emplace_back(1);
			indices.emplace_back(3);
			ab->UpdateIndicies(indices);
			*/

			//setColor(255,255,255);
			setOrder(0);
		}
		virtual ~SquareRenderable() {}
	private:
		SquareRenderable(const SquareRenderable&);
		SquareRenderable& operator=(const SquareRenderable&);
	};
	typedef std::shared_ptr<SquareRenderable> SquareRenderablePtr;
}

struct SimpleTextureHolder : public KRE::Blittable
{
	SimpleTextureHolder(const std::string& filename) {
		using namespace KRE;
		setColor(1.0f, 1.0f, 1.0f, 1.0f);
		auto tex = Texture::createTexture(filename, TextureType::TEXTURE_2D, 4);
		tex->setFiltering(-1, Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::POINT);
		tex->setAddressModes(-1, Texture::AddressMode::BORDER, Texture::AddressMode::BORDER);
		setTexture(tex);
	}
};

struct FreeTextureHolder : public KRE::SceneObject
{
	FreeTextureHolder(KRE::SurfacePtr surface)
		: KRE::SceneObject("FreeTextureHolder") 
	{
		using namespace KRE;
		auto tex = Texture::createTexture(surface);
		//tex->setFiltering(Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::POINT);
		//tex->setAddressModes(Texture::AddressMode::BORDER, Texture::AddressMode::BORDER);
		setTexture(tex);
		init();
	}
	FreeTextureHolder(KRE::TexturePtr tex)
		: KRE::SceneObject("FreeTextureHolder") 
	{
		using namespace KRE;
		setTexture(tex);
		init();
	}
	FreeTextureHolder(const std::string& filename)
		: KRE::SceneObject("FreeTextureHolder") 
	{
		using namespace KRE;
		auto tex = Texture::createTexture(filename, TextureType::TEXTURE_2D, 4);
		//tex->setFiltering(Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::POINT);
		tex->setAddressModes(-1, Texture::AddressMode::BORDER, Texture::AddressMode::BORDER);
		setTexture(tex);
		init();
	}
	void init()
	{
		setDrawRect(rect(0, 0, getTexture()->surfaceWidth(), getTexture()->surfaceHeight()));
		using namespace KRE;
		setColor(1.0f, 1.0f, 1.0f, 1.0f);
		auto as = DisplayDevice::createAttributeSet();
		attribs_.reset(new Attribute<vertex_texcoord>(AccessFreqHint::DYNAMIC, AccessTypeHint::DRAW));
		attribs_->addAttributeDesc(AttributeDesc(AttrType::POSITION, 2, AttrFormat::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, vtx)));
		attribs_->addAttributeDesc(AttributeDesc(AttrType::TEXTURE,  2, AttrFormat::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, tc)));
		as->addAttribute(AttributeBasePtr(attribs_));
		as->setDrawMode(DrawMode::TRIANGLE_STRIP);
		
		addAttributeSet(as);
	}
	void preRender(const KRE::WindowPtr& wm) override
	{
		const float offs_x = 0.0f;
		const float offs_y = 0.0f;
		// XXX we should only do this if things changed.
		const float vx1 = draw_rect_.x() + offs_x;
		const float vy1 = draw_rect_.y() + offs_y;
		const float vx2 = draw_rect_.x2() + offs_x;
		const float vy2 = draw_rect_.y2() + offs_y;

		const rectf& r = getTexture()->getSourceRectNormalised();

		std::vector<KRE::vertex_texcoord> vertices;
		vertices.emplace_back(glm::vec2(vx1,vy1), glm::vec2(r.x(),r.y()));
		vertices.emplace_back(glm::vec2(vx2,vy1), glm::vec2(r.x2(),r.y()));
		vertices.emplace_back(glm::vec2(vx1,vy2), glm::vec2(r.x(),r.y2()));
		vertices.emplace_back(glm::vec2(vx2,vy2), glm::vec2(r.x2(),r.y2()));
		getAttributeSet().back()->setCount(vertices.size());
		attribs_->update(&vertices);
	}
	template<typename T>
	void setDrawRect(const geometry::Rect<T>& r) {
		draw_rect_ = r.template as_type<float>();
	}
private:
	std::shared_ptr<KRE::Attribute<KRE::vertex_texcoord>> attribs_;
	rectf draw_rect_;
};

void set_alpha_masks()
{
	using namespace KRE;
	std::vector<Color> alpha_colors;

	auto surf = Surface::create("alpha-colors.png");
	surf->iterateOverSurface([&alpha_colors](int x, int y, int r, int g, int b, int a) {
		alpha_colors.emplace_back(r, g, b);
		LOG_INFO("Added alpha color: (" << r << "," << g << "," << b << ")");	
	});

	Surface::setAlphaFilter([=](int r, int g, int b) {
		for(auto& c : alpha_colors) {
			if(c.ri() == r && c.gi() == g && c.bi() == b) {
				return true;
			}
		}
		return false;
	});
}

struct water_distort_uniforms
{
	unsigned int texture;
	glm::mat4 mvp;
	float cycle;
	glm::vec4 sprite_area;
	glm::vec4 draw_area;
	float intensity;
	glm::vec4 water_area[2];
};


void texture_packing_test()
{
	using namespace KRE;
	const std::string tile_file1 = "brown-rock1.png";
	const std::string tile_file2 = "brown-rock1.png";
	SurfacePtr s1 = Surface::create(tile_file1);
	SurfacePtr s2 = Surface::create(tile_file2);
	std::vector<SurfaceAreas> sa;
	sa.emplace_back(s1);
	sa.back().addRect(16, 0, 16, 32);
	sa.back().addRect(32, 0, 16, 32);
	sa.emplace_back(s2);
	sa.back().addRect(0, 0, 16, 32);
	sa.back().addRect(16, 0, 16, 32);
	sa.back().addRect(0, 32, 16, 32);
	sa.back().addRect(16, 32, 16, 32);
	sa.back().addRect(32, 32, 16, 32);
	sa.back().addRect(48, 32, 16, 32);

	Packer tp(sa, 2048, 2048);
	for(auto& r : tp) {
		LOG_DEBUG("New rect: " << r);
	}
	// output surface available as tp.getOutputSurface()
}

int main(int argc, char *argv[])
{
#ifdef _MSC_VER
	SetProcessDPIAware();
#endif

	std::list<double> smoothed_time;
	double cumulative_time = 0.0;
	int cnt = 0;

	using namespace KRE;

	SDL::SDL_ptr manager(new SDL::SDL());

	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

	WindowManager wm("SDL");

	variant_builder hints;
	hints.add("renderer", "opengl");
	hints.add("dpi_aware", true);
	hints.add("use_vsync", true);
	int neww = 1600, newh = 900;
	//if(!autoWindowSize(neww, newh)) {
	//	LOG_DEBUG("Couldn't get automatic window size. Defaulting to " << neww << "x" << newh);
	//}
	LOG_DEBUG("Creating window of size: " << neww << "x" << newh);
	auto main_wnd = wm.createWindow(neww, newh, hints.build());
	main_wnd->enableVsync(true);
	const float aspect_ratio = static_cast<float>(neww) / newh;

	std::map<std::string, std::string> font_paths;
#if defined(__linux__)
	LOG_DEBUG("setting image file filter to 'images/'");
	Surface::setFileFilter(FileFilterType::LOAD, [](const std::string& fname) { return "images/" + fname; });
	Surface::setFileFilter(FileFilterType::SAVE, [](const std::string& fname) { return "images/" + fname; });
	
	font_paths["FreeSans.ttf"] = "data/fonts/FreeSans.ttf";
#else
	LOG_DEBUG("setting image file filter to '../images/'");
	Surface::setFileFilter(FileFilterType::LOAD, [](const std::string& fname) { return "../images/" + fname; });
	Surface::setFileFilter(FileFilterType::SAVE, [](const std::string& fname) { return "../images/" + fname; });

	font_paths["FreeSans.ttf"] = "../data/fonts/FreeSans.ttf";
#endif
	Font::setAvailableFonts(font_paths);

	/*auto surf = Surface::create("test_image.png");
	for(auto col : *surf) {
		LOG_DEBUG(col.red << "," << col.green << "," << col.blue << "," << col.alpha);
	}*/
	
	set_alpha_masks();	
	
	// XXX should a scenegraph be created from a specific window? It'd solve a couple of issues
	SceneGraphPtr scene = SceneGraph::create("main");
	SceneNodePtr root = scene->getRootNode();
	root->setNodeName("root_node");
	/*auto scenecam = std::make_shared<Camera>("cam0", 0, neww, 0, newh);
	scenecam->attachFrustum(std::make_shared<Frustum>());
	root->attachCamera(scenecam);
	auto sunlight = std::make_shared<Light>("the_sun", glm::vec3(1.0f, 1.0f, 1.0f));
	sunlight->setAmbientColor(Color(1.0f,1.0f,1.0f,1.0f));
	root->attachLight(0, sunlight);*/

	DisplayDevice::getCurrent()->setDefaultCamera(std::make_shared<Camera>("ortho1", 0, neww, 0, newh));
	
	/*SquareRenderablePtr square(std::make_shared<SquareRenderable>());
	square->setPosition(600.0f, 400.0f);
	square->setScale(2.0f,2.0f);
	root->attachObject(square);

	auto rman = std::make_shared<RenderManager>();
	auto rq = rman->addQueue(0, "opaques");

	auto cairo_canvas = Vector::Context::CreateInstance("cairo", 512, 512);
	cairo_canvas->SetSourceColor(0.0, 1.0, 0.0);
	cairo_canvas->Paint();
	cairo_canvas->Fill();
	auto path = cairo_canvas->NewPath();
	path->Circle(256, 256, 100);
	cairo_canvas->AddPath(path);
	cairo_canvas->SetSourceColor(0.0, 0.0, 1.0);
	cairo_canvas->Fill();
	auto text = cairo_canvas->NewPath();
	text->MoveTo(10, 10);
	text->TextPath("ABCDabcde");
	cairo_canvas->AddPath(text);
	cairo_canvas->Fill();
	cairo_canvas->setOrder(5);
	cairo_canvas->setPosition(256.0f,256.0f);
	cairo_canvas->setColor(1.0f,1.0f,1.0f,1.0f);
	root->attachObject(cairo_canvas);*/

#if defined(__linux__)
	std::string psys_test_file = "data/psystem1.cfg";
#else
	std::string psys_test_file = "../data/psystem1.cfg";
#endif
	//auto psystem = scene->createNode("particle_system_container", json::parse_from_file(psys_test_file));
	//auto particle_cam = std::make_shared<Camera>("particle_cam");
	//particle_cam->lookAt(glm::vec3(0.0f, 10.0f, 20.0f), glm::vec3(0.0f), glm::vec3(0.0f,1.0f,0.0f));
	//psystem->attachCamera(particle_cam);
	//root->attachNode(psystem);
	//auto rt = DisplayDevice::RenderTargetInstance(400, 300);
	//rt->SetClearColor(0.0f,0.0f,0.0f,0.0f);
	//rt->SetDrawRect(rect(400,300,400,300));
	//rt->Create();
	//psystem->AttachRenderTarget(rt);
	//root->AttachObject(rt);

#if defined(__linux__)
	std::string shader_test_file = "data/shaders.cfg";
#else
	std::string shader_test_file = "../data/shaders.cfg";
#endif
	ShaderProgram::loadFromVariant(json::parse_from_file(shader_test_file));

/*	auto tex = std::make_shared<SimpleTextureHolder>("card-back.png");
	tex->setDrawRect(rectf(0.0f,0.0f,146.0f,260.0f));
	tex->setPosition(146.0f/2.0f, newh-130.0f);
	tex->setOrder(10);
	root->attachObject(tex);

	// test cloning
	auto new_tex = std::make_shared<SimpleTextureHolder>(*tex);
	new_tex->setOrder(tex->getOrder()+1);
	new_tex->setPosition(neww/2.0f, newh/2.0f);
	root->attachObject(new_tex);

	auto free_surf = Surface::create("editor-tools.png", SurfaceFlags::NO_ALPHA_FILTER);
	//auto free_tex = std::make_shared<FreeTextureHolder>("editor-tools.png");
	auto free_tex = std::make_shared<FreeTextureHolder>(free_surf);
	free_tex->setDrawRect(rectf(0.0f,0.0f,256.0f,256.0f));
	free_tex->setPosition(neww - 256.0f, 0.0f);

	// Test that cache surfaces and textures is working.
	std::vector<std::shared_ptr<FreeTextureHolder>> texture_list;		
	for(int n = 0; n != 100; ++n) {
		auto surf2 = Surface::create("editor-tools.png");
		auto tex1 = std::make_shared<FreeTextureHolder>(surf2);
		texture_list.emplace_back(tex1);
	}

	auto fnt_surf = Font::getInstance()->renderText("test", Color::colorOrange(), 20, false, "FreeSans.ttf");
	auto text_tex = std::make_shared<FreeTextureHolder>(fnt_surf);
	//text_tex->setDrawRect(rect(0, 0, fnt_surf->surfaceWidth(), fnt_surf->surfaceHeight()));
	text_tex->setPosition(fnt_surf->surfaceWidth()/2, fnt_surf->surfaceHeight()/2);
	//text_tex->setPosition(fnt_surf->surfaceWidth()/2, fnt_surf->surfaceHeight()/2);
	//text_tex->setDrawRect(rectf(0.0f, 0.0f, fnt_surf->surfaceWidth(), fnt_surf->surfaceHeight()));
	//text_tex->setDrawRect(rectf(0.0f,0.0f,256.0f,256.0f));
	//text_tex->setPosition(128.0f,128.0f);

	float angle = 1.0f;
	float angle_step = 0.5f;

	auto canvas = Canvas::getInstance();
	canvas->setDimensions(800, static_cast<int>(800/aspect_ratio));

	auto canvas_texture = Texture::createTexture("widgets.png");
	canvas_texture->setFiltering(-1, Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::NONE);
	
	texture_packing_test();

	// render target test.
	auto rt1 = RenderTarget::create(300, 300);
	{
		RenderTarget::RenderScope render_scope(rt1);
		//rt1->setClearColor(Color::colorRed());
		rt1->setClearColor(Color(0,0,0,0));
		rt1->clear();
		canvas->setDimensions(300, 300);
		std::vector<glm::u8vec4> circle_colors2;
		generate_color_wheel(60, &circle_colors2, Color(0,0,0,0), 0.8f, 0.8f);
		canvas->drawSolidCircle(point(150, 150), 150.0f, circle_colors2);
		canvas->drawSolidRect(rect(50, 50, 200, 200), Color::colorGrey());
		canvas->drawSolidCircle(point(150, 150), 20.0f, Color::colorCyan());
		rt1->setCentre(Blittable::Centre::TOP_LEFT);
		rt1->preRender(main_wnd);
		canvas->setDimensions(800, static_cast<int>(800/aspect_ratio));//(neww, newh);
	}

	auto test1 = std::make_shared<FreeTextureHolder>("cave2.png");
	test1->setPosition(0,512);
	auto palette_tex = std::make_shared<FreeTextureHolder>("cave2.png");
	palette_tex->getTexture()->addPalette(1, Surface::create("cave_pearl.png"));
	palette_tex->getTexture()->addPalette(2, Surface::create("cave_mossy.png"));
	
	//auto test1 = std::make_shared<FreeTextureHolder>("sand.png");
	//test1->setPosition(0,512);
	//auto palette_tex = std::make_shared<FreeTextureHolder>("sand.png");
	//palette_tex->getTexture()->addPalette(Surface::create("seaside_stormy.png"));

	palette_tex->getTexture()->setPalette(0);
	//auto palette_tex = std::make_shared<FreeTextureHolder>("checkerboard1.png");
	//palette_tex->getTexture()->addPalette(Surface::create("checkerboard-palette.png"));
	palette_tex->setPosition(0, 0);
	auto palette_cam = std::make_shared<Camera>("palette_cam", 0, 800, 0, 800);
	palette_tex->setCamera(palette_cam);
	test1->setCamera(palette_cam);
	
	//auto rt2 = RenderTarget::create(palette_tex->getTexture()->width(), palette_tex->getTexture()->height());
	//rt2->setCentre(Blittable::Centre::TOP_LEFT);

	// Test code for setting shader uniforms.
	auto water_shader = ShaderProgram::getProgram("water_distort");
	water_distort_uniforms wd;
	uniform_mapping wd_mapping;
	UniformBuffer<water_distort_uniforms> water_uniforms("anura_uniforms", wd);
	wd_mapping["u_anura_tex_map"] = offsetof(water_distort_uniforms, texture);
	wd_mapping["u_anura_mvp_matrix"] = offsetof(water_distort_uniforms, mvp);
	wd_mapping["u_anura_cycle"] = offsetof(water_distort_uniforms, cycle);
	wd_mapping["u_anura_sprite_area"] = offsetof(water_distort_uniforms, sprite_area);
	wd_mapping["u_anura_draw_area"] = offsetof(water_distort_uniforms, draw_area);
	wd_mapping["u_intensity"] = offsetof(water_distort_uniforms, intensity);
	wd_mapping["u_water_area"] = offsetof(water_distort_uniforms, water_area);
	water_uniforms.setMapping(&wd_mapping);
	auto water_tex = SimpleTextureHolder("checkerboard1.png");
	water_tex.setPosition(neww/2, newh/2);
	water_tex.setShader(water_shader);
	water_tex.addUniformBuffer(std::move(water_uniforms));
*/

	auto tiled_map = tiled::Map::create();
	tiled::TmxReader tmx_reader(tiled_map);
	//tmx_reader.parseFile("data/isometric_grass_and_water.tmx");
	//tmx_reader.parseFile("data/hex-mini.tmx");
	tmx_reader.parseFile("data/sewer_tileset.tmx");

	SDL_Event e;
	bool done = false;
	profile::timer timer;
	while(!done) {
		timer.start();
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_KEYUP && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				done = true;
			} else if(e.type == SDL_KEYDOWN) {
				LOG_DEBUG("KEY PRESSED: " << SDL_GetKeyName(e.key.keysym.sym) << " : " << e.key.keysym.sym << " : " << e.key.keysym.scancode);
			} else if(e.type == SDL_QUIT) {
				done = true;
			}
		}

		main_wnd->clear(ClearFlags::ALL);

		// Called once a cycle before rendering.
		//scene->process(SDL_GetTicks() / 1000.0f);

		/*tex->setRotation(angle, glm::vec3(0.0f,0.0f,1.0f));
		new_tex->setRotation(360.0f - angle, glm::vec3(0.0f,0.0f,1.0f));
		cairo_canvas->setRotation(angle, glm::vec3(0.0f,0.0f,1.0f));
		angle += angle_step;
		while(angle >= 360.0f) {
			angle -= 360.0f;
		}

		canvas->blitTexture(rt1->getTexture(), 0.0f, 800-rt1->getTexture()->surfaceWidth(), 0);

		{
			//RenderTarget::RenderScope render_scope(rt2);
			//rt2->setClearColor(Color(0,0,0,0));
			//rt2->clear();
			palette_tex->preRender(main_wnd);
			main_wnd->render(palette_tex.get());

			test1->preRender(main_wnd);
			main_wnd->render(test1.get());
		}
		//rt2->preRender(main_wnd);
		//canvas->blitTexture(rt2->getTexture(), 0.0f, 0, 0);

		static int counter_pal = 0;
		static int pal = -1;
		if(++counter_pal >= 60*2) {
			counter_pal = 0;
			if(++pal >= 2) {
				pal = -1;
			}
			palette_tex->getTexture()->setPalette(pal);
		}

		water_tex.preRender(main_wnd);
		main_wnd->render(&water_tex);*/

		tiled_map->draw(main_wnd);

		/*scene->renderScene(rman);
		rman->render(main_wnd);

		free_tex->preRender(main_wnd);
		main_wnd->render(free_tex.get());

		text_tex->preRender(main_wnd);
		main_wnd->render(text_tex.get());*/

		//canvas->drawSolidCircle(point(canvas->width()/2, canvas->height()/2), 50.0f, Color::colorGold());
		//canvas->drawHollowCircle(pointf(400.0f, 400.0f), 150.0f, 150.0f-1.0f,Color::colorAqua());

		//std::vector<glm::u8vec4> circle_colors;
		//generate_color_wheel(60, &circle_colors, Color(0,0,0,0), 0.1f, 0.1f);
		//canvas->drawSolidCircle(point(400, 300), 150.0f, circle_colors);


		/*{
			Canvas::ModelManager mm(0, 0, 10.0f, 1.0f);
			canvas->drawSolidRect(rect(600, 400, 100, 100), Color::colorCoral());
			{
				Canvas::ModelManager mm(500, 300, 20.0f, 3.0f);*/
				/*mm.setIdentity();
				mm.translate(700, 500);
				mm.translate(-200, -200);
				mm.rotate(20);
				mm.scale(1.5f);
				mm.scale(2.0f);*/
				/*canvas->drawSolidRect(rect(0, 0, 100, 100), Color::colorChartreuse());
			}
		}*/

		/*canvas->blitTexture(canvas_texture, 
			rect(3,4,56,22), 
			0.0f, 
			//rect(800-56, 0, 56, 22), 
			rect(0,0,112,44),
			Color(1.0f,1.0f,1.0f,0.5f));
		canvas->drawSolidRect(rect(100, 100, 100, 100), Color("red"));
		canvas->drawHollowRect(rect(125, 125, 50, 50), Color("blue"));
		canvas->drawLine(point(95,95), point(205,205), Color("yellow"));
		std::vector<glm::vec2> varray;
		//varray.emplace_back(400, 400);
		//varray.emplace_back(500, 500);
		varray.emplace_back(400, 400);
		varray.emplace_back(500, 400);
		varray.emplace_back(400, 400);
		varray.emplace_back(500, 500);
		varray.emplace_back(400, 400);
		varray.emplace_back(400, 500);
		canvas->drawLines(varray, 10.0f, Color("pink"));*/

		double t1 = timer.check();
		if(t1 < 1.0/50.0) {
		//	SDL_Delay(Uint32((1.0/50.0-t1)*1000.0));
		}
		double t = timer.check();

		smoothed_time.push_back(t);
		cumulative_time += t;
		if(++cnt > 10) {
			cnt = 0;
			//LOG_DEBUG("FPS: " << (smoothed_time.size()/cumulative_time) << ", Time: " << t1*1000.0);
			LOG_DEBUG("Draw Time: " << (t * 1000000.0) << " us.");
		}
		if(smoothed_time.size() > 50) {
			cumulative_time -= smoothed_time.front();
			smoothed_time.pop_front();
		}

		main_wnd->swap();
	}
	return 0;
}
