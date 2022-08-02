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

#include "../ParticleRendererFactory.h"
#include "ParticleEntityRenderer.h"

/**
 * A particle entity renderer factory.
 */
class ParticleEntityRendererFactory : public ParticleRendererFactory{

    public:

        /**
         * Constructor.
         */
        ParticleEntityRendererFactory() {};

        /**
         * Destructor.
         */
        virtual ~ParticleEntityRendererFactory(){};

        /**
         * Retrieves the renderer type.
         *
         * @return The renderer type (always "Entity").
         */
        Ogre::String GetRendererType() const{return "Entity";}

        /**
         * Create a renderer.
         *
         * @return A new renderer.
         */
        ParticleRenderer* CreateRenderer(){
            return _createRenderer<ParticleEntityRenderer>();
        }
};
