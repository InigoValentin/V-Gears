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

#include <OgreLogManager.h>
#include "core/particles/ParticleSystemTranslatorManager.h"

size_t ParticleSystemTranslatorManager::getNumTranslators() const{
    // TODO: Calculate.
    return 1;
}


Ogre::ScriptTranslator* ParticleSystemTranslatorManager::getTranslator(
  const Ogre::AbstractNodePtr &node
){
    Ogre::ScriptTranslator* translator = nullptr;
    if (node->type == Ogre::ANT_OBJECT){
        Ogre::ObjectAbstractNode* obj = reinterpret_cast<Ogre::ObjectAbstractNode*>(node.get());

        if (obj->cls == "effect"){
            Ogre::LogManager::getSingletonPtr()->logMessage(
              "Found object 'effect'."
            );
            translator = &particle_system_translator_;
        }
        else if (obj->cls == "group"){
            Ogre::LogManager::getSingletonPtr()->logMessage(
              "Found object 'technique'."
            );
            translator = &technique_translator_;
        }
        else if (obj->cls == "renderer"){
            Ogre::LogManager::getSingletonPtr()->logMessage(
              "Found object 'renderer'."
            );
            translator = &renderer_translator_;
        }
        else if (obj->cls == "emitter"){
            Ogre::LogManager::getSingletonPtr()->logMessage(
              "Found object 'emitter'."
            );
            translator = &emitter_translator_;
        }
    }
    return translator;
}


