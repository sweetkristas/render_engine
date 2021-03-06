#include <cstddef>
#include <deque>
#include <list>
#include <vector>

#include <glm/gtc/type_precision.hpp>

#include "AttributeSet.hpp"
#include "json.hpp"
#include "filesystem.hpp"
#include "profile_timer.hpp"
#include "SDLWrapper.hpp"
#include "Blend.hpp"
#include "CameraObject.hpp"
#include "Canvas.hpp"
#include "Font.hpp"
#include "LightObject.hpp"
#include "ModelMatrixScope.hpp"
#include "ParticleSystem.hpp"
#include "ParticleSystemAffectors.hpp"
#include "ParticleSystemEmitters.hpp"
#include "ParticleSystemParameters.hpp"
#include "Renderable.hpp"
#include "RenderManager.hpp"
#include "RenderQueue.hpp"
#include "RenderTarget.hpp"
#include "SceneGraph.hpp"
#include "SceneNode.hpp"
#include "SceneTree.hpp"
#include "Shaders.hpp"
#include "Surface.hpp"
#include "TexPack.hpp"
#include "UniformBuffer.hpp"
#include "WindowManager.hpp"
#include "VGraph.hpp"

#include "tmx_reader.hpp"

#include "variant_utils.hpp"

#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"
#endif

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace ImGui
{
	int Curve(const char *label, const ImVec2& size, int maxpoints, ImVec2 *points);
	float CurveValue(float p, int maxpoints, const ImVec2 *points);

	bool Spline(const char *label, const ImVec2& size, int maxpoints, ImVec2 *points);
};
bool ColorPicker4(float* col, bool show_alpha);

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

			setShader(ShaderProgram::getProgram("attr_color_shader"));

			auto ab = DisplayDevice::createAttributeSet(false, false, false);
			auto pc = new Attribute<vertex_color>(AccessFreqHint::DYNAMIC, AccessTypeHint::DRAW);
			pc->addAttributeDesc(AttributeDesc(AttrType::POSITION, 2, AttrFormat::FLOAT, false, sizeof(vertex_color), offsetof(vertex_color, vertex)));
			pc->addAttributeDesc(AttributeDesc(AttrType::COLOR, 4, AttrFormat::UNSIGNED_BYTE, true, sizeof(vertex_color), offsetof(vertex_color, color)));
			ab->addAttribute(AttributeBasePtr(pc));
			ab->setDrawMode(DrawMode::TRIANGLE_STRIP);
			addAttributeSet(ab);

			std::vector<vertex_color> vertices;
			vertices.emplace_back(glm::vec2(0.0f,0.0f), glm::u8vec4(255,0,0,255));
			vertices.emplace_back(glm::vec2(0.0f,100.0f), glm::u8vec4(0,255,0,255));
			vertices.emplace_back(glm::vec2(100.0f,0.0f), glm::u8vec4(0,0,255,255));
			vertices.emplace_back(glm::vec2(100.0f,100.0f), glm::u8vec4(255,0,0,255));
			ab->setCount(vertices.size());
			pc->update(&vertices, pc->end());

			//std::vector<uint8_t> indices;
			//indices.emplace_back(0);
			//indices.emplace_back(1);
			//indices.emplace_back(2);
			//indices.emplace_back(3);
			//ab->UpdateIndicies(indices);

			//pc->Update(vertices, pc->begin()+5);

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

			//setColor(255,255,255);
			setOrder(0);
		}
		virtual ~SquareRenderable() {}
	private:
		SquareRenderable(const SquareRenderable&);
		SquareRenderable& operator=(const SquareRenderable&);
	};
	typedef std::shared_ptr<SquareRenderable> SquareRenderablePtr;
}

class SimpleRenderable : public KRE::SceneObject
{
public:
	explicit SimpleRenderable(const rect& r, const KRE::Color& color)
		: KRE::SceneObject("simple_renderable")
	{
		init();

		setColor(color);

		const float vx1 = static_cast<float>(r.x1());
		const float vy1 = static_cast<float>(r.y1());
		const float vx2 = static_cast<float>(r.x2());
		const float vy2 = static_cast<float>(r.y2());

		std::vector<glm::vec2> vc;
		vc.emplace_back(vx1, vy2);
		vc.emplace_back(vx1, vy1);
		vc.emplace_back(vx2, vy1);

		vc.emplace_back(vx2, vy1);
		vc.emplace_back(vx2, vy2);
		vc.emplace_back(vx1, vy2);
		attribs_->update(&vc);
	}
	void init(KRE::DrawMode draw_mode = KRE::DrawMode::TRIANGLES)
	{
		using namespace KRE;
		setShader(ShaderProgram::getProgram("simple"));

		auto as = DisplayDevice::createAttributeSet();
		attribs_.reset(new KRE::Attribute<glm::vec2>(AccessFreqHint::DYNAMIC, AccessTypeHint::DRAW));
		attribs_->addAttributeDesc(AttributeDesc(AttrType::POSITION, 2, AttrFormat::FLOAT, false));
		as->addAttribute(AttributeBasePtr(attribs_));
		as->setDrawMode(draw_mode);
		
		addAttributeSet(as);
	}
private:
	std::shared_ptr<KRE::Attribute<glm::vec2>> attribs_;
};


struct SimpleTextureHolder : public KRE::Blittable
{
	SimpleTextureHolder(const std::string& filename) {
		using namespace KRE;
		setColor(1.0f, 1.0f, 1.0f, 1.0f);
		auto tex = Texture::createTexture(filename, TextureType::TEXTURE_2D, 4);
		tex->setFiltering(-1, Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::POINT);
		tex->setAddressModes(-1, Texture::AddressMode::BORDER, Texture::AddressMode::BORDER);
		setTexture(tex);
	}
};

KRE::ShaderProgramPtr generateBlurShader(const std::string& name, float sigma, int radius)
{
	using namespace KRE;

	std::vector<float> std_gaussian_weights;
	float weights_sum = 0;
	const float sigma_pow_2 = std::pow(sigma, 2.0f);
	const float term1 = (1.0f / std::sqrt(2.0f * static_cast<float>(M_PI) * sigma_pow_2));
	for(int n = 0; n < radius + 2; ++n) {
		std_gaussian_weights.emplace_back(term1 * std::exp(-std::pow(static_cast<float>(n), 2.0f) / (2.0f * sigma_pow_2)));
		weights_sum += (n == 0 ? 1.0f : 2.0f) * std_gaussian_weights.back();
	}
	// normalise weights
	for(auto& weight : std_gaussian_weights) {
		weight /= weights_sum;
	}

	int optimized_offsets = std::min(radius/2 + (radius%2), 7);
	std::vector<float> opt_gaussian_weights;
	opt_gaussian_weights.resize(optimized_offsets);
	for(int n = 0; n < optimized_offsets; ++n) {
		const float first_weight = std_gaussian_weights[n * 2 + 1];
		const float second_weight = std_gaussian_weights[n * 2 + 2];
		const float sum_weights = first_weight + second_weight;
		opt_gaussian_weights[n] = (first_weight * static_cast<float>(n * 2 + 1) + second_weight * static_cast<float>(n * 2 + 1)) / sum_weights;
	}

	std::stringstream v_shader;
	v_shader << "uniform mat4 mvp_matrix;\n"
			<< "attribute vec2 position;\n"
			<< "attribute vec4 texcoord;\n"
			<< "uniform float texel_width_offset;\n"
			<< "uniform float texel_height_offset;\n"
			<< "varying vec2 blur_coords[" << (optimized_offsets * 2 + 1) << "];\n"
			<< "void main() {\n"
			<< "    gl_Position = mvp_matrix * vec4(position, 0.0, 1.0);\n"
			<< "    vec2 step_offset = vec2(texel_width_offset, texel_height_offset);\n"
			<< "    blur_coords[0] = texcoord.xy;\n"
			;
	for(int n = 0; n < optimized_offsets; ++n) {
		v_shader << "    blur_coords[" << (n * 2 + 1) << "] = texcoord.xy + step_offset * " << opt_gaussian_weights[n] << ";\n";
		v_shader << "    blur_coords[" << (n * 2 + 2) << "] = texcoord.xy + step_offset * " << opt_gaussian_weights[n] << ";\n";
	}
	v_shader << "}\n";

	std::stringstream f_shader;
	f_shader << "uniform sampler2D tex_map;\n"
		<< "varying highp vec2 blur_coords[" << (optimized_offsets * 2 + 1) << "];\n"
		<< "void main() {\n"
		<< "    lowp vec4 sum = vec4(0.0);\n"
		<< "    sum += texture2D(tex_map, blur_coords[0]) * " << std_gaussian_weights[0] << ";\n"
		;
	for(int n = 0; n < optimized_offsets; ++n) {
		const float first_weight = std_gaussian_weights[n * 2 + 1];
		const float second_weight = std_gaussian_weights[n * 2 + 2];
		const float sum_weights = first_weight + second_weight;
		f_shader 
			<< "    sum += texture2D(tex_map, blur_coords[" << (n * 2 + 1) << "]) * " << sum_weights << ";\n"
			<< "    sum += texture2D(tex_map, blur_coords[" << (n * 2 + 2) << "]) * " << sum_weights << ";\n"
			;
	}
	f_shader << "gl_FragColor = sum;\n}\n";

	variant_builder sp;
	sp.add("vertex", v_shader.str());
	sp.add("fragment", f_shader.str());
	sp.add("name", name);
	ShaderProgram::loadFromVariant(sp.build());
	return ShaderProgram::getProgram(name);
}

struct BlurredBlittable : public KRE::Blittable
{
	explicit BlurredBlittable(const std::string& filename) {
		using namespace KRE;
		setColor(1.0f, 1.0f, 1.0f, 1.0f);
		//auto shader = ShaderProgram::getProgram("blur1");
		//auto shader = generateBlurShader("blur-5.0-4", 5.0f, 9);
		auto shader = ShaderProgram::getProgram("blur2");
		two_ = shader->getUniform("texel_width_offset");
		tho_ = shader->getUniform("texel_height_offset");

		setShader(shader);

		auto tex = Texture::createTexture(filename, TextureType::TEXTURE_2D, 4);
		tex->setFiltering(-1, Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::POINT);
		tex->setAddressModes(-1, Texture::AddressMode::CLAMP, Texture::AddressMode::CLAMP);
		setTexture(tex);

		shader->setUniformValue(two_, 1.0f / (tex->width()-1));
		shader->setUniformValue(tho_, /*1.0f / (tex->height()-1)*/0.0f);
	}
	int two_;
	int tho_;
};

struct FreeTextureHolder : public KRE::SceneObject
{
	FreeTextureHolder(KRE::SurfacePtr surface)
		: KRE::SceneObject("FreeTextureHolder") 
	{
		using namespace KRE;
		auto tex = Texture::createTexture(surface);
		//tex->setFiltering(Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::POINT);
		//tex->setAddressModes(Texture::AddressMode::BORDER, Texture::AddressMode::BORDER);
		setTexture(tex);
		init();
	}
	FreeTextureHolder(KRE::TexturePtr tex)
		: KRE::SceneObject("FreeTextureHolder") 
	{
		using namespace KRE;
		setTexture(tex);
		init();
	}
	FreeTextureHolder(const std::string& filename)
		: KRE::SceneObject("FreeTextureHolder") 
	{
		using namespace KRE;
		auto tex = Texture::createTexture(filename, TextureType::TEXTURE_2D, 4);
		//tex->setFiltering(Texture::Filtering::LINEAR, Texture::Filtering::LINEAR, Texture::Filtering::POINT);
		tex->setAddressModes(-1, Texture::AddressMode::BORDER, Texture::AddressMode::BORDER);
		setTexture(tex);
		init();
	}
	void init()
	{
		setDrawRect(rect(0, 0, getTexture()->surfaceWidth(), getTexture()->surfaceHeight()));
		using namespace KRE;
		setColor(1.0f, 1.0f, 1.0f, 1.0f);
		auto as = DisplayDevice::createAttributeSet();
		attribs_.reset(new Attribute<vertex_texcoord>(AccessFreqHint::DYNAMIC, AccessTypeHint::DRAW));
		attribs_->addAttributeDesc(AttributeDesc(AttrType::POSITION, 2, AttrFormat::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, vtx)));
		attribs_->addAttributeDesc(AttributeDesc(AttrType::TEXTURE,  2, AttrFormat::FLOAT, false, sizeof(vertex_texcoord), offsetof(vertex_texcoord, tc)));
		as->addAttribute(AttributeBasePtr(attribs_));
		as->setDrawMode(DrawMode::TRIANGLE_STRIP);
		
		addAttributeSet(as);
	}
	void preRender(const KRE::WindowPtr& wm) override
	{
		const float offs_x = 0.0f;
		const float offs_y = 0.0f;
		// XXX we should only do this if things changed.
		const float vx1 = draw_rect_.x() + offs_x;
		const float vy1 = draw_rect_.y() + offs_y;
		const float vx2 = draw_rect_.x2() + offs_x;
		const float vy2 = draw_rect_.y2() + offs_y;

		const rectf& r = getTexture()->getSourceRectNormalised();

		std::vector<KRE::vertex_texcoord> vertices;
		vertices.emplace_back(glm::vec2(vx1,vy1), glm::vec2(r.x(),r.y()));
		vertices.emplace_back(glm::vec2(vx2,vy1), glm::vec2(r.x2(),r.y()));
		vertices.emplace_back(glm::vec2(vx1,vy2), glm::vec2(r.x(),r.y2()));
		vertices.emplace_back(glm::vec2(vx2,vy2), glm::vec2(r.x2(),r.y2()));
		getAttributeSet().back()->setCount(vertices.size());
		attribs_->update(&vertices);
	}
	template<typename T>
	void setDrawRect(const geometry::Rect<T>& r) {
		draw_rect_ = r.template as_type<float>();
	}
private:
	std::shared_ptr<KRE::Attribute<KRE::vertex_texcoord>> attribs_;
	rectf draw_rect_;
};

void set_alpha_masks()
{
	using namespace KRE;
	std::vector<Color> alpha_colors;

	auto surf = Surface::create("alpha-colors.png");
	surf->iterateOverSurface([&alpha_colors](int x, int y, int r, int g, int b, int a) {
		alpha_colors.emplace_back(r, g, b);
		LOG_INFO("Added alpha color: (" << r << "," << g << "," << b << ")");	
	});

	Surface::setAlphaFilter([=](int r, int g, int b) {
		for(auto& c : alpha_colors) {
			if(c.ri() == r && c.gi() == g && c.bi() == b) {
				return true;
			}
		}
		return false;
	});
}

struct water_distort_uniforms
{
	unsigned int texture;
	glm::mat4 mvp;
	float cycle;
	glm::vec4 sprite_area;
	glm::vec4 draw_area;
	float intensity;
	glm::vec4 water_area[2];
};


void texture_packing_test()
{
	using namespace KRE;
	const std::string tile_file1 = "brown-rock1.png";
	const std::string tile_file2 = "brown-rock1.png";
	SurfacePtr s1 = Surface::create(tile_file1);
	SurfacePtr s2 = Surface::create(tile_file2);
	std::vector<SurfaceAreas> sa;
	sa.emplace_back(s1);
	sa.back().addRect(16, 0, 16, 32);
	sa.back().addRect(32, 0, 16, 32);
	sa.emplace_back(s2);
	sa.back().addRect(0, 0, 16, 32);
	sa.back().addRect(16, 0, 16, 32);
	sa.back().addRect(0, 32, 16, 32);
	sa.back().addRect(16, 32, 16, 32);
	sa.back().addRect(32, 32, 16, 32);
	sa.back().addRect(48, 32, 16, 32);

	Packer tp(sa, 2048, 2048);
	for(auto& r : tp) {
		LOG_DEBUG("New rect: " << r);
	}
	// output surface available as tp.getOutputSurface()
}

std::vector<float> generate_gaussian(float sigma, int radius = 4)
{
	std::vector<float> std_gaussian_weights;
	float weights_sum = 0;
	const float sigma_pow_2 = std::pow(sigma, 2.0f);
	const float term1 = (1.0f / std::sqrt(2.0f * static_cast<float>(M_PI) * sigma_pow_2));
	for(int n = 0; n < radius + 1; ++n) {
		std_gaussian_weights.emplace_back(term1 * std::exp(-std::pow(static_cast<float>(n), 2.0f) / (2.0f * sigma_pow_2)));
		weights_sum += (n == 0 ? 1.0f : 2.0f) * std_gaussian_weights.back();
	}
	// normalise weights
	for(auto& weight : std_gaussian_weights) {
		weight /= weights_sum;
	}

	std::vector<float> res;
	for(auto it = std_gaussian_weights.crbegin(); it != std_gaussian_weights.crend(); ++it) {
		res.emplace_back(*it);
	}
	for(auto it = std_gaussian_weights.cbegin()+1; it != std_gaussian_weights.cend(); ++it) {
		res.emplace_back(*it);
	}

	std::stringstream ss;
	ss << "Gaussian(sigma=" << sigma << ", radius=" << radius << "):";
	for(auto it = std_gaussian_weights.crbegin(); it != std_gaussian_weights.crend(); ++it) {
		ss << " " << (*it);
	}
	//ss << " " << std_gaussian_weights[0];
	for(auto it = std_gaussian_weights.cbegin()+1; it != std_gaussian_weights.cend(); ++it) {
		ss << " " << (*it);
	}
	LOG_DEBUG(ss.str());
	return res;
}


void ParameterGui(const char* label, const KRE::Particles::ParameterPtr& param, float fmin = 0.0f, float fmax = 0.0f)
{
	// XXX we need to deal with this condition. Probably add another option to type.
	if(param == nullptr) {
		return;
	}

	using namespace KRE::Particles;
	ImGui::Text(label);
	const char* const ptype[] = { "Fixed", "Random", "Linear", "Spline", "Oscillate" };
	int current_type = static_cast<int>(param->getType());
	std::string combo_label = "Type##";
	combo_label += label;
	if(ImGui::Combo(combo_label.c_str(), &current_type, ptype, 5)) {
		param->setType(static_cast<ParameterType>(current_type));
	}

	float v_speed = 1.0f;
	if(fmax != 0.0f) {
		v_speed = fmax / 100.0f;
	}

	switch(param->getType()){
		case ParameterType::FIXED: {
			FixedParams fp;
			param->getFixedValue(&fp);
			std::string fixed_label = "Value##";
			fixed_label += label;
			if(ImGui::DragFloat(fixed_label.c_str(), &fp.value, v_speed, fmin, fmax)) {
				param->setFixedValue(fp);
			}
			break;
		}
		case ParameterType::RANDOM: {
			RandomParams rp;
			param->getRandomRange(&rp);
			std::string min_label = "Min Value##";
			min_label += label;
			if(ImGui::DragFloat(min_label.c_str(), &rp.min_value, v_speed, fmin, fmax)) {
				param->setRandomRange(rp);
			}
			std::string max_label = "Max Value##";
			max_label += label;
			if(ImGui::DragFloat(max_label.c_str(), &rp.max_value, v_speed, fmin, fmax)) {
				param->setRandomRange(rp);
			}
			break;
		}
		case ParameterType::CURVED_LINEAR: {
			CurvedParams cp;
			param->getCurvedParams(&cp);
			ImVec2 points[10];
			std::fill(points, points+10, ImVec2(-1.0f, 0.0f));
			if(cp.control_points.size() < 2) {
				points[0].x = -1.0f;
			} else {
				int n = 0;
				for(auto& p : cp.control_points) {
					points[n].x = static_cast<float>(p.first);
					points[n].y = static_cast<float>(p.second);
					if(++n >= 10) {
						break;
					}
				}
			}
			std::string linear_label = "Linear##";
			linear_label += label;
			if(ImGui::Curve(linear_label.c_str(), ImVec2(300, 200), 10, points)) {
				cp.control_points.clear();
				for(int n = 0; n != 10 && points[n].x >= 0; ++n) {
					cp.control_points.emplace_back(points[n].x, points[n].y);
				}
				param->setControlPoints(InterpolationType::LINEAR,  cp);
			}
			break;
		}
		case ParameterType::CURVED_SPLINE: {
			CurvedParams cp;
			param->getCurvedParams(&cp);
			ImVec2 points[10];
			std::fill(points, points+10, ImVec2(-1.0f, 0.0f));
			if(cp.control_points.size() < 2) {
				points[0].x = -1.0f;
			} else {
				int n = 0;
				for(auto& p : cp.control_points) {
					points[n].x = static_cast<float>(p.first);
					points[n].y = static_cast<float>(p.second);
					if(++n >= 10) {
						break;
					}
				}
			}
			std::string spline_label = "Spline##";
			spline_label += label;
			if(ImGui::Spline(spline_label.c_str(), ImVec2(300.0f, 200.0f), 10, points)) {
				cp.control_points.clear();
				for(int n = 0; n != 10 && points[n].x >= 0; ++n) {
					cp.control_points.emplace_back(points[n].x, points[n].y);
				}
				param->setControlPoints(InterpolationType::SPLINE,  cp);
			}
			break;
		}

		case ParameterType::OSCILLATE: {
			OscillationParams op;
			param->getOscillation(&op);
			const char* const osc_items[] = { "Sine", "Square" };
			int otype = static_cast<int>(op.osc_type);
			std::string wtype_label = "Wave Type##";
			wtype_label += label;
			if(ImGui::Combo(wtype_label.c_str(), &otype, osc_items, 2)) {
				op.osc_type = static_cast<WaveType>(otype);
				param->setOscillation(op);
			}
			std::string freq_label = "Frequency##";
			freq_label += label;
			if(ImGui::DragFloat(freq_label.c_str(), &op.frequency, 1.0f, 1.0f, 10000.0f)) {
				param->setOscillation(op);
			}
			std::string phase_label = "Phase##";
			phase_label += label;
			if(ImGui::DragFloat(phase_label.c_str(), &op.phase, 1.0f, 0.0f, 360.0f)) {
				param->setOscillation(op);
			}
			std::string base_label = "Base##";
			base_label += label;
			if(ImGui::DragFloat(base_label.c_str(), &op.base, 1.0f, 0.0f, 1000.0f)) {
				param->setOscillation(op);
			}
			std::string amplitude_label = "Amplitude##";
			amplitude_label += label;
			if(ImGui::DragFloat(amplitude_label.c_str(), &op.amplitude, v_speed, fmin, fmax)) {
				param->setOscillation(op);
			}
			break;
		}
	default: break;
	}
}

void theme_imgui()
{
	// Setup style
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(0.31f, 0.25f, 0.24f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.74f, 0.74f, 0.94f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.68f, 0.68f, 0.68f, 0.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.62f, 0.70f, 0.72f, 0.56f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.33f, 0.14f, 0.47f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.97f, 0.31f, 0.13f, 0.81f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.42f, 0.75f, 1.00f, 0.53f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.65f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(0.89f, 0.98f, 1.00f, 0.99f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.47f, 0.47f, 0.71f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(1.00f, 0.79f, 0.18f, 0.78f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.42f, 0.82f, 1.00f, 0.81f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.72f, 1.00f, 1.00f, 0.86f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.78f, 0.84f, 0.80f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.68f, 0.74f, 0.80f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.80f, 0.30f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.41f, 0.75f, 0.98f, 0.50f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(1.00f, 0.47f, 0.41f, 0.60f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(1.00f, 0.16f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.99f, 0.54f, 0.43f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	
	style.Alpha = 1.0f;
	style.FrameRounding = 4;
	style.IndentSpacing = 12.0f;
}

bool vector_string_getter(void* data, int n, const char** out_text)
{
	const std::vector<std::string>& v = *static_cast<std::vector<std::string>*>(data);
	*out_text = v[n].c_str();
	return true;
}

void EmitObjectUI(const KRE::Particles::EmitObjectPtr& eo)
{
	using namespace KRE::Particles;

	std::array<char, 64> text;
	std::fill(text.begin(), text.end(), 0);
	std::copy(eo->getName().begin(), eo->getName().end(), text.data());
	if(ImGui::InputText("Name", text.data(), text.size())) {
		eo->setName(text.data());
	}

	bool enabled = eo->isEnabled();
	if(ImGui::Checkbox("Enabled", &enabled)) {
		eo->setEnable(enabled);
	}

	bool debug_draw = eo->doDebugDraw();
	if(ImGui::Checkbox("Debug Draw", &debug_draw)) {
		eo->setDebugDraw(debug_draw);
	}
}

int main(int argc, char *argv[])
{
#ifdef _MSC_VER
	SetProcessDPIAware();
#endif

	std::list<double> smoothed_time;
	double cumulative_time = 0.0;
	int cnt = 0;

	using namespace KRE;

	SDL::SDL_ptr manager(new SDL::SDL());

	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

	WindowManager wm("SDL");

	variant_builder hints;
	hints.add("renderer", "GLESv2");
	hints.add("dpi_aware", true);
	hints.add("use_vsync", true);
	//int neww = 2560, newh = 1440;
	int neww = 1600, newh = 900;
	//if(!autoWindowSize(neww, newh)) {
	//	LOG_DEBUG("Couldn't get automatic window size. Defaulting to " << neww << "x" << newh);
	//}
	LOG_DEBUG("Creating window of size: " << neww << "x" << newh);
	auto main_wnd = wm.createWindow(neww, newh, hints.build());
	main_wnd->enableVsync(true);
	const float aspect_ratio = static_cast<float>(neww) / newh;

	std::string image_path;
	std::map<std::string, std::string> font_paths;
#if defined(__linux__)
	LOG_DEBUG("setting image file filter to 'images/'");
	Surface::setFileFilter(FileFilterType::LOAD, [](const std::string& fname) { return "images/" + fname; });
	Surface::setFileFilter(FileFilterType::SAVE, [](const std::string& fname) { return "images/" + fname; });
	
	font_paths["FreeSans.ttf"] = "data/fonts/FreeSans.ttf";
	image_path = "images/";
#else
	LOG_DEBUG("setting image file filter to '../images/'");
	Surface::setFileFilter(FileFilterType::LOAD, [](const std::string& fname) { return "../images/" + fname; });
	Surface::setFileFilter(FileFilterType::SAVE, [](const std::string& fname) { return "../images/" + fname; });

	font_paths["FreeSans.ttf"] = "../data/fonts/FreeSans.ttf";
	image_path = "../images/";
#endif
	Font::setAvailableFonts(font_paths);

	set_alpha_masks();	
	
	// XXX should a scenegraph be created from a specific window? It'd solve a couple of issues
	SceneGraphPtr scene = SceneGraph::create("main");
	SceneNodePtr root = scene->getRootNode();
	root->setNodeName("root_node");

	DisplayDevice::getCurrent()->setDefaultCamera(std::make_shared<Camera>("ortho1", 0, neww, 0, newh));
	//DisplayDevice::getCurrent()->setDefaultCamera(std::make_shared<Camera>("xxx", 45.0f, aspect_ratio, 0.01f, 99.0f));

	auto rman = std::make_shared<RenderManager>();
	auto rq = rman->addQueue(0, "opaques");

#if defined(__linux__)
	std::string shader_test_file = "data/shaders.cfg";
#else
	std::string shader_test_file = "../data/shaders.cfg";
#endif
	ShaderProgram::loadFromVariant(json::parse_from_file(shader_test_file));

#if defined(__linux__)
	std::string psys_test_file = "data/psystem4.cfg";
#else
	std::string psys_test_file = "../data/psystem4.cfg";
#endif

	Particles::ParticleSystemContainerPtr psystem = nullptr;

	try {
		psystem = Particles::ParticleSystemContainer::create(scene, json::parse_from_file(psys_test_file));
		root->attachNode(psystem);

		for(auto& p : psystem->getActiveParticleSystems()) {
			p->fastForward();
		}
	} catch(json::parse_error& e) {
		ASSERT_LOG(false, "parse error: " << e.what());
	}

	ASSERT_LOG(psystem != nullptr, "No particle system loaded.");

	Uint32 t = SDL_GetTicks();

	theme_imgui();

	auto ps_camera = std::make_shared<Camera>("ps_camera", 0, neww, 0, newh);
	psystem->getTechniques().front()->setCamera(ps_camera);

	SDL_Event e;
	bool done = false;
	while(!done) {
		while(SDL_PollEvent(&e)) {
#ifdef USE_IMGUI
			ImGui_ImplSdlGL3_ProcessEvent(&e);
#endif
			if(e.type == SDL_KEYUP && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				done = true;
			} else if(e.type == SDL_KEYDOWN) {
				LOG_DEBUG("KEY PRESSED: " << SDL_GetKeyName(e.key.keysym.sym) << " : " << e.key.keysym.sym << " : " << e.key.keysym.scancode);
			} else if(e.type == SDL_QUIT) {
				done = true;
			}
		}

		Uint32 cur_time = SDL_GetTicks();
		scene->process((cur_time - t) / 1000.0f);
		t = cur_time;

		//main_wnd->setClearColor(KRE::Color(255, 255, 255, 255));
		main_wnd->setClearColor(KRE::Color::colorBlack());
		main_wnd->clear(ClearFlags::ALL);

		scene->renderScene(rman);
		rman->render(main_wnd);

#ifdef USE_IMGUI
		{
			//ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiSetCond_Always);

			int n = 1;

			auto& ps = psystem->getActiveParticleSystems();
			auto& tq = ps.front()->getActiveTechniques();

			ImGui::Begin("Particle System Editor");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			if(ImGui::CollapsingHeader("Camera")) {
				static std::vector<std::string> camera_types{ "Perspective", "Orthogonal" };
					tq.front()->setCamera(ps_camera);
					int current_item = ps_camera->getType();
				
				if(ImGui::Combo("Type", &current_item, vector_string_getter, static_cast<void*>(&camera_types), camera_types.size())) {
					if(current_item == Camera::CAMERA_PERSPECTIVE) {
						ps_camera.reset(new Camera("ps_camera", 45.0, aspect_ratio, 0.01f, 100.0f));
					} else if(current_item == Camera::CAMERA_ORTHOGONAL) {
						ps_camera.reset(new Camera("ps_camera", 0, neww, 0, newh));
					} else {
						ASSERT_LOG(false, "Bad camera type: " << current_item);
					}
				}

				if(current_item == Camera::CAMERA_PERSPECTIVE) {
					
				} else if(current_item == Camera::CAMERA_ORTHOGONAL) {				
					int tbv[2]{ ps_camera->getOrthoTop(), ps_camera->getOrthoBottom() };
					if(ImGui::DragInt2("Top/Bottom", tbv, 1.0f, 0.0f, 4000.0f)) {
						ps_camera->setOrthoWindow(ps_camera->getOrthoLeft(),ps_camera->getOrthoRight(), tbv[0], tbv[1]);
					}
					int lrv[2]{ ps_camera->getOrthoLeft(), ps_camera->getOrthoRight() };
					if(ImGui::DragInt2("Left/Right", lrv, 1.0f, 0.0f, 4000.0f)) {
						ps_camera->setOrthoWindow(lrv[0], lrv[1], ps_camera->getOrthoTop(), ps_camera->getOrthoBottom());
					}
				}
			}

			{
				if(ImGui::CollapsingHeader("Particle System")) {
					auto&ps1 = ps.front();
					float sv = ps1->getScaleVelocity();
					if(ImGui::DragFloat("Scale Velocity", &sv, 0.5f, -100.0f, 100.0f)) {
						ps1->setScaleVelocity(sv);
					}
					float st = ps1->getScaleTime();
					if(ImGui::DragFloat("Scale Time", &st, 0.5f, 0.1f, 100.0f)) {
						ps1->setScaleTime(st);
					}
					glm::vec3 scale_dims = ps1->getScaleDimensions();
					float sd[3]{ scale_dims[0], scale_dims[1], scale_dims[2] };
					if(ImGui::DragFloat3("Scale Dimensions", sd, 0.1f, 0.1f, 100.0f)) {
						ps1->setScaleDimensions(sd);
					}

					// technique stuff
					auto q = tq.front();
					auto dim = q->getDefaultDimensions();
				
					if(ImGui::SliderFloat("Default Width", &dim.x, 0.0f, 100.0f)) {
						q->setDefaultWidth(dim.x);
					}
					if(ImGui::SliderFloat("Default Height", &dim.y, 0.0f, 100.0f)) {
						q->setDefaultHeight(dim.y);
					}
					if(ImGui::SliderFloat("Default Depth", &dim.z, 0.0f, 100.0f)) {
						q->setDefaultDepth(dim.z);
					}

					int quota = q->getQuota();
					if(ImGui::DragInt("Particle Quota", &quota, 1, 1, 100000)) {
						q->setParticleQuota(quota);
					}

					int current_tex = 0;

					sys::file_path_map image_files;
					sys::get_unique_files(image_path, image_files);
					std::vector<std::string> ifiles;
					for(auto& f : image_files) {
						ifiles.emplace_back(f.first);
					}
				
					std::string selected_texture = q->getTexture()->getSurface(0)->getName();
					int current_file = std::distance(ifiles.begin(), std::find(ifiles.begin(), ifiles.end(), selected_texture));
					ImGui::Text(selected_texture.c_str());
					if(ImGui::ListBox("Textures", &current_file, vector_string_getter, static_cast<void*>(&ifiles), ifiles.size())) {
						q->setTexture(Texture::createTexture(ifiles[current_file]));
					}

					bool has_max_velocity = q->hasMaxVelocity();
					if(ImGui::Checkbox("Has Max Velocity", &has_max_velocity)) {
						if(has_max_velocity) {
							q->setMaxVelocity(0);
						} else {
							q->clearMaxVelocity();
						}
					}

					if(q->hasMaxVelocity()) {
						float maxv = q->getMaxVelocity();
						if(ImGui::DragFloat("Max Velocity", &maxv, 1.0f, 0.0f, 1000.0f)) {
							q->setMaxVelocity(maxv);
						}
					}

					// stuff from renderable
					bool ignore_global_mm = q->ignoreGlobalModelMatrix();
					if(ImGui::Checkbox("Ignore Global Transform", &ignore_global_mm)) {
						q->useGlobalModelMatrix(ignore_global_mm);
					}
					const auto& pos = q->getPosition();
					float v[3]{ pos.x, pos.y, pos.z };
					if(ImGui::DragFloat3("Position", v)) {
						q->setPosition(v[0], v[1], v[2]);
					}
					// blend mode
					const auto& bm = q->getBlendMode();
					auto blend_modes = BlendMode::getBlendModeStrings();

					std::string bm_string = bm.to_string();
					auto it = std::find(blend_modes.begin(), blend_modes.end(), bm_string);
					int current_item = 0;
					if(it != blend_modes.end()) {
						current_item = std::distance(blend_modes.begin(), it);
					}
				
					if(ImGui::Combo("Blend Mode", &current_item, vector_string_getter, static_cast<void*>(&blend_modes), blend_modes.size())) {
						q->setBlendMode(BlendMode(blend_modes[current_item]));
					}

					bool depth_write = q->isDepthWriteEnable();
					if(ImGui::Checkbox("Depth Write", &depth_write)) {
						q->setDepthWrite(depth_write);
					}
					bool depth_check = q->isDepthEnabled();
					if(ImGui::Checkbox("Depth Check", &depth_check)) {
						q->setDepthEnable(depth_check);
					}
				}
			}
			
			std::vector<std::pair<Particles::EmitterPtr, Particles::EmitterPtr>> emitter_replace;
			for(auto& e : tq.front()->getActiveEmitters()) {
				std::stringstream ss;
				ss << "Emitter " << n++;

				if(ImGui::CollapsingHeader(ss.str().c_str())) {
					Particles::EmitterType type = e->getType();

					EmitObjectUI(e);

					const char* const ptype[] = { "Point", "Line", "Box", "Circle", "Sphere Surface" };
					int current_type = static_cast<int>(type);
					if(ImGui::Combo("Type", &current_type, ptype, 5)) {
						auto new_e = Particles::Emitter::factory(psystem, static_cast<Particles::EmitterType>(current_type));
						emitter_replace.emplace_back(std::make_pair(e, new_e));
					}

					ParameterGui("Emission Rate", e->getEmissionRate());
					ParameterGui("Time to live", e->getTimeToLive());
					ParameterGui("Velocity", e->getVelocity());
					ParameterGui("Angle", e->getAngle(), 0.0f, 360.0f);
					ParameterGui("Mass", e->getMass());
					ParameterGui("Duration", e->getDuration());
					ParameterGui("Repeat Delay", e->getRepeatDelay());
					//orientation_range_
					//color_range_
					float col[4] = { e->getColorFloat().r, e->getColorFloat().g, e->getColorFloat().b, e->getColorFloat().a };
					if(ImGui::ColorEdit4("color", col, true)) {
						e->setColor(glm::vec4(col[0], col[1], col[2], col[3]));
					}
					ParameterGui("Width", e->getParticleWidth());
					ParameterGui("Height", e->getParticleHeight());
					ParameterGui("Depth", e->getParticleDepth());

					bool force_emission = e->getForceEmission();
					if(ImGui::Checkbox("Force Emission", &force_emission)) {
						e->setForceEmission(force_emission);
					}
					bool can_be_deleted = e->getCanBeDeleted();
					if(ImGui::Checkbox("Can Be Deleted", &can_be_deleted)) {
						e->setCanBeDeleted(can_be_deleted);
					}

					switch (type) {
					case Particles::EmitterType::POINT: {
						//auto& pe = std::dynamic_pointer_cast<Particles::PointEmitter>(e);
						// no extra UI
						break;
					}
					case KRE::Particles::EmitterType::LINE: {
						auto& le = std::dynamic_pointer_cast<Particles::LineEmitter>(e);
						float mini = le->getMinIncrement();
						if(ImGui::DragFloat("Min Increment", &mini, 0.1f, 0.0f, 100.0f)) {
							le->setMinIncrement(mini);
						}
						float maxi = le->getMaxIncrement();
						if(ImGui::DragFloat("Max Increment", &maxi, 0.1f, 0.0f, 100.0f)) {
							le->setMinIncrement(maxi);
						}
						float ld = le->getLineDeviation();
						if(ImGui::DragFloat("Line Deviation", &ld, 0.1f, 0.0f, 100.0f)) {
							le->setLineDeviation(ld);
						}
						break;
					}
					case KRE::Particles::EmitterType::BOX: {
						auto& be = std::dynamic_pointer_cast<Particles::BoxEmitter>(e);
						const auto& dims = be->getDimensions();
						float v[3]{ dims.x, dims.y, dims.z };
						if(ImGui::SliderFloat3("Dimensions", v, 0.0f, 100.0f)) {
							be->setDimensions(v);
						}
						break;
					}
					case KRE::Particles::EmitterType::CIRCLE: {
						auto& ce = std::dynamic_pointer_cast<Particles::CircleEmitter>(e);
						ParameterGui("Radius", ce->getRadius());
						float step = ce->getStep();
						if(ImGui::DragFloat("Step", &step, 0.1f, 0.0f, 100.0f)) {
							ce->setStep(step);
						}
						float angle = ce->getAngle();
						if(ImGui::DragFloat("Angle", &angle, 0.1f, 0.0f, 100.0f)) {
							ce->setAngle(angle);
						}
						bool random_loc = ce->isRandomLocation();
						if(ImGui::Checkbox("Random Location", &random_loc)) {
							ce->setRandomLocation(random_loc);
						}
						break;
					}
					case KRE::Particles::EmitterType::SPHERE_SURFACE: {
						auto& sse = std::dynamic_pointer_cast<Particles::SphereSurfaceEmitter>(e);
						ParameterGui("Radius", sse->getRadius());
						break;
					}
					default:
						ASSERT_LOG(false, "Unrecognized emitter type: " << static_cast<int>(type));
						break;
					}
				}
			}
			for(auto& e : emitter_replace) {
				auto it = std::find(tq.front()->getActiveEmitters().begin(), tq.front()->getActiveEmitters().end(), e.first);
				if(it != tq.front()->getActiveEmitters().end()) {
					tq.front()->getActiveEmitters().erase(it);
				}
				tq.front()->getActiveEmitters().emplace_back(e.second);
				e.second->init(tq.front());
			}

			std::vector<std::pair<Particles::AffectorPtr, Particles::AffectorPtr>> affector_replace;
			n = 1;
			for(auto& a : tq.front()->getActiveAffectors()) {
				std::stringstream ss;
				ss << "Affector " << n++ << " " << Particles::get_affector_name(a->getType());
				// should have a function to get pretty name of affector here, based on type.
				if(ImGui::CollapsingHeader(ss.str().c_str())) {
					EmitObjectUI(a);

					const char* const ptype[] = { "Color", "Jet", "Vortex", "Gravity", "Linear Force", "Scale", "Particle Follower", "Align", "Flock Centering", "Black Hole", "Path Follower", "Radomizer", "Sine Force" };
					int current_type = static_cast<int>(a->getType());
					if(ImGui::Combo("Type", &current_type, ptype, 13)) {
						auto new_a = Particles::Affector::factory(psystem, static_cast<Particles::AffectorType>(current_type));
						affector_replace.emplace_back(std::make_pair(a, new_a));
					}

					//mass
					float mass = a->getMass();
					if(ImGui::SliderFloat("Mass", &mass, 0.0f, 1000.0f)) {
						a->setMass(mass);
					}
					//position
					const auto& pos = a->getPosition();
					float posf[3] = { pos.x, pos.y, pos.z };
					if(ImGui::SliderFloat3("Position", posf, 0.0f, 1000.0f)) {
						a->setPosition(glm::vec3(posf[0], posf[1], posf[2]));
					}
					//scale
					const auto& scale = a->getScale();
					float scalef[3] = { scale.x, scale.y, scale.z };
					if(ImGui::SliderFloat3("Scale", scalef, 0.0f, 1000.0f)) {
						a->setScale(glm::vec3(scalef[0], scalef[1], scalef[2]));
					}

					switch(a->getType()) {
						case Particles::AffectorType::COLOR: {
							auto tca = std::dynamic_pointer_cast<Particles::TimeColorAffector>(a);
							auto op = tca->getOperation();
							int current_item = static_cast<int>(op);
							const char* const optype[] = { "Set", "Multiply" };
							if(ImGui::Combo("Operation", &current_item, optype, 2)) {
								tca->setOperation(static_cast<Particles::TimeColorAffector::ColourOperation>(current_item));
							}					
							auto tcdata = tca->getTimeColorData();
							bool data_changed = false;
							ImGui::BeginGroup();
							if(ImGui::SmallButton("Clear")) {
								tca->clearTimeColorData();
							}
							ImGui::SameLine();
							if(ImGui::SmallButton("+")) {
								tca->addTimecolorEntry(std::make_pair(0.0f, glm::vec4(0.0f)));
							}
							ImGui::EndGroup();
							int tc_counter = 1;
							std::vector<Particles::TimeColorAffector::tc_pair> tc_data_to_remove;
							for(auto& tc : tcdata) {
								std::stringstream tlabel;
								tlabel << "T##" << tc_counter;
								ImGui::BeginGroup();
								ImGui::PushItemWidth(ImGui::CalcItemWidth() * 0.5f);
								if(ImGui::DragFloat(tlabel.str().c_str(), &tc.first, 0.01f, 0.0f, 1.0f)) {
									data_changed = true;
								}
								ImGui::SameLine();
								float col[4] = { tc.second.r,tc.second.g, tc.second.b, tc.second.a };
								std::stringstream clabel;
								clabel << "C##" << tc_counter;
								if(ImGui::ColorEdit4(clabel.str().c_str(), col)) {
								//if(ColorPicker4(col, true)) {
									tc.second = glm::vec4(col[0], col[1], col[2], col[3]);
									data_changed = true;
								}
								ImGui::PopItemWidth();
								ImGui::SameLine();
								std::stringstream xlabel;
								xlabel << "X##" << tc_counter;
								if(ImGui::SmallButton(xlabel.str().c_str())) {
									tc_data_to_remove.emplace_back(tc);
									data_changed = true;
								}
								ImGui::EndGroup();
								++tc_counter;
							}
							for(auto& p : tc_data_to_remove) {
								auto it = std::find(tcdata.begin(), tcdata.end(), p);
								if(it != tcdata.end()) {
									tcdata.erase(it);
									data_changed = true;
								}
							}
							if(data_changed) {
								tca->setTimeColorData(tcdata);
							}
							break;
						}
						case Particles::AffectorType::JET: {
							auto ja = std::dynamic_pointer_cast<Particles::JetAffector>(a);
							ParameterGui("acceleration", ja->getAcceleration(), 0.0f, 100.0f);
							break;
						}
						case Particles::AffectorType::VORTEX: {
							auto va = std::dynamic_pointer_cast<Particles::VortexAffector>(a);
							// rotation axis
							ImGui::BeginGroup();
							if(ImGui::Button(" +X ")) {
								va->setRotationAxis(glm::vec3(1.0f, 0.0f, 0.0f));
							}
							ImGui::SameLine();
							if(ImGui::Button(" +Y ")) {
								va->setRotationAxis(glm::vec3(0.0f, 1.0f, 0.0f));
							}
							ImGui::SameLine();
							if(ImGui::Button(" +Z ")) {
								va->setRotationAxis(glm::vec3(0.0f, 0.0f, 1.0f));
							}
							if(ImGui::Button(" -X ")) {
								va->setRotationAxis(glm::vec3(-1.0f, 0.0f, 0.0f));
							}
							ImGui::SameLine();
							if(ImGui::Button(" -Y ")) {
								va->setRotationAxis(glm::vec3(0.0f, -1.0f, 0.0f));
							}
							ImGui::SameLine();
							if(ImGui::Button(" -Z ")) {
								va->setRotationAxis(glm::vec3(0.0f, 0.0f, -1.0f));
							}
							const auto& axis = va->getRotationAxis();
							float v[3] { axis.x, axis.y, axis.z };
							if(ImGui::SliderFloat3("Rotation Axis", v, -1.0f, 1.0f)) {
								va->setRotationAxis(glm::vec3(v[0], v[1], v[2]));
							}
							ImGui::EndGroup();
							// rotation speed
							ParameterGui("Rotation Speed", va->getRotationSpeed());
							break;
						}
						case Particles::AffectorType::GRAVITY: {
							auto ga = std::dynamic_pointer_cast<Particles::GravityAffector>(a);
							ParameterGui("Gravity", ga->getGravity());
							break;
						}
						case Particles::AffectorType::LINEAR_FORCE: {
							auto fa = std::dynamic_pointer_cast<Particles::LinearForceAffector>(a);
							ParameterGui("Force", fa->getForce());

							// rdirection
							ImGui::BeginGroup();
							if(ImGui::Button(" +X ")) {
								fa->setDirection(glm::vec3(1.0f, 0.0f, 0.0f));
							}
							ImGui::SameLine();
							if(ImGui::Button(" +Y ")) {
								fa->setDirection(glm::vec3(0.0f, 1.0f, 0.0f));
							}
							ImGui::SameLine();
							if(ImGui::Button(" +Z ")) {
								fa->setDirection(glm::vec3(0.0f, 0.0f, 1.0f));
							}
							if(ImGui::Button(" -X ")) {
								fa->setDirection(glm::vec3(-1.0f, 0.0f, 0.0f));
							}
							ImGui::SameLine();
							if(ImGui::Button(" -Y ")) {
								fa->setDirection(glm::vec3(0.0f, -1.0f, 0.0f));
							}
							ImGui::SameLine();
							if(ImGui::Button(" -Z ")) {
								fa->setDirection(glm::vec3(0.0f, 0.0f, -1.0f));
							}

							const auto& dir = fa->getDirection();
							float v[3]{ dir[0], dir[1], dir[2] };
							if(ImGui::SliderFloat3("Direction", v, -1.0f, 1.0f)) {
								fa->setDirection(glm::vec3(v[0], v[1], v[2]));
							}
							ImGui::EndGroup();
							break;
						}
						case Particles::AffectorType::SCALE: {
							auto& sa = std::dynamic_pointer_cast<Particles::ScaleAffector>(a);
							auto& xyz_scale = sa->getScaleXYZ();
							auto& x_scale = sa->getScaleX();
							auto& y_scale = sa->getScaleY();
							auto& z_scale = sa->getScaleZ();
							// XXX need some way to deal with locked scales as opposed to
							// independent scales.
							if(xyz_scale) {
								ParameterGui("XYZ Scale", xyz_scale, 0.0f, 100.0f);
							} else {
								if(x_scale) {
									ParameterGui("X Scale", x_scale, 0.0f, 100.0f);
								}
								if(y_scale) {
									ParameterGui("Y Scale", y_scale, 0.0f, 100.0f);
								}
								if(z_scale) {
									ParameterGui("Z Scale", z_scale, 0.0f, 100.0f);
								}
							}
							break;
						}
						case Particles::AffectorType::PARTICLE_FOLLOWER: {
							auto& pfa = std::dynamic_pointer_cast<Particles::ParticleFollowerAffector>(a);
							float minmaxd[2]{ pfa->getMinDistance(), pfa->getMaxDistance() };
							if(ImGui::DragFloat2("Min/Max Distance", minmaxd, 1.0f, 0.0f, 1000.0f)) {
								pfa->setMinDistance(minmaxd[0]);
								pfa->setMaxDistance(minmaxd[1]);
							}
							break;
						}
						case Particles::AffectorType::ALIGN: {
							auto& aa = std::dynamic_pointer_cast<Particles::AlignAffector>(a);
							bool resize = aa->getResizeable();
							if(ImGui::Checkbox("Resize", &resize)) {
								aa->setResizeable(resize);
							}
							break;
						}
						case Particles::AffectorType::FLOCK_CENTERING: {
							// no extra UI components.
							break;
						}
						case Particles::AffectorType::BLACK_HOLE: {
							auto& bha = std::dynamic_pointer_cast<Particles::BlackHoleAffector>(a);
							ParameterGui("Velocity", bha->getVelocity());
							ParameterGui("Acceleration", bha->getAcceleration());
							break;
						}
						case Particles::AffectorType::PATH_FOLLOWER: {
							// XXX todo
							break;
						}
						case Particles::AffectorType::RANDOMISER: {
							auto& ra = std::dynamic_pointer_cast<Particles::RandomiserAffector>(a);
							float ts = ra->getTimeStep();
							if(ImGui::DragFloat("Time Step", &ts, 1.0f, 0.0f, 10.0f)) {
								ra->setTimeStep(ts);
							}

							bool random_direction = ra->isRandomDirection();
							if(ImGui::Checkbox("Random Direction", &random_direction)) {
								ra->setRandomDirection(random_direction);
							}

							auto& max_deviation = ra->getDeviation();
							float v[3]{ max_deviation.x, max_deviation.y, max_deviation.z };
							if(ImGui::DragFloat3("Max Deviation", v, 0.1f, 0.0f, 100.0f)) {
								ra->setDeviation(v[0], v[1], v[2]);
							}
							break;
						}
						case Particles::AffectorType::SINE_FORCE: {
							auto& sfa = std::dynamic_pointer_cast<Particles::SineForceAffector>(a);

							float v[2]{ sfa->getMinFrequency(), sfa->getMaxFrequency() };
							if(ImGui::DragFloat2("Min/Max Frequency", v, 0.1f, 0.0f, 1000.0f)) {
								sfa->setMinFrequency(v[0]);
								sfa->setMaxFrequency(v[1]);
							}

							auto& force_vector = sfa->getForceVector();
							float fv[3]{ force_vector.x, force_vector.y, force_vector.z };
							if(ImGui::DragFloat3("Force Vector", fv, 0.1f, 0.0f, 1000.0f)) {
								sfa->setForceVector(fv[0], fv[1], fv[2]);
							}
							auto op = sfa->getForceApplication();
							int current_item = static_cast<int>(op);
							const char* const optype[] = { "Add", "Average" };
							if(ImGui::Combo("Force Application", &current_item, optype, 2)) {
								sfa->setForceApplication(static_cast<Particles::SineForceAffector::ForceApplication>(current_item));
							}			
							break;
						}
						default:
							ASSERT_LOG(false, "Unhandled affector type: " << static_cast<int>(a->getType()));
							break;
					}
					//excluded emitters
				}
			}
			for(auto& aff : affector_replace) {
				auto it = std::find(tq.front()->getActiveAffectors().begin(), tq.front()->getActiveAffectors().end(), aff.first);
				if(it != tq.front()->getActiveAffectors().end()) {
					tq.front()->getActiveAffectors().erase(it);
				}
				tq.front()->getActiveAffectors().emplace_back(aff.second);
				aff.second->setParentTechnique(tq.front());
			}
		}
		ImGui::End();
#endif

		main_wnd->swap();
	}
	return 0;
}
