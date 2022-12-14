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

#include <OgreString.h>
#include <OgreStringInterface.h>
#include "ParticlePool.h"

class ParticleTechnique;
class VisualParticle;

/**
 * A particle renderer.
 */
class ParticleRenderer : public Ogre::StringInterface{

    public:

        /**
         * Constructor.
         */
        ParticleRenderer();

        /**
         * Destructor.
         */
        virtual ~ParticleRenderer();

        /**
         * Copies the renderer attributes to other renderer.
         *
         * @param[out] renderer The renderer to copy the attributes to.
         */
        virtual void CopyAttributesTo(ParticleRenderer* renderer);

        /**
         * Toggles renderer initialization state.
         *
         * @param[in] renderer_initialised True to mark the renderer as
         * initialized, false to mark it as not initialized.
         */
        void SetRendererInitialised(bool renderer_initialised);

        /**
         * Checks the renderer initialization status.
         *
         * @return True if the renderer is initialized, false if not.
         */
        bool IsRendererInitialised() const;

        /**
         * Sets the renderer type.
         *
         * @param[in] renderer_type The renderer type.
         */
        void SetRendererType(Ogre::String renderer_type);

        /**
         * Retrieves the renderer type.
         *
         * @return The renderer type.
         */
        const Ogre::String& GetRendererType() const;

        /**
         * Sets the renderer parent technique.
         *
         * @param[in] parent_technique Parent technique for the renderer.
         */
        void SetParentTechnique(ParticleTechnique* parent_technique);

        /**
         * Sets the renderer particle technique.
         *
         * @return The renderer particle technique.
         */
        const ParticleTechnique* GetParentTechnique() const;

        /**
         * Toggles the visibility of the particle renderer.
         *
         * @param[in] visible True to make th renderer visibible, false to make
         * it invisible.
         */
        virtual void SetVisible(bool visible = true) = 0;

        /**
         * Initializes the renderer.
         */
        virtual void Initialize() = 0;

        /**
         * Adds the particle to the scene render queue.
         *
         * @param[in] queue Unused
         * @param[in] pool The particle pool.
         */
        virtual void UpdateRenderQueue(
          Ogre::RenderQueue* queue,
          ParticlePool<VisualParticle>& pool
        ) = 0;

    protected:

        /**
         * Indicates if the renderer has been initialized.
         */
        bool                renderer_initialized_;

        /**
         * The type of renderer.
         */
        Ogre::String        renderer_type_;

        /**
         * The renderer particle tchnique.
         */
        ParticleTechnique*  parent_technique_;
};
