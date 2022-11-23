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
#include <OgreSceneNode.h>
#include "core/particles/ParticleSystem.h"
#include "core/particles/ParticleSystemManager.h"

ParticleSystem::ParticleSystem(const Ogre::String& name): MovableObject(name){}


ParticleSystem::~ParticleSystem(){
    // Destroy nodes that are used for renderers.
    if (getParentSceneNode())
        getParentSceneNode()->removeAndDestroyAllChildren();
    DestroyAllTechniques();
}

void ParticleSystem::CopyAttributesTo(ParticleSystem* ps){
    // Destroy all existing techniques.
    ps->DestroyAllTechniques();
    // Copy techniques
    ParticleTechnique* cloned_technique = nullptr;
    for (unsigned int i = 0; i < techniques_.size(); ++ i){
        cloned_technique =
          ParticleSystemManager::getSingletonPtr()->CloneTechnique(
            techniques_[i]
          );
        ps->AddTechnique(cloned_technique);
    }
}

const Ogre::String& ParticleSystem::getMovableType() const{
    Ogre::String type = "ParticleSystem";
    return type;
}

const Ogre::AxisAlignedBox& ParticleSystem::getBoundingBox() const{
    return Ogre::AxisAlignedBox::BOX_INFINITE;
}

Ogre::Real ParticleSystem::getBoundingRadius() const{
    // TODO: Calculate.
    return 999;
}

void ParticleSystem::_updateRenderQueue(Ogre::RenderQueue* queue){
    Ogre::LogManager::getSingletonPtr()->logMessage(
      "ParticleSystem::UpdateRenderQueue"
    );
    for(unsigned int i = 0; i < techniques_.size(); ++ i){
        techniques_[i]->UpdateRenderQueue(queue);
    }
}

void ParticleSystem::Update(Ogre::Real time_elapsed){
    Ogre::LogManager::getSingletonPtr()->logMessage(
      "ParticleSystem::Update STARTED. Technique number: "
      + Ogre::StringConverter::toString(techniques_.size())
    );
    for (unsigned int i = 0; i < techniques_.size(); ++ i){
         techniques_[i]->Update(time_elapsed);
    }
}

ParticleTechnique*ParticleSystem::CreateTechnique(){
    ParticleTechnique* technique =
      ParticleSystemManager::getSingletonPtr()->CreateTechnique();
    AddTechnique(technique);
    return technique;
}

void ParticleSystem::AddTechnique(ParticleTechnique* technique){
    techniques_.push_back(technique);
    technique->SetParentSystem(this);
}

void ParticleSystem::DestroyAllTechniques(){
    for (unsigned int i = 0; i < techniques_.size(); ++ i){
        ParticleSystemManager::getSingletonPtr()->DestroyTechnique(
          techniques_[i]
        );
    }
    techniques_.clear();
}

