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
#include <OgreStringConverter.h>
#include "core/particles/ParticleTechnique.h"
#include "core/particles/ParticleSystemManager.h"
#include "core/particles/ParticleVisual.h"


ParticleTechnique::ParticleTechnique():
  renderer_(nullptr),
  visual_particle_quota_(10),
  visual_particle_pool_increased_(false),
  emitted_emitter_quota_(10),
  particle_emitter_pool_increased_(false)
{}

ParticleTechnique::~ParticleTechnique(){
    DestroyAllEmitters();
    DestroyRenderer();
}

void ParticleTechnique::CopyAttributesTo(ParticleTechnique* technique){
    // Destroy the emitters, affectors, etc.
    technique->DestroyAllEmitters();
    technique->DestroyRenderer();
    technique->parent_system_ = parent_system_;
    // Copy the renderer
    ParticleRenderer* cloned_renderer =
      ParticleSystemManager::getSingletonPtr()->CloneRenderer(renderer_);
    technique->SetRenderer(cloned_renderer);
    technique->visual_particle_quota_ = visual_particle_quota_;
    technique->visual_particle_pool_increased_ = false;
    technique->emitted_emitter_quota_ = emitted_emitter_quota_;
    technique->particle_emitter_pool_increased_ = false;
    // Copy all emitters
    for (unsigned int i = 0; i < emitters_.size(); ++ i){
        ParticleEmitter* cloned_emitter = ParticleSystemManager::getSingletonPtr()->CloneEmitter(
          emitters_[i]
        );
        technique->AddEmitter( cloned_emitter );
    }
}

void ParticleTechnique::UpdateRenderQueue(Ogre::RenderQueue* queue){
    if (renderer_ && renderer_->IsRendererInitialised())
        renderer_->UpdateRenderQueue(queue, visual_particles_pool_);

}

void ParticleTechnique::Initialize(){
    if (renderer_ && renderer_->IsRendererInitialised() == false) renderer_->Initialize();
    // Create new visual particles if the quota has been increased
    if (visual_particle_pool_increased_ == false){
        int old_size = visual_particles_pool_.GetSize();
        if (visual_particle_quota_ > old_size){
            // Create new visual particles
            VisualParticle* particle = nullptr;
            for (int i = old_size; i < visual_particle_quota_; ++ i){
                particle = new VisualParticle();
                visual_particles_pool_.AddElement(particle);
                visual_particles_.push_back(particle);
            }
        }
        visual_particle_pool_increased_ = true;
    }
    // Create new emitter particles if the quota has been increased
    if (particle_emitter_pool_increased_ == false){
        int old_size = particle_emitter_pool_.GetSize();
        if (emitted_emitter_quota_ > old_size){
            // Create new emitters, based on the already created emitters
            // in the technique and which are marked for emission.
            int number_of_emitted_emitters = GetNumEmittableEmitters();
            if (number_of_emitted_emitters == 0) return;
            ParticleEmitter* cloned_emitter;
            // Distribute size equally
            int increment = emitted_emitter_quota_ / number_of_emitted_emitters;
            // Run through emitters of the technique
            for( unsigned int i = 0; i < emitters_.size(); ++i ){
                if( emitters_[ i ]->IsEmittable() ){
                    // Clone the emitter 'increment' times and add to the pool
                    for (int j = 0; j < increment; ++ j){
                        cloned_emitter = ParticleSystemManager::getSingletonPtr()->CloneEmitter(
                          emitters_[i]
                        );
                        cloned_emitter->SetEmittable(true);
                        particle_emitter_pool_.AddElement(
                          cloned_emitter->GetName(), cloned_emitter
                        );
                    }
                }
            }
        }
        particle_emitter_pool_increased_ = true;
    }
}

void ParticleTechnique::Update(Ogre::Real time_elapsed){
    // Perform some initialisation type of activities (if needed). This must be done within the
    // update-loop, because settings could be changed (i.e. changing quota), which must trigger a
    // re-initialisation.
    Initialize();
    // Process the emitters.
    for (unsigned int i = 0; i < emitters_.size(); ++ i){
        // Wmitted particles handled in pool update.
        if (emitters_[ i ]->IsEmittable() == false){
            ExecuteEmitParticles(
              emitters_[i], emitters_[i]->CalculateRequestedParticles(time_elapsed), time_elapsed
            );
        }
    }

    if (visual_particles_pool_.IsEmpty() == false){
        VisualParticle* particle = visual_particles_pool_.GetFirst();
        while (!visual_particles_pool_.End()){
            if (particle != nullptr){
                if (particle->time_to_live > time_elapsed) particle->Update(time_elapsed);
                else{
                    particle->InitForExpiration();
                    visual_particles_pool_.LockLatestElement();
                }
                // Decrement time to live
                particle->time_to_live -= time_elapsed;
            }
            particle = visual_particles_pool_.GetNext();
        }
    }
    // Process all particles
    if (particle_emitter_pool_.IsEmpty() == false){
        ParticleEmitter* particle = particle_emitter_pool_.GetFirst();
        while (!particle_emitter_pool_.End()){
            if (particle){
                if (particle->time_to_live > time_elapsed){
                    particle->Update(time_elapsed);
                    ExecuteEmitParticles(
                      particle, particle->CalculateRequestedParticles(time_elapsed), time_elapsed
                    );
                }
                else{
                    particle->InitForExpiration();
                    particle_emitter_pool_.LockLatestElement();
                }
                // Decrement time to live
                particle->time_to_live -= time_elapsed;
            }
            particle = particle_emitter_pool_.GetNext();
        }
    }
}

ParticleRenderer* ParticleTechnique::CreateRenderer(const Ogre::String& renderer_type){
    if (renderer_) DestroyRenderer();
    if (renderer_type != Ogre::BLANKSTRING){
        renderer_ = ParticleSystemManager::getSingletonPtr()->CreateRenderer(renderer_type);
        renderer_->SetParentTechnique(this);
        renderer_->SetRendererInitialised(false);
    }
    return renderer_;
}

void ParticleTechnique::SetRenderer(ParticleRenderer* renderer){
    if (renderer_) DestroyRenderer();
    renderer_ = renderer;
    renderer_->SetParentTechnique(this);
    renderer_->SetRendererInitialised(false);
}

void ParticleTechnique::DestroyRenderer(){
    if (renderer_){
        ParticleSystemManager::getSingletonPtr()->DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
}

ParticleEmitter* ParticleTechnique::CreateEmitter(const Ogre::String& emitter_type){
    ParticleEmitter* emitter = nullptr;
    if (emitter_type != Ogre::BLANKSTRING){
        emitter = ParticleSystemManager::getSingletonPtr()->CreateEmitter(emitter_type);
        emitters_.push_back(emitter);
        emitter->SetParentTechnique(this);
        EmissionChange();
    }
    return emitter;
}

void ParticleTechnique::AddEmitter(ParticleEmitter* emitter){
    if (emitter != nullptr){
        emitters_.push_back(emitter);
        emitter->SetParentTechnique(this);
        EmissionChange();
    }
}

void ParticleTechnique::DestroyAllEmitters(){
    for (unsigned int i = 0; i < emitters_.size(); ++ i)
        ParticleSystemManager::getSingletonPtr()->DestroyEmitter(emitters_[i]);
    emitters_.clear();
}

int ParticleTechnique::GetNumEmittableEmitters() const{
    int count = 0;
    for (unsigned int i = 0; i < emitters_.size(); ++ i) if (emitters_[i]->IsEmittable()) count++;
    return count;
}

void ParticleTechnique::EmissionChange() {
    // Run through all emitters and set MarkedForEmission to false.
    for (unsigned int i = 0; i < emitters_.size(); ++ i) emitters_[i]->SetEmittable(false);
    // Run through all emitters and determine which objects each one emits.
    for (unsigned int i = 0; i < emitters_.size(); ++ i){
        Ogre::String name = emitters_[i]->GetEmitsName();
        if (name != emitters_[i]->GetName()){
            // Search the emitter to be emitted
            for (unsigned int j = 0; j < emitters_.size(); ++ j)
                if (emitters_[j]->GetName() == name) emitters_[j]->SetEmittable(true);
        }
    }
}

void ParticleTechnique::ExecuteEmitParticles(
  ParticleEmitter* emitter, int requested, Ogre::Real time_elapsed
){
    // Only proceed if the emitter and technique are enabled
    if (emitter->IsEnabled() == false) return;
    Ogre::LogManager::getSingletonPtr()->logMessage(
      "ParticleTechnique::ExecuteEmitParticles request "
      + Ogre::StringConverter::toString(requested) + " particles to emit."
    );
    for (int j = 0; j < requested; ++ j){
        // Create a new particle & init using emitter
        Particle* particle = nullptr;
        switch (emitter->GetEmitsType()){
            case Particle::PT_VISUAL:
                particle = visual_particles_pool_.ReleaseElement();
                break;
            case Particle::PT_EMITTER:
                particle = particle_emitter_pool_.ReleaseElement(emitter->GetEmitsName());
                break;
        }
        // Return if there is no particle left anymore, or the name cannot be found.
        if (particle == nullptr){
            Ogre::LogManager::getSingletonPtr()->logMessage(
              "ParticleTechnique::ExecuteEmitParticles request there are no particles left."
            );
            return;
        }
        particle->InitForEmission();
        // Initialize the particle with data from the emitter.
        emitter->InitParticleForEmission(particle);
    }
}

void ParticleTechnique::ResetVisualParticles(){
    for (unsigned int i = 0; i < visual_particles_.size(); ++ i)
        visual_particles_[i]->additional_data = nullptr;
    visual_particles_pool_.LockAllElements();
}
