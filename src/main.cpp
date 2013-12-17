#include <list>
#include <vector>

#include "logger.hpp"
#include "profile_timer.hpp"
#include "SDLWrapper.hpp"
#include "CameraObject.hpp"
#include "LightObject.hpp"
#include "Renderable.hpp"
#include "RenderManager.hpp"
#include "RenderQueue.hpp"
#include "RenderVariable.hpp"
#include "SceneGraph.hpp"
#include "SceneNode.hpp"
#include "WindowManager.hpp"

namespace
{	
	struct vertex_color
	{
		vertex_color() : vertex(0.0f), color(0) {}
		vertex_color(const glm::vec2& v, const glm::u8vec4& c) : vertex(v), color(c) {}
		glm::vec2 vertex;
		glm::u8vec4 color;
	};

	class SquareRenderable : public Render::Renderable
	{
	public:
		SquareRenderable() {
			render_vars_.resize(2);
			render_vars_[0] = std::make_shared<Render::TypedRenderVariable<vertex_color>>(4, false);
			render_vars_[0]->AddVariableDescription(Render::RenderVariable::VERTEX_POSITION, 2, Render::RenderVariable::TYPE_FLOAT, sizeof(vertex_color), 0);
			render_vars_[0]->AddVariableDescription(Render::RenderVariable::VERTEX_COLOR, 4, Render::RenderVariable::TYPE_UNSIGNED_BYTE, sizeof(vertex_color), sizeof(glm::vec2));
			render_vars_[0]->SetDrawMode(Render::RenderVariable::TRIANGLE_STRIP);

			std::vector<vertex_color> vertices;
			vertices.emplace_back(glm::vec2(0.0f,0.0f), glm::u8vec4(255,0,0,255));
			vertices.emplace_back(glm::vec2(0.0f,1.0f), glm::u8vec4(0,255,0,255));
			vertices.emplace_back(glm::vec2(1.0f,0.0f), glm::u8vec4(0,0,255,255));
			vertices.emplace_back(glm::vec2(1.0f,1.0f), glm::u8vec4(255,0,0,255));
			//render_vars_[0]->Update(vertices);
		}
		virtual ~SquareRenderable() {}
	protected:
		void handle_draw() const {}
	private:
		SquareRenderable(const SquareRenderable&);
		SquareRenderable& operator=(const SquareRenderable&);
	};
	typedef std::shared_ptr<SquareRenderable> SquareRenderablePtr;
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

	Scene::SceneGraphPtr scene = Scene::SceneGraph::Create("main");
	Scene::SceneNodePtr root = scene->RootNode();
	auto scenecam = std::make_shared<Scene::Camera>("cam0", 0, 800, 0, 600);
	scenecam->AttachFrustum(std::make_shared<Scene::Frustum>());
	root->AttachCamera(scenecam);
	auto sunlight = std::make_shared<Scene::Light>("the_sun", glm::vec3(1.0f, 1.0f, 1.0f));
	sunlight->SetAmbientColor(glm::vec4(1.0f,1.0f,1.0f,1.0f));
	root->AttachLight(0, sunlight);

	SquareRenderablePtr square(std::make_shared<SquareRenderable>());
	square->SetPosition(0.5f, 0.5f);

	auto rman = new Render::RenderManager();
	auto rq = std::make_shared<Render::RenderQueue>("opaques");
	rman->AddQueue(0, rq);

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

		// Render stuff here
		rq->Enqueue(0, std::dynamic_pointer_cast<Render::Renderable>(square));
		// End rendering
		rman->Render(main_wnd);

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