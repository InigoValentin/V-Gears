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

#include <OgreSingleton.h>
#include "ParticleEmitter.h"
#include "ParticleEmitterFactory.h"
#include "ParticleRendererFactory.h"
#include "ParticleSystem.h"
#include "ParticleSystemFactory.h"
#include "ParticleTechnique.h"
#include "ParticleSystemTranslatorManager.h"

/**
 * The particle system manager.
 */
class ParticleSystemManager: public Ogre::Singleton<ParticleSystemManager>{

    public:

        /**
         * Constructor.
         */
        ParticleSystemManager();

        /**
         * Constructor.
         */
        ~ParticleSystemManager();

        /**
         * Creates a particle system template.
         *
         * @param[in] name Name for the template.
         * @return A new particle system template.
         */
        ParticleSystem* CreateParticleSystemTemplate(const Ogre::String& name);

        /**
         * Retrieves a particle system template.
         *
         * @param[in] template_name The name of the template to retrieve.
         * @return The template. 0 if no template with that name was found.
         */
        ParticleSystem* GetParticleSystemTemplate(
          const Ogre::String& template_name
        );

        /**
         * Gets all template names.
         *
         * @param[out] v List where the names will be loaded.
         */
        void ParticleSystemTemplateNames(std::vector<Ogre::String>& v);

        /**
         * Creates a particle system.
         *
         * @param[in] name Name for the template system.
         * @param[in] template_name Name for the system template.
         */
        ParticleSystem* CreateParticleSystem(
          const Ogre::String& name, const Ogre::String& template_name
        );

        /**
         * Destroys a particle system.
         *
         * @param[out] particle_system Particle system to destroy.
         */
        void DestroyParticleSystem(ParticleSystem* particle_system);

        /**
         * Creates a new technique.
         *
         * @return The new technique.
         */
        ParticleTechnique* CreateTechnique();

        /**
         * Duplicates a technique.
         *
         * @param[in] technique Technique to duplicate.
         * @return The new, duplicated technique.
         */
        ParticleTechnique* CloneTechnique(ParticleTechnique* technique);

        /**
         * Destroys a technique.
         *
         * @param[out] technique Technique to destroy.
         */
        void DestroyTechnique(ParticleTechnique* technique);

        /**
         * Creates a new particle emitter.
         *
         * @param[in] emitter_type Type for the emitter.
         */
        ParticleEmitter* CreateEmitter(const Ogre::String& emitter_type);

        /**
         * Duplicates a particle emitter.
         *
         * @param[in] emitter Emitter to duplicate.
         * @return The new, duplicated emitter.
         */
        ParticleEmitter* CloneEmitter(ParticleEmitter* emitter);

        /**
         * Destroys a particle emitter.
         *
         * @param[out] emitter Emitter to destroy.
         */
        void DestroyEmitter(ParticleEmitter* emitter);

        /**
         * Creates a new particle renderer.
         *
         * @param[in] renderer_type Type for the renderer.
         * @return The newly created renderer.
         */
        ParticleRenderer* CreateRenderer(const Ogre::String& renderer_type);

        /**
         * Duplicates a renderer.
         *
         * @param[in] renderer renderer to duplicate.
         * @return The new, duplicated renderer.
         */
        ParticleRenderer* CloneRenderer(ParticleRenderer* renderer);

        /**
         * Destroys a renderer.
         *
         * @param[out] renderer The renderer to destroy.
         */
        void DestroyRenderer(ParticleRenderer* renderer);

    private:

        /**
         * Adds a new {@see ParticleEmitterFactory} to the manager.
         *
         * @param[in] factory The factory to add to the manager.
         */
        void AddEmitterFactory(ParticleEmitterFactory* factory);

        /**
         * Adds a new {@see ParticleRendererFactory} to the manager.
         *
         * @param[in] factory The factory to add to the manager.
         */
        void AddRendererFactory(ParticleRendererFactory* factory);

    private:

        /**
         * The particle system factory.
         */
        ParticleSystemFactory* particle_system_factory_;

        /**
         * The translator manager.
         */
        ParticleSystemTranslatorManager* translator_manager_;

        typedef std::map<Ogre::String, ParticleSystem*>
          ParticleSystemTemplateMap;

        /**
         * The particle system template map.
         */
        ParticleSystemTemplateMap particle_system_templates_;

        typedef std::map<Ogre::String, ParticleEmitterFactory*>
          EmitterFactoryMap;

        /**
         * List of particle emitter factories.
         */
        EmitterFactoryMap emitter_factories_;

        typedef std::map<Ogre::String, ParticleRendererFactory*>
          RendererFactoryMap;

        /**
         * List of renderer factories.
         */
        RendererFactoryMap renderer_factories_;
};

