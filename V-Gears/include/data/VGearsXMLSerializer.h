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

#include <Ogre.h>
#include <OgreAxisAlignedBox.h>
#include <OgreColourValue.h>
#include <OgreSerializer.h>
#include <tinyxml.h>
#include "common/TypeDefine.h"

namespace VGears{

#ifdef NDEBUG
#define assertElement(node)((void)0)
#else
#define assertElement(node)\
{\
    if(node.Type() != TiXmlNode::TINYXML_ELEMENT)\
    {\
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS\
            ,"node has wrong type, needs to be TINYXML_ELEMENT"\
            ,"XMLSerializer::assertElement");\
    }\
}
#endif

    /**
     * Handles the serialization of XML files.
     */
    class XMLSerializer{

        public:

            /**
             * Constructor.
             */
            XMLSerializer();

            /**
             * Destructor.
             */
            virtual ~XMLSerializer();

        protected:

            /**
             * Reads and parses the XML file.
             *
             * @param stream[in] The contents of the XML file.
             * @param dest[out] The formed XML file.
             */
            virtual void Parse(
              Ogre::DataStreamPtr &stream, TiXmlDocument &dest
            );

            /**
             * Reads an XMl node attribute as a string.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @return The value of the specified attribute.
             */
            virtual const String* ReadAttribute(
              TiXmlNode &node, const String &attribute
            );

            /**
             * Reads an XMl node attribute as a boolean.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @param dest[out] The value of the specified attribute. If the
             * attribute doesn't exists, the value of def will be set here.
             * @return True if the attribute was actually read, false if it
             * didn't exist and the default value was loaded into dest.
             */
            virtual bool ReadAttribute(
              TiXmlNode &node, const String &attribute, bool &dest,
              const bool &def = false
            );

            /**
             * Reads an XMl node attribute as an integer.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @param dest[out] The value of the specified attribute. If the
             * attribute doesn't exists, the value of def will be set here.
             * @return True if the attribute was actually read, false if it
             * didn't exist and the default value was loaded into dest.
             */
            virtual bool ReadAttribute(
              TiXmlNode &node, const String &attribute, int &dest,
              const int  &def = 0
            );

            /**
             * Reads an XMl node attribute as a string.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @param dest[out] The value of the specified attribute. If the
             * attribute doesn't exists, the value of def will be set here.
             * @return True if the attribute was actually read, false if it
             * didn't exist and the default value was loaded into dest.
             */
            virtual bool ReadAttribute(
              TiXmlNode &node, const String &attribute, String &dest,
              const String &def = ""
            );

            /**
             * Reads an XMl node attribute as a real.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @param dest[out] The value of the specified attribute. If the
             * attribute doesn't exists, the value of def will be set here.
             * @return True if the attribute was actually read, false if it
             * didn't exist and the default value was loaded into dest.
             */
            virtual bool ReadAttribute(
              TiXmlNode &node, const String &attribute, Ogre::Real &dest,
              const Ogre::Real &def = 0
            );

            /**
             * Reads an XMl node attribute as a 2-dimension vector.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @param dest[out] The value of the specified attribute. If the
             * attribute doesn't exists, the value of def will be set here.
             * @return True if the attribute was actually read, false if it
             * didn't exist and the default value was loaded into dest.
             */
            virtual bool ReadAttribute(
              TiXmlNode &node, const String &attribute, Ogre::Vector2 &dest,
              const Ogre::Vector2 &def = Ogre::Vector2::ZERO
            );

            /**
             * Reads an XMl node attribute as a 3-dimension vector.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @param dest[out] The value of the specified attribute. If the
             * attribute doesn't exists, the value of def will be set here.
             * @return True if the attribute was actually read, false if it
             * didn't exist and the default value was loaded into dest.
             */
            virtual bool ReadAttribute(
              TiXmlNode &node, const String &attribute, Ogre::Vector3 &dest,
              const Ogre::Vector3 &def = Ogre::Vector3::ZERO
            );

            /**
             * Reads an XMl node attribute as a 4-dimension vector.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @param dest[out] The value of the specified attribute. If the
             * attribute doesn't exists, the value of def will be set here.
             * @return True if the attribute was actually read, false if it
             * didn't exist and the default value was loaded into dest.
             */
            virtual bool ReadAttribute(
              TiXmlNode &node, const String &attribute, Ogre::Vector4 &dest,
              const Ogre::Vector4 &def = Ogre::Vector4::ZERO
            );

            /**
             * Reads an XMl node attribute as a quaternion.
             *
             * @param node[in] The XML node.
             * @param attribute[in] The name of the attribute to read.
             * @param dest[out] The value of the specified attribute. If the
             * attribute doesn't exists, the value of def will be set here.
             * @return True if the attribute was actually read, false if it
             * didn't exist and the default value was loaded into dest.
             */
            virtual bool ReadAttribute(
              TiXmlNode &node, const String &attribute, Ogre::Quaternion &dest,
              const Ogre::Quaternion &def = Ogre::Quaternion::IDENTITY
            );

            /**
             * Finds a child node of a XML node by name.
             *
             * It doesn't search recursively, just among the direct children.
             *
             * @param node[in] The XML node to search.
             * @param tag[in] Name of the child to search for.
             * @return The child XML node by the specified name, or nullprt if
             * there is no one that matches the name.
             */
            virtual TiXmlNode* FindChildNode(
              TiXmlNode &node, const String &tag
            );
    };
}
