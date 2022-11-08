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

#include "data/VGearsTriggersFile.h"

/**
 * Triggers file manager singleton.
 */
template<> VGears::TriggersFileManager
  *Ogre::Singleton<VGears::TriggersFileManager>::msSingleton = nullptr;

namespace VGears{

    TriggersFileManager::TriggersFileManager(){
        mResourceType = TriggersFile::RESOURCE_TYPE;
        // Low, because it will likely reference other resources.
        mLoadOrder = 30.0f;
        // This is how the ResourceManager registers with OGRE.
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(
          mResourceType, this
        );
    }

    TriggersFileManager::~TriggersFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(
          mResourceType
        );
    }

    TriggersFileManager& TriggersFileManager::GetSingleton(){
        assert(msSingleton);
        return(*msSingleton);
    }

    /*static*/ TriggersFileManager* TriggersFileManager::GetSingletonPtr(){
        return msSingleton;
    }

    TriggersFile::TriggersFile(
      Ogre::ResourceManager* creator, const String &name,
      Ogre::ResourceHandle handle, const String& group, bool is_manual,
      Ogre::ManualResourceLoader* loader
    ) : Resource(creator, name, handle, group, is_manual, loader){}

    TriggersFile::~TriggersFile(){unload();}
 
    void TriggersFile::loadImpl(void){
        TriggerFileSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        serializer.ImportTriggerFile(stream, this);
    }

    void TriggersFile::unloadImpl(void){}

    size_t TriggersFile::calculateSize(void) const{return sizeof(TriggerData);}

    const String TriggersFile::RESOURCE_TYPE("FF7FieldTriggersSection");

    Ogre::Resource* TriggersFileManager::createImpl(
      const Ogre::String &name, Ogre::ResourceHandle handle,
      const Ogre::String& group, bool is_manual,
      Ogre::ManualResourceLoader* loader,
      const Ogre::NameValuePairList* create_params
    ){return new TriggersFile(this, name, handle, group, is_manual, loader);}

    TriggerFileSerializer::TriggerFileSerializer(){}

    void TriggerFileSerializer::ReadTriggerVertex(
      Ogre::DataStreamPtr& stream, TriggersFile::TriggerVertex& vertex
    ){
        ReadInt16(stream, vertex.x);
        ReadInt16(stream, vertex.y);
        ReadInt16(stream, vertex.z);
    }

    void TriggerFileSerializer::ReadRange(
      Ogre::DataStreamPtr& stream, TriggersFile::Range& range
    ){
        ReadInt16(stream, range.left);
        ReadInt16(stream, range.top);
        ReadInt16(stream, range.right);
        ReadInt16(stream, range.bottom);
    }

    void TriggerFileSerializer::ReadGateway(
      Ogre::DataStreamPtr& stream, TriggersFile::Gateway& exit
    ){
        for (size_t i = 0; i < exit.exit_line.size(); i ++)
            ReadTriggerVertex(stream, exit.exit_line[i]);
        ReadTriggerVertex(stream, exit.destination);
        ReadShort(stream, exit.destination_field_id);
        ReadUInt8(stream, exit.dir);
        ReadUInt8(stream, exit.dir_copy1);
        ReadUInt8(stream, exit.dir_copy2);
        ReadUInt8(stream, exit.dir_copy3);
    }

    void TriggerFileSerializer::ReadArrow(
      Ogre::DataStreamPtr& stream, TriggersFile::Arrow& arrow
    ){
        ReadSInt32(stream, arrow.x);
        ReadSInt32(stream, arrow.z);
        ReadSInt32(stream, arrow.y);
        ReadUInt32(stream, arrow.type);
    }

    void TriggerFileSerializer::ReadTrigger(
      Ogre::DataStreamPtr& stream, TriggersFile::Trigger& trigger
    ){
        for (size_t i = 0; i < trigger.trigger_line.size(); i ++)
            ReadTriggerVertex(stream, trigger.trigger_line[i]);
        ReadUInt8(stream, trigger.background_parameter);
        ReadUInt8(stream, trigger.background_state);
        ReadUInt8(stream, trigger.behavior);
        ReadUInt8(stream, trigger.sound_id);
    }

    void TriggerFileSerializer::ImportTriggerFile(
      Ogre::DataStreamPtr &stream, TriggersFile *dest
    ){
        std::unique_ptr<TriggersFile::TriggerData> data
          = std::unique_ptr<TriggersFile::TriggerData>(
            new TriggersFile::TriggerData()
          );
        ReadChars(stream, data->name.data(), data->name.size());
        ReadUInt8(stream, data->control);
        ReadInt16(stream, data->camera_focus_height);
        ReadRange(stream, data->camera_range);
        ReadUInt8(stream, data->bg_layer1_flag);
        ReadUInt8(stream, data->bg_layer2_flag);
        ReadUInt8(stream, data->bg_layer3_flag);
        ReadUInt8(stream, data->bg_layer4_flag);

        ReadInt16(stream, data->bg_layer3_width);
        ReadInt16(stream, data->bg_layer3_height);
        ReadInt16(stream, data->bg_layer4_width);
        ReadInt16(stream, data->bg_layer4_height);
        stream->read(data->unknown.data(), data->unknown.size());
        for (size_t i = 0; i < data->doors.size(); i ++){
            TriggersFile::Gateway& exit = data->doors[i];
            ReadGateway(stream, exit);
        }
        
        for (size_t i = 0; i < data->triggers.size(); i ++){
            TriggersFile::Trigger& trigger = data->triggers[i];
            ReadTrigger(stream, trigger);
        }
        stream->read(data->display_arrow.data(), data->display_arrow.size());
        for (size_t i = 0; i < data->arrows.size(); i ++){
            TriggersFile::Arrow& arrow = data->arrows[i];
            ReadArrow(stream, arrow);
        }
        dest->trigger_data_ = std::move(data);
    }

}
