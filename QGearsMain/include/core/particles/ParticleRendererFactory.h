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

#include "ParticleRenderer.h"

/**
 * A particle renderer factory.
 */
class ParticleRendererFactory{

    public:

        /**
         * Constructor.
         */
        ParticleRendererFactory(){};

        /**
         * Destructor.
         */
        virtual ~ParticleRendererFactory(){};

        /**
         * Retrieves the renderer type.
         *
         * @return Renderer type.
         */
        virtual Ogre::String GetRendererType() const = 0;

        /**
         * Creates a rendered.
         *
         * @return Pointer to the new renderer.
         */
        virtual ParticleRenderer* CreateRenderer() = 0;

        /**
         * Destroys a renderer.
         *
         * @param renderer[in|out] The renderer to destroy.
         */
        void DestroyRenderer (ParticleRenderer* renderer){
            if (renderer != NULL) delete renderer;
        };

    protected:

        /**
         * Creates a particle renderer.
         *
         * @tparam T The particle renderer type.
         * @return The new particle renderer.
         */
        template <class T>
        ParticleRenderer* _createRenderer(){
            ParticleRenderer* particle_renderer = new T();
            particle_renderer->SetRendererType(GetRendererType());
            return particle_renderer;
        };
};
