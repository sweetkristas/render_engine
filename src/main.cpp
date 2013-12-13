#include <list>
#include <vector>

#include "logger.hpp"
#include "profile_timer.hpp"
#include "SDLWrapper.hpp"
#include "SceneGraph.hpp"
#include "WindowManager.hpp"


int main(int argc, char *argv[])
{
	std::list<double> smoothed_time;
	double cumulative_time = 0.0;
	int cnt = 0;

	SDL::SDL_ptr manager(new SDL::SDL());

	graphics::WindowManagerPtr main_wnd = graphics::WindowManager::factory("SDL", "opengl");
	main_wnd->enable_vsync(false);
	main_wnd->create_window(800,600);

	Scene::SceneGraphPtr scene = Scene::SceneGraph::Create("main");
	Scene::SceneNodePtr root = scene->RootNode();
	auto sunlight = Scene::SceneGraph::CreateObject("light", "the_sun");
	

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