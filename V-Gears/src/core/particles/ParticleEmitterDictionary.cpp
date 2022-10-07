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

#include <OgreStringConverter.h>
#include "core/particles/ParticleEmitterDictionary.h"
#include "core/particles/ParticleEmitter.h"
#include "core/Logger.h"
#include "core/Utilites.h"

namespace ParticleEmitterDictionary{

    void EmissionRate::doSet(void* target, const Ogre::String& val){
        static_cast<ParticleEmitter*>(target)->SetEmissionRate(
          Ogre::StringConverter::parseInt(val)
        );
    }

    void TotalTimeToLive::doSet(void* target, const Ogre::String& val){
        static_cast<ParticleEmitter*>(target)->SetEmitTotalTimeToLive(
          Ogre::StringConverter::parseReal(val)
        );
    }

    void Direction::doSet(void* target, const Ogre::String& val){
        Ogre::StringVector strings = StringTokenise(val);
        if (strings.size() == 1){
            static_cast<ParticleEmitter*>(target)->SetEmitDirection(
              Ogre::StringConverter::parseVector3(strings[0])
            );
        }
        else if (strings.size() > 1){
            static_cast<ParticleEmitter*>(target)->SetEmitDirectionRange(
              Ogre::StringConverter::parseVector3(strings[0]),
              Ogre::StringConverter::parseVector3(strings[1])
            );
        }
        else{
            LOG_WARNING(
              "The direction of a particle emitter must have at least a value."
            );
        }
    }
}
