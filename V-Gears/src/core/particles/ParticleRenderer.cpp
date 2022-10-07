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

#include "core/particles/ParticleRenderer.h"

ParticleRenderer::ParticleRenderer():
    parent_technique_(nullptr),
    renderer_initialized_(false)
{}

ParticleRenderer::~ParticleRenderer(){}

void ParticleRenderer::CopyAttributesTo(ParticleRenderer* renderer){}

void ParticleRenderer::SetRendererInitialised(bool renderer_initialised){
    renderer_initialized_ = renderer_initialised;
}

bool ParticleRenderer::IsRendererInitialised() const{
    return renderer_initialized_;
}

void ParticleRenderer::SetRendererType(Ogre::String renderer_type){
    renderer_type_ = renderer_type;
}

const Ogre::String&ParticleRenderer::GetRendererType() const{
    return renderer_type_;
}

void ParticleRenderer::SetParentTechnique(ParticleTechnique* parent_technique){
    parent_technique_ = parent_technique;
}

const ParticleTechnique* ParticleRenderer::GetParentTechnique() const{
    return parent_technique_;
}
