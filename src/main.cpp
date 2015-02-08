#include <cstddef>
#include <deque>
#include <list>
#include <vector>

#include <glm/gtc/type_precision.hpp>

#include "kre/AttributeSet.hpp"
#include "json.hpp"
#include "profile_timer.hpp"
#include "SDLWrapper.hpp"
#include "kre/CameraObject.hpp"
#include "kre/Canvas.hpp"
#include "kre/LightObject.hpp"
#include "kre/ParticleSystem.hpp"
#include "kre/Renderable.hpp"
#include "kre/RenderManager.hpp"
#include "kre/RenderQueue.hpp"
#include "kre/RenderTarget.hpp"
#include "kre/SceneGraph.hpp"
#include "kre/SceneNode.hpp"
#include "kre/WindowManager.hpp"
#include "kre/VGraph.hpp"

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
			pc->update(&vertices);

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
	protected:
		void doAttach(const KRE::DisplayDevicePtr& dd, KRE::DisplayDeviceDef* def) override {
			def->setHint("shader", "attr_color_shader");
		}
	private:
		SquareRenderable(const SquareRenderable&);
		SquareRenderable& operator=(const SquareRenderable&);
	};
	typedef std::shared_ptr<SquareRenderable> SquareRenderablePtr;
}

class SimpleTextureHolder : public KRE::Blittable
{
public:
	SimpleTextureHolder(const std::string& filename) {
		using namespace KRE;
		setColor(1.0f, 1.0f, 1.0f, 1.0f);
		auto tex = DisplayDevice::createTexture(filename, Texture::Type::TEXTURE_2D, 4);
		tex->setFiltering(Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::POINT);
		tex->setAddressModes(Texture::AddressMode::BORDER, Texture::AddressMode::BORDER);
		setTexture(tex);
	}
private:
};

int main(int argc, char *argv[])
{
	std::list<double> smoothed_time;
	double cumulative_time = 0.0;
	int cnt = 0;

	using namespace KRE;

	SDL::SDL_ptr manager(new SDL::SDL());

	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

	WindowManagerPtr main_wnd = WindowManager::createInstance("SDL", "opengl");
	main_wnd->enableVsync(true);
	main_wnd->createWindow(800,600);

	// XXX should a scenegraph be created from a specific window? It'd solve a couple of issues
	SceneGraphPtr scene = SceneGraph::create("main");
	SceneNodePtr root = scene->getRootNode();
	root->setNodeName("root_node");
	auto scenecam = std::make_shared<Camera>("cam0", 0, 800, 0, 600);
	scenecam->attachFrustum(std::make_shared<Frustum>());
	root->attachCamera(scenecam);
	auto sunlight = std::make_shared<Light>("the_sun", glm::vec3(1.0f, 1.0f, 1.0f));
	sunlight->setAmbientColor(Color(1.0f,1.0f,1.0f,1.0f));
	root->attachLight(0, sunlight);

	SquareRenderablePtr square(std::make_shared<SquareRenderable>());
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
	root->attachObject(cairo_canvas);

	auto psystem = scene->createNode("particle_system_container", json::parse_from_file("psystem1.cfg"));
	auto particle_cam = std::make_shared<Camera>("particle_cam");
	particle_cam->lookAt(glm::vec3(0.0f, 10.0f, 20.0f), glm::vec3(0.0f), glm::vec3(0.0f,1.0f,0.0f));
	psystem->attachCamera(particle_cam);
	root->attachNode(psystem);
	//auto rt = DisplayDevice::RenderTargetInstance(400, 300);
	//rt->SetClearColor(0.0f,0.0f,0.0f,0.0f);
	//rt->SetDrawRect(rect(400,300,400,300));
	//rt->Create();
	//psystem->AttachRenderTarget(rt);
	//root->AttachObject(rt);

	auto tex = std::shared_ptr<Blittable>(new SimpleTextureHolder("card-back.png"));
	tex->setDrawRect(rectf(0.0f,0.0f,146.0f,260.0f));
	tex->setPosition(146.0f/2.0f, 600.0f-130.0f);
	tex->setOrder(10);
	root->attachObject(tex);

	float angle = 1.0f;
	float angle_step = 0.5f;

	auto canvas = Canvas::getInstance();
	canvas->setDimensions(800, 600);

	auto canvas_texture = DisplayDevice::createTexture("widgets.png");
	canvas_texture->setFiltering(Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::NONE);

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
		scene->process(SDL_GetTicks() / 1000.0f);

		tex->setRotation(angle, glm::vec3(0.0f,0.0f,1.0f));
		cairo_canvas->setRotation(angle, glm::vec3(0.0f,0.0f,1.0f));
		angle += angle_step;
		while(angle >= 360.0f) {
			angle -= 360.0f;
		}

		scene->renderScene(rman);
		rman->render(main_wnd);

		canvas->blitTexture(canvas_texture, 
			rect(3,4,56,22), 
			0.0f, 
			//rect(800-56, 0, 56, 22), 
			rect(0,0,112,44),
			Color(1.0f,1.0f,1.0f,0.5f));

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
		}
		if(smoothed_time.size() > 50) {
			cumulative_time -= smoothed_time.front();
			smoothed_time.pop_front();
		}

		main_wnd->swap();
	}
	return 0;
}