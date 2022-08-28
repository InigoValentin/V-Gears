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

#pragma once

#include "common/TypeDefine.h"
#include "QGearsPFile.h"
#include "QGearsSerializer.h"

namespace QGears{

    /**
     * Handles the serialization of P files.
     */
    class PFileSerializer : public Serializer{

        public:

            /**
             * Constructor.
             */
            PFileSerializer();

            /**
             * Destructor.
             */
            virtual ~PFileSerializer();

            /**
             * Imports a P file.
             *
             * @param stream[in] The contents of the P file.
             * @param dest[out] The formed P file.
             */
            virtual void ImportPFile(Ogre::DataStreamPtr &stream, PFile* dest);

            /**
             * The header of a P file.
             */
            struct Header{

                /**
                 * The file format version.
                 */
                uint32 version;

                /**
                 * Unknown data.
                 */
                uint32 unknown_04;

                /**
                 * Type of vertices.
                 */
                uint32 vertex_type;

                /**
                 * The number of vertices.
                 */
                uint32 num_vertices;

                /**
                 * The number of normals.
                 */
                uint32 num_normals;

                /**
                 * Number of unknown data blocks.
                 */
                uint32 num_unknown1;

                /**
                 * Number of texture coordinates.
                 */
                uint32 num_texture_coordinates;

                /**
                 * Number of vertex colours.
                 */
                uint32 num_vertex_colors;

                /**
                 * Number of edges.
                 */
                uint32 num_edges;

                /**
                 * Number of polygons.
                 */
                uint32 num_polygons;

                /**
                 * Number of unknown data blocks.
                 */
                uint32 num_unknown2;

                /**
                 * Number of unknown data blocks.
                 */
                uint32 num_unknown3;

                /**
                 * Number of materials.
                 */
                uint32 num_materials;

                /**
                 * Number of groups.
                 */
                uint32 num_groups;

                /**
                 * Number of bounding boxes.
                 */
                uint32 num_bboxes;

                /**
                 * @todo Understand and document.
                 */
                uint32 norm_index_table_flag;

                /**
                 * @todo Understand and document.
                 */
                uint32 runtime_data[0x10];
            };

            typedef PFile::BBoxEntry BBoxEntry;

            typedef PFile::Edge Edge;

            typedef PFile::Group Group;

            typedef PFile::MaterialInformation MaterialInformation;

            typedef PFile::PolygonDefinition PolygonDefinition;

            typedef PFile::Colour Colour;

        protected:

            /**
             * Reads a file header and sets the instance data.
             *
             * @param stream[in] The contents of the P file.
             */
            virtual void ReadFileHeader(Ogre::DataStreamPtr &stream);

            /**
             * Reads an object as a colour.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed colour data.
             */
            virtual void readObject(Ogre::DataStreamPtr &stream, Colour &dest);

            /**
             * Reads an object as an edge.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed edge data.
             */
            virtual void readObject(Ogre::DataStreamPtr &stream, Edge &dest);

            /**
             * Reads an object as a polygon definition.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed poligon definition data.
             */
            virtual void readObject(
              Ogre::DataStreamPtr &stream, PolygonDefinition &dest
            );

            /**
             * Reads an object as a group.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed group data.
             */
            virtual void readObject(Ogre::DataStreamPtr &stream, Group &dest);

            /**
             * Reads an object as a bounding box.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed bounding box data.
             */
            virtual void readObject(
              Ogre::DataStreamPtr &stream, BBoxEntry &dest
            );

            using Serializer::readObject;

            /**
             * Reads a stream as a vector.
             *
             * @param stream[in] The input stream.
             * @param dest[out] The vector data will be loaded here.
             * @param count[in] Data units to copy.
             */
            template<typename ValueType> void ReadVector(
              Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest,
              size_t count
            );

        private:

            /**
             * The file header.
             */
            Header header_;
    };
}
