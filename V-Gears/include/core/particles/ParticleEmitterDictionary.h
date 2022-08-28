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

#include <OgreStringInterface.h>

namespace ParticleEmitterDictionary{

    /**
     * The emission rate of a particle emitter.
     */
    class EmissionRate : public Ogre::ParamCommand{

        public:

            /**
             * Retrieves the emission rate.
             *
             * @return Emission rate.
             */
            Ogre::String doGet(const void* target) const{return "";};

            /**
             * Sets the emission rate
             *
             * @param val[in] The emission rate.
             */
            void doSet(void* target, const Ogre::String& val);
    };

    /**
     * The total time to live for particles emitted by an emitter.
     */
    class TotalTimeToLive : public Ogre::ParamCommand{

        public:

            /**
             * Retrieves the total time to live.
             *
             * @return Total time to live.
             */
            Ogre::String doGet(const void* target) const{return "";};

            /**
             * Sets the total time to live.
             *
             * @param val[in] Total time to live.
             */
            void doSet(void* target, const Ogre::String& val);
    };

    /**
     * The direction at which an emitter emits particles.
     */
    class Direction : public Ogre::ParamCommand{

        public:

            /**
             * Retrieves the direction.
             *
             * @return The direction.
             */
            Ogre::String doGet(const void* target) const {return "";};

            /**
             * Sets the direction.
             *
             * @param val[in] The direction.
             */
            void doSet(void* target, const Ogre::String& val);
    };
}

