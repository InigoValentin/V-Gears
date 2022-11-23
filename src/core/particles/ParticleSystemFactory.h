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

/**
 * A particle system factory.
 */
class ParticleSystemFactory : public Ogre::MovableObjectFactory{

    public:

        /**
         * Constructor.
         */
        ParticleSystemFactory(){};

        /**
         * Destructor.
         */
        ~ParticleSystemFactory(){};

        /**
         * Type of the particle systems
         */
        static Ogre::String FACTORY_TYPE_NAME;

        /**
         * Retrieves the particle systems type.
         *
         * @return The particle systems type.
         */
        const Ogre::String& getType() const;

        /**
         * Destroys a particle system.
         *
         * @param[in,out] obj System to destroy.
         */
        void DestroyInstance(Ogre::MovableObject* obj);

    private:

        /**
         * Creates an instance from a template.
         *
         * @param[in] name Name for the particle system
         * @param[in] params List of parameters for the particle system.
         */
        Ogre::MovableObject* createInstanceImpl(
          const Ogre::String& name, const Ogre::NameValuePairList* params
        );
};

