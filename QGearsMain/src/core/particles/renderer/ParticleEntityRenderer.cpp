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

#include <OgreEntity.h>
#include <OgreRoot.h>
#include "core/particles/renderer/ParticleEntityRenderer.h"
#include "core/particles/ParticleSystem.h"
#include "core/particles/ParticleTechnique.h"
#include "core/particles/ParticleVisual.h"

ParticleEntityRendererDictionary::MeshName
  ParticleEntityRenderer::mesh_name_dictionary_;

ParticleEntityRenderer::ParticleEntityRenderer():
    ParticleRenderer(), mesh_name_(Ogre::BLANKSTRING)
{
    renderer_type_ = "Entity";
    if (createParamDictionary("ParticleEntityRenderer")){
        Ogre::ParamDictionary* dict = getParamDictionary();
        dict->addParameter(
          Ogre::ParameterDef("mesh_name", "", Ogre::PT_STRING),
          &mesh_name_dictionary_
        );
    }
}

ParticleEntityRenderer::~ParticleEntityRenderer(){
    if (parent_technique_ == nullptr) return;
    Clear();
}

void ParticleEntityRenderer::CopyAttributesTo(ParticleRenderer* renderer){
    // First copy parent attributes
    ParticleRenderer::CopyAttributesTo(renderer);

    // First cast to ParticleEntityRenderer
    ParticleEntityRenderer* entity_renderer =
      static_cast<ParticleEntityRenderer*>(renderer);

    // Copy attributes in case there is a entityRenderer (which should be
    // available)
    entity_renderer->mesh_name_ = mesh_name_;
}

const Ogre::String& ParticleEntityRenderer::GetMeshName() const{
    return mesh_name_;
}


void ParticleEntityRenderer::SetMeshName(const Ogre::String& mesh_name){
    mesh_name_ = mesh_name;
    renderer_initialized_ = false; // Re-triggers the _prepare function.
    Clear(); // Delete all nodes in entities, they will be rebuilt.
}

void ParticleEntityRenderer::Clear(){
    if (parent_technique_ == nullptr) return;

    // Note: The created ChildSceneNodes are destroyed when the ParticleSystem
    // is destroyed
    std::vector<ParticleEntityAdditionalData*>::const_iterator it;
    std::vector<ParticleEntityAdditionalData*>::const_iterator itEnd =
      all_additional_data_.end();
    for (it = all_additional_data_.begin(); it != itEnd; ++ it) delete (*it);
    all_additional_data_.clear();
    unassigned_additional_data_.clear();

    // Destroy the Entities. Do it like this, because it must be assured that
    // the entity still exists and has not already been destroyed.
    Ogre::SceneManager* scene_manager =
      Ogre::Root::getSingleton().getSceneManager("Scene");
    for(unsigned int i = 0; i < entities_.size(); i ++)
        scene_manager->destroyEntity( entities_[ i ] );
    entities_.clear();

    // Reset the visual data in the pool
    parent_technique_->ResetVisualParticles();
}

void ParticleEntityRenderer::SetVisible(bool visible){
    std::vector<ParticleEntityAdditionalData*>::const_iterator it;
    std::vector<ParticleEntityAdditionalData*>::const_iterator itEnd =
      all_additional_data_.end();
    for (it = all_additional_data_.begin(); it != itEnd; ++ it)
        (*it)->node->setVisible(visible);
}

void ParticleEntityRenderer::Initialize(){
    // Use the given technique, although it should be the same as
    // parent_technique_ (must be set already)
    if (!parent_technique_ || renderer_initialized_) return;

    std::stringstream ss;
    ss << this;
    Ogre::String entity_name = mesh_name_ + ss.str();
    int quota = parent_technique_->GetVisualParticlesQuota();
    Ogre::SceneNode* parent_node =
      parent_technique_->GetParentSystem()->getParentSceneNode();

    if (parent_node){
        // Create number of visual_data objects including SceneNodes
        for (int i = 0; i < quota; i++){
            ParticleEntityAdditionalData* visual_data =
              new ParticleEntityAdditionalData(
                parent_node->createChildSceneNode()
              );

            // Used to assign to a particle
            all_additional_data_.push_back(visual_data);

            // Managed by this renderer
            unassigned_additional_data_.push_back(visual_data);
        }

        // Create number of Entities
        Ogre::Entity* entity =
          Ogre::Root::getSingleton().getSceneManager("Scene")->createEntity(
            entity_name, mesh_name_
          ); // Base entity

        for (unsigned int i = 0; i < all_additional_data_.size(); ++ i){
            Ogre::Entity* cloned_entity =
              entity->clone(
                entity_name + Ogre::StringConverter::toString(i)
              );
            entities_.push_back(cloned_entity);
            all_additional_data_[i]->node->attachObject(cloned_entity);
        }
        Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(
          entity_name
        );
    }

    SetVisible(false);
    renderer_initialized_ = true;
}

void ParticleEntityRenderer::UpdateRenderQueue(
  Ogre::RenderQueue* queue, ParticlePool<VisualParticle>& pool
){
    // Fast check to determine whether there are visual particles
    if (pool.IsEmpty()) return;

    VisualParticle* particle = pool.GetFirst();
    while (!pool.End()){
        if (particle){
            if (
              !particle->additional_data && !unassigned_additional_data_.empty()
            ){
                particle->additional_data = unassigned_additional_data_.back();
                unassigned_additional_data_.pop_back();
            }

            if (particle->additional_data){
                Ogre::SceneNode* node =
                  (
                     static_cast<ParticleEntityAdditionalData*>(
                       particle->additional_data
                     )
                   )->node;
                if (node){
                    node->setPosition(particle->position);
                    node->setVisible(true);
                }
            }
        }
        particle = pool.GetNext();
    }
}
