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
#include "core/particles/ParticleEmitterTranslator.h"
#include "core/particles/ParticleSystemManager.h"

void ParticleEmitterTranslator::translate(
  Ogre::ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node
){
    Ogre::ObjectAbstractNode *obj =
      reinterpret_cast<Ogre::ObjectAbstractNode*>(node.get());

    // Name can't be empty because the renderer type is retrieved from it.
    if (obj->name.empty()){
        compiler->addError(
          Ogre::ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line
        );
        return;
    }

    ParticleTechnique* tech = Ogre::any_cast<ParticleTechnique*>(
      obj->parent->context
    );

    ParticleEmitter* emitter = tech->CreateEmitter(obj->name);
    if (emitter == nullptr){
        compiler->addError(
          Ogre::ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line
        );
        return;
    }
    Ogre::LogManager::getSingletonPtr()->logMessage(
      "ParticleEmitterTranslator: create emitter " + obj->name
    );

    for (
      Ogre::AbstractNodeList::iterator i = obj->children.begin();
      i != obj->children.end();
      ++i
    ){
        if ((*i)->type == Ogre::ANT_PROPERTY){
            Ogre::PropertyAbstractNode *prop =
              reinterpret_cast<Ogre::PropertyAbstractNode*>((*i).get());
            Ogre::String value = prop->getValue();
            /*
            // Glob the values together
            for (
              Ogre::AbstractNodeList::iterator i = prop->values.begin();
              i != prop->values.end();
              ++ i
            ){
                if((*i)->type == Ogre::ANT_ATOM){
                    if (value.empty()){
                        value = ((Ogre::AtomAbstractNode*)(*i).get())->value;
                    }
                    else{
                        value =
                          value
                          + " " + ((Ogre::AtomAbstractNode*)(*i).get())->value;
                    }
                }
                else{
                    compiler->addError(
                      Ogre::ScriptCompiler::CE_INVALIDPARAMETERS,
                      prop->file, prop->line
                    );
                    break;
                }
            }
            */
            Ogre::LogManager::getSingletonPtr()->logMessage(
              "ParticleEmitterTranslator: Set param '"
              + prop->name + "' to '" + value + "'."
            );
            if (!emitter->setParameter(prop->name, value)){
                compiler->addError(
                  Ogre::ScriptCompiler::CE_INVALIDPARAMETERS,
                  prop->file, prop->line
                );
            }
        }
    }
}

