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
#include <OgreEntity.h>
#include <OgreRoot.h>
#include "core/EntityModel.h"
#include "core/Logger.h"
#include "core/Timer.h"

EntityModel::EntityModel(
  const Ogre::String& name, const Ogre::String file_name, Ogre::SceneNode* node
): Entity(name, node), animation_current_(nullptr)
{
    Ogre::SceneManager* scene_manager;
    scene_manager = Ogre::Root::getSingleton().getSceneManager("Scene");
    model_ = scene_manager->createEntity(name_, file_name);
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
