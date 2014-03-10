#include <cstddef>
#include <list>
#include <vector>

#include <glm/gtc/type_precision.hpp>

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

	class SquareRenderable : public Scene::SceneObject
	{
	public:
		SquareRenderable() : Scene::SceneObject("square") {
			using namespace Render;
			auto& arv = std::make_shared<AttributeRenderVariable<vertex_color>>();
			arv->AddVariableDescription(AttributeRenderVariableDesc::POSITION, 2, AttributeRenderVariableDesc::FLOAT, false, sizeof(vertex_color), offsetof(vertex_color, vertex));
			arv->AddVariableDescription(AttributeRenderVariableDesc::COLOR, 4, AttributeRenderVariableDesc::UNSIGNED_BYTE, true, sizeof(vertex_color), offsetof(vertex_color, color));
			arv->SetDrawMode(RenderVariable::TRIANGLE_STRIP);
			AddAttributeRenderVariable(arv);

			auto& urv = std::make_shared<UniformRenderVariable<glm::vec4>>();
			urv->AddVariableDescription(UniformRenderVariableDesc::COLOR, UniformRenderVariableDesc::FLOAT_VEC4);
			AddUniformRenderVariable(urv);

			std::vector<vertex_color> vertices;
			vertices.emplace_back(glm::vec2(0.0f,0.0f), glm::u8vec4(255,0,0,255));
			vertices.emplace_back(glm::vec2(0.0f,100.0f), glm::u8vec4(0,255,0,255));
			vertices.emplace_back(glm::vec2(100.0f,0.0f), glm::u8vec4(0,0,255,255));
			vertices.emplace_back(glm::vec2(100.0f,100.0f), glm::u8vec4(255,0,0,255));
			arv->Update(&vertices);

			urv->Update(glm::vec4(1.0f,1.0f,1.0f,1.0f));

			SetOrder(0);
		}
		virtual ~SquareRenderable() {}
	protected:
		Graphics::DisplayDeviceDef Attach(const Graphics::DisplayDevicePtr& dd) {
			Graphics::DisplayDeviceDef def(AttributeRenderVariables(), UniformRenderVariables());
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

	SDL::SDL_ptr manager(new SDL::SDL());

	Graphics::WindowManagerPtr main_wnd = Graphics::WindowManager::factory("SDL", "opengl");
	main_wnd->enable_vsync(false);
	main_wnd->create_window(800,600);

	Scene::SceneGraphPtr scene = Scene::SceneGraph::Create("main", main_wnd);
	Scene::SceneNodePtr root = scene->RootNode();
	root->SetNodeName("root_node");
	auto scenecam = std::make_shared<Scene::Camera>("cam0", 0, 800, 0, 600);
	scenecam->AttachFrustum(std::make_shared<Scene::Frustum>());
	root->AttachCamera(scenecam);
	auto sunlight = std::make_shared<Scene::Light>("the_sun", glm::vec3(1.0f, 1.0f, 1.0f));
	sunlight->SetAmbientColor(glm::vec4(1.0f,1.0f,1.0f,1.0f));
	root->AttachLight(0, sunlight);

	SquareRenderablePtr square(std::make_shared<SquareRenderable>());
	square->SetPosition(600.0f, 400.0f);
	square->SetScale(2.0f,2.0f);
	root->AttachObject(square);

	auto rman = std::make_shared<Render::RenderManager>();
	auto rq = std::make_shared<Render::RenderQueue>("opaques");
	rman->AddQueue(0, rq);

	auto cairo_canvas = Graphics::Vector::Context::CreateInstance("cairo", main_wnd, 512, 512);
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

	/// XXXX Need to be able to set a render target, either a texture or an fbo object.
	auto psystem = scene->CreateNode("particle_system_container", json::parse_from_file("psystem1.cfg"));
	root->AttachNode(psystem);
	psystem->SetNodeName("psc_node");
	auto particle_cam = std::make_shared<Scene::Camera>("particle_cam", main_wnd);
	particle_cam->LookAt(glm::vec3(0.0f, 10.0f, 20.0f), glm::vec3(0.0f), glm::vec3(0.0f,1.0f,0.0f));
	psystem->AttachCamera(particle_cam);

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

		main_wnd->swap();
	}
	return 0;
}