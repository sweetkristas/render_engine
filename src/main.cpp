#include <list>
#include <vector>

#include "logger.hpp"
#include "profile_timer.hpp"
#include "RenderManager.hpp"
#include "SDLWrapper.hpp"
#include "WindowManager.hpp"

namespace Render
{
	class SquareRenderable : public Renderable
	{
	public:
		SquareRenderable(int x1, int y1, int x2, int y2) 
			: x1_(x1), y1_(y1), x2_(x2), y2_(y2) {
			vars_.push_back(RenderVariablePtr(new RenderVariable("VertexPosition")));
		}
		~SquareRenderable() {}
	protected:
		void handle_draw() const {
		}
	private:
		int x1_;
		int y1_;
		int x2_;
		int y2_;
		RenderVariableList vars_;
	};
}

int main(int argc, char *argv[])
{
	std::list<double> smoothed_time;
	double cumulative_time = 0.0;
	int cnt = 0;
	Render::RenderManager render_man;

	SDL::SDL_ptr manager(new SDL::SDL());

	graphics::WindowManagerPtr main_wnd = graphics::WindowManager::factory("SDL", "opengl");
	main_wnd->enable_vsync(false);
	main_wnd->create_window(800,600);

	Render::RenderQueuePtr q(new Render::RenderQueue("opaques"));
	render_man.add_queue(0, q);

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

		render_man.draw();

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