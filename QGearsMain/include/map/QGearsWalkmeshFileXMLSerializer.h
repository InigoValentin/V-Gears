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
#include "data/QGearsXMLSerializer.h"
#include "QGearsWalkmeshFile.h"

namespace QGears{

    /**
     * Handles the serialization of walkmesh files.
     */
    class WalkmeshFileXMLSerializer : public XMLSerializer{

        public:
        
            /**
             * Constructor.
             */
            WalkmeshFileXMLSerializer();

            /**
             * Destructor.
             */
            virtual ~WalkmeshFileXMLSerializer();

            /**
             * Imports a walkmesh file.
             *
             * @param stream[in] The contents of the map file.
             * @param dest[out] The formed walkmesh file.
             */
            virtual void ImportWalkmeshFile(
              Ogre::DataStreamPtr &stream, WalkmeshFile *dest
            );

            typedef WalkmeshFile::Triangle Triangle;

            typedef WalkmeshFile::TriangleList TriangleList;

        protected:

            /**
             * Reads a file header from an XML node and sets the instance data.
             *
             * @param node[in] The XML node to read.
             */
            virtual void ReadHeader(TiXmlNode *node);

            /**
             * Reads an XML node as a triangle.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The formed triangle data.
             */
            virtual void readObject(TiXmlNode &node, Triangle &dest);

            /**
             * Reads a XML node as a vector.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The vector data will be loaded here.
             * @param tag[in] XML tag to read.
             */
            template<typename ValueType> void ReadVector(
              TiXmlNode &node, std::vector<ValueType> &dest, const String &tag
            ){
                dest.clear();
                TiXmlNode* child(node.FirstChild());
                while (child != NULL){
                    if (
                      child->Type() == TiXmlNode::TINYXML_ELEMENT
                      && child->ValueStr() == tag
                    ){
                        ValueType in_tmp;
                        readObject(*child, in_tmp);
                        dest.push_back(in_tmp);
                    }
                    child = child->NextSibling();
                }
            }

    };
}
