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
#include <vector>
#include <OgreEntity.h>
#include <OgreSubEntity.h>
#include <OgreBone.h>
#include <OgreSkeletonInstance.h>
#include <OgreMaterialManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreRoot.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
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
    resource_group_ = res_group;
    model_ = scene_manager->createEntity(name_, file_name, res_group);
    model_->setVisible(false);
    PlayAnimation(animation_default_, Entity::AUTO_ANIMATION, Entity::PLAY_LOOPED, 0, -1);
    model_node_->attachObject(model_);
}

EntityModel::~EntityModel(){
    ScriptAttachModel("", -1, 0.0f, 0.0f, 0.0f);
    Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(model_);
}

void EntityModel::Update(){
    if (animation_auto_play_ == true) UpdateAnimation(Timer::getSingleton().GetGameTimeDelta());
    Entity::Update();
}

void EntityModel::SetVisible(const bool visible){
    model_->setVisible(visible);
    if (attached_model_ != nullptr) attached_model_->setVisible(visible);
}

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

Ogre::String EntityModel::GetAnimationName(const unsigned int index) const{
    Ogre::SkeletonInstance* skeleton = model_->getSkeleton();
    if (skeleton == nullptr || index >= skeleton->getNumAnimations()) return "";
    return skeleton->getAnimation(index)->getName();
}

Ogre::Entity* EntityModel::GetModel(){return model_;}

Ogre::MaterialPtr EntityModel::OwnMaterial(Ogre::SubEntity* sub_entity){
    Ogre::MaterialPtr material = sub_entity->getMaterial();
    if (std::find(own_materials_.begin(), own_materials_.end(), material->getName()) == own_materials_.end()){
        const Ogre::String own_name = material->getName() + "/" + name_;
        Ogre::MaterialPtr own = Ogre::MaterialManager::getSingleton().getByName(own_name, material->getGroup());
        if (own == nullptr) own = material->clone(own_name);
        own_materials_.push_back(own_name);
        sub_entity->setMaterial(own);
        material = own;
    }
    return material;
}

void EntityModel::ScriptSetTexture(const char* texture){
    const Ogre::String image(texture);
    Ogre::String group;
    if (Ogre::ResourceGroupManager::getSingleton().resourceExists("BATTLE_MODELS", image)) group = "BATTLE_MODELS";
    else if (Ogre::ResourceGroupManager::getSingleton().resourceExists("IMAGES", image)) group = "IMAGES";
    else{
        LOG_WARNING("Image '" + image + "' for the model '" + name_ + "' doesn't exist.");
        return;
    }
    Ogre::TexturePtr loaded = Ogre::TextureManager::getSingleton().load(image, group);
    for (unsigned int s = 0; s < model_->getNumSubEntities(); ++ s){
        Ogre::MaterialPtr material = OwnMaterial(model_->getSubEntity(s));
        for (unsigned short t = 0; t < material->getNumTechniques(); ++ t){
            Ogre::Technique* technique = material->getTechnique(t);
            for (unsigned short p = 0; p < technique->getNumPasses(); ++ p){
                Ogre::Pass* pass = technique->getPass(p);
                pass->removeAllTextureUnitStates();
                pass->createTextureUnitState()->setTexture(loaded);
                pass->setLightingEnabled(false);
            }
        }
    }
}

std::tuple<float, float, float> EntityModel::ScriptGetBonePosition(const int bone){
    Ogre::SkeletonInstance* skeleton = model_->getSkeleton();
    const Ogre::String name = "bone" + std::to_string(bone);
    if (skeleton == nullptr || !skeleton->hasBone(name)) return std::make_tuple(0.0f, 0.0f, 0.0f);
    // Bring the skeleton to the current frame of the animation.
    model_->_updateAnimation();
    const Ogre::Vector3 position
      = model_->_getParentNodeFullTransform() * skeleton->getBone(name)->_getDerivedPosition();
    return std::make_tuple(position.x, position.y, position.z);
}

std::tuple<float, float, float> EntityModel::ScriptGetBoneEnd(const int bone, const float length){
    Ogre::SkeletonInstance* skeleton = model_->getSkeleton();
    const Ogre::String name = "bone" + std::to_string(bone);
    if (skeleton == nullptr || !skeleton->hasBone(name)) return std::make_tuple(0.0f, 0.0f, 0.0f);
    // Bring the skeleton to the current frame of the animation.
    model_->_updateAnimation();
    Ogre::Bone* node = skeleton->getBone(name);
    // The bones go along their Y axis, where their children are placed.
    const Ogre::Vector3 end = node->_getDerivedPosition()
      + node->_getDerivedOrientation() * (node->_getDerivedScale() * Ogre::Vector3(0, length, 0));
    const Ogre::Vector3 position = model_->_getParentNodeFullTransform() * end;
    return std::make_tuple(position.x, position.y, position.z);
}

bool EntityModel::ScriptAttachModel(
  const char* model, const int bone, const float rx, const float ry, const float rz
){
    Ogre::SceneManager* scene_manager = Ogre::Root::getSingleton().getSceneManager("Scene");
    if (attached_model_ != nullptr){
        model_->detachObjectFromBone(attached_model_);
        scene_manager->destroyEntity(attached_model_);
        attached_model_ = nullptr;
    }
    const Ogre::String file(model);
    if (file.empty()) return true;
    Ogre::SkeletonInstance* skeleton = model_->getSkeleton();
    if (skeleton == nullptr){
        LOG_WARNING("The model '" + name_ + "' has no skeleton to attach '" + file + "' to.");
        return false;
    }
    // Bones are called bone0, bone1... and the skeleton also has a root bone.
    const int bone_number = (bone >= 0) ? bone : static_cast<int>(skeleton->getNumBones()) - 2;
    const Ogre::String bone_name = "bone" + std::to_string(bone_number);
    if (!skeleton->hasBone(bone_name)){
        LOG_WARNING("The model '" + name_ + "' has no bone '" + bone_name + "'.");
        return false;
    }
    if (!Ogre::ResourceGroupManager::getSingleton().resourceExists(resource_group_, file)){
        LOG_WARNING("The model '" + file + "' to attach to '" + name_ + "' doesn't exist.");
        return false;
    }
    attached_model_ = scene_manager->createEntity(name_ + "/" + bone_name + "/" + file, file, resource_group_);
    // The vertices of an exported model already include the orientation of the root of its skeleton, the
    // same one the bone of this skeleton has. Undo it, or it would be applied twice.
    Ogre::Quaternion offset = Ogre::Quaternion::IDENTITY;
    Ogre::SkeletonInstance* attached_skeleton = attached_model_->getSkeleton();
    if (attached_skeleton != nullptr && attached_skeleton->hasBone("root")){
        offset = attached_skeleton->getBone("root")->getOrientation().Inverse();
    }
    // Extra rotation around the axes of the model itself (the ones of the original files).
    const Ogre::Quaternion extra
      = Ogre::Quaternion(Ogre::Degree(rx), Ogre::Vector3::UNIT_X)
      * Ogre::Quaternion(Ogre::Degree(ry), Ogre::Vector3::UNIT_Y)
      * Ogre::Quaternion(Ogre::Degree(rz), Ogre::Vector3::UNIT_Z);
    offset = extra * offset;
    model_->attachObjectToBone(bone_name, attached_model_, offset);
    attached_model_->setVisible(model_->isVisible());
    return true;
}

void EntityModel::ScriptSetBlend(const int blend){
    if (blend == blend_) return;
    blend_ = blend;
    // The model held in a bone (a weapon) is drawn like the body.
    std::vector<Ogre::Entity*> entities {model_};
    if (attached_model_ != nullptr) entities.push_back(attached_model_);
    for (Ogre::Entity* entity : entities)
    for (unsigned int s = 0; s < entity->getNumSubEntities(); ++ s){
        Ogre::SubEntity* sub_entity = entity->getSubEntity(s);
        // Materials are shared among all the entities of a mesh, so use one of its own.
        Ogre::MaterialPtr material = OwnMaterial(sub_entity);
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

/**
 * Gets a texture of one white pixel, for the parts that have no texture (they only have the colours of
 * the vertices), so a texture unit can give them an opacity.
 */
static Ogre::TexturePtr GetWhiteTexture(){
    const Ogre::String name = "vgears_white_1x1";
    Ogre::TextureManager& manager = Ogre::TextureManager::getSingleton();
    Ogre::TexturePtr texture = manager.getByName(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    if (texture != nullptr) return texture;
    texture = manager.createManual(
      name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_BYTE_RGBA
    );
    unsigned char pixel[4] = {255, 255, 255, 255};
    Ogre::PixelBox box(Ogre::Box(0, 0, 1, 1), Ogre::PF_BYTE_RGBA, pixel);
    texture->getBuffer()->blitFromMemory(box);
    return texture;
}

void EntityModel::ScriptSetAlpha(const float alpha){
    if (blend_ == BLEND_NORMAL) return;
    const float a = std::max(0.0f, std::min(1.0f, alpha));
    std::vector<Ogre::Entity*> entities {model_};
    if (attached_model_ != nullptr) entities.push_back(attached_model_);
    for (Ogre::Entity* entity : entities)
    for (unsigned int s = 0; s < entity->getNumSubEntities(); ++ s){
        Ogre::MaterialPtr material = entity->getSubEntity(s)->getMaterial();
        for (unsigned short t = 0; t < material->getNumTechniques(); ++ t){
            Ogre::Technique* technique = material->getTechnique(t);
            for (unsigned short p = 0; p < technique->getNumPasses(); ++ p){
                Ogre::Pass* pass = technique->getPass(p);
                // Parts with only vertex colours have no texture unit to change, so give them a white one.
                if (pass->getNumTextureUnitStates() == 0) pass->createTextureUnitState()->setTexture(GetWhiteTexture());
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
        // A pose at the very end of the animation (the dead soldiers of shpin_3, that hold the last
        // frame) is shown as the first frame: past the last key, the interpolation goes on to the
        // first one, as it does when it loops. The time stays a hair before the end.
        float time = (start == -1) ? animation_current_->getLength() : start;
        const float length = animation_current_->getLength();
        if (length > 0.0f && time >= length) time = std::max(0.0f, length - 0.0005f);
        animation_current_->setTimePosition(time);
        animation_end_time_ = (end == -1) ? animation_current_->getLength() : end;
        animation_hold_ = (start != -1 && start == end) ? 1.0f / 30.0f : 0.0f;
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

void EntityModel::StopAnimation(){
    if (animation_current_ != nullptr){
        animation_current_->setEnabled(false);
        animation_current_ = nullptr;
    }
    animation_current_name_.clear();
}

void EntityModel::PlayAnimationPaused(const Ogre::String& animation){
    if (!HasAnimation(animation)) return;
    if (animation_frozen_ && animation_current_name_ == animation) return;
    PlayAnimation(animation, Entity::AUTO_ANIMATION, Entity::PLAY_LOOPED, 0, -1);
    animation_speed_ = 0.0f;
    animation_frozen_ = true;
}

void EntityModel::UnfreezeAnimation(){
    if (!animation_frozen_) return;
    animation_speed_ = 1.0f;
    animation_frozen_ = false;
}

bool EntityModel::HasAnimation(const Ogre::String& animation) const{
    return model_->getAllAnimationStates() != nullptr
      && model_->getAllAnimationStates()->hasAnimationState(animation);
}

void EntityModel::UpdateAnimation(const float delta){
    if (animation_current_ != nullptr){
        if (animation_hold_ > 0.0f){
            animation_hold_ -= delta;
            if (animation_hold_ > 0.0f) return;
        }
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
