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

#include <OgreSceneNode.h>
#include "ParticleEntityAdditionalData.h"
#include "ParticleEntityRendererDictionary.h"
#include "../ParticleRenderer.h"

/**
 * A particle entity renderer.
 */
class ParticleEntityRenderer : public ParticleRenderer{

    public:

        /**
         * Constructor.
         */
        ParticleEntityRenderer();

        /**
         * Destructor.
         */
        virtual ~ParticleEntityRenderer();

        /**
         * Copies the atributtes to other renderer.
         *
         * @param[out] renderer Renderer to copy the attributes to.
         */
        virtual void CopyAttributesTo(ParticleRenderer* renderer);

        /**
         * Retrieves the emitter's mesh name.
         *
         * @return The emitter mesh name.
         */
        const Ogre::String& GetMeshName() const;

        /**
         * Sets the emitter's mesh name.
         *
         * @param[in] mesh_name The name for the emitter's mesh.
         */
        void SetMeshName(const Ogre::String& mesh_name);

        /**
         * Clears all data in the entity.
         */
        void Clear();

        /**
         * Toggles the particle entity visibility.
         *
         * @param[in] visible True to make the particle visible, false to make
         * it invisible.
         */
        virtual void SetVisible(bool visible);

        /**
         * Initializes the particle.
         */
        virtual void Initialize();

        /**
         * Adds the particle to the scene render queue.
         *
         * @param[in] queue Unused.
         * @param[in] pool The particle pool.
         */
        virtual void UpdateRenderQueue(
          Ogre::RenderQueue* queue, ParticlePool<VisualParticle>& pool
        );

    protected:

        /**
         * Additional data for the rendered.
         */
        std::vector<ParticleEntityAdditionalData*> all_additional_data_;

        /**
         * Additional data for the rendered that is not yet assigned.
         */
        std::vector<ParticleEntityAdditionalData*> unassigned_additional_data_;

        /**
         * List of entities.
         *
         * @todo What are these entities?
         */
        std::vector<Ogre::Entity*> entities_;

        /**
         * A dictionary of mesh names.
         */
        static ParticleEntityRendererDictionary::MeshName mesh_name_dictionary_;

        /**
         * The mesh name for the particles.
         */
        Ogre::String mesh_name_;
};

