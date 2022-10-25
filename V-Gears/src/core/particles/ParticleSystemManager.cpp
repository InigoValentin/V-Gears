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
#include <OgreRoot.h>
#include "core/particles/ParticleSystemManager.h"

#include "../../../include/core/particles/emitters/PointEmitterFactory.h"
#include "core/particles/ParticleSystemFactory.h"
#include "core/particles/renderer/ParticleEntityRendererFactory.h"

template<> ParticleSystemManager*
  Ogre::Singleton<ParticleSystemManager>::msSingleton = nullptr;


ParticleSystemManager::ParticleSystemManager(){
    translator_manager_ = new ParticleSystemTranslatorManager();
    Ogre::ScriptCompilerManager::getSingleton().addTranslatorManager(
      translator_manager_
    );
    Ogre::ScriptCompilerManager::getSingleton().addScriptPattern("*.effects");
    particle_system_factory_ = new ParticleSystemFactory();
    Ogre::Root::getSingleton().addMovableObjectFactory(
      particle_system_factory_
    );
    AddEmitterFactory(new PointEmitterFactory());
    AddRendererFactory(new ParticleEntityRendererFactory());
}

ParticleSystemManager::~ParticleSystemManager(){
    Ogre::ScriptCompilerManager::getSingleton().removeTranslatorManager(
      translator_manager_
    );
    delete translator_manager_;
    if (particle_system_factory_){
        Ogre::Root::getSingleton().removeMovableObjectFactory(
          particle_system_factory_
        );
        delete particle_system_factory_;
        particle_system_factory_ = 0;
    }

    // Delete all Particle System Templates
    ParticleSystemTemplateMap::iterator t;
    for (
      t = particle_system_templates_.begin();
      t != particle_system_templates_.end();
      ++ t
    ){
        delete t->second;
    }
    particle_system_templates_.clear();
    EmitterFactoryMap::iterator efi;
    for (
      efi = emitter_factories_.begin();
      efi != emitter_factories_.end();
      ++ efi
    ){
        delete efi->second;
    }
    RendererFactoryMap::iterator rfi;
    for (
      rfi = renderer_factories_.begin();
      rfi != renderer_factories_.end();
      ++ rfi
    ){
        delete rfi->second;
    }
}

ParticleSystem* ParticleSystemManager::CreateParticleSystemTemplate(
  const Ogre::String& name
){
    // Validate name
    if (
      particle_system_templates_.find(name) != particle_system_templates_.end()
    ){
        OGRE_EXCEPT(
          Ogre::Exception::ERR_DUPLICATE_ITEM,
          "ParticleSystem template with name '" + name + "' already exists.",
          "ParticleSystemManager::CreateParticleSystemTemplate"
        );
    }
    ParticleSystem* particle_system_template = new ParticleSystem(name);
    particle_system_templates_[name] = particle_system_template;
    return particle_system_template;
}

ParticleSystem* ParticleSystemManager::GetParticleSystemTemplate(
  const Ogre::String& template_name
){
    ParticleSystemTemplateMap::iterator i =
      particle_system_templates_.find(template_name);
    if (i != particle_system_templates_.end()) return i->second;
    return 0;
}

void ParticleSystemManager::ParticleSystemTemplateNames(
  std::vector<Ogre::String>& v
){
    ParticleSystemTemplateMap::iterator it;
    ParticleSystemTemplateMap::iterator it_end =
      particle_system_templates_.end();
    for (it = particle_system_templates_.begin(); it != it_end; ++ it)
        v.push_back((*it).first);
}

ParticleSystem* ParticleSystemManager::CreateParticleSystem(
  const Ogre::String& name, const Ogre::String& template_name
){
    Ogre::NameValuePairList params;
    params["template_name"] = template_name;
    ParticleSystem* system = static_cast<ParticleSystem*>(
      Ogre::Root::getSingleton().getSceneManager("Scene")->createMovableObject(
        name, ParticleSystemFactory::FACTORY_TYPE_NAME, &params
      )
    );
    return system;
}

void ParticleSystemManager::DestroyParticleSystem(
  ParticleSystem* particle_system
){
    Ogre::Root::getSingleton().getSceneManager("Scene")->destroyMovableObject(
      particle_system
    );
}

ParticleTechnique* ParticleSystemManager::CreateTechnique(){
    ParticleTechnique* technique = new ParticleTechnique();
    technique->CreateRenderer("Entity"); // Set default renderer
    return technique;
}

ParticleTechnique* ParticleSystemManager::CloneTechnique(
  ParticleTechnique* technique
){
    if (technique == nullptr) return nullptr;
    ParticleTechnique* cloned_technique = CreateTechnique();
    technique->CopyAttributesTo(cloned_technique);
    return cloned_technique;
}

void ParticleSystemManager::DestroyTechnique(ParticleTechnique* technique){
    // Deletion must be done by the same object that also created it
    delete technique;
}

ParticleEmitter* ParticleSystemManager::CreateEmitter(
  const Ogre::String& emitter_type
){
    // Locate emitter type
    EmitterFactoryMap::iterator it = emitter_factories_.find(emitter_type);
    if (it == emitter_factories_.end()){
        OGRE_EXCEPT(
          Ogre::Exception::ERR_INVALIDPARAMS,
          "Particle System: Cannot find requested emitter type.",
          "ParticleSystemManager::CreateEmitter"
        );
    }
    return it->second->CreateEmitter();
}

ParticleEmitter* ParticleSystemManager::CloneEmitter(ParticleEmitter* emitter){
    if (!emitter) return nullptr;
    ParticleEmitter* cloned_emitter = CreateEmitter(emitter->GetEmitterType());
    emitter->CopyAttributesTo(cloned_emitter);
    return cloned_emitter;
}

void ParticleSystemManager::DestroyEmitter(ParticleEmitter* emitter){
    // Locate emitter type
    EmitterFactoryMap::iterator it = emitter_factories_.find(
      emitter->GetEmitterType()
    );
    if (it == emitter_factories_.end()){
        OGRE_EXCEPT(
          Ogre::Exception::ERR_INVALIDPARAMS,
          "Particle System: Cannot find requested emitter type.",
          "ParticleSystemManager::DestroyEmitter"
        );
    }
    it->second->DestroyEmitter(emitter);
}

ParticleRenderer* ParticleSystemManager::CreateRenderer(
  const Ogre::String& renderer_type
){
    // Locate renderer type
    RendererFactoryMap::iterator it = renderer_factories_.find(renderer_type);
    if (it == renderer_factories_.end()){
        OGRE_EXCEPT(
          Ogre::Exception::ERR_INVALIDPARAMS,
          "Renderer: Cannot find requested renderer type.",
          "ParticleSystemManager::CreateRenderer"
        );
    }
    return it->second->CreateRenderer();
}

ParticleRenderer* ParticleSystemManager::CloneRenderer(
  ParticleRenderer* renderer
){
    if (renderer == nullptr) return nullptr;
    ParticleRenderer* cloned_renderer =
      CreateRenderer(renderer->GetRendererType());
    renderer->CopyAttributesTo(cloned_renderer);
    return cloned_renderer;
}

void ParticleSystemManager::DestroyRenderer(ParticleRenderer* renderer){
    // Locate renderer type
    RendererFactoryMap::iterator it =
      renderer_factories_.find(renderer->GetRendererType());
    if (it == renderer_factories_.end()){
        OGRE_EXCEPT(
          Ogre::Exception::ERR_INVALIDPARAMS,
          "Renderer: Cannot find requested renderer type.",
          "ParticleSystemManager::DestroyRenderer"
        );
    }
    it->second->DestroyRenderer(renderer);
}

void ParticleSystemManager::AddEmitterFactory(ParticleEmitterFactory* factory){
    Ogre::String type = factory->GetEmitterType();
    emitter_factories_[type] = factory;
    Ogre::LogManager::getSingleton().logMessage(
      "Particle system: Particle Emitter Type '" + type + "' registered."
    );
}

void ParticleSystemManager::AddRendererFactory(
  ParticleRendererFactory* factory
){
    Ogre::String type = factory->GetRendererType();
    renderer_factories_[type] = factory;
    Ogre::LogManager::getSingleton().logMessage(
      "Particle system: Particle Renderer Type '" + type + "' registered"
    );
}
