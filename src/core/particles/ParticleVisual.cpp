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

#include "core/particles/ParticleVisual.h"

VisualParticle::VisualParticle(): Particle(){
    particle_type_ = PT_VISUAL;
    // Default is false, but visual particles are always emitted.
    emittable_ = true;
}

void VisualParticle::InitForExpiration(){
    Particle::InitForExpiration();
    if (additional_data != nullptr) additional_data->SetVisible(false);
}

