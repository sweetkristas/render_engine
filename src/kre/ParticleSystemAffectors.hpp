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

#pragma once

#include "ParticleSystemFwd.hpp"
#include "spline3d.hpp"

namespace KRE
{
	namespace Particles
	{
		enum class AffectorType
		{
			COLOR,
			JET,
			VORTEX,
			GRAVITY,
			LINEAR_FORCE,
			SCALE,
			PARTICLE_FOLLOWER,
			ALIGN,
			FLOCK_CENTERING,
			BLACK_HOLE,
			PATH_FOLLOWER,
			RANDOMISER,
			SINE_FORCE,
		};

		class Affector : public EmitObject
		{
		public:
			explicit Affector(std::weak_ptr<ParticleSystemContainer> parent, AffectorType type);
			explicit Affector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node, AffectorType type);
			virtual ~Affector();
			virtual AffectorPtr clone() const = 0;

			TechniquePtr getTechnique() const;
			void setParentTechnique(std::weak_ptr<Technique> tq) { technique_ = tq; }

			AffectorType getType() const { return type_; }

			float getMass() const { return mass_; }
			void setMass(float m) { mass_ = m; }
			const glm::vec3& getPosition() const override { return position_; }
			void setPosition(const glm::vec3& pos) override { position_ = pos; }
			const glm::vec3& getScale() const { return scale_; }
			void setScale(const glm::vec3& scale) { scale_ = scale; }

			const std::vector<std::string>& getExcludedEmitters() const { return excluded_emitters_; }
			bool isEmitterExcluded(const std::string& name) const;
			void addExcludedEmitter(const std::string& e) { excluded_emitters_.emplace_back(e); }
			void clearExcludedEmitters() { excluded_emitters_.clear(); }
			void removeExcludedEmitter(const std::string& e) {
				excluded_emitters_.erase(std::remove_if(excluded_emitters_.begin(), excluded_emitters_.end(), 
					[&e](const std::string& emitter){ 
					return e == emitter; 
				}), excluded_emitters_.end());
			}

			const variant& node() const { return node_; }
			void setNode(const variant& new_node) { node_ = new_node; init(new_node); }

			static AffectorPtr factory(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			static AffectorPtr factory(std::weak_ptr<ParticleSystemContainer> parent, AffectorType type);
		protected:
			virtual void handleEmitProcess(float t) override;
		private:
			virtual void init(const variant& node) = 0;
			virtual void internalApply(Particle& p, float t) = 0;

			AffectorType type_;
			float mass_;
			glm::vec3 position_;
			glm::vec3 scale_;
			std::vector<std::string> excluded_emitters_;
			std::weak_ptr<Technique> technique_;
			variant node_;

			Affector() = delete;
		};

		class TimeColorAffector : public Affector
		{
		public:
			enum class ColourOperation {
				COLOR_OP_SET,
				COLOR_OP_MULTIPLY,
			};
			typedef std::pair<float,glm::vec4> tc_pair;

			explicit TimeColorAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit TimeColorAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			ColourOperation getOperation() const { return operation_; }
			void setOperation(ColourOperation op) { operation_ = op; }

			const std::vector<tc_pair>& getTimeColorData() const { return tc_data_; }
			void clearTimeColorData() { tc_data_.clear(); }
			void addTimecolorEntry(const tc_pair& tc) { tc_data_.emplace_back(tc); sort_tc_data(); }
			void setTimeColorData(const std::vector<tc_pair>& tc) { tc_data_ = tc; sort_tc_data(); }
			void removeTimeColorEntry(const tc_pair& f);
		private:
			void internalApply(Particle& p, float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<TimeColorAffector>(*this);
			}
		
			ColourOperation operation_;
			std::vector<tc_pair> tc_data_;

			void sort_tc_data();
			std::vector<tc_pair>::iterator find_nearest_color(float dt);

			TimeColorAffector() = delete;
		};

		class JetAffector : public Affector
		{
		public:
			explicit JetAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit JetAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			const ParameterPtr& getAcceleration() const { return acceleration_; }
		private:
			void internalApply(Particle& p, float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<JetAffector>(*this);
			}
		
			ParameterPtr acceleration_;
			JetAffector() = delete;
		};

		class GravityAffector : public Affector
		{
		public:
			explicit GravityAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit GravityAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			const ParameterPtr& getGravity() const { return gravity_; }
		private:
			void internalApply(Particle& p, float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<GravityAffector>(*this);
			}
		
			ParameterPtr gravity_;
			GravityAffector() = delete;
		};

		class LinearForceAffector : public Affector
		{
		public:
			explicit LinearForceAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit LinearForceAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			const ParameterPtr& getForce() const { return force_; }
			const glm::vec3& getDirection() const { return direction_; }
			void setDirection(const glm::vec3& d) { direction_ = d; }
		private:
			void internalApply(Particle& p, float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<LinearForceAffector>(*this);
			}

			ParameterPtr force_;
			glm::vec3 direction_;
			LinearForceAffector() = delete;
		};

		class ScaleAffector : public Affector
		{
		public:
			explicit ScaleAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit ScaleAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			const ParameterPtr& getScaleX() const { return scale_x_; }
			const ParameterPtr& getScaleY() const { return scale_y_; }
			const ParameterPtr& getScaleZ() const { return scale_z_; }
			const ParameterPtr& getScaleXYZ() const { return scale_xyz_; }
			bool getSinceSystemStart() const { return since_system_start_; }
			void setSinceSystemStart(bool f) { since_system_start_ = f; }
		private:
			void internalApply(Particle& p, float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<ScaleAffector>(*this);
			}
		
			ParameterPtr scale_x_;
			ParameterPtr scale_y_;
			ParameterPtr scale_z_;
			ParameterPtr scale_xyz_;
			bool since_system_start_;
			float calculateScale(ParameterPtr s, const Particle& p);
			ScaleAffector() = delete;
		};

		class VortexAffector : public Affector
		{
		public:
			explicit VortexAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit VortexAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			const glm::vec3& getRotationAxis() const { return rotation_axis_; }
			void setRotationAxis(const glm::vec3& axis) { rotation_axis_ = axis; }
			const ParameterPtr& getRotationSpeed() const { return rotation_speed_; }
		private:
			void internalApply(Particle& p, float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<VortexAffector>(*this);
			}
		
			glm::vec3 rotation_axis_;
			ParameterPtr rotation_speed_;
			VortexAffector() = delete;
		};

		class ParticleFollowerAffector : public Affector
		{
		public:
			explicit ParticleFollowerAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit ParticleFollowerAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			float getMinDistance() const { return min_distance_; }
			void setMinDistance(float min_dist) { 
				min_distance_ = min_dist; 
				if(min_distance_ > max_distance_ ) {
					min_distance_ = max_distance_;
				}
			}
			float getMaxDistance() const { return max_distance_; }
			void setMaxDistance(float max_dist) { 
				max_distance_ = max_dist; 
				if(max_distance_ < min_distance_) {
					max_distance_ = min_distance_;
				}
			}
		private:
			void handleEmitProcess(float t) override;
			void internalApply(Particle& p, float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<ParticleFollowerAffector>(*this);
			}
		
			float min_distance_;
			float max_distance_;
			// working variables
			std::vector<Particle>::iterator prev_particle_;
			ParticleFollowerAffector() = delete;
		};

		class AlignAffector : public Affector
		{
		public:
			explicit AlignAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit AlignAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			bool getResizeable() const { return resize_; }
			void setResizeable(bool r) { resize_ = r; }
		private:
			void internalApply(Particle& p, float t) override;
			void handleEmitProcess(float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<AlignAffector>(*this);
			}
		
			bool resize_;			
			std::vector<Particle>::iterator prev_particle_;
			AlignAffector() = delete;
		};

		class FlockCenteringAffector : public Affector
		{
		public:
			explicit FlockCenteringAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit FlockCenteringAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;
		private:
			void internalApply(Particle& p, float t) override;
			void handleEmitProcess(float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<FlockCenteringAffector>(*this);
			}
		
			glm::vec3 average_;
			std::vector<Particle>::iterator prev_particle_;
			FlockCenteringAffector() = delete;
		};

		class BlackHoleAffector : public Affector
		{
		public:
			explicit BlackHoleAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit BlackHoleAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			const ParameterPtr& getVelocity() const { return velocity_; };
			const ParameterPtr& getAcceleration() const { return acceleration_; }
		private:
			void handleEmitProcess(float t) override;
			void internalApply(Particle& p, float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<BlackHoleAffector>(*this);
			}

			ParameterPtr velocity_;
			ParameterPtr acceleration_;
			// working
			float wvelocity_;
			BlackHoleAffector() = delete;
		};

		class PathFollowerAffector : public Affector
		{
		public:
			explicit PathFollowerAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit PathFollowerAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			const std::vector<glm::vec3>& getPoints() const { return points_; }
			void clearPoints();
			void addPoint(const glm::vec3& p);
			void setPoints(const std::vector<glm::vec3>& points);
		private:
			void internalApply(Particle& p, float t) override;
			void handleEmitProcess(float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<PathFollowerAffector>(*this);
			}
		
			std::vector<glm::vec3> points_;
			// working variables.
			std::shared_ptr<geometry::spline3d<float>> spl_;
			std::vector<Particle>::iterator prev_particle_;
			PathFollowerAffector() = delete;
		};

		class RandomiserAffector : public Affector
		{
		public:
			explicit RandomiserAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit RandomiserAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			const glm::vec3& getDeviation() const { return max_deviation_; }
			void setDeviation(const glm::vec3& d) { max_deviation_ = d; }
			void setDeviation(float x, float y, float z) { max_deviation_ = glm::vec3(x, y, z); }
			bool isRandomDirection() const { return random_direction_; }
			void setRandomDirection(bool f) { random_direction_ = f; }
			float getTimeStep() const { return time_step_; }
			void setTimeStep(float step) { time_step_ = step; }
		private:
			void internalApply(Particle& p, float t) override;
			void handle_apply(std::vector<Particle>& particles, float t);
			void handle_apply(std::vector<EmitterPtr>& objs, float t);
			virtual void handleProcess(float t);
			AffectorPtr clone() const override {
				return std::make_shared<RandomiserAffector>(*this);
			}
		
			// randomiser (bool random_direction_, float time_step_ glm::vec3 max_deviation_)
			bool random_direction_;
			float time_step_;
			glm::vec3 max_deviation_;
			float last_update_time_[2];
			RandomiserAffector() = delete;
		};

		class SineForceAffector : public Affector
		{
		public:
			enum class ForceApplication {
				FA_ADD,
				FA_AVERAGE,
			};

			explicit SineForceAffector(std::weak_ptr<ParticleSystemContainer> parent);
			explicit SineForceAffector(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			void init(const variant& node) override;

			ForceApplication getForceApplication() const { return fa_; }
			void setForceApplication(ForceApplication fa) { fa_ = fa; }
			const glm::vec3& getForceVector() const { return force_vector_; }
			void setForceVector(const glm::vec3& fv) { force_vector_ = fv; }
			void setForceVector(float x, float y, float z) { force_vector_ = glm::vec3(x, y, z); }
			const glm::vec3& getScaleVector() const { return scale_vector_; }
			void setScaleVector(const glm::vec3& sv) { scale_vector_ = sv; }
			float getMinFrequency() const { return min_frequency_; }
			void setMinFrequency(float min_freq) { min_frequency_ = min_freq; }
			float getMaxFrequency() const { return max_frequency_; }
			void setMaxFrequency(float max_freq) { max_frequency_ = max_freq; }
			float getAngle() const { return angle_; }
			void setAngle(float a) { angle_ = a; }

		private:
			void handleEmitProcess(float t) override;
			void internalApply(Particle& p, float t) override;
			AffectorPtr clone() const override {
				return std::make_shared<SineForceAffector>(*this);
			}
		
			glm::vec3 force_vector_;
			glm::vec3 scale_vector_;
			float min_frequency_;
			float max_frequency_;
			ForceApplication fa_;
			// working variable
			float frequency_;
			float angle_;
			SineForceAffector() = delete;
		};

		const char* get_affector_name(AffectorType type);
	}
}
