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

#include <OgreMovableObject.h>
#include "ParticleTechnique.h"

/**
 * A particle system.
 *
 * @todo What is this?
 */
class ParticleSystem : public Ogre::MovableObject{

    public:

        /**
         * Constructor.
         */
        ParticleSystem(const Ogre::String& name);

        /**
         * Destructor.
         */
        virtual ~ParticleSystem();

        /**
         * Copies the system attributes to another ParticleSystem.
         *
         * @param[out] ps The ParticleSystem to copy the attributes to.
         */
        void CopyAttributesTo(ParticleSystem* ps);

        // realization of Ogre::MovableObject

        /**
         * Retrieves the movable type.
         *
         * @return The movable type.
         * @todo Understand and document.
         */
        const Ogre::String& getMovableType() const;

        /**
         * Retrieves the object bounding box.
         *
         * @return The bounding box.
         */
        const Ogre::AxisAlignedBox& getBoundingBox() const;

        /**
         * Retrieves the bounding radius.
         *
         * @return The bounding radius.
         */
        Ogre::Real getBoundingRadius() const;

        /**
         * Adds the system to the scene render queue.
         *
         * @param[in,out] queue The queue to update.
         */
        void _updateRenderQueue(Ogre::RenderQueue* queue);

        /**
         * Visits renderables.
         *
         * @param[in] visitor The visitor.
         * @param[in] debug_renderables True to debug the renderables, default
         * is false
         * @todo Understand and document.
         */
        void visitRenderables(
          Ogre::Renderable::Visitor* visitor, bool debug_renderables = false
        ){};

        /**
         * Updates the system.
         *
         * Updates the system status based on the elapsed time.
         *
         * @param[in] time_elapsed The elapsed time.
         */
        void Update(Ogre::Real time_elapsed);

        /**
         * Technique creator.
         *
         * Creates a new technique and adds it to the system.
         *
         * @return Pointer to the newly created technique.
         */
        ParticleTechnique* CreateTechnique();

        /**
         * Adds a technique to the system.
         *
         * @param[in] technique Technique to add to the system
         */
        void AddTechnique(ParticleTechnique* technique);

        /**
         * Removes al techniques from the system.
         */
        void DestroyAllTechniques();

    private:

        /**
         * List of the system techniques.
         */
        std::vector<ParticleTechnique*> techniques_;
};

