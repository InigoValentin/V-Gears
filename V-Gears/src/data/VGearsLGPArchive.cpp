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
#include <string>
//#include <OgreThreadDefines.h>
#include "data/VGearsLGPArchive.h"
#include "common/VGearsStringUtil.h"
#include "data/VGearsLGPArchiveSerializer.h"
#include "core/Logger.h"

namespace VGears{

    LGPArchive::LGPArchive(const String &name, const String &arch_type) :
      Ogre::Archive(name, arch_type){}

    LGPArchive::~LGPArchive(){unload();}

    void LGPArchive::load(){
        //OGRE_LOCK_AUTO_MUTEX
        std::ifstream *ifs(
          OGRE_NEW_T(std::ifstream,Ogre::MEMCATEGORY_GENERAL)
            (mName.c_str(), std::ifstream::binary)
        );
        Load(OGRE_NEW Ogre::FileStreamDataStream(ifs));
    }

    void LGPArchive::Load(Ogre::DataStream* lgp){
        lgp_file_.reset();
        lgp_file_.bind(lgp);
        LGPArchiveSerializer lgp_archive_serializer;
        lgp_archive_serializer.ImportLGPArchive(lgp_file_, this);
        FileList::const_iterator it(files_.begin()), it_end(files_.end());
        while (it != it_end){
            Ogre::FileInfo file_info;
            file_info.archive = this;
            file_info.filename = it->file_name;
            StringUtil::splitFilename(
              it->file_name, file_info.basename, file_info.path
            );
            // TODO: This is actually still the compressed size!!
            file_info.uncompressedSize = it->data_size;
            file_info.compressedSize = file_info.uncompressedSize;
            //LOG_DEBUG("add file:" + file_info.filename);
            file_infos_.push_back(file_info);
            ++ it;
        }
        // Writing not implemented
        mReadOnly = true;
    }

    void LGPArchive::unload(){
        //OGRE_LOCK_AUTO_MUTEX
        files_.clear();
        file_infos_.clear();
        lgp_file_.reset();
    }

    Ogre::DataStreamPtr LGPArchive::open(
      const String& filename, bool readOnly
    ) const{
        //LOG_DEBUG(
        //  "file:" + filename + " readOnly: "
        //  + Ogre::StringConverter::toString(readOnly)
        //);
        Ogre::MemoryDataStream* buffer(nullptr);
        if (lgp_file_ != nullptr){
            FileList::const_iterator it(files_.begin());
            FileList::const_iterator it_end(files_.end());
            while(it != it_end){
                if (it->file_name == filename){
                    lgp_file_->seek(it->data_offset);
                    size_t length(it->data_size);
                    buffer = new Ogre::MemoryDataStream(length);
                    lgp_file_->read(buffer->getPtr(), length);
                    break;
                }
                ++ it;
            }
        }
        return Ogre::DataStreamPtr(buffer);
    }

    Ogre::DataStreamPtr LGPArchive::create(const String& filename) const{
        OGRE_EXCEPT(
          Ogre::Exception::ERR_NOT_IMPLEMENTED,
          "Modification of lgp archives is not supported",
          "LGPArchive::create"
        );
    }

    void LGPArchive::remove(const String& filename) const{
        OGRE_EXCEPT(
          Ogre::Exception::ERR_NOT_IMPLEMENTED,
          "Modification of lgp archives is not supported",
          "LGPArchive::remove"
        );
    }

    Ogre::StringVectorPtr LGPArchive::list(bool recursive, bool dirs) const{
        //OGRE_LOCK_AUTO_MUTEX
        Ogre::StringVector* file_names(
          OGRE_NEW_T(Ogre::StringVector, Ogre::MEMCATEGORY_GENERAL)()
        );
        FileList::const_iterator it(files_.begin());
        FileList::const_iterator it_end(files_.end());
        while (it != it_end){
            file_names->push_back(it->file_name);
            ++ it;
        }
        return Ogre::StringVectorPtr(file_names, Ogre::SPFM_DELETE_T);
    }

    Ogre::FileInfoListPtr LGPArchive::listFileInfo(
      bool recursive, bool dirs
    ) const{
        //OGRE_LOCK_AUTO_MUTEX
        return Ogre::FileInfoListPtr(
          OGRE_NEW_T(Ogre::FileInfoList, Ogre::MEMCATEGORY_GENERAL)
            (file_infos_),
          Ogre::SPFM_DELETE_T
        );
    }

    Ogre::StringVectorPtr LGPArchive::find(
      const String& pattern, bool recursive, bool dirs
    ) const{
        LOG_DEBUG(pattern);
        //OGRE_LOCK_AUTO_MUTEX
        Ogre::StringVector* file_names(
          OGRE_NEW_T(Ogre::StringVector, Ogre::MEMCATEGORY_GENERAL)()
        );
        Ogre::FileInfoListPtr found_infos(
          findFileInfo(pattern, recursive, dirs)
        );
        Ogre::FileInfoList::const_iterator
          it(found_infos->begin()), it_end(found_infos->end());
        while (it != it_end){
            file_names->push_back(it->filename);
            ++ it;
        }
        return Ogre::StringVectorPtr(file_names, Ogre::SPFM_DELETE_T);
    }

    Ogre::FileInfoListPtr LGPArchive::findFileInfo(
      const String& pattern, bool recursive, bool dirs
    ) const{
        LOG_DEBUG(pattern);
        //OGRE_LOCK_AUTO_MUTEX
        Ogre::FileInfoList* file_infos(
          OGRE_NEW_T(Ogre::FileInfoList, Ogre::MEMCATEGORY_GENERAL)()
        );
        // If pattern contains a directory name, do a full match.
        bool full_match
          = (pattern.find ('/') != String::npos)
            || (pattern.find ('\\') != String::npos);

        Ogre::FileInfoList::const_iterator
          it(file_infos_.begin()), it_end(file_infos_.end());
        while (it != it_end){
            const String& file_name(full_match ? it->filename : it->basename);
            if (StringUtil::match(file_name, pattern, isCaseSensitive()))
                file_infos->push_back(*it);
            ++ it;
        }
        return Ogre::FileInfoListPtr(file_infos, Ogre::SPFM_DELETE_T);
    }

    bool LGPArchive::exists(const String& filename) const{
        FileList::const_iterator it(files_.begin());
        FileList::const_iterator it_end(files_.end());
        while (it != it_end){
            if (it->file_name == filename){
                LOG_DEBUG("Found " + filename + " in " + mName);
                return true;
            }
            ++ it;
        }
        LOG_DEBUG("Couldn't find " + filename + " in " + mName);
        return false;
    }

    time_t LGPArchive::getModifiedTime(const String& filename) const{
        struct stat tagStat;
        bool ret = (stat(mName.c_str(), &tagStat) == 0);
        if (ret) return tagStat.st_mtime;
        else return 0;
    }

    LGPArchive::FileList& LGPArchive::GetFiles(void){return files_;}

}
