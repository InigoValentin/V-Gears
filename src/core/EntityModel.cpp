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

#include <iostream>
#include <algorithm>
#include <OgreEntity.h>
#include <OgreSubEntity.h>
#include <OgreMaterialManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreRoot.h>
#include "core/EntityModel.h"
#include "core/EntityManager.h"
#include "core/Logger.h"
#include "core/Timer.h"

EntityModel::EntityModel(
  const Ogre::String& name, const Ogre::String file_name, Ogre::SceneNode* node,
  const Ogre::String& resource_group
): Entity(name, node), animation_current_(nullptr)
{
    Ogre::SceneManager* scene_manager;
  Ogre::String res_group = resource_group;
  if (res_group.empty()){
    switch(EntityManager::getSingleton().GetModule()){
      case Manager::BATTLE:
        res_group = "BATTLE_MODELS";
        break;
      case Manager::WORLD:
        res_group = "WORLD_MODELS";
        break;
      case Manager::FIELD:
      default:
        res_group = "FIELD_MODELS";
    }
    }
    scene_manager = Ogre::Root::getSingleton().getSceneManager("Scene");
    model_ = scene_manager->createEntity(name_, file_name, res_group);
    model_->setVisible(false);
    PlayAnimation(animation_default_, Entity::AUTO_ANIMATION, Entity::PLAY_LOOPED, 0, -1);
    model_node_->attachObject(model_);
}

EntityModel::~EntityModel(){
    Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(model_);
}

void EntityModel::Update(){
    if (animation_auto_play_ == true) UpdateAnimation(Timer::getSingleton().GetGameTimeDelta());
    Entity::Update();
}

void EntityModel::SetVisible(const bool visible){model_->setVisible(visible);}

bool EntityModel::IsVisible() const{return model_->isVisible();}

float EntityModel::GetAnimationLength(const Ogre::String& animation) const{
    Ogre::AnimationStateSet* states = model_->getAllAnimationStates();
    if (states == nullptr || states->hasAnimationState(animation) == false) return 0.0f;
    return states->getAnimationState(animation)->getLength();
}

unsigned int EntityModel::GetAnimationCount() const{
    Ogre::AnimationStateSet* states = model_->getAllAnimationStates();
    if (states == nullptr) return 0;
    return states->getAnimationStates().size();
}

Ogre::Entity* EntityModel::GetModel(){return model_;}

void EntityModel::ScriptSetBlend(const int blend){
    if (blend == blend_) return;
    blend_ = blend;
    for (unsigned int s = 0; s < model_->getNumSubEntities(); ++ s){
        Ogre::SubEntity* sub_entity = model_->getSubEntity(s);
        // Materials are shared among all the entities of a mesh, so use one of its own.
        Ogre::MaterialPtr material = sub_entity->getMaterial();
        if (std::find(own_materials_.begin(), own_materials_.end(), material->getName()) == own_materials_.end()){
            const Ogre::String own_name = material->getName() + "/" + name_;
            Ogre::MaterialPtr own = Ogre::MaterialManager::getSingleton().getByName(own_name, material->getGroup());
            if (own == nullptr) own = material->clone(own_name);
            own_materials_.push_back(own_name);
            sub_entity->setMaterial(own);
            material = own;
        }
        for (unsigned short t = 0; t < material->getNumTechniques(); ++ t){
            Ogre::Technique* technique = material->getTechnique(t);
            for (unsigned short p = 0; p < technique->getNumPasses(); ++ p){
                Ogre::Pass* pass = technique->getPass(p);
                switch (blend){
                    case BLEND_ADDITIVE:
                        pass->setSceneBlending(Ogre::SBT_ADD);
                        pass->setDepthWriteEnabled(false);
                        pass->setLightingEnabled(false);
                        break;
                    case BLEND_ALPHA:
                        pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
                        pass->setDepthWriteEnabled(false);
                        pass->setLightingEnabled(false);
                        break;
                    default:
                        pass->setSceneBlending(Ogre::SBT_REPLACE);
                        pass->setDepthWriteEnabled(true);
                }
            }
        }
    }
}

void EntityModel::ScriptSetAlpha(const float alpha){
    if (blend_ == BLEND_NORMAL) return;
    const float a = std::max(0.0f, std::min(1.0f, alpha));
    for (unsigned int s = 0; s < model_->getNumSubEntities(); ++ s){
        Ogre::MaterialPtr material = model_->getSubEntity(s)->getMaterial();
        for (unsigned short t = 0; t < material->getNumTechniques(); ++ t){
            Ogre::Technique* technique = material->getTechnique(t);
            for (unsigned short p = 0; p < technique->getNumPasses(); ++ p){
                Ogre::Pass* pass = technique->getPass(p);
                for (unsigned short u = 0; u < pass->getNumTextureUnitStates(); ++ u){
                    Ogre::TextureUnitState* unit = pass->getTextureUnitState(u);
                    if (blend_ == BLEND_ADDITIVE){
                        // The colour is added, so less brightness is less opacity.
                        unit->setColourOperationEx(
                          Ogre::LBX_MODULATE, Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL,
                          Ogre::ColourValue::White, Ogre::ColourValue(a, a, a)
                        );
                    }
                    else{
                        unit->setAlphaOperation(
                          Ogre::LBX_MODULATE, Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL, 1.0f, a
                        );
                    }
                }
            }
        }
    }
}

void EntityModel::ReleaseResources(){
    // The materials are used by the model, so make it use none before removing them.
    for (const Ogre::String& name : own_materials_){
        // The group is not known here (the material has the one of the original), so search all.
        Ogre::MaterialManager& manager = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = manager.getByName(
          name, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME
        );
        if (material != nullptr) manager.remove(material);
    }
    own_materials_.clear();
}

void EntityModel::PlayAnimation(
  const Ogre::String& animation, Entity::AnimationState state,
  Entity::AnimationPlayType play_type, const float start, const float end
){
    if (animation_current_ != nullptr) animation_current_->setEnabled(false);
    if (
      model_->getAllAnimationStates() != nullptr
      && model_->getAllAnimationStates()->hasAnimationState(animation) == true
    ){
        animation_current_name_ = animation;
        animation_current_ = model_->getAnimationState(animation);
        animation_current_->setLoop((play_type == Entity::PLAY_LOOPED) ? true : false);
        // Single frame animations have no length. A looping animation without length wraps its
        // time position dividing by zero, ends up with a time that is not a number, and never
        // finishes, so any script waiting for it would wait forever.
        if (animation_current_->getLength() <= 0.0f) animation_current_->setLoop(false);
        animation_current_->setEnabled(true);
        animation_current_->setTimePosition(
          (start == -1) ? animation_current_->getLength() : start
        );
        animation_end_time_ = (end == -1) ? animation_current_->getLength() : end;
        animation_state_ = state;
        animation_play_type_ = play_type;
    }
    else{
        // Idle is hard coded to the default animation,
        // so don't spam crazy amounts of errors if its not found.
        if (animation != "Idle"){
            LOG_ERROR(
              "Animation '" + animation + "' doesn't exist in model '" + model_->getName() + "'."
            );
        }
    }
}

void EntityModel::PlayAnimationContinue(const Ogre::String& animation){
    if (
      // If animation isn't being played...
      animation_current_ == nullptr
      // ... the animatio to play exists (use this to avoid exception)...
      || (model_->getAllAnimationStates()->hasAnimationState(animation) == true
      && (
        // and (the requested animation is not currently playing...
        animation_current_ != model_->getAnimationState(animation)
        // or it's currently playing  but it's not looped)
        || animation_play_type_ != Entity::PLAY_LOOPED))
    ){
        PlayAnimation(animation, Entity::AUTO_ANIMATION, Entity::PLAY_LOOPED, 0, -1);
    }
}

void EntityModel::UpdateAnimation(const float delta){
    if (animation_current_ != nullptr){
        float delta_mod = delta * animation_speed_;
        bool stop_check
          = (animation_current_->hasEnded() == true)
          || (animation_current_->getTimePosition() + delta_mod >= animation_end_time_);

        if (stop_check == true){
            //LOG_TRIVIAL("Animation finished for entity \"" + name_ + "\".");
            for (size_t i = 0; i < animation_sync_.size(); ++ i)
                ScriptManager::getSingleton().ContinueScriptExecution(animation_sync_[i]);
            animation_sync_.clear();

            if (animation_play_type_ == Entity::PLAY_DEFAULT){
                float time = (animation_current_->hasEnded() != true)
                  ? animation_current_->getTimePosition() : 0;
                PlayAnimation(
                  animation_default_, Entity::AUTO_ANIMATION, Entity::PLAY_LOOPED, time, -1
                );
                animation_current_->addTime(delta_mod);
            }
            else if (animation_play_type_ == Entity::PLAY_LOOPED){
                float time = (animation_current_->hasEnded() != true)
                  ? animation_current_->getTimePosition()
                  : animation_current_->getTimePosition() - animation_current_->getLength();
                PlayAnimation(
                  animation_current_name_, Entity::AUTO_ANIMATION, Entity::PLAY_LOOPED, time, -1
                );
                animation_current_->addTime(delta_mod);
            }
        }
        else animation_current_->addTime(delta_mod);
    }
}
