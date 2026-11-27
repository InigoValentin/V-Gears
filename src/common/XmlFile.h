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

#include <OgreColourValue.h>
#include <OgreMatrix4.h>
#include <OgreString.h>
#include <Ogre.h>
#include <Overlay/OgreUTFString.h>
#include <tinyxml.h>

/**
 * Handles XML files.
 */
class XmlFile{


    public:

        /**
         * Constructor.
         *
         * @param[in] file Path to the XML file.
         */
        XmlFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlFile();

        /**
         * Retrieves a boolean from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found or it's not a boolean value.
         * @return Boolean value of the tag. If it's not found or it's not a boolean, def is
         * returned.
         */
        bool GetBool(TiXmlNode* node, const Ogre::String& tag, bool def = false) const;

        /**
         * Retrieves an integer from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found or it's not a numeric value.
         * @return Integer value of the tag. If it's not found or it's not a number, def is
         * returned.
         */
        int GetInt(TiXmlNode* node, const Ogre::String& tag, int def = 0) const;

        /**
         * Retrieves a defcimal from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found or it's not a numeric value.
         * @return Decimal value of the tag. If it's not found or it's not a number, def is
         * returned.
         */
        float GetFloat(TiXmlNode* node, const Ogre::String& tag, float def = 0.0f) const;

        /**
         * Retrieves a string from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found.
         * @return String value of the tag. If it's not found, def is returned.
         */
        const Ogre::String GetString(
          TiXmlNode* node, const Ogre::String& tag, const Ogre::String& def = ""
        ) const;

        /**
         * Retrieves a string from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found.
         * @return String value of the tag, in UFT8. If not found, def is returned.
         */
        const Ogre::UTFString GetUTFString(
          TiXmlNode* node, const Ogre::String& tag, const Ogre::UTFString& def = ""
        ) const;

        /**
         * Retrieves a 2-dimensional vector from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found.
         * @return Vector in the tag. If it's not found, def is returned.
         */
        const Ogre::Vector2 GetVector2(
          TiXmlNode* node, const Ogre::String& tag, const Ogre::Vector2& def = Ogre::Vector2::ZERO
        ) const;

        /**
         * Retrieves a 3-dimensional vector from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found.
         * @return Vector in the tag. If it's not found, def is returned.
         */
        const Ogre::Vector3 GetVector3(
          TiXmlNode* node, const Ogre::String& tag, const Ogre::Vector3& def = Ogre::Vector3::ZERO
        ) const;

        /**
         * Retrieves a 4-dimensional vector from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found.
         * @return Vector in the tag. If it's not found, def is returned.
         */
        const Ogre::Vector4 GetVector4(
          TiXmlNode* node, const Ogre::String& tag, const Ogre::Vector4& def = Ogre::Vector4::ZERO
        ) const;

        /**
         * Retrieves a 4-dimensional matrix from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found.
         * @return The matrix in the tag. If it's not found, def is returned.
         */
        const Ogre::Matrix4 GetMatrix4(
          TiXmlNode* node, const Ogre::String& tag,
          const Ogre::Matrix4& def = Ogre::Matrix4::IDENTITY
        ) const;

        /**
         * Retrieves a quaternion from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found.
         * @return Quaternion in the tag. If it's not found, def is returned.
         */
        const Ogre::Quaternion GetQuaternion(
          TiXmlNode* node, const Ogre::String& tag,
          const Ogre::Quaternion& def = Ogre::Quaternion::IDENTITY
        ) const;

        /**
         * Retrieves a colour from an XMl tag.
         *
         * @param[in] node The XML node.
         * @param[in] tag The name of the tag.
         * @param[in] def Default value, in case the tag is not found.
         * @return Colour in the tag. If it's not found, def is returned.
         */
        const Ogre::ColourValue GetColourValue(
          TiXmlNode* node, const Ogre::String& tag,
          const Ogre::ColourValue& def = Ogre::ColourValue::ZERO
        ) const;

    protected:

        /**
         * Indicates if the file is a normal file.
         *
         * @todo As opposed to what?
         */
        bool normal_file_;

        /**
         * The XML file.
         */
        TiXmlDocument file_;
};
