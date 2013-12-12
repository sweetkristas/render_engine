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
		SquareRenderable(int x1, int y1, int x2, int y2, const glm::vec4& color) 
			: x1_(x1), y1_(y1), x2_(x2), y2_(y2), color_(color) {
			verts_.resize(4);
			vars_.emplace_back(new TypedRenderVariable<std::vector<glm::vec2>>(RenderVariableDescription("VertexPosition", RenderVariableDescription::TYPE_FLOAT_VEC2, RenderVariableDescription::USAGE_VERTEX)));
			vars_.emplace_back(new TypedRenderVariable<glm::vec4>(RenderVariableDescription("ColorUniform", RenderVariableDescription::TYPE_FLOAT_VEC4, RenderVariableDescription::USAGE_PRIMITIVE, 1, true, true, false)));
			update_vertices();
			vars_[1]->update->Visit(color_);
		}
		~SquareRenderable() {}
		int x() const { return x1_; }
		int y() const { return x1_; }
		void move_to(int x, int y) {
			x2_ = x2_ - x1_ + x;
			x1_ = x;
			y2_ = y2_ - y1_ + y;
			y1_ = y;
			update_vertices();
		}
		void update_vertices() {
			verts_[0] = glm::vec2(x1_, y1_);
			verts_[1] = glm::vec2(x2_, y1_);
			verts_[2] = glm::vec2(x1_, y2_);
			verts_[3] = glm::vec2(x2_, y2_);
			vars_[0]->update->Visit(verts_);
		}
		void set_color(const glm::vec4& c) { color_ = c; vars_[1]->update->Visit(color_); }
	protected:
		void handle_draw() const {
		}
	private:
		int x1_;
		int y1_;
		int x2_;
		int y2_;
		std::vector<glm::vec2> verts_;
		RenderVariableList vars_;
		glm::vec4 color_;
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

	Render::SquareRenderable* square(new Render::SquareRenderable(100,100,200,200,glm::vec4(1.0f,0.0f,0.0f,1.0f)));

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

		square->move_to(square->x()+1, square->y()+1);
		q->enqueue(0, Render::RenderablePtr(square));

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