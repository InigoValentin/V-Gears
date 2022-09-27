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
#include "VGearsBackground2DFile.h"

namespace VGears{

    /**
     * Handles the serialization of 2D background XML files.
     */
    class Background2DFileXMLSerializer : public XMLSerializer{

        public:

            /**
             * Constructor.
             */
            Background2DFileXMLSerializer();

            /**
             * Destructor.
             */
            virtual  ~Background2DFileXMLSerializer();

            /**
             * Imports a 2D background XML file.
             *
             * @param stream[in] The contents of the background file.
             * @param dest[out] The formed background file.
             */
            virtual void ImportBackground2DFile(
              Ogre::DataStreamPtr &stream, Background2DFile *dest
            );

        protected:

            /**
             * Reads a file header and sets the instance data.
             *
             * @param stream[in] The contents of the HRC file.
             */
            virtual void ReadHeader(TiXmlNode *node);

            /**
             * Reads an XMl node attribute as tile blending information.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @param dest[out] The value of the specified attribute. If the
             * attribute doesn't exists, the value of def will be set here.
             * @return True if the attribute was actually read, false if it
             * didn't exist and the default value was loaded into dest.
             */
            virtual bool ReadAttribute(
              TiXmlNode &node, const String &attribute, Blending &dest,
              const Blending &def
            );

            using XMLSerializer::ReadAttribute;

            /**
             * Reads an XML node as a tile.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The formed tile data.
             */
            virtual void readObject(TiXmlNode &node, Tile &dest);

            /**
             * Reads an XML node as an animation.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The formed animation data.
             */
            virtual void readObject(TiXmlNode &node, Animation &dest);

            /**
             * Reads an XML node as a key frame.
             *
             * @param node[in] The XML node to read.
             * @param dest[out] The formed key frame data.
             */
            virtual void readObject(TiXmlNode &node, KeyFrame &dest);

            /**
             * Alpha-type blending key.
             */
            static const String BLENDING_ALPHA;

            /**
             * Add-type blending key.
             */
            static const String BLENDING_ADD;

            /**
             * Substract-type blending key.
             */
            static const String BLENDING_SUBTRACT;

            /**
             * Reads a XML node as a vector.
             *
             * @param node[in] The XML node to read.
             * @param tag[in] XML tag to read.
             * @param dest[out] The vector data will be loaded here.
             */
            template<typename ValueType> void ReadVector(
              TiXmlNode &node, const String &tag, std::vector<ValueType> &dest
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

            /**
             * Reads a XML node as a map.
             *
             * @param node[in] The XML node to read.
             * @param tag[in] XML tag to read.
             * @param key_attribute[in] @todo Understand and document.
             * @param dest[out] The map data will be loaded here.
             */
            template<typename KeyType, typename ValueType> void ReadMap(
              TiXmlNode& node, const String& tag, const String& key_attribute,
              std::map<KeyType, ValueType> &dest
            ){
                dest.clear();
                TiXmlNode* child(node.FirstChild());
                while (child != NULL){
                    if (
                      child->Type() == TiXmlNode::TINYXML_ELEMENT
                      && child->ValueStr() == tag
                    ){
                        KeyType key;
                        if (ReadAttribute(*child, key_attribute, key)){
                            ValueType in_tmp;
                            readObject(*child, in_tmp);
                            dest[key] = in_tmp;
                        }
                    }
                    child = child->NextSibling();
                }
            }

    };
}
