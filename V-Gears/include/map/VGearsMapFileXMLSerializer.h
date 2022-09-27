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
#include "data/VGearsXMLSerializer.h"
#include "VGearsMapFile.h"

namespace VGears{

    /**
     * Handles the serialization of map XML files.
     */
    class MapFileXMLSerializer : public XMLSerializer{
        
        public:
        
            /**
             * Constructor.
             */
            MapFileXMLSerializer();

            /**
             * Destructor.
             */
            virtual ~MapFileXMLSerializer();

            /**
             * Imports a map file.
             *
             * @param stream[in] The contents of the map file.
             * @param dest[out] The formed map file.
             */
            virtual void ImportMapFile(Ogre::DataStreamPtr &stream, MapFile *dest);

            typedef MapFile::Point Point;

            typedef MapFile::PointList PointList;

            typedef MapFile::Trigger Trigger;

            typedef MapFile::TriggerList TriggerList;

        protected:

            /**
             * Reads a file header from an XML node and sets the instance data.
             *
             * @param node[in] The XML node to read.
             */
            virtual void ReadHeader(TiXmlNode *node);

            /**
             * Reads a XML node as a script.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The script data will be loaded here.
             */
            virtual void ReadScript(TiXmlNode &node, MapFile *dest);

            /**
             * Reads a XML node as a 2D background.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The background data will be loaded here.
             */
            virtual void ReadBackground2D(TiXmlNode &node, MapFile *dest);

            /**
             * Reads a XML node as a walkmesh.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The walkmesh data will be loaded here.
             */
            virtual void ReadWalkmesh(TiXmlNode &node, MapFile *dest);

            /**
             * Reads a XML node as a forward direction.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The direction data will be loaded here.
             */
            virtual void ReadForwardDirection(TiXmlNode &node, MapFile *dest);

            /**
             * Reads a XML node as a list of entities.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The entity data will be loaded here.
             */
            virtual void ReadEntities(TiXmlNode &node, MapFile *dest);

            /**
             * Reads an XML node as an entity point.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The formed point data.
             */
            virtual void readObject(TiXmlNode &node, Point &dest);

            /**
             * Reads an XML node as a trigger.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The formed trigger data.
             */
            virtual void readObject(TiXmlNode &node, Trigger &dest);

            /**
             * Reads an object as a vector.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed vector data.
             * @param count[in] The size of the data to read.
             */
            template<typename ValueType> void ReadEntities(
              TiXmlNode &node, const String &tag, std::vector<ValueType> &dest
            );

    };
}

