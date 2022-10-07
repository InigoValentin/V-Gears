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

#include "ParticleEmitter.h"
#include "ParticleRenderer.h"
#include "ParticlePool.h"
#include "ParticlePoolMap.h"

class ParticleSystem;

/**
 * A particle technique.
 */
class ParticleTechnique{

    public:

        /**
         * Constructor.
         */
        ParticleTechnique();

        /**
         * Destructor.
         */
        virtual ~ParticleTechnique();

        /**
         * Copy the technique attributes to another technique.
         */
        void CopyAttributesTo(ParticleTechnique* technique);

        /**
         * Sets the parent system for the technique.
         *
         * @param[in] parent_system The parent particle system.
         */
        void SetParentSystem(ParticleSystem* parent_system){
            parent_system_ = parent_system;
        };

        /**
         * Retrieves the parent system.
         *
         * @return The parent system.
         */
        ParticleSystem* GetParentSystem() const {return parent_system_;};

        /**
         * Adds the technique to the scene render queue.
         *
         * @param[in,out] queue The queue to update.
         */
        void UpdateRenderQueue(Ogre::RenderQueue* queue);

        /**
         * Initializes the tehnique.
         */
        void Initialize();

        /**
         * Updates the technique.
         *
         * Updates the technique status based on the elapsed time.
         *
         * @param[in] time_elapsed The elapsed time.
         */
        void Update(Ogre::Real time_elapsed);

        /**
         * Creates a renderer.
         *
         * The renderer gets assigned to the technique.
         *
         * @param[in] renderer_type The type for the renderer.
         * @return The newly created renderer.
         */
        ParticleRenderer* CreateRenderer(const Ogre::String& renderer_type);

        /**
         * Sets the renderer for the technique.
         *
         * @param[in] renderer Renderer for the technique.
         */
        void SetRenderer(ParticleRenderer* renderer);

        /**
         * Destroys the technique renderer.
         */
        void DestroyRenderer();

        /**
         * Creates a particle emitter for the technique.
         *
         * The emitter gets added to the technique.
         *
         * @param[in] emitter_type The type for the emitter.
         * @return The newly created emitter.
         */
        ParticleEmitter* CreateEmitter(const Ogre::String& emitter_type);

        /**
         * Adds a particle emitter to the technique
         *
         * @param[in] emitter The emitter to add to the technique.
         */
        void AddEmitter(ParticleEmitter* emitter);

        /**
         * Destroys all the technique's emitters.
         */
        void DestroyAllEmitters();

        /**
         * Counts the number of emitters assigned to the techniques.
         *
         * @return The number of emitters assigned to the techniques.
         */
        int GetNumEmittableEmitters() const;

        /**
         * Marks all emitters as emittables.
         */
        void EmissionChange();

        /**
         * Retrieves the visual particles quota
         *
         * @return The visual particles quota
         * @todo What is the quta for? How is it set?
         */
        int GetVisualParticlesQuota() const {return visual_particle_quota_;};

        /**
         * Initializes the particles of an emitter for emission.
         *
         * @param[in] emitter The emited to mark.
         * @param[in] requested The number of particles to emit.
         * @param[in] time_elapsed Unused.
         * @todo Understand and document properly.
         */
        void ExecuteEmitParticles(
          ParticleEmitter* emitter, int requested, Ogre::Real time_elapsed
        );

        /**
         * Locks all particles and removes all their additional data.
         */
        void ResetVisualParticles();

    private:

        /**
         * The parent particle system.
         */
        ParticleSystem* parent_system_;

        /**
         * The technique renderer.
         */
        ParticleRenderer* renderer_;

        /**
         * The visual particle quota.
         */
        int visual_particle_quota_;

        /**
         * Indicates if the visual particle quota has been increased.
         */
        bool visual_particle_pool_increased_;

        /**
         * The visual particle pool.
         */
        ParticlePool<VisualParticle> visual_particles_pool_;

        /**
         * List of visual particles.
         */
        std::vector<VisualParticle*> visual_particles_;

        /**
         * The particle emitter quota.
         */
        int emitted_emitter_quota_;

        /**
         * Indicates if the particle emitte quota has been increased.
         */
        bool particle_emitter_pool_increased_;

        /**
         * Particle emitter pool.
         */
        ParticlePoolMap<ParticleEmitter> particle_emitter_pool_;

        /**
         * List of particle emitters.
         */
        std::vector<ParticleEmitter*> emitters_;
};

