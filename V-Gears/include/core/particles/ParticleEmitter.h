/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#pragma once

#include <OgreStringInterface.h>
#include "Particle.h"
#include "ParticleEmitterDictionary.h"

class ParticleTechnique;

/**
 * A particle emitter.
 */
class ParticleEmitter : public Ogre::StringInterface, public Particle{

    public:

        /**
         * Constructor.
         */
        ParticleEmitter();

        /**
         * Destructor.
         */
        virtual ~ParticleEmitter();

        /**
         * Copies attributes to other particle emmiter.
         *
         * @param[out] emitter The particle emmite to copy the attributes to.
         */
        virtual void CopyAttributesTo(ParticleEmitter* emitter);

        /**
         * @todo Understand and document.
         */
        virtual void InitForEmission();

        /**
         * Calculates how many particles are to be emitted in a given time.
         *
         * @param[in] time_elapsed The elapsed time since the emitter has been
         * emitting.
         * @todo Understand and document.
         */
        virtual int CalculateRequestedParticles(Ogre::Real time_elapsed);

        /**
         * Initializes a article to be emmited.
         *
         * @param[out] particle The particle to be initialized.
         */
        virtual void InitParticleForEmission(Particle* particle);

        /**
         * Sets the parent technique for emitter.
         *
         * @param[in] tech Technique.
         * @todo Understand and document.
         */
        void SetParentTechnique(ParticleTechnique* tech){
            parent_technique_ = tech;
        };

        /**
         * Retrieves the parent technique of the emitter.
         *
         * @return Technique.
         * @todo Understand and document.
         */
        ParticleTechnique* GetParentTechnique() const {
            return parent_technique_;
        };

        /**
         * Set a name for the emitter.
         *
         * @param[in] name Name for the emitter.
         */
        void SetName(const Ogre::String& name){name_ = name;};

        /**
         * Retrieves the emitter name.
         *
         * @return Emitter name.
         */
        const Ogre::String& GetName() const{return name_;};

        /**
         * Sets the emitter type.
         *
         * @param[in] emitter_type The emitter type.
         */
        void SetEmitterType(const Ogre::String& emitter_type){
            emitter_type_ = emitter_type;
        };

        /**
         * Retrieves the emitter type.
         *
         * @return The emitter type.
         */
        const Ogre::String& GetEmitterType() const{return emitter_type_;};

        /**
         * Sets the name of the emits.
         *
         * When changed, notifies the technique of the change.
         *
         * @param[in] emits_name Name for the emits.
         * @todo What are the emits?
         */
        void SetEmitsName(const Ogre::String& emits_name);

        /**
         * Retrieves the name of the emits.
         *
         * @return Name of the emits
         * @todo What are the emits?
         */
        const Ogre::String& GetEmitsName() const{return emits_name_;};

        /**
         * Sets the type of the emits.
         *
         * @param[in] emits_type Type for the emits.
         * @todo What are the emits?
         */
        void SetEmitsType(ParticleType emits_type) {emits_type_ = emits_type;};

        /**
         * Retrieves the type of the emits.
         *
         * @return Type of the emits.
         * @todo What are the emits?
         */
        ParticleType GetEmitsType() const {return emits_type_;};

        /**
         * Set the emission rate for the emitter.
         *
         * @param[in] rate Emission rate.
         */
        void SetEmissionRate(int rate) {emission_rate_ = rate;};

        /**
        * Retrieves the emission rate of the emitter.
        *
        * @return Emission rate.
        */
        int GetEmissionRate() const {return emission_rate_;};

        /**
         * Sets the particle direction for the emitter.
         *
         * It sets a fixed direction. For a variable, ranged direction, use
         * {@see setEmitDirectionRange}.
         *
         * @param[in] dir Particle direction.
         */
        void SetEmitDirection(const Ogre::Vector3& dir){
            emit_direction_1_ = dir;
            emit_direction_2_ = dir;
        };

        /**
         * Sets the particle direction for the emitter.
         *
         * It sets a ranged direction. For each paticle, a random direction
         * between DIR_1 and DIR_2 will be assigned. For a fixed direction, use
         * {@see setEmitDirection}.
         *
         * @param[in] dir_1 One of the limits for the range of directions.
         * @param[in] dir_2 One of the limits for the range of directions.
         */
        void SetEmitDirectionRange(
          const Ogre::Vector3& dir_1, const Ogre::Vector3& dir_2
        ){
            emit_direction_1_ = dir_1;
            emit_direction_2_ = dir_2;
        };

        /**
         * Sets the time the particles must be displayed between being emitted
         * and disappearing.
         *
         * @param[in] time Duration of the particles.
         * @todo Time is in secons? milliseconds? frames?
         */
        void SetEmitTotalTimeToLive(float time){
            emit_total_time_to_live_ = time;
        };

    protected:

        /**
         * The particle technique.
         */
        ParticleTechnique* parent_technique_;

        /**
         * The emitter name.
         */
        Ogre::String name_;

        /**
         * The emitter type.
         */
        Ogre::String emitter_type_;

        /**
         * The emits name.
         */
        Ogre::String emits_name_;

        /**
         * The emits type.
         */
        ParticleType emits_type_;

        /**
         * Dictionary for emission rates.
         */
        static ParticleEmitterDictionary::EmissionRate
          emission_rate_dictionary_;

        /**
         * The emission rate.
         */
        int emission_rate_;

        /**
         * The remaining particles to be emitted.
         */
        Ogre::Real emission_remainder_;

        /**
         * Dictionary for particle directions.
         */
        static ParticleEmitterDictionary::Direction direction_dictionary_;

        /**
         * One of the limits for the range of directions.
         */
        Ogre::Vector3 emit_direction_1_;

        /**
         * One of the limits for the range of directions.
         */
        Ogre::Vector3 emit_direction_2_;

        /**
         * Disctionary for particle durations.
         */
        static ParticleEmitterDictionary::TotalTimeToLive
          total_time_to_live_dictionary_;

        /**
         * Particle duration.
         */
        float emit_total_time_to_live_;
};
