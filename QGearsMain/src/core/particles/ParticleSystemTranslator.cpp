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
#include "core/particles/ParticleSystemTranslator.h"
#include "core/particles/ParticleSystemManager.h"

void ParticleSystemTranslator::translate(
  Ogre::ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node
){
    Ogre::ObjectAbstractNode *obj =
      reinterpret_cast<Ogre::ObjectAbstractNode*>(node.get());
    // Name can't be empty because particle systems are added by it.
    if(obj->name.empty()){
        compiler->addError(
          Ogre::ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line
        );
        return;
    }
    ParticleSystem* system =
      ParticleSystemManager::getSingleton().CreateParticleSystemTemplate(
        obj->name
      );
    Ogre::LogManager::getSingletonPtr()->logMessage(
      "ParticleSystemTranslator: create template for particle system '"
      + obj->name + "'"
    );
    if (!system){
        compiler->addError(
          Ogre::ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line
        );
        return;
    }
    obj->context = Ogre::Any(system);
    for (
      Ogre::AbstractNodeList::iterator i = obj->children.begin();
      i != obj->children.end();
      ++ i
    ){
        if ((*i)->type != Ogre::ANT_PROPERTY) processNode(compiler, *i);
    }
}


