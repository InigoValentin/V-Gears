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
#include "core/particles/ParticleTechniqueTranslator.h"
#include "core/particles/ParticleSystemManager.h"

void ParticleTechniqueTranslator::translate(
  Ogre::ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node
){
    Ogre::ObjectAbstractNode *obj =
      reinterpret_cast<Ogre::ObjectAbstractNode*>(node.get());
    // Create the technique from the material
    ParticleSystem* system = Ogre::any_cast<ParticleSystem*>(
      obj->parent->context
    );
    ParticleTechnique* tech = system->CreateTechnique();
    if (!tech){
        compiler->addError(
          Ogre::ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line
        );
        return;
    }
    obj->context = Ogre::Any(tech);
    Ogre::LogManager::getSingletonPtr()->logMessage(
      "ParticleTechniqueTranslator: create tech"
    );
    for (
      Ogre::AbstractNodeList::iterator i = obj->children.begin();
      i != obj->children.end();
      ++i
    ){
        if ((*i)->type == Ogre::ANT_OBJECT) processNode(compiler, *i);
    }
}
