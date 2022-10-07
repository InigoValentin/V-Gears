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

#include <OgrePrerequisites.h>
#include <Ogre.h>

#include "ParticleAdditionalData.h"

class ParticleEmitter;

/**
 * A particle.
 *
 * Particles are emitted by one {@see ParticleEmitter}.
 */
class Particle{

    public:

        /**
         * Types of particles.
         */
        enum ParticleType{

            /**
             * A visual particle.
             */
            PT_VISUAL,

            /**
             * @todo Document.
             */
            PT_EMITTER
        };

        /**
         * Constructor.
         */
        Particle();

        /**
         * Destructor.
         */
        virtual ~Particle() = 0;

        /**
         * Initializes the particle.
         *
         * It makes it ready for emission.
         */
        virtual void InitForEmission(){};

        /**
         * Initializes the particle.
         *
         * It makes it ready for expiration.
         * @todo: Document this properly.
         */
        virtual void InitForExpiration(){};

        /**
         * Updates the particle.
         *
         * Updates it position based on the time lived by the particle.
         *
         * @param[in] time_elapsed Time lived by the particle.
         */
        virtual void Update(Ogre::Real time_elapsed);

        /**
         * Copies the particle attributes to another particle.
         *
         * @param[out] particle The particl to copy the atttributes to.
         */
        virtual void CopyAttributesTo(Particle* particle);

        /**
         * Retrieves the particle type.
         *
         * @return The particle type.
         */
        ParticleType GetParticleType() const {return particle_type_;};

        /**
         * Sets the particle emitter.
         *
         * @param[in] parent_emitter The emitter for the particle.
         * @todo Check if parent_emitter is an in or out parameter.
         */
        void SetParentEmitter(ParticleEmitter* parent_emitter){
            parent_emitter_ = parent_emitter;
        };

        /**
         * Retrieves the particle emitter.
         *
         * @return The emitter.
         */
        ParticleEmitter* GetParentEmitter() const {return parent_emitter_;};

        /**
         * Enables or disables the particle
         *
         * @param[in] enabled True to enable the particle, false to disable it.
         */
        void SetEnabled(bool enabled) {enabled_ = enabled;};

        /**
         * Checks if the particle is enabled.
         *
         * @return True if the particle is enabled, false otherwise.
         */
        bool IsEnabled() const {return enabled_;};

        /**
         * Toggles the particle emitability.
         *
         * @param[in] emittable True to make the particle emittable, false to
         * prevent it to be emitted.
         */
        void SetEmittable(bool emittable) {emittable_ = emittable;};

        /**
         * Checks if the particle can be emitted.
         *
         * @return True if the particle can be emitted, false if it can't.
         */
        bool IsEmittable() const {return emittable_;};

        /**
         * Additional data for the particle.
         */
        ParticleAdditionalData* additional_data;

        /**
         * The particle position.
         */
        Ogre::Vector3 position;

        /**
         * The direction that the particle is or will be emitted to.
         */
        Ogre::Vector3 direction;

        /**
         * The particle duration.
         *
         * @todo Seconds? Milliseconds? Frames?
         * @todo Differenche with total_time_to_live?
         */
        float time_to_live;

        /**
         * The particle duration.
         *
         * @todo Seconds? Milliseconds? Frames?
         * @todo Differenche with time_to_live?
         */
        float total_time_to_live;

    protected:

        /**
         * The particle emitter.
         */
        ParticleEmitter* parent_emitter_;

        /**
         * The particle type.
         */
        ParticleType particle_type_;

        /**
         * Indicates if the particle is enabled.
         */
        bool enabled_;

        /**
         * Indicates if the particl can be emitted.
         */
        bool emittable_;
};
