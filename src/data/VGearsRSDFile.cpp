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

#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include "data/VGearsRSDFile.h"
#include "common/VGearsStringUtil.h"
#include "data/VGearsRSDFileSerializer.h"

namespace VGears{

    const Ogre::String  RSDFile::RESOURCE_TYPE("VGearsRSDFile");


    RSDFile::RSDFile(
      Ogre::ResourceManager *creator, const Ogre::String &name,
      Ogre::ResourceHandle handle, const Ogre::String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader
    ) : Ogre::Resource(creator, name, handle, group, is_manual, loader)
    {createParamDictionary(RESOURCE_TYPE);}

    RSDFile::~RSDFile(){unload();}

    void RSDFile::loadImpl(){
        RSDFileSerializer serializer;
        Ogre::DataStreamPtr stream(
          Ogre::ResourceGroupManager::getSingleton().openResource(
            mName, mGroup, true, this
          )
        );
        serializer.ImportRSDFile(stream, this);
        Ogre::MaterialManager &material_manager(
          Ogre::MaterialManager::getSingleton()
        );
        const String material_base_name(GetMaterialBaseName());
        Ogre::MaterialPtr material;
        for (size_t i(0); i <= GetTextureNameCount(); ++ i){
            Ogre::String material_name(
              material_base_name + "/" + Ogre::StringConverter::toString(i)
            );
            material = material_manager.getByName(material_name, mGroup);
            if (material == nullptr){
#ifndef NDEBUG
                Ogre::LogManager::getSingleton().stream()
                   << " creating Material for RSDFile"
                   << " '" << material_name << "'";
#endif
                material = material_manager.create(material_name, mGroup);
                material->_notifyOrigin(getName());
                materials_.push_back(material);
                Ogre::Pass *pass(material->getTechnique(0)->getPass(0));
                pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
                if (i > 0) AddTexture(pass, i - 1);
            }
        }
    }

    void RSDFile::unloadImpl(){
        polygon_name_.clear();
        material_name_.clear();
        group_name_.clear();
        texture_names_.clear();
        for (
          MaterialList::iterator it(materials_.begin());
          it != materials_.end();
          ++ it
        ){
            Ogre::MaterialManager::getSingleton().remove((*it)->getHandle());
        }
        materials_.clear();
    }

    size_t RSDFile::calculateSize() const{
        size_t size_texture_names(0);
        for (
          TextureNameList::const_iterator it(texture_names_.begin());
          it != texture_names_.end();
          ++ it
        ){
            size_texture_names += it->size();
        }
        return
          polygon_name_.size() + material_name_.size()
          + group_name_.size() + size_texture_names;
    }

    String RSDFile::GetMaterialBaseName() const{
        String base_name, ext, path;
        StringUtil::splitPath(getName(), path);
        StringUtil::splitBase(material_name_, base_name);
        StringUtil::toLowerCase(base_name);
        return path + base_name;
    }

    void RSDFile::AddTexture(Ogre::Pass *pass, const size_t index) const{
        String texture_name;
        StringUtil::splitPath(getName(), texture_name);
        texture_name += texture_names_[ index ];
        if (StringUtil::endsWith(texture_name, EXT_TIM)){
            StringUtil::toLowerCase(texture_name);
            if (
              !Ogre::ResourceGroupManager::getSingleton().resourceExists(
                mGroup, texture_name
              )
            ){
                texture_name = StringUtil::replaceAll(
                  texture_name, EXT_TIM, EXT_TEX
                );
            }
        }
        if (!texture_name.empty()){
            pass->setAlphaRejectFunction(Ogre::CMPF_GREATER);
            pass->setAlphaRejectValue(0);
            Ogre::TextureUnitState *texture_unit(
              pass->createTextureUnitState()
            );
            texture_unit->setTextureName(texture_name);
            texture_unit->setNumMipmaps(0);
            texture_unit->setTextureFiltering(Ogre::TFO_NONE);
        }
    }

    void RSDFile::SetPolygonName(const String &polygon_name){
        polygon_name_ = polygon_name;
    }

    void RSDFile::SetMaterialName(const String &material_name){
        material_name_ = material_name;
    }

    void RSDFile::SetGroupName(const String &group_name){
        group_name_ = group_name;
    }

    const String& RSDFile::GetPolygonName() const{return polygon_name_;}

    const String&RSDFile::GetMaterialName() const{return material_name_;}

    const String& RSDFile::GetGroupName() const{return group_name_;}

    size_t RSDFile::GetTextureNameCount() const{return texture_names_.size();}

    RSDFile::TextureNameList& RSDFile::GetTextureNames(){return texture_names_;}

}
