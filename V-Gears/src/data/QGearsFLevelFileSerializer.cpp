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
#include <OgreException.h>
#include "data/QGearsFLevelFileSerializer.h"
#include "FF7Common.h"
#include "common/QGearsStringUtil.h"
#include "data/QGearsBackgroundFileManager.h"
#include "data/QGearsBackgroundFileSerializer.h"
#include "data/QGearsCameraMatrixFileManager.h"
#include "data/QGearsCameraMatrixFileSerializer.h"
#include "data/FinalFantasy7/FF7ModelListFileManager.h"
#include "data/FinalFantasy7/FF7ModelListFileSerializer.h"
#include "data/QGearsPaletteFileManager.h"
#include "data/QGearsPaletteFileSerializer.h"
#include "map/QGearsWalkmeshFileManager.h"
#include "map/FinalFantasy7/FF7WalkmeshFileSerializer.h"
#include "data/QGearsTriggersFile.h"

namespace QGears{
    // TODO: move flevel stuff to ff7 as it is ff7 related

    using FF7::ModelListFileManager;

    using FF7::ModelListFileSerializer;

    using FF7::WalkmeshFileSerializer;

    const String FLevelFileSerializer::TAG_FILE_END("FINAL FANTASY7");

    FLevelFileSerializer::FLevelFileSerializer() : Serializer(){}

    FLevelFileSerializer::~FLevelFileSerializer(){}

    void FLevelFileSerializer::ImportFLevelFile(
      Ogre::DataStreamPtr &stream, FLevelFile* dest
    ){
        // Read header.
        const size_t start_position(stream->tell());
        readFileHeader(stream);
        // Read section offsets.
        std::vector<uint32> section_offsetsVec(header_.section_count);
        std::vector<uint32> section_sizesVec(header_.section_count);
        readInts(stream, section_offsetsVec.data(), header_.section_count);
        // Now calculate section sizes
        size_t fileSize = stream->size();
        for (size_t i = 0; i < section_offsetsVec.size(); i ++){
            if (i+1 == section_offsetsVec.size()){
                // The file size is needed to calculate this, since it's not
                // available, just read till end of stream on the final
                // section.
                section_sizesVec[i] = 0;
            }
            else{
                section_sizesVec[i]
                  = section_offsetsVec[i + 1] - section_offsetsVec[i] - 4;
            }
        }
        Ogre::DataStreamPtr section;
        for (size_t i(0); i < header_.section_count; ++ i){
            const size_t currentoffset_(stream->tell() - start_position);
            const size_t section_gap(section_offsetsVec[i] - currentoffset_);
            if (currentoffset_ > section_offsetsVec[i]){
                OGRE_EXCEPT(
                  Ogre::Exception::ERR_INVALIDPARAMS,
                  "FLevel sections overlap"
                  "FLevelFileSerializer::importFLevelFile"
               );
            }
            else if (section_gap){
                stream->skip(section_gap);
                Ogre::LogManager::getSingleton().stream()
                  << "Warning: skiping gap in front of section " << i
                  << " gap size " << section_gap
                  << " FLevelFileSerializer::importFLevelFile";
            }
            ReadSectionData(stream, section, section_sizesVec[i]);
            ReadSection(section, dest, i);
            section->close();
            section.reset();
        }
        ReadEnd(stream);
    }

    void FLevelFileSerializer::ReadFileHeader(Ogre::DataStreamPtr &stream){
        // Read version, check if its zero.
        ReadShort(stream, header_.version);
        if (header_.version != 0){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "FLevel has unsupported version",
              "FLevelFileSerializer::importFLevelFile"
            );
        }

        // Read number of sections, check if its 9.
        ReadUInt32(  stream, header_.section_count);
        if (header_.section_count != SECTION_COUNT){
            Ogre::LogManager::getSingleton().stream()
              << "Warning: FLevel section_count has unexpected value "
              << header_.section_count << " should be " << SECTION_COUNT
              << " FLevelFileSerializer::importFLevelFile";
        }
    }

    void FLevelFileSerializer::ReadSectionData(
      Ogre::DataStreamPtr &stream, Ogre::DataStreamPtr &out_buffer
      , size_t section_size
    ){
        // This can be wrong, it gets read so it's at the correct stream
        // position but use the manually calculated section size instead.
        uint32 length(0);
        ReadUInt32(stream, length);
        // TODO: implement SubDataStream class to restrict access size etc
        // so there is no need to copy the whole memory.
        if (section_size == 0){
            // TODO FIX ME - read to EOS
            // Hope this is actually correct :)
            section_size = length;
        }
        Ogre::MemoryDataStream *buffer(
          new Ogre::MemoryDataStream(section_size)
        );
        stream->read(buffer->getPtr(), section_size);
        out_buffer = Ogre::DataStreamPtr(buffer);
    }

    void FLevelFileSerializer::ReadSection(
      Ogre::DataStreamPtr &stream, FLevelFile *dest,
      const size_t section_index
    ){
        switch (section_index){
            case SECTION_SCRIPT:
                {
                    std::vector<u8> buffer(stream->size());
                    stream->read(buffer.data(), buffer.size());
                    dest->SetRawScript(buffer);
                }
                break;
            case SECTION_CAMERA_MATRIX:
                ReadCameraMatrix(stream, dest);
                break;
            case SECTION_MODEL_LOADER:
                ReadModelList(stream, dest);
                break;
            case SECTION_PALETTE:
                ReadPalette(stream, dest);
                break;
            case SECTION_WALKMESH:
                ReadWalkmesh(stream, dest);
                break;
            case SECTION_BACKGROUND:
                ReadBackground(stream, dest);
                break;
            case SECTION_TILE_MAP:
                // TODO
                break;
            case SECTION_ENCOUNTER:
                // TODO
                break;
            case SECTION_TRIGGER:
                ReadTriggers(stream, dest);
                break;
        }
    }

    template<typename ResourceManagerType> Ogre::ResourcePtr
      FLevelFileSerializer::CreateResource(
        FLevelFile *dest, const String &extension
      ){
        assert(dest);
        Ogre::ResourceManager &mgr(ResourceManagerType::getSingleton());
        String name(GetBaseName(dest) + extension);
        Ogre::ResourcePtr resource =  mgr.createResource(
          name, dest->getGroup(), true
        );
        resource->_notifyOrigin(dest->getName());
        return resource;
    }

    void FLevelFileSerializer::ReadCameraMatrix(
      Ogre::DataStreamPtr &stream, FLevelFile *dest
    ){
        CameraMatrixFilePtr camera_matrix
          = CreateResource<CameraMatrixFileManager>(dest, EXT_CAMERA_MATRIX)
            .staticCast<CameraMatrixFile>();
        CameraMatrixFileSerializer ser;
        ser.ImportCameraMatrixFile(stream, camera_matrix.getPointer());
        dest->SetCameraMatrix(camera_matrix);
    }

    void FLevelFileSerializer::ReadModelList(
      Ogre::DataStreamPtr &stream, FLevelFile *dest
    ){
        ModelListFilePtr model_list
          = CreateResource<ModelListFileManager>(dest, EXT_MODEL_LIST)
            .staticCast<ModelListFile>();
        ModelListFileSerializer ser;
        ser.ImportModelListFile(stream, model_list.getPointer());
        dest->SetModelList(model_list);
    }

    void FLevelFileSerializer::ReadPalette(
      Ogre::DataStreamPtr &stream, FLevelFile *dest
    ){
        PaletteFilePtr palette
          = CreateResource<PaletteFileManager>(dest, EXT_PALETTE)
            .staticCast<PaletteFile>();
        PaletteFileSerializer ser;
        ser.ImportPaletteFile(stream, palette.getPointer());
        dest->SetPalette(palette);
    }

    void FLevelFileSerializer::ReadWalkmesh(
      Ogre::DataStreamPtr &stream, FLevelFile *dest
    ){
        WalkmeshFilePtr walkmesh
          = CreateResource<WalkmeshFileManager>(dest, EXT_WALKMESH)
            .staticCast<WalkmeshFile>();
        WalkmeshFileSerializer ser;
        ser.ImportWalkmeshFile(stream, walkmesh.getPointer());
        WalkmeshFile::TriangleList& triangles(walkmesh->GetTriangles());
        WalkmeshFile::TriangleList::iterator it(triangles.begin());
        WalkmeshFile::TriangleList::const_iterator it_end(triangles.end());
        Ogre::Real scale
          = static_cast<Ogre::Real>(dest->GetCameraMatrix()->GetCount());
        scale *= FF7::FIELD_POSITION_SCALE;
        while (it != it_end){
            it->a /= scale;
            it->b /= scale;
            it->c /= scale;
            ++ it;
        }
        dest->SetWalkmesh(walkmesh);
    }

    void FLevelFileSerializer::ReadBackground(
      Ogre::DataStreamPtr &stream, FLevelFile *dest
    ){
        BackgroundFilePtr background
          = CreateResource<BackgroundFileManager>(dest, EXT_BACKGROUND)
            .staticCast<BackgroundFile>();
        BackgroundFileSerializer ser;
        ser.ImportBackgroundFile(stream, background.getPointer());
        dest->SetBackground(background);
    }

    void FLevelFileSerializer::ReadTriggers(
      Ogre::DataStreamPtr &stream, FLevelFile* dest
    ){
        TriggersFilePtr triggers
          = CreateResource<TriggersFileManager>(dest, EXT_TRIGGERS)
            .staticCast<TriggersFile>();
        TriggerFileSerializer ser;
        ser.ImportTriggerFile(stream, triggers.getPointer());
        dest->SetTriggers(triggers);
    }

    String FLevelFileSerializer::GetBaseName(const FLevelFile *dest) const{
        String base_name;
        StringUtil::splitBase(dest->getName(), base_name);
        return base_name;
    }

    void FLevelFileSerializer::ReadEnd(Ogre::DataStreamPtr &stream){
        ReadEndString(stream, TAG_FILE_END);
    }

    template<typename ValueType> void FLevelFileSerializer::ReadVector(
      Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest, size_t count
    ){
        dest.clear();
        dest.reserve(count);
        for (size_t i(count); i --;){
            ValueType in_tmp;
            readObject(stream, in_tmp);
            dest.push_back(in_tmp);
        }
    }

}
