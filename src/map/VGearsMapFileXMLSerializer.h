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
             * @param[in] stream The contents of the map file.
             * @param[out] dest The formed map file.
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
             * @param[in] node The XML node to read.
             */
            virtual void ReadHeader(TiXmlNode *node);

            /**
             * Reads the script file location from the field XML file.
             *
             * @param[in] node The XML node to read.
             * @param[out] dest The script data will be loaded here.
             */
            virtual void ReadScript(TiXmlNode &node, MapFile *dest);

            /**
             * Reads the backgrouns file location from the field XML file.
             *
             * @param[in] node The XML node to read.
             * @param[out] dest The background data will be loaded here.
             */
            virtual void ReadBackground2D(TiXmlNode &node, MapFile *dest);

            /**
             * Reads the text file location from the field XML file.
             *
             * @param[in] node The XML node to read.
             * @param[out] dest The background data will be loaded here.
             */
            virtual void ReadTexts(TiXmlNode &node, MapFile *dest);

            /**
             * Reads the walkmesh file location from the field XML file.
             *
             * @param[in] node The XML node to read.
             * @param[out] dest The walkmesh data will be loaded here.
             */
            virtual void ReadWalkmesh(TiXmlNode &node, MapFile *dest);

            /**
             * Reads the field forward direction location from the field XML file.
             *
             * @param[in] node The XML node to read.
             * @param[out] dest The direction data will be loaded here.
             */
            virtual void ReadForwardDirection(TiXmlNode &node, MapFile *dest);

            /**
             * Reads the entities location from the field XML file.
             *
             * @param[in] node The XML node to read.
             * @param[out] dest The entity data will be loaded here.
             */
            virtual void ReadEntities(TiXmlNode &node, MapFile *dest);

            /**
             * Reads an object from the field XML file.
             *
             * @param[in] node The XML node to read.
             * @param[out] dest The formed point data.
             */
            virtual void readObject(TiXmlNode &node, Point &dest);

            /**
             * Reads a trigger from the field XML file.
             *
             * @param[in] node The XML node to read.
             * @param[out] dest The formed trigger data.
             */
            virtual void readObject(TiXmlNode &node, Trigger &dest);

            /**
             * Reads entities from a map.
             *
             * @param[in] node Input data.
             * @param[in] tag Entity section tag.
             * @param[out] dest The formed vector data.
             */
            template<typename ValueType> void ReadEntities(
              TiXmlNode &node, const String &tag, std::vector<ValueType> &dest
            );

    };
}

