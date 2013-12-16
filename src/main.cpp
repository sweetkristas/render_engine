#include <list>
#include <vector>

#include "logger.hpp"
#include "profile_timer.hpp"
#include "SDLWrapper.hpp"
#include "CameraObject.hpp"
#include "LightObject.hpp"
#include "Renderable.hpp"
#include "RenderVariable.hpp"
#include "SceneGraph.hpp"
#include "SceneNode.hpp"
#include "WindowManager.hpp"

namespace
{
	class SquareRenderable : public Render::Renderable
	{
	public:
		SquareRenderable() {
			render_vars_.reset(new Render::RenderVariableList(4));
		}
		virtual ~SquareRenderable() {}
	protected:
		void handle_draw() const {
		}
	private:
		SquareRenderable(const SquareRenderable&);
		SquareRenderable& operator=(const SquareRenderable&);
	};
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