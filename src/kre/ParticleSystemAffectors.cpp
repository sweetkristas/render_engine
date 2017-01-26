/*
	Copyright (C) 2013-2014 by Kristina Simpson <sweet.kristas@gmail.com>
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

#include "asserts.hpp"
#include "ParticleSystem.hpp"
#include "ParticleSystemAffectors.hpp"
#include "ParticleSystemEmitters.hpp"
#include "ParticleSystemParameters.hpp"
#include "variant_utils.hpp"

namespace KRE
{
	namespace Particles
	{
		const char* get_affector_name(AffectorType type)
		{
			switch(type) {
				case KRE::Particles::AffectorType::COLOR:				return "Time/Color";
				case KRE::Particles::AffectorType::JET:					return "jet";
				case KRE::Particles::AffectorType::VORTEX:				return "Vortex";
				case KRE::Particles::AffectorType::GRAVITY:				return "Gravity";
				case KRE::Particles::AffectorType::LINEAR_FORCE:		return "Linear Force";
				case KRE::Particles::AffectorType::SCALE:				return "Scale";
				case KRE::Particles::AffectorType::PARTICLE_FOLLOWER:	return "Particle Follower";
				case KRE::Particles::AffectorType::ALIGN:				return "Align";
				case KRE::Particles::AffectorType::FLOCK_CENTERING:		return "Flock Centering";
				case KRE::Particles::AffectorType::BLACK_HOLE:			return "Black Hole";
				case KRE::Particles::AffectorType::PATH_FOLLOWER:		return "Path Follower";
				case KRE::Particles::AffectorType::RANDOMISER:			return "Randomizer";
				case KRE::Particles::AffectorType::SINE_FORCE:			return "Sine Force";
				default:
					ASSERT_LOG(false, "No name for affector: " << static_cast<int>(type));
					break;
			}
			return nullptr;
		}

		// affectors to add: box_collider (width,height,depth, inner or outer collide, friction)
		// forcefield (delta, force, octaves, frequency, amplitude, persistence, size, worldsize(w,h,d), movement(x,y,z),movement_frequency)
		// geometry_rotator (use own rotation, speed(parameter), axis(x,y,z))
		// inter_particle_collider (sounds like a lot of calculations)
		// line
		// linear_force
		// path_follower
		// plane_collider
		// scale_velocity (parameter_ptr scale; bool since_system_start, bool stop_at_flip)
		// sphere_collider
		// texture_animator
		// texture_rotator
		// velocity matching

		Affector::Affector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node, AffectorType type)
			: EmitObject(parent, node), 
			  type_(type),
			  mass_(float(node["mass_affector"].as_float(1.0f))),
			  position_(0.0f), 
			  scale_(1.0f),
			  node_(node)
		{
			if(node.has_key("position")) {
				position_ = variant_to_vec3(node["position"]);
			}
			if(node.has_key("exclude_emitters")) {
				if(node["exclude_emitters"].is_list()) {
					excluded_emitters_ = node["exclude_emitters"].as_list_string();
				} else {
					excluded_emitters_.push_back(node["exclude_emitters"].as_string());
				}
			}
		}
		
		Affector::~Affector()
		{
		}

		TechniquePtr Affector::getTechnique() const
		{
			auto tq = technique_.lock();
			ASSERT_LOG(tq != nullptr, "No parent technique found.");
			return tq;
		}

		void Affector::handleEmitProcess(float t) 
		{
			auto tq = getTechnique();
			for(auto& e : tq->getActiveEmitters()) {
				if(e->emitted_by != nullptr) {
					if(!isEmitterExcluded(e->emitted_by->name())) {
						internalApply(*e,t);
					}
				}
			}
			for(auto& p : tq->getActiveParticles()) {
				ASSERT_LOG(p.emitted_by != nullptr, "p.emitted_by is null");
				if(!isEmitterExcluded(p.emitted_by->name())) {
					internalApply(p,t);
				}
			}
		}

		bool Affector::isEmitterExcluded(const std::string& name) const
		{
			return std::find(excluded_emitters_.begin(), excluded_emitters_.end(), name) != excluded_emitters_.end();
		}

		AffectorPtr Affector::factory(std::weak_ptr<ParticleSystemContainer> parent, const variant& node)
		{
			ASSERT_LOG(node.has_key("type"), "affector must have 'type' attribute");
			const std::string& ntype = node["type"].as_string();
			if(ntype == "color" || ntype == "colour") {
				return std::make_shared<TimeColorAffector>(parent, node);
			} else if(ntype == "jet") {
				return std::make_shared<JetAffector>(parent, node);
			} else if(ntype == "vortex") {
				return std::make_shared<VortexAffector>(parent, node);
			} else if(ntype == "gravity") {
				return std::make_shared<GravityAffector>(parent, node);
			} else if(ntype == "linear_force") {
				return std::make_shared<LinearForceAffector>(parent, node);
			} else if(ntype == "scale") {
				return std::make_shared<ScaleAffector>(parent, node);
			} else if(ntype == "particle_follower") {
				return std::make_shared<ParticleFollowerAffector>(parent, node);
			} else if(ntype == "align") {
				return std::make_shared<AlignAffector>(parent, node);
			} else if(ntype == "randomiser" || ntype == "randomizer") {
				return std::make_shared<RandomiserAffector>(parent, node);
			} else if(ntype == "sine_force" || ntype == "sin_force") {
				return std::make_shared<SineForceAffector>(parent, node);
			} else if(ntype == "path_follower") {
				return std::make_shared<PathFollowerAffector>(parent, node);
			} else if(ntype == "black_hole") {
				return std::make_shared<BlackHoleAffector>(parent, node);
			} else if(ntype == "flock_centering") {
				return std::make_shared<FlockCenteringAffector>(parent, node);
			} else {
				ASSERT_LOG(false, "Unrecognised affector type: " << ntype);
			}
			return nullptr;
		}

		TimeColorAffector::TimeColorAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node)
			: Affector(parent, node, AffectorType::COLOR), 
			  operation_(ColourOperation::COLOR_OP_SET)
		{
			init(node);
		}

		void TimeColorAffector::init(const variant& node)
		{
			std::string op;
			if(node.has_key("color_operation")) {
				op = node["color_operation"].as_string();
			} else if(node.has_key("colour_operation")) {
				op = node["colour_operation"].as_string();
			}
			if(!op.empty()) {
				if(op == "multiply") {
					operation_ = ColourOperation::COLOR_OP_MULTIPLY;
				} else if(op == "set") {
					operation_ = ColourOperation::COLOR_OP_SET;
				} else {
					ASSERT_LOG(false, "unrecognised time_color affector operation: " << op);
				}
			}
			ASSERT_LOG(node.has_key("time_colour") || node.has_key("time_color"), "Must be a 'time_colour' attribute");
			const variant& tc_node = node.has_key("time_colour") ? node["time_colour"] : node["time_color"];
			if(tc_node.is_map()) {
				float t = tc_node["time"].as_float();
				glm::vec4 result;
				if(tc_node.has_key("color")) {
					ASSERT_LOG(tc_node["color"].is_list() && tc_node["color"].num_elements() == 4, "Expected vec4 variant but found " << tc_node["color"].write_json());
					result.r = tc_node["color"][0].as_float();
					result.g = tc_node["color"][1].as_float();
					result.b = tc_node["color"][2].as_float();
					result.a = tc_node["color"][3].as_float();
				} else if(tc_node.has_key("colour")) {
					ASSERT_LOG(tc_node["colour"].is_list() && tc_node["colour"].num_elements() == 4, "Expected vec4 variant but found " << tc_node["colour"].write_json());
					result.r = tc_node["colour"][0].as_float();
					result.g = tc_node["colour"][1].as_float();
					result.b = tc_node["colour"][2].as_float();
					result.a = tc_node["colour"][3].as_float();
				} else {
					ASSERT_LOG(false, "PSYSTEM2, time_colour nodes must have a 'color' or 'colour' attribute");
				}
				tc_data_.push_back(std::make_pair(t, result));
			} else if(tc_node.is_list()) {
				for(int n = 0; n != tc_node.num_elements(); ++n) {
					float t = tc_node[n]["time"].as_float();
					glm::vec4 result;
					if(tc_node[n].has_key("color")) {
						ASSERT_LOG(tc_node[n]["color"].is_list() && tc_node[n]["color"].num_elements() == 4, "Expected vec4 variant but found " << tc_node[n]["color"].write_json());
						result.r = tc_node[n]["color"][0].as_float();
						result.g = tc_node[n]["color"][1].as_float();
						result.b = tc_node[n]["color"][2].as_float();
						result.a = tc_node[n]["color"][3].as_float();
					} else if(tc_node[n].has_key("colour")) {
						ASSERT_LOG(tc_node[n]["colour"].is_list() && tc_node[n]["colour"].num_elements() == 4, "Expected vec4 variant but found " << tc_node[n]["colour"].write_json());
						result.r = tc_node[n]["colour"][0].as_float();
						result.g = tc_node[n]["colour"][1].as_float();
						result.b = tc_node[n]["colour"][2].as_float();
						result.a = tc_node[n]["colour"][3].as_float();
					} else {
						ASSERT_LOG(false, "PSYSTEM2, time_colour nodes must have a 'color' or 'colour' attribute");
					}
					tc_data_.push_back(std::make_pair(t, result));
				}
			}
			sort_tc_data();
		}

		void TimeColorAffector::sort_tc_data()
		{
			std::sort(tc_data_.begin(), tc_data_.end(), [](const tc_pair& lhs, const tc_pair& rhs){
				return lhs.first < rhs.first;
			});
		}

		void TimeColorAffector::internalApply(Particle& p, float t)
		{
			glm::vec4 c;
			float ttl_percentage = 1.0f - p.current.time_to_live / p.initial.time_to_live;
			auto it1 = find_nearest_color(ttl_percentage);
			auto it2 = it1 + 1;
			if(it2 != tc_data_.end()) {
				c = it1->second + ((it2->second - it1->second) * ((ttl_percentage - it1->first)/(it2->first - it1->first)));
			} else {
				c = it1->second;
			}
			if(operation_ == ColourOperation::COLOR_OP_SET) {
				p.current.color = color_vector(color_vector::value_type(c.r*255.0f), 
					color_vector::value_type(c.g*255.0f), 
					color_vector::value_type(c.b*255.0f), 
					color_vector::value_type(c.a*255.0f));
			} else {
				p.current.color = color_vector(color_vector::value_type(c.r*p.initial.color.r), 
					color_vector::value_type(c.g*p.initial.color.g), 
					color_vector::value_type(c.b*p.initial.color.b), 
					color_vector::value_type(c.a*p.initial.color.a));
			}
		}

		// Find nearest iterator to the time fraction "dt"
		std::vector<TimeColorAffector::tc_pair>::iterator TimeColorAffector::find_nearest_color(float dt)
		{
			auto it = tc_data_.begin();
			for(; it != tc_data_.end(); ++it) {
				if(dt < it->first) {
					if(it == tc_data_.begin()) {
						return it;
					} else {
						return --it;
					}
				} 
			}
			return --it;
		}

		JetAffector::JetAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node)
			: Affector(parent, node, AffectorType::JET)
		{
			init(node);
		}

		void JetAffector::init(const variant& node)
		{
			if(node.has_key("acceleration")) {
				acceleration_ = Parameter::factory(node["acceleration"]);
			} else {
				acceleration_.reset(new Parameter(1.0f));
			}
		}

		void JetAffector::internalApply(Particle& p, float t)
		{
			float scale = t * acceleration_->getValue(1.0f - p.current.time_to_live/p.initial.time_to_live);
			if(p.current.direction.x == 0 && p.current.direction.y == 0 && p.current.direction.z == 0) {
				p.current.direction += p.initial.direction * scale;
			} else {
				p.current.direction += p.initial.direction * scale;
			}
		}

		VortexAffector::VortexAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node)
			: Affector(parent, node, AffectorType::VORTEX), 
			  rotation_axis_(0.0f, 1.0f, 0.0f)
		{
			init(node);
		}

		void VortexAffector::init(const variant& node)
		{
			if(node.has_key("rotation_speed")) {
				rotation_speed_ = Parameter::factory(node["rotation_speed"]);
			} else {
				rotation_speed_.reset(new Parameter(1.0f));
			}
			if(node.has_key("rotation_axis")) {
				rotation_axis_ = variant_to_vec3(node["rotation_axis"]);
			}
		}

		void VortexAffector::internalApply(Particle& p, float t)
		{
			glm::vec3 local = p.current.position - getPosition();
			float spd = rotation_speed_->getValue(getTechnique()->getParticleSystem()->getElapsedTime());
			glm::quat rotation = glm::angleAxis(glm::radians(spd), rotation_axis_);
			p.current.position = getPosition() + rotation * local;
			p.current.direction = rotation * p.current.direction;
		}

		GravityAffector::GravityAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node)
			: Affector(parent, node, AffectorType::GRAVITY), 
			  gravity_()
		{
			init(node);
		}

		void GravityAffector::init(const variant& node)
		{
			if(node.has_key("gravity")) {
				gravity_ = Parameter::factory(node["gravity"]);
			} else {
				gravity_ .reset(new Parameter(1.0f));
			}
		}

		void GravityAffector::internalApply(Particle& p, float t)
		{
			glm::vec3 d = getPosition() - p.current.position;
			float len_sqr = sqrt(d.x*d.x + d.y*d.y + d.z*d.z);
			if(len_sqr > 0) {
				float force = (gravity_->getValue(t) * p.current.mass * getMass()) / len_sqr;
				p.current.direction += (force * t) * d;
			}
		}

		ScaleAffector::ScaleAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node)
			: Affector(parent, node, AffectorType::SCALE), 
			  since_system_start_(false)
		{
			init(node);
		}

		void ScaleAffector::init(const variant& node)
		{
			since_system_start_ = (node["since_system_start"].as_bool(false));
			if(node.has_key("scale_x")) {
				scale_x_ = Parameter::factory(node["scale_x"]);
			}
			if(node.has_key("scale_y")) {
				scale_y_ = Parameter::factory(node["scale_y"]);
			}
			if(node.has_key("scale_z")) {
				scale_z_ = Parameter::factory(node["scale_z"]);
			}
			if(node.has_key("scale_xyz")) {
				scale_xyz_ = Parameter::factory(node["scale_xyz"]);
			}
		}

		float ScaleAffector::calculateScale(ParameterPtr s, const Particle& p)
		{
			float scale;
			if(since_system_start_) {
				scale = s->getValue(getTechnique()->getParticleSystem()->getElapsedTime());
			} else {
				scale = s->getValue(1.0f - p.current.time_to_live / p.initial.time_to_live);
			}
			return scale;
		}

		void ScaleAffector::internalApply(Particle& p, float t)
		{
			if(scale_xyz_) {
				float calc_scale = calculateScale(scale_xyz_, p);
				float value = p.initial.dimensions.x * calc_scale * getScale().x;
				if(value > 0) {
					p.current.dimensions.x = value;
				}
				value = p.initial.dimensions.y * calc_scale * getScale().y;
				if(value > 0) {
					p.current.dimensions.y = value;
				}
				value = p.initial.dimensions.z * calc_scale * getScale().z;
				if(value > 0) {
					p.current.dimensions.z = value;
				}
			} else {
				if(scale_x_) {
					float calc_scale = calculateScale(scale_x_, p);
					float value = p.initial.dimensions.x * calc_scale * getScale().x;
					if(value > 0) {
						p.current.dimensions.x = value;
					}
				}
				if(scale_y_) {
					float calc_scale = calculateScale(scale_y_, p);
					float value = p.initial.dimensions.x * calc_scale * getScale().y;
					if(value > 0) {
						p.current.dimensions.y = value;
					}
				}
				if(scale_z_) {
					float calc_scale = calculateScale(scale_z_, p);
					float value = p.initial.dimensions.z * calc_scale * getScale().z;
					if(value > 0) {
						p.current.dimensions.z = value;
					}
				}
			}
		}

		LinearForceAffector::LinearForceAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node)
			: Affector(parent, node, AffectorType::LINEAR_FORCE)
		{
			init(node);
		}

		void LinearForceAffector::init(const variant& node)
		{
			if(node.has_key("force")) {
				force_ = Parameter::factory(node["force"]);
			} else {
				force_.reset(new Parameter(1.0f));
			}

			direction_ = variant_to_vec3(node["direction"]);
		}

		void LinearForceAffector::internalApply(Particle& p, float t) 
		{
			float scale = t * force_->getValue(1.0f - p.current.time_to_live/p.initial.time_to_live);
			p.current.position += direction_*scale;
		}

		ParticleFollowerAffector::ParticleFollowerAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node)
			: Affector(parent, node, AffectorType::PARTICLE_FOLLOWER),
			min_distance_(node["min_distance"].as_float(1.0f)),
			max_distance_(node["max_distance"].as_float(std::numeric_limits<float>::max())) 
		{
			init(node);
		}

		void ParticleFollowerAffector::init(const variant& node) 
		{
		}

		void ParticleFollowerAffector::handleEmitProcess(float t) 
		{
			std::vector<Particle>& particles = getTechnique()->getActiveParticles();
			// keeps particles following wihin [min_distance, max_distance]
			if(particles.size() < 1) {
				return;
			}
			prev_particle_ = particles.begin();
			for(auto p = particles.begin(); p != particles.end(); ++p) {
				internalApply(*p, t);
				prev_particle_ = p;
			}
		}

		void ParticleFollowerAffector::internalApply(Particle& p, float t) 
		{
			auto distance = glm::length(p.current.position - prev_particle_->current.position);
			if(distance > min_distance_ && distance < max_distance_) {
				p.current.position = prev_particle_->current.position + (min_distance_/distance)*(p.current.position-prev_particle_->current.position);
			}
		}

		AlignAffector::AlignAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node) 
			: Affector(parent, node, AffectorType::ALIGN), 
			resize_(false) 
		{
			init(node);
		}

		void AlignAffector::init(const variant& node) 
		{
			resize_ = (node["resize"].as_bool(false));
		}

		void AlignAffector::internalApply(Particle& p, float t) 
		{
			glm::vec3 distance = prev_particle_->current.position - p.current.position;
			if(resize_) {
				p.current.dimensions.y = glm::length(distance);
			}
			if(std::abs(glm::length(distance)) > 1e-12) {
				distance = glm::normalize(distance);
			}
			p.current.orientation.x = distance.x;
			p.current.orientation.y = distance.y;
			p.current.orientation.z = distance.z;
		}

		void AlignAffector::handleEmitProcess(float t) 
		{
			std::vector<Particle>& particles = getTechnique()->getActiveParticles();
			if(particles.size() < 1) {
				return;
			}
			prev_particle_ = particles.begin();				
			for(auto p = particles.begin(); p != particles.end(); ++p) {
				internalApply(*p, t);
				prev_particle_ = p;
			}
		}

		FlockCenteringAffector::FlockCenteringAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node) 
			: Affector(parent, node, AffectorType::FLOCK_CENTERING), 
		 	  average_(0.0f)
		{
			init(node);
		}

		void FlockCenteringAffector::init(const variant& node) 
		{
		}

		void FlockCenteringAffector::internalApply(Particle& p, float t) 
		{
			p.current.direction = (average_ - p.current.position) * t;
		}

		void FlockCenteringAffector::handleEmitProcess(float t) 
		{
			std::vector<Particle>& particles = getTechnique()->getActiveParticles();
			if(particles.size() < 1) {
				return;
			}
			auto count = particles.size();
			glm::vec3 sum(0.0f);
			for(const auto& p : particles) {
				sum += p.current.position;
			}
			average_ /= static_cast<float>(count);

			prev_particle_ = particles.begin();				
			for(auto p = particles.begin(); p != particles.end(); ++p) {
				internalApply(*p, t);
				prev_particle_ = p;
			}
		}

		BlackHoleAffector::BlackHoleAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node) 
			: Affector(parent, node, AffectorType::BLACK_HOLE), 
			  velocity_(0.0), 
			  acceleration_(0.0)
		{
			init(node);
		}

		void BlackHoleAffector::init(const variant& node) 
		{
			velocity_ = (node["velocity"].as_float());
			acceleration_ = (node["acceleration"].as_float());
		}

		void BlackHoleAffector::handleEmitProcess(float t) 
		{
			velocity_ += acceleration_;
			Affector::handleEmitProcess(t);
		}

		void BlackHoleAffector::internalApply(Particle& p, float t) 
		{
			glm::vec3 diff = getPosition() - p.current.position;
			float len = glm::length(diff);
			if(len > velocity_) {
				diff *= velocity_/len;
			} else {
				p.current.time_to_live = 0;
			}

			p.current.position += diff;
		}

		PathFollowerAffector::PathFollowerAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node) 
			: Affector(parent, node, AffectorType::PATH_FOLLOWER)
		{
			init(node);
		}

		void PathFollowerAffector::init(const variant& node) 
		{
			ASSERT_LOG(node.has_key("path") && node["path"].is_list(),
				"path_follower must have a 'path' attribute.");
			for(unsigned n = 0; n != node["path"].num_elements(); ++n) {
				const auto& pt = node["path"][n];
				ASSERT_LOG(pt.is_list() && pt.num_elements() > 0, "points in path must be lists of more than one element.");
				const double x = pt[0].as_float();
				const double y = pt.num_elements() > 1 ? pt[1].as_float() : 0.0;
				const double z = pt.num_elements() > 2 ? pt[2].as_float() : 0.0;
				points_.emplace_back(x,y,z);
			}
			spl_ = std::make_shared<geometry::spline3d<float>>(points_);
		}

		void PathFollowerAffector::clearPoints()
		{
			points_.clear();
			spl_.reset();
		}

		void PathFollowerAffector::addPoint(const glm::vec3& p)
		{
			points_.emplace_back(p);
			spl_.reset(new geometry::spline3d<float>(points_));
		}

		void PathFollowerAffector::setPoints(const std::vector<glm::vec3>& points)
		{
			points_ = points;
			spl_.reset(new geometry::spline3d<float>(points_));
		}

		void PathFollowerAffector::internalApply(Particle& p, float t) 
		{
			const float time_fraction = p.current.time_to_live / p.initial.time_to_live;
			const float time_fraction_next = (p.current.time_to_live + t) > p.initial.time_to_live 
				? 1.0f 
				: (p.current.time_to_live + t) / p.initial.time_to_live;
			p.current.position += spl_->interpolate(time_fraction_next) - spl_->interpolate(time_fraction);
		}

		void PathFollowerAffector::handleEmitProcess(float t) 
		{
			if(spl_ == nullptr) {
				return;
			}
			std::vector<Particle>& particles = getTechnique()->getActiveParticles();
			if(particles.size() < 1) {
				return;
			}

			prev_particle_ = particles.begin();				
			for(auto& p = particles.begin(); p != particles.end(); ++p) {
				internalApply(*p, t);
				prev_particle_ = p;
			}
		}

		RandomiserAffector::RandomiserAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node) 
			: Affector(parent, node, AffectorType::RANDOMISER), 
			max_deviation_(0.0f), 
			time_step_(0),
			random_direction_(true)
		{
			init(node);
		}

		void RandomiserAffector::init(const variant& node)
		{
			time_step_ = (float(node["time_step"].as_float(0)));
			random_direction_ = (node["use_direction"].as_bool(true));

			if(node.has_key("max_deviation_x")) {
				max_deviation_.x = float(node["max_deviation_x"].as_float());
			}
			if(node.has_key("max_deviation_y")) {
				max_deviation_.y = float(node["max_deviation_y"].as_float());
			}
			if(node.has_key("max_deviation_z")) {
				max_deviation_.z = float(node["max_deviation_z"].as_float());
			}
			last_update_time_[0] = last_update_time_[1] = 0.0f;
		}

		void RandomiserAffector::internalApply(Particle& p, float t)
		{
			if(random_direction_) {
				// change direction per update
				p.current.direction += glm::vec3(get_random_float(-max_deviation_.x, max_deviation_.x),
					get_random_float(-max_deviation_.y, max_deviation_.y),
					get_random_float(-max_deviation_.z, max_deviation_.z));
			} else {
				// change position per update.
				p.current.position += getScale() * glm::vec3(get_random_float(-max_deviation_.x, max_deviation_.x),
					get_random_float(-max_deviation_.y, max_deviation_.y),
					get_random_float(-max_deviation_.z, max_deviation_.z));
			}
		}

		void RandomiserAffector::handle_apply(std::vector<Particle>& particles, float t)
		{
			last_update_time_[0] += t;
			if(last_update_time_[0] > time_step_) {
				last_update_time_[0] -= time_step_;
				for(auto& p : particles) {
					internalApply(p, t);
				}
			}
		}

		void RandomiserAffector::handle_apply(std::vector<EmitterPtr>& objs, float t) 
		{
			last_update_time_[1] += t;
			if(last_update_time_[1] > time_step_) {
				last_update_time_[1] -= time_step_;
				for(auto e : objs) {
					internalApply(*e, t);
				}
			}
		}
		
		void RandomiserAffector::handleProcess(float t) 
		{
			handle_apply(getTechnique()->getActiveParticles(), t);
			handle_apply(getTechnique()->getActiveEmitters(), t);
		}

		SineForceAffector::SineForceAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node) 
			: Affector(parent, node, AffectorType::SINE_FORCE),
			   min_frequency_(1.0f),
			  max_frequency_(1.0f),
			  angle_(0.0f),
			  frequency_(1.0f),
			  force_vector_(0.0f),
			  scale_vector_(0.0f),
			  fa_(ForceApplication::FA_ADD)
		{
			init(node);
		}

		void SineForceAffector::init(const variant& node)
		{
			if(node.has_key("max_frequency")) {
				max_frequency_ = static_cast<float>(node["max_frequency"].as_float());
				frequency_ = max_frequency_;
			}
			if(node.has_key("min_frequency")) {
				min_frequency_ = static_cast<float>(node["min_frequency"].as_float());					
				if(min_frequency_ > max_frequency_) {
					frequency_ = min_frequency_;
				}
			}
			if(node.has_key("force_vector")) {
				force_vector_ = variant_to_vec3(node["force_vector"]);
			}
			if(node.has_key("force_application")) {
				const std::string& fa = node["force_application"].as_string();
				if(fa == "average") {
					fa_ = ForceApplication::FA_AVERAGE;
				} else if(fa == "add") {
					fa_ = ForceApplication::FA_ADD;
				} else {
					ASSERT_LOG(false, "'force_application' attribute should have value average or add");
				}
			}
		}

		void SineForceAffector::handleEmitProcess(float t)
		{
			angle_ += /*2.0f * M_PI **/ frequency_ * t;
			float sine_value = sin(angle_);
			scale_vector_ = force_vector_ * t * sine_value;
			//std::cerr << "XXX: angle: " << angle_ << " scale_vec: " << scale_vector_ << std::endl;
			if(angle_ > static_cast<float>(M_PI * 2.0f)) {
				angle_ -= static_cast<float>(M_PI * 2.0f);
				if(min_frequency_ != max_frequency_) {
					frequency_ = get_random_float(min_frequency_, max_frequency_);
				}
			}
			Affector::handleEmitProcess(t);
		}

		void SineForceAffector::internalApply(Particle& p, float t)
		{
			if(fa_ == ForceApplication::FA_ADD) {
				p.current.direction += scale_vector_;
			} else {
				p.current.direction = (p.current.direction + force_vector_) / 2.0f;
			}
		}



	}
}
