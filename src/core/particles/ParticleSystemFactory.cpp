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

#include "core/particles/ParticleSystemFactory.h"
#include "core/particles/ParticleSystem.h"
#include "core/particles/ParticleSystemManager.h"

Ogre::String ParticleSystemFactory::FACTORY_TYPE_NAME = "VgearsParticleSystem";


const Ogre::String& ParticleSystemFactory::getType() const{
    return FACTORY_TYPE_NAME;
}

void ParticleSystemFactory::DestroyInstance(Ogre::MovableObject* obj){
    delete obj;
}

Ogre::MovableObject* ParticleSystemFactory::createInstanceImpl(
  const Ogre::String& name, const Ogre::NameValuePairList* params
){
    if (params != 0){
        Ogre::NameValuePairList::const_iterator ni =
          params->find("template_name");
        if (ni != params->end()){
            Ogre::String template_name = ni->second;

            // Look up template
            ParticleSystem* p_template =
              ParticleSystemManager::getSingleton().GetParticleSystemTemplate(
                template_name
              );
            if (!p_template){
                OGRE_EXCEPT(
                  Ogre::Exception::ERR_INVALIDPARAMS,
                  "Cannot find required template '" + template_name + "'",
                  "ParticleSystemManager::CreateSystem"
                );
            }

            ParticleSystem* sys = new ParticleSystem(name);

            // Copy template settings
            p_template->CopyAttributesTo(sys);
            return sys;
        }
    }

    // Not template based, just create one with the given name
    return new ParticleSystem(name);
}


