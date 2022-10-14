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
#include <OgreTextureManager.h>
#include "data/VGearsFLevelFile.h"
#include "common/FinalFantasy7/FF7NameLookup.h"
#include "common/VGearsStringUtil.h"
#include "data/VGearsAFileManager.h"
#include "data/VGearsFLevelBackground2DLoader.h"
#include "data/VGearsFLevelFileSerializer.h"
#include "data/VGearsFLevelTextureLoader.h"
#include "data/VGearsHRCFileManager.h"
#include "map/VGearsBackground2DFileManager.h"
#include "FF7Common.h"
#include "core/Logger.h"

namespace VGears{

    const String FLevelFile::RESOURCE_TYPE("VGearsFLevelFile");

    const String FLevelFile::SUFFIX_BACKGROUND_TEXTURE("/texture");

    const String FLevelFile::SUFFIX_BACKGROUND_2D("/background_2d");

    FLevelFile::FLevelFile(
      Ogre::ResourceManager *creator, const String &name,
      Ogre::ResourceHandle handle, const String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader
    ) :
      Resource(creator, name, handle, group, is_manual, loader),
      background_texture_loader_(nullptr), background_2d_loader_(nullptr)
    {createParamDictionary(GetResourceType());}

    FLevelFile::~FLevelFile(){
        if (background_texture_loader_){
            assert(background_texture_ != nullptr);
            Ogre::TextureManager::getSingleton().remove(
              background_texture_->getHandle()
            );
            delete background_texture_loader_;
            background_texture_loader_ = nullptr;
        }
        background_texture_.reset();
        if (background_2d_loader_){
            assert(background_2d_ != nullptr);
            Background2DFileManager::getSingleton().remove(
              background_2d_->getHandle()
            );
            delete background_2d_loader_;
            background_2d_loader_ = nullptr;
        }
        background_2d_.reset();
        unload();
    }

    void FLevelFile::loadImpl(){
        FLevelFileSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        if (stream == nullptr) LOG_ERROR("Failed to open resource");
        serializer.ImportFLevelFile(stream, this);
        String background_texture_name(GetBackgroundTextureName());
        if (background_texture_loader_ == nullptr){
            background_texture_loader_ = new FLevelTextureLoader(*this);
            background_texture_ = Ogre::TextureManager::getSingleton().create(
              background_texture_name, mGroup, true, background_texture_loader_
            );
        }
        String background_2d_name(GetBackground2DName());
        if (background_2d_loader_ == nullptr){
            background_2d_loader_ = new FLevelBackground2DLoader(*this);
            background_2d_
              = Background2DFileManager::getSingleton().createResource(
                background_2d_name, mGroup, true,
                background_2d_loader_
              ).staticCast<Background2DFile>();
        }
    }

    void FLevelFile::LoadModels(){
        HRCFileManager &hrc_mgr(HRCFileManager::getSingleton());
        ModelList &models(model_list_->GetModels());
        ModelList::const_iterator it(models.begin()), it_end(models.end());
        while (it != it_end){
            String hrc_name(it->hrc_name);
            Ogre::LogManager::getSingleton().stream() << "Loading Model: "
              << hrc_name;
            StringUtil::toLowerCase(hrc_name);
            HRCFilePtr hrc
              = hrc_mgr.load(hrc_name, mGroup).staticCast<HRCFile>();
            LoadAnimations(hrc, it->animations);
            hrc_files_.push_back(hrc);
            ++ it;
        }
    }

    void FLevelFile::LoadAnimations(
      const HRCFilePtr &model, const AnimationList &animations
    ){
        AFileManager &a_mgr(AFileManager::getSingleton());
        AnimationList::const_iterator
          it(animations.begin()), it_end(animations.end());
        while (it != it_end){
            String animation_name;
            StringUtil::splitBase(it->name, animation_name);
            StringUtil::toLowerCase(animation_name);
            String animation_filename(animation_name + EXT_A);
            AFilePtr animation
              = a_mgr.load(
                animation_filename, model->getGroup()
              ).staticCast<AFile>();
            animation_name = NameLookup::Animation(animation_name);
            Ogre::LogManager::getSingleton().stream() << " Adding Animation: "
              << animation_name;
            animation->AddTo(model->GetSkeleton(), animation_name);
            ++ it;
        }
    }

    void FLevelFile::unloadImpl(){
        background_.reset();
        camera_matrix_.reset();
        palette_.reset();
        model_list_.reset();
        walkmesh_.reset();
        hrc_files_.clear();
        triggers_.reset();
    }

    const String& FLevelFile::GetResourceType(void) const{return RESOURCE_TYPE;}

    size_t FLevelFile::calculateSize() const{
        // Data is only stored in section resources.
        return 0;
    }

    const std::vector<u8>& FLevelFile::GetRawScript() const{return raw_script_;}

    const BackgroundFilePtr& FLevelFile::GetBackground(void) const{
        return background_;
    }

    void FLevelFile::SetRawScript(const std::vector<u8>& script_data){
        raw_script_ = script_data;
    }

    void FLevelFile::SetBackground(const BackgroundFilePtr &background){
        background_ = background;
    }

    const CameraMatrixFilePtr& FLevelFile::GetCameraMatrix(void) const{
        return camera_matrix_;
    }

    void FLevelFile::SetCameraMatrix(const CameraMatrixFilePtr &camera_matrix){
        camera_matrix_ = camera_matrix;
    }

    const PaletteFilePtr& FLevelFile::GetPalette(void) const{return palette_;}

    void FLevelFile::SetPalette(const PaletteFilePtr &palette){
        palette_ = palette;
    }

    const ModelListFilePtr& FLevelFile::GetModelList() const{
        return model_list_;
    }

    void FLevelFile::SetModelList(const ModelListFilePtr &model_list){
        model_list_ = model_list;
    }

    const WalkmeshFilePtr& FLevelFile::GetWalkmesh() const{return walkmesh_;}

    const TriggersFilePtr& FLevelFile::GetTriggers() const{return triggers_;}

    void FLevelFile::SetWalkmesh(const WalkmeshFilePtr &walkmesh){
        walkmesh_ = walkmesh;
    }

    void FLevelFile::SetTriggers(const TriggersFilePtr& triggers){
        triggers_ = triggers;
    }

    String FLevelFile::GetBackgroundTextureName(void) const{
        String base_name;
        StringUtil::splitFull(getName(), base_name);
        base_name += SUFFIX_BACKGROUND_TEXTURE;
        return base_name;
    }

    String FLevelFile::GetBackground2DName(void) const{
        String base_name;
        StringUtil::splitFull(getName(), base_name);
        base_name += SUFFIX_BACKGROUND_2D;
        return base_name;

    }

}
