#include <cstddef>
#include <deque>
#include <list>
#include <vector>

#include <glm/gtc/type_precision.hpp>

#include "attribute_set.hpp"
#include "json.hpp"
#include "logger.hpp"
#include "profile_timer.hpp"
#include "SDLWrapper.hpp"
#include "camera_object.hpp"
#include "canvas.hpp"
#include "LightObject.hpp"
#include "particle_system.hpp"
#include "renderable.hpp"
#include "renderManager.hpp"
#include "renderQueue.hpp"
#include "renderTarget.hpp"
#include "scene_graph.hpp"
#include "scene_node.hpp"
#include "window_manager.hpp"
#include "vector_graphics.hpp"

namespace
{	
	struct vertex_color
	{
		vertex_color() : vertex(0.0f), color(0) {}
		vertex_color(const glm::vec2& v, const glm::u8vec4& c) : vertex(v), color(c) {}
		glm::vec2 vertex;
		glm::u8vec4 color;
	};

	class square_renderable : public KRE::scene_object
	{
	public:
		square_renderable() : KRE::scene_object("square") {
			using namespace KRE;

			auto ab = display_device::create_attribute_set(false, false, false);
			auto pc = new attribute<vertex_color>(AccessFreqHint::DYNAMIC, AccessTypeHint::DRAW);
			pc->set_attr_desc(attribute_desc(attribute_desc::Type::POSITION, 2, attribute_desc::VariableType::FLOAT, false, sizeof(vertex_color), offsetof(vertex_color, vertex)));
			pc->set_attr_desc(attribute_desc(attribute_desc::Type::COLOR, 4, attribute_desc::VariableType::UNSIGNED_BYTE, true, sizeof(vertex_color), offsetof(vertex_color, color)));
			ab->add_attribute(attribute_base_ptr(pc));
			ab->set_draw_mode(attribute_set::DrawMode::TRIANGLE_STRIP);
			add_attribute_set(ab);

			std::vector<vertex_color> vertices;
			vertices.emplace_back(glm::vec2(0.0f,0.0f), glm::u8vec4(255,0,0,255));
			vertices.emplace_back(glm::vec2(0.0f,100.0f), glm::u8vec4(0,255,0,255));
			vertices.emplace_back(glm::vec2(100.0f,0.0f), glm::u8vec4(0,0,255,255));
			vertices.emplace_back(glm::vec2(100.0f,100.0f), glm::u8vec4(255,0,0,255));
			ab->set_count(vertices.size());
			pc->update(&vertices);

			//std::vector<uint8_t> indices;
			//indices.emplace_back(0);
			//indices.emplace_back(1);
			//indices.emplace_back(2);
			//indices.emplace_back(3);
			//ab->update_indicies(indices);

			//pc->update(vertices, pc->begin()+5);

			/*
			// first parameter is a hint to wether the buffer should be hardware backed.
			// second parameter is whether we are using indexed drawing.
			// third parameter is wether this is instanced.
			auto ab = display_device::create_attribute_set(false, true, false);
			// If these were instanced then there is an extra parameter on the end, which defaults to 1.
			// A 0 indictes that there is no advancement per instance
			auto pos = ab->Createattribute();
			pos->set_attr_desc(attribute_desc(attribute_desc::type::POSITION, 2, attribute_desc::VariableType::FLOAT, false));
			auto col = ab->Createattribute();
			col->set_attr_desc(attribute_desc(attribute_desc::type::COLOR, 4, attribute_desc::VariableType::UNSIGNED_BYTE, true));
			ab->set_draw_mode(attribute_set::DrawMode::TRIANGLES);
			add_attribute_set(ab);

			std::vector<glm::vec2> vertices;
			vertices.emplace_back(0.0f, 0.0f);
			vertices.emplace_back(0.0f, 1.0f);
			vertices.emplace_back(1.0f, 0.0f);
			vertices.emplace_back(1.0f, 1.0f);
			//ab->set_count(vertices.size());
			pos->update(&vertices[0], 0, vertices.size() * sizeof(glm::vec2));

			std::vector<glm::u8vec4> colors;
			colors.emplace_back(255,0,0,255);
			colors.emplace_back(0,255,0,255);
			colors.emplace_back(0,0,255,255);
			colors.emplace_back(255,0,0,255);			
			col->update(&colors[0], 0, colors.size() * sizeof(glm::u8vec4));			

			glm::u8vec4 x;

			std::vector<uint8_t> indices;
			indices.emplace_back(0);
			indices.emplace_back(1);
			indices.emplace_back(2);
			indices.emplace_back(2);
			indices.emplace_back(1);
			indices.emplace_back(3);
			ab->update_indicies(indices);
			*/

			//Setcolor(255,255,255);
			set_order(0);
		}
		virtual ~square_renderable() {}
	protected:
		KRE::display_device_def attach(const KRE::display_device_ptr& dd) {
			KRE::display_device_def def(get_attribute_set()/*, get_uniform_set()*/);
			def.set_hint("shader", "attr_color_shader");
			return def;
		}
	private:
		square_renderable(const square_renderable&);
		square_renderable& operator=(const square_renderable&);
	};
	typedef std::shared_ptr<square_renderable> square_renderable_ptr;
}

class simple_texture_holder : public KRE::blittable
{
public:
	simple_texture_holder(const std::string& filename) {
		using namespace KRE;
		set_color(1.0f, 1.0f, 1.0f, 1.0f);
		auto tex = display_device::create_texture(filename, texture::Type::TEXTURE_2D, 4);
		tex->set_filtering(texture::filtering::LINEAR, texture::filtering::LINEAR, texture::filtering::POINT);
		tex->set_address_modes(texture::address_mode::BORDER, texture::address_mode::BORDER);
		set_texture(tex);
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

	window_manager_ptr main_wnd = window_manager::factory("SDL", "opengl");
	main_wnd->enable_vsync(false);
	main_wnd->create_window(800,600);

	scene_graph_ptr scene = scene_graph::create("main");
	scene_node_ptr root = scene->root_node();
	root->set_node_name("root_node");
	auto scenecam = std::make_shared<camera>("cam0", 0, 800, 0, 600);
	scenecam->attach_frustum(std::make_shared<frustum>());
	root->attach_camera(scenecam);
	auto sunlight = std::make_shared<Light>("the_sun", glm::vec3(1.0f, 1.0f, 1.0f));
	sunlight->SetAmbientcolor(glm::vec4(1.0f,1.0f,1.0f,1.0f));
	root->attach_light(0, sunlight);

	square_renderable_ptr square(std::make_shared<square_renderable>());
	square->set_position(600.0f, 400.0f);
	square->set_scale(2.0f,2.0f);
	root->attach_object(square);

	auto rman = std::make_shared<renderManager>();
	auto rq = std::make_shared<renderQueue>("opaques");
	rman->AddQueue(0, rq);

	auto cairo_canvas = vector::context::create_instance("cairo", 512, 512);
	cairo_canvas->set_source_color(0.0, 1.0, 0.0);
	cairo_canvas->paint();
	cairo_canvas->fill();
	auto path = cairo_canvas->new_path();
	path->circle(256, 256, 100);
	cairo_canvas->add_path(path);
	cairo_canvas->set_source_color(0.0, 0.0, 1.0);
	cairo_canvas->fill();
	auto text = cairo_canvas->new_path();
	text->move_to(10, 10);
	text->text_path("ABCDabcde");
	cairo_canvas->add_path(text);
	cairo_canvas->fill();
	cairo_canvas->set_order(5);
	cairo_canvas->set_position(256.0f,256.0f);
	cairo_canvas->set_color(1.0f,1.0f,1.0f,1.0f);
	root->attach_object(cairo_canvas);

	auto psystem = scene->create_node("particle_system_container", json::parse_from_file("psystem1.cfg"));
	auto particle_cam = std::make_shared<camera>("particle_cam", main_wnd);
	particle_cam->look_at(glm::vec3(0.0f, 10.0f, 20.0f), glm::vec3(0.0f), glm::vec3(0.0f,1.0f,0.0f));
	psystem->attach_camera(particle_cam);
	root->attach_node(psystem);
	//auto rt = display_device::render_target_instance(400, 300);
	//rt->set_clear_color(0.0f,0.0f,0.0f,0.0f);
	//rt->SetDrawRect(rect(400,300,400,300));
	//rt->Create();
	//psystem->AttachrenderTarget(rt);
	//root->AttachObject(rt);

	auto tex = std::shared_ptr<blittable>(new simple_texture_holder("card-back.png"));
	tex->set_draw_rect(rectf(0.0f,0.0f,146.0f,260.0f));
	tex->set_position(146.0f/2.0f, 600.0f-130.0f);
	tex->set_order(10);
	root->attach_object(tex);

	float angle = 1.0f;
	float angle_step = 0.5f;

	auto canvas = canvas::get_instance();
	canvas->set_dimensions(800, 600);

	auto canvas_texture = display_device::create_texture("widgets.png");
	canvas_texture->set_filtering(texture::filtering::LINEAR, texture::filtering::LINEAR, texture::filtering::NONE);

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

		main_wnd->clear(display_device::ClearFlags::DISPLAY_CLEAR_ALL);

		// Called once a cycle before rendering.
		scene->process(SDL_GetTicks() / 1000.0f);

		tex->set_rotation(angle, glm::vec3(0.0f,0.0f,1.0f));
		cairo_canvas->set_rotation(angle, glm::vec3(0.0f,0.0f,1.0f));
		angle += angle_step;
		while(angle >= 360.0f) {
			angle -= 360.0f;
		}

		scene->render_scene(rman);
		rman->render(main_wnd);

		canvas->blit_texture(canvas_texture, 
			rect(3,4,56,22), 
			0.0f, 
			//rect(800-56, 0, 56, 22), 
			rect(0,0,112,44),
			color(1.0f,1.0f,1.0f,0.5f));

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