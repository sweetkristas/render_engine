#include <cstddef>
#include <list>
#include <vector>

#include <glm/gtc/type_precision.hpp>

#include "AttributeSet.hpp"
#include "json.hpp"
#include "logger.hpp"
#include "profile_timer.hpp"
#include "SDLWrapper.hpp"
#include "CameraObject.hpp"
#include "LightObject.hpp"
#include "ParticleSystem.hpp"
#include "Renderable.hpp"
#include "RenderManager.hpp"
#include "RenderQueue.hpp"
#include "RenderTarget.hpp"
#include "RenderVariable.hpp"
#include "SceneGraph.hpp"
#include "SceneNode.hpp"
#include "WindowManager.hpp"
#include "VGraph.hpp"

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

			auto ab = DisplayDevice::CreateAttributeSet(false, false, false);
			auto pc = ab->CreateAttribute();
			pc->AddAttributeDescription(AttributeDesc(AttributeDesc::Type::POSITION, 2, AttributeDesc::VariableType::FLOAT, false, sizeof(vertex_color), offsetof(vertex_color, vertex)));
			pc->AddAttributeDescription(AttributeDesc(AttributeDesc::Type::COLOR, 4, AttributeDesc::VariableType::UNSIGNED_BYTE, true, sizeof(vertex_color), offsetof(vertex_color, color)));
			ab->SetDrawMode(AttributeSet::DrawMode::TRIANGLE_STRIP);
			AddAttributeSet(ab);

			std::vector<vertex_color> vertices;
			vertices.emplace_back(glm::vec2(0.0f,0.0f), glm::u8vec4(255,0,0,255));
			vertices.emplace_back(glm::vec2(0.0f,100.0f), glm::u8vec4(0,255,0,255));
			vertices.emplace_back(glm::vec2(100.0f,0.0f), glm::u8vec4(0,0,255,255));
			vertices.emplace_back(glm::vec2(100.0f,100.0f), glm::u8vec4(255,0,0,255));
			pc->Update(&vertices, 0, sizeof(vertex_color) * vertices.size());
			ab->SetCount(vertices.size());

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

			//SetColor(255,255,255);
			SetOrder(0);
		}
		virtual ~SquareRenderable() {}
	protected:
		KRE::DisplayDeviceDef Attach(const KRE::DisplayDevicePtr& dd) {
			KRE::DisplayDeviceDef def(GetAttributeSet()/*, GetUniformSet()*/);
			def.SetHint("shader", "attr_color_shader");
			return def;
		}
	private:
		SquareRenderable(const SquareRenderable&);
		SquareRenderable& operator=(const SquareRenderable&);
	};
	typedef std::shared_ptr<SquareRenderable> SquareRenderablePtr;
}

void recurse_tree(const the::tree<int>& xt, the::tree<int>::pre_iterator& it)
{
	if(xt.end() == it) {
		return;
	}
	std::cerr << *it << std::endl;
	recurse_tree(xt, ++it);
}

int main(int argc, char *argv[])
{
	std::list<double> smoothed_time;
	double cumulative_time = 0.0;
	int cnt = 0;

	using namespace KRE;

	SDL::SDL_ptr manager(new SDL::SDL());

	WindowManagerPtr main_wnd = WindowManager::Factory("SDL", "opengl");
	main_wnd->EnableVsync(false);
	main_wnd->CreateWindow(800,600);

	SceneGraphPtr scene = SceneGraph::Create("main", main_wnd);
	SceneNodePtr root = scene->RootNode();
	root->SetNodeName("root_node");
	auto scenecam = std::make_shared<Camera>("cam0", 0, 800, 0, 600);
	scenecam->AttachFrustum(std::make_shared<Frustum>());
	root->AttachCamera(scenecam);
	auto sunlight = std::make_shared<Light>("the_sun", glm::vec3(1.0f, 1.0f, 1.0f));
	sunlight->SetAmbientColor(glm::vec4(1.0f,1.0f,1.0f,1.0f));
	root->AttachLight(0, sunlight);

	SquareRenderablePtr square(std::make_shared<SquareRenderable>());
	square->SetPosition(600.0f, 400.0f);
	square->SetScale(2.0f,2.0f);
	root->AttachObject(square);

	auto rman = std::make_shared<RenderManager>();
	auto rq = std::make_shared<RenderQueue>("opaques");
	rman->AddQueue(0, rq);

	//auto cairo_canvas = Vector::Context::CreateInstance("cairo", main_wnd, 512, 512);
	//cairo_canvas->SetSourceColor(0.0, 1.0, 0.0);
	//cairo_canvas->Paint();
	//cairo_canvas->Fill();
	//auto path = cairo_canvas->NewPath();
	//path->Circle(256, 256, 100);
	//cairo_canvas->AddPath(path);
	//cairo_canvas->SetSourceColor(0.0, 0.0, 1.0);
	//cairo_canvas->Fill();
	//auto text = cairo_canvas->NewPath();
	//text->MoveTo(10, 10);
	//text->TextPath("ABCDabcde");
	//cairo_canvas->AddPath(text);
	//cairo_canvas->Fill();

	//auto psystem = scene->CreateNode("particle_system_container", json::parse_from_file("psystem1.cfg"));
	//root->AttachNode(psystem);
	//psystem->SetNodeName("psc_node");
	//auto particle_cam = std::make_shared<Camera>("particle_cam", main_wnd);
	//particle_cam->LookAt(glm::vec3(0.0f, 10.0f, 20.0f), glm::vec3(0.0f), glm::vec3(0.0f,1.0f,0.0f));
	//psystem->AttachCamera(particle_cam);
	//auto rt = WindowManager::GetDisplayDevice()->RenderTargetInstance(400, 300);
	//rt->SetDisplayRect(0, 0, 400, 300);
	//rt->Create();
	//psystem->AttachRenderTarget(rt);
	//root->AttachObject(rt);

	SDL_Event e;
	bool done = false;
	profile::timer timer;
	while(!done) {
		timer.start();
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) {
				done = true;
			}
		}

		// Called once a cycle before rendering.
		scene->Process(SDL_GetTicks() / 1000.0f);

		scene->RenderScene(rman);
		rman->Render(main_wnd);

		//cairo_canvas->Render(main_wnd);

		double t1 = timer.check();
		if(t1 < 1.0/50.0) {
			SDL_Delay(Uint32((1.0/50.0-t1)*1000.0));
		}
		double t = timer.check();

		smoothed_time.push_back(t);
		cumulative_time += t;
		if(++cnt > 10) {
			cnt = 0;
			LOG_DEBUG("FPS: " << (smoothed_time.size()/cumulative_time) << ", Time: " << t1*1000.0);
		}
		if(smoothed_time.size() > 50) {
			cumulative_time -= smoothed_time.front();
			smoothed_time.pop_front();
		}

		main_wnd->Swap();
	}
	return 0;
}