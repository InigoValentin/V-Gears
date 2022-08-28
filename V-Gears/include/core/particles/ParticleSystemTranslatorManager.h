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

#include <OgreScriptTranslator.h>
#include <OgreSingleton.h>
#include "ParticleEmitterTranslator.h"
#include "ParticleRendererTranslator.h"
#include "ParticleSystemTranslator.h"
#include "ParticleTechniqueTranslator.h"


/**
 * The particle system translator manager.
 */
class ParticleSystemTranslatorManager : public Ogre::ScriptTranslatorManager{

    public:

        /**
         * Get the translator count.
         *
         * @return The translator count.
         */
        size_t getNumTranslators() const;

        /**
         * Retreieves the translator for a node.
         *
         * @param node[in] The node for whose type to get the translator.
         * @return The translator for the type of NODE, of nullptr if there is
         * no translator for it's type of node.
         */
        Ogre::ScriptTranslator *getTranslator(
          const Ogre::AbstractNodePtr &node
        );

    private:

        /**
         * The particle emitter translator.
         */
        ParticleEmitterTranslator emitter_translator_;

        /**
         * The renderer translator.
         */
        ParticleRendererTranslator renderer_translator_;

        /**
         * The particle system translator.
         */
        ParticleSystemTranslator particle_system_translator_;

        /**
         * The technique translator.
         */
        ParticleTechniqueTranslator technique_translator_;
};

