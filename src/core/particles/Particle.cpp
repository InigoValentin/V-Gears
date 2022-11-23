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

#include "core/particles/Particle.h"

Particle::Particle():
    parent_emitter_(nullptr),
    particle_type_(PT_VISUAL),
    enabled_(true),
    emittable_(false),
    additional_data(nullptr),
    position(Ogre::Vector3::ZERO),
    direction(Ogre::Vector3::ZERO),
    time_to_live(10),
    total_time_to_live(10)
{}

Particle::~Particle(){}

void Particle::Update(Ogre::Real time_elapsed){
    position += direction * time_elapsed;
}

void Particle::CopyAttributesTo(Particle* particle){
    // Don't copy parent_emitter_ and particle_type_
    // This can be emited from other emitter and have different type
    particle->enabled_ = enabled_;
    particle->SetEmittable(emittable_);
    particle->position = position;
    particle->direction = direction;
    particle->time_to_live = time_to_live;
    particle->total_time_to_live = total_time_to_live;
}
