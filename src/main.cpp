#include <list>

#include "logger.hpp"
#include "profile_timer.hpp"
#include "SDLWrapper.hpp"
#include "WindowManager.hpp"

int main(int argc, char *argv[])
{
	std::list<double> smoothed_time;
	double cumulative_time = 0.0;
	int cnt = 0;

	SDL::SDL_ptr manager(new SDL::SDL());

	graphics::WindowManagerPtr main_wnd = graphics::WindowManager::factory("SDL", "opengl");
	main_wnd->enable_vsync(false);
	main_wnd->create_window(1600,900);

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
		double t = timer.check();
		if(t < 1.0/50.0) {
			SDL_Delay(Uint32((1.0/50.0-t)*1000.0));
		}
		t = timer.check();

		smoothed_time.push_back(t);
		cumulative_time += t;
		if(++cnt > 10) {
			cnt = 0;
			LOG_DEBUG("FPS: " << (smoothed_time.size()/cumulative_time));
		}
		if(smoothed_time.size() > 50) {
			cumulative_time -= smoothed_time.front();
			smoothed_time.pop_front();
		}

		main_wnd->swap();
	}
	return 0;
}