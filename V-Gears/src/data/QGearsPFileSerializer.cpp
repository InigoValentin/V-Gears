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

#include "data/QGearsPFileSerializer.h"
#include <OgreLogManager.h>

namespace QGears{

    PFileSerializer::PFileSerializer() : Serializer(){}

    PFileSerializer::~PFileSerializer(){}

    template<typename ValueType> void PFileSerializer::ReadVector(
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

    void PFileSerializer::ReadFileHeader(Ogre::DataStreamPtr &stream){
        size_t header_size(sizeof(header_));
        stream->read(&header_, header_size);
        flipFromLittleEndian(&header_, 4, header_size / 4);
    }

    void PFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, Edge &dest
    ){
        readShorts(stream, dest.index, 2);
        Ogre::Log::Stream log(Ogre::LogManager::getSingleton().stream());
    }

    void PFileSerializer::readObject(Ogre::DataStreamPtr &stream, Colour &dest){
        uint32 colour;
        ReadUInt32(stream, colour);
        dest.setAsARGB(colour);
    }

    void PFileSerializer::readObject(
      Ogre::DataStreamPtr &stream , PolygonDefinition &dest
    ){
        size_t definition_size(sizeof(dest));
        stream->read(&dest, definition_size);
        flipFromLittleEndian(&dest, 2, definition_size / 2);
    }

    void PFileSerializer::readObject(Ogre::DataStreamPtr &stream, Group &dest){
        size_t definition_size(sizeof(dest));
        stream->read(&dest, definition_size);
        flipFromLittleEndian(&dest, 4, definition_size / 4);
    }

    void PFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, BBoxEntry &dest
    ){
        readInts(stream, &(dest.unknown), 1);
        readObject(stream, dest.bbox);
    }

    void PFileSerializer::ImportPFile(Ogre::DataStreamPtr &stream, PFile* dest){
        readFileHeader(stream);
        ReadVector(stream, dest->GetVertices(), header_.num_vertices);
        ReadVector(stream, dest->GetNormals(), header_.num_normals);
        ReadVector(stream, dest->GetUnknown1(), header_.num_unknown1);
        ReadVector(
          stream, dest->GetTextureCoordinates(), header_.num_texture_coordinates
        );
        ReadVector(stream, dest->GetVertexColors(), header_.num_vertex_colors);
        ReadVector(stream, dest->GetPolygonColors(), header_.num_polygons);
        ReadVector(stream, dest->GetEdges(), header_.num_edges);
        ReadVector(stream, dest->GetPolygonDefinitions() ,header_.num_polygons);
        stream->skip(header_.num_materials * sizeof(MaterialInformation));
        ReadVector(stream, dest->GetGroups(), header_.num_groups);
        ReadVector(stream, dest->GetBBoxes(), header_.num_bboxes);
    }

}
