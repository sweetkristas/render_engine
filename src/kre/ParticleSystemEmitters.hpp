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

namespace KRE
{
	namespace Particles
	{
		enum class EmitsType {
			VISUAL,
			EMITTER,
			AFFECTOR,
			TECHNIQUE,
			SYSTEM,
		};

		enum class EmitterType {
			POINT,
			LINE,
			BOX,
			CIRCLE,
			SPHERE_SURFACE,
		};

		class Emitter : public EmitObject
		{
		public:
			explicit Emitter(std::weak_ptr<ParticleSystemContainer> parent, EmitterType type);
			explicit Emitter(std::weak_ptr<ParticleSystemContainer> parent, const variant& node, EmitterType type);
			virtual ~Emitter();
			Emitter(const Emitter&);

			void initPhysics();

			EmitterType getType() const { return type_; }

			int getEmittedParticleCountPerCycle(float t);
			color_vector getColor() const;
			TechniquePtr getTechnique() const;
			void init(std::weak_ptr<Technique> tq);

			void setEmissionRate(variant node);

			const ParameterPtr& getEmissionRate() const { return emission_rate_; }
			const ParameterPtr& getTimeToLive() const { return time_to_live_; }
			const ParameterPtr& getVelocity() const { return velocity_; }
			const ParameterPtr& getAngle() const { return angle_; }
			const ParameterPtr& getMass() const { return mass_; }

			const ParameterPtr& getDuration() const { return duration_; }
			const ParameterPtr& getRepeatDelay() const { return repeat_delay_; }


			const ParameterPtr& getParticleWidth() const { return particle_width_; }
			const ParameterPtr& getParticleHeight() const { return particle_height_; }
			const ParameterPtr& getParticleDepth() const { return particle_depth_; }

			bool getForceEmission() const { return force_emission_; }
			void setForceEmission(bool f) { force_emission_ = f; }

			bool getCanBeDeleted() const { return can_be_deleted_; }
			void setCanBeDeleted(bool f) { can_be_deleted_ = f; }

			const glm::vec4& getColorFloat() const { return color_; }
			void setColor(const glm::vec4& col) { color_ = col; }

			virtual EmitterPtr clone() = 0;
			static EmitterPtr factory(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			static EmitterPtr factory(std::weak_ptr<ParticleSystemContainer> parent, EmitterType type);
		protected:
			virtual void internalCreate(Particle& p, float t) = 0;
			virtual bool durationExpired() override { return can_be_deleted_; }
		private:
			virtual void handleEmitProcess(float t) override;
			virtual void handleDraw(const WindowPtr& wnd) const override;
			void handleEnable() override;
			void visualEmitProcess(float t);
			void emitterEmitProcess(float t);
			std::weak_ptr<Technique> technique_;

			EmitterType type_;

			// These are generation parameters.
			ParameterPtr emission_rate_;
			ParameterPtr time_to_live_;
			ParameterPtr velocity_;
			ParameterPtr angle_;
			ParameterPtr mass_;
			// This is the duration that the emitter lives for
			ParameterPtr duration_;
			// this is the delay till the emitter repeats.
			ParameterPtr repeat_delay_;
			std::unique_ptr<std::pair<glm::quat, glm::quat>> orientation_range_;
			typedef std::pair<color_vector,color_vector> color_range;
			std::shared_ptr<color_range> color_range_;
			glm::vec4 color_;
			ParameterPtr particle_width_;
			ParameterPtr particle_height_;
			ParameterPtr particle_depth_;
			bool force_emission_;
			bool force_emission_processed_;
			bool can_be_deleted_;

			EmitsType emits_type_;
			std::string emits_name_;

			void initParticle(Particle& p, float t);
			void setParticleStartingValues(const std::vector<Particle>::iterator& start, const std::vector<Particle>::iterator& end);
			void createParticles(std::vector<Particle>& particles, std::vector<Particle>::iterator& start, std::vector<Particle>::iterator& end, float t);
			int calculateParticlesToEmit(float t, int quota, int current_size);
			void calculateQuota();

			float generateAngle() const;
			glm::vec3 getInitialDirection() const;

			//BoxOutlinePtr debug_draw_outline_;

			// working items
			// Any "left over" fractional count of emitted particles
			float emission_fraction_;
			// time till the emitter stops emitting.
			float duration_remaining_;
			// time remaining till a stopped emitter restarts.
			float repeat_delay_remaining_;

			int particles_remaining_;

			glm::vec3 scale_;

			Emitter() = delete;
		};

		class CircleEmitter : public Emitter
		{
		public:
			explicit CircleEmitter(std::weak_ptr<ParticleSystemContainer> parent);
			explicit CircleEmitter(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);

			const ParameterPtr& getRadius() const { return circle_radius_; }
			float getStep() const { return circle_step_; }
			void setStep(float step) { circle_step_ = step; }
			float getAngle() const { return circle_angle_; }
			void setAngle(float angle) { circle_angle_ = angle; }
			bool isRandomLocation() const { return circle_random_; }
			void setRandomLocation(bool f) { circle_random_ = f; }
		private:
			void internalCreate(Particle& p, float t) override;
			virtual EmitterPtr clone() override {
				return std::make_shared<CircleEmitter>(*this);
			}
			ParameterPtr circle_radius_;
			float circle_step_;
			float circle_angle_;
			bool circle_random_;

			CircleEmitter() = delete;
		};

		class BoxEmitter : public Emitter
		{
		public:
			explicit BoxEmitter(std::weak_ptr<ParticleSystemContainer> parent);
			explicit BoxEmitter(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
			const glm::vec3& getDimensions() const { return box_dimensions_; }
			void setDimensions(const glm::vec3& d) { box_dimensions_ = d; }
			void setDimensions(float x, float y, float z) { box_dimensions_ = glm::vec3(x, y, z); }
			void setDimensions(float* v) { box_dimensions_ = glm::vec3(v[0], v[1], v[2]); }
		protected:
			void internalCreate(Particle& p, float t) override;
			virtual EmitterPtr clone() override {
				return std::make_shared<BoxEmitter>(*this);
			}
		private:
			glm::vec3 box_dimensions_;
			BoxEmitter() = delete;
		};

		class LineEmitter : public Emitter
		{
		public:
			explicit LineEmitter(std::weak_ptr<ParticleSystemContainer> parent);
			explicit LineEmitter(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);

			float getLineDeviation() const { return line_deviation_; }
			void setLineDeviation(float d) { line_deviation_ = d; }
			float getMinIncrement() const { return min_increment_; }
			void setMinIncrement(float minc) { min_increment_ = minc; }
			float getMaxIncrement() const { return max_increment_; }
			void setMaxIncrement(float maxc) { max_increment_ = maxc; }
		private:
			void internalCreate(Particle& p, float t) override;
			EmitterPtr clone() override {
				return std::make_shared<LineEmitter>(*this);
			}
			glm::vec3 line_end_;
			float line_deviation_;
			float min_increment_;
			float max_increment_;

			LineEmitter() = delete;
		};

		class PointEmitter : public Emitter
		{
		public:
			explicit PointEmitter(std::weak_ptr<ParticleSystemContainer> parent);
			explicit PointEmitter(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);
		protected:
			void internalCreate(Particle& p, float t) override;
			EmitterPtr clone() override {
				return std::make_shared<PointEmitter>(*this);
			}
		private:
			PointEmitter();
		};

		class SphereSurfaceEmitter : public Emitter
		{
		public:
			explicit SphereSurfaceEmitter(std::weak_ptr<ParticleSystemContainer> parent);
			explicit SphereSurfaceEmitter(std::weak_ptr<ParticleSystemContainer> parent, const variant& node);

			const ParameterPtr& getRadius() const { return radius_; }
		protected:
			void internalCreate(Particle& p, float t) override;
			EmitterPtr clone() override {
				return std::make_shared<SphereSurfaceEmitter>(*this);
			}
		private:
			ParameterPtr radius_;
			SphereSurfaceEmitter() = delete;
		};
	}
}
