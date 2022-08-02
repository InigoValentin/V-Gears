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
#include <OgreString.h>
#include <OgreStringConverter.h>
#include "core/particles/ParticleEmitter.h"
#include "core/particles/ParticleTechnique.h"

ParticleEmitterDictionary::EmissionRate
  ParticleEmitter::emission_rate_dictionary_;
ParticleEmitterDictionary::TotalTimeToLive
  ParticleEmitter::total_time_to_live_dictionary_;
ParticleEmitterDictionary::Direction ParticleEmitter::direction_dictionary_;

ParticleEmitter::ParticleEmitter(void) :
    Particle(),
    parent_technique_(nullptr),
    name_(Ogre::BLANKSTRING),
    emits_name_(Ogre::BLANKSTRING),
    emits_type_(PT_VISUAL),
    emission_rate_(1),
    emission_remainder_(0),
    emit_direction_1_(Ogre::Vector3::ZERO),
    emit_direction_2_(Ogre::Vector3::ZERO),
    emit_total_time_to_live_(10)
{
    particle_type_ = PT_EMITTER;
    if (createParamDictionary("ParticleEmitter")){
        Ogre::ParamDictionary* dict = getParamDictionary();
        dict->addParameter(
          Ogre::ParameterDef("emission_rate", "", Ogre::PT_INT),
          &emission_rate_dictionary_
        );
        dict->addParameter(
          Ogre::ParameterDef("time_to_live", "", Ogre::PT_REAL),
          &total_time_to_live_dictionary_
        );
        dict->addParameter(
          Ogre::ParameterDef("direction", "", Ogre::PT_STRING),
          &direction_dictionary_
        );
    }
}

ParticleEmitter::~ParticleEmitter(){}

void ParticleEmitter::CopyAttributesTo(ParticleEmitter* emitter){
    Particle::CopyAttributesTo(emitter);
    emitter->SetParentTechnique(parent_technique_);
    emitter->SetName(name_);
    emitter->SetEmitterType(emitter_type_);
    emitter->SetEmitsName(emits_name_);
    emitter->SetEmitsType(emits_type_);
    emitter->SetEmissionRate(emission_rate_);
    emitter->SetEmitDirectionRange(emit_direction_1_, emit_direction_2_);
    emitter->SetEmitTotalTimeToLive(emit_total_time_to_live_);
}

void ParticleEmitter::InitForEmission(){emission_remainder_ = 0;}

int ParticleEmitter::CalculateRequestedParticles(Ogre::Real time_elapsed){
    int request = 0;
    Ogre::LogManager::getSingletonPtr()->logMessage(
      "ParticleEmitter::CalculateRequestedParticles time:"
      + Ogre::StringConverter::toString(time_elapsed)
      + " emission rate:"
      + Ogre::StringConverter::toString(emission_rate_)
      + " reminder:"
      + Ogre::StringConverter::toString(emission_remainder_)
    );
    if (enabled_ && emission_rate_ > 0){
        emission_remainder_ += emission_rate_ * time_elapsed;
        request = (int) emission_remainder_;
        emission_remainder_ -= request;
    }
    return request;
}

void ParticleEmitter::InitParticleForEmission(Particle* particle){
    particle->SetParentEmitter(this);
    particle->position = position; // Particle emits from emitter position
    particle->direction.x = Ogre::Math::RangeRandom(
      emit_direction_1_.x, emit_direction_2_.x
    );
    particle->direction.y = Ogre::Math::RangeRandom(
      emit_direction_1_.y, emit_direction_2_.y
    );
    particle->direction.z = Ogre::Math::RangeRandom(
      emit_direction_1_.z, emit_direction_2_.z
    );
    particle->time_to_live = emit_total_time_to_live_;
    particle->total_time_to_live = emit_total_time_to_live_;
}

void ParticleEmitter::SetEmitsName(const Ogre::String& emits_name){
    emits_name_ = emits_name;
    if (parent_technique_ && emits_name_ != Ogre::BLANKSTRING){
        // Notify the Technique that something changed
        parent_technique_->EmissionChange();
    }
}
