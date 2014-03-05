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
			arv->AddVariableDescription(AttributeRenderVariableDesc::POSITION, 2, AttributeRenderVariableDesc::FLOAT, false, sizeof(vertex_color), 0);
			arv->AddVariableDescription(AttributeRenderVariableDesc::COLOR, 4, AttributeRenderVariableDesc::UNSIGNED_BYTE, true, sizeof(vertex_color), sizeof(glm::vec2));
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
			arv->Update(vertices);

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


#include "Shaders.hpp"
void gl_test2()
{
	const float x = 200.0;
	const float y = 150.0;
	const float w = 400.0f;
	const float h = 300.0f;
	const float vcoords[] = {
		  x,   y,
		  x, y+h,
		x+w,   y,
		x+w, y+h,
	};
	const uint8_t ccoords[] = {
		255, 255, 255, 255,
		255,   0,   0, 255,
		  0, 255,   0, 255,
		  0,   0, 255, 255,
	};

	auto shader = Shader::ShaderProgram::Factory("attr_color_shader");
	shader->MakeActive();

	glm::mat4 pmat = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f);
	shader->SetUniformValue(shader->GetMvpUniform(), glm::value_ptr(pmat));
	shader->SetUniformValue(shader->GetColorUniform(), glm::value_ptr(glm::vec4(1.0f,1.0f,1.0f,1.0f)));
	
	glEnableVertexAttribArray(shader->GetVertexAttribute()->second.location);
	glVertexAttribPointer(shader->GetVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vcoords);
	
	glEnableVertexAttribArray(shader->GetColorAttribute()->second.location);
	glVertexAttribPointer(shader->GetColorAttribute()->second.location, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, ccoords);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(shader->GetVertexAttribute()->second.location);
	glDisableVertexAttribArray(shader->GetColorAttribute()->second.location);
}

void gl_test()
{
	const float w = 800.0f;
	const float h = 600.0f;
	const float vcoords[] = {
		0.0f, 0.0f,
		0.0f,    h,
		   w, 0.0f,
		   w,    h,
	};

	auto shader = Shader::ShaderProgram::Factory("simple");
	shader->MakeActive();

	glm::mat4 pmat = glm::ortho(0.0f, 512.0f, 512.0f, 0.0f);
	shader->SetUniformValue(shader->GetMvpUniform(), glm::value_ptr(pmat));
	shader->SetUniformValue(shader->GetColorUniform(), glm::value_ptr(glm::vec4(0.0f,1.0f,0.0f,1.0f)));
	
	int value = 0;
	shader->SetUniformValue(shader->GetUniformIterator("discard"), &value);
	const float point_size = 1.0f;
	shader->SetUniformValue(shader->GetUniformIterator("point_size"), &point_size);

	glEnableVertexAttribArray(shader->GetVertexAttribute()->second.location);
	glVertexAttribPointer(shader->GetVertexAttribute()->second.location, 2, GL_FLOAT, GL_FALSE, 0, vcoords);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(shader->GetVertexAttribute()->second.location);
}


int main(int argc, char *argv[])
{
	std::list<double> smoothed_time;
	double cumulative_time = 0.0;
	int cnt = 0;

	// "(1 (5 (6 7 8)) (9 (10 11) 12) (2 3 4 42))
	//the::tree<int> xt = the::tree_of(1)(the::tree_of(5)(the::tree_of(6)(7,8)),
    //                  the::tree_of(9)(the::tree_of(10)(11),12),
    //                  the::tree_of(2)(3,4,42));
	//
	//              1
	//             / \
	//            /   \
	//           /     \
	//          5       9
	//         / \     / \
	//        6   7   10 12
	//                /
	//               11
	//the::tree<int> xt = the::tree_of(1)(the::tree_of(5)(6,7), the::tree_of(9)(the::tree_of(10)(11),12));
	//recurse_tree(xt, xt.begin());

	SDL::SDL_ptr manager(new SDL::SDL());

	Graphics::WindowManagerPtr main_wnd = Graphics::WindowManager::factory("SDL", "opengl");
	main_wnd->enable_vsync(false);
	main_wnd->create_window(800,600);

	Scene::SceneGraphPtr scene = Scene::SceneGraph::Create("main", main_wnd);
	Scene::SceneNodePtr root = scene->RootNode();
	auto scenecam = std::make_shared<Scene::Camera>("cam0", 0, 800, 0, 600);
	scenecam->AttachFrustum(std::make_shared<Scene::Frustum>());
	root->AttachCamera(scenecam);
	auto sunlight = std::make_shared<Scene::Light>("the_sun", glm::vec3(1.0f, 1.0f, 1.0f));
	sunlight->SetAmbientColor(glm::vec4(1.0f,1.0f,1.0f,1.0f));
	root->AttachLight(0, sunlight);

	SquareRenderablePtr square(std::make_shared<SquareRenderable>());
	square->SetPosition(100.0f, 100.0f);
	root->AttachObject(square);

	auto rman = std::make_shared<Render::RenderManager>();
	auto rq = std::make_shared<Render::RenderQueue>("opaques");
	rman->AddQueue(0, rq);

	auto canvas = Graphics::Vector::Context::CreateInstance("cairo", main_wnd, 512, 512);
	canvas->SetSourceColor(0.0, 1.0, 0.0);
	canvas->Paint();
	canvas->Fill();
	auto path = canvas->NewPath();
	path->Circle(256, 256, 100);
	canvas->AddPath(path);
	canvas->SetSourceColor(0.0, 0.0, 1.0);
	canvas->Fill();
	auto text = canvas->NewPath();
	text->MoveTo(10, 10);
	text->TextPath("ABCDabcde");
	canvas->AddPath(text);
	canvas->Fill();

	/// XXXX Need to be able to set a render target, either a texture or an fbo object.


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


		//gl_test();
		//gl_test2();
		scene->RenderScene(rman);
		rman->Render(main_wnd);

		//canvas->Render(main_wnd);


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