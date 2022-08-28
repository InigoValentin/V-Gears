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
#include <OgreStringVector.h>
#include <Ogre.h>
#include <Overlay/OgreUTFString.h>
#include <OIS/OIS.h>
#include <tinyxml.h>

// TODO: All the methods defined here that refer to XML files are implemented
// as methods of XmlFile. Can they be deleted from here?

/**
 * Retrieves a boolean from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found or it's not a
 * boolean value.
 * @return Boolean value of the tag. If it's not found or it's not a boolean,
 * def is returned.
 */
bool GetBool(TiXmlNode* node, const Ogre::String& tag, bool def = false);

/**
 * Retrieves an integer from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found or it's not a
 * numeric value.
 * @return Integer value of the tag. If it's not found or it's not a number,
 * def is returned.
 */
int GetInt(TiXmlNode* node, const Ogre::String& tag, int def = 0);

/**
 * Retrieves a decimal from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found or it's not a
 * numeric value.
 * @return Floating value of the tag. If it's not found or it's not a number,
 * def is returned.
 */
float GetFloat(TiXmlNode* node, const Ogre::String& tag, float def = 0.0f);

/**
 * Retrieves a string from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found.
 * @return String value of the tag. If it's not found, def is returned.
 */
const Ogre::String GetString(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::String& def = ""
);

/**
 * Retrieves a string from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found.
 * @return String value of the tag, in UFT8. If not found, def is returned.
 */
const Ogre::UTFString GetUTFString(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::UTFString& def = ""
);

/**
 * Retrieves a 2-dimensional vector from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found.
 * @return Vector in the tag. If it's not found, def is returned.
 */
const Ogre::Vector2 GetVector2(
  TiXmlNode* node, const Ogre::String& tag,
  const Ogre::Vector2& def = Ogre::Vector2::ZERO
);

/**
 * Retrieves a 3-dimensional vector from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found.
 * @return Vector in the tag. If it's not found, def is returned.
 */
const Ogre::Vector3 GetVector3(
  TiXmlNode* node, const Ogre::String& tag,
  const Ogre::Vector3& def = Ogre::Vector3::ZERO
);

/**
 * Retrieves a 4-dimensional vector from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found.
 * @return Vector in the tag. If it's not found, def is returned.
 */
const Ogre::Vector4 GetVector4(
  TiXmlNode* node, const Ogre::String& tag,
  const Ogre::Vector4& def = Ogre::Vector4::ZERO
);

/**
 * Retrieves a 4-dimensional matrix from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found.
 * @return Martix in the tag. If it's not found, def is returned.
 */
const Ogre::Matrix4 GetMatrix4(
  TiXmlNode* node, const Ogre::String& tag,
  const Ogre::Matrix4& def = Ogre::Matrix4::IDENTITY
);

/**
 * Retrieves a quaternion from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found.
 * @return quaternion in the tag. If it's not found, def is returned.
 */
const Ogre::Quaternion GetQuaternion(
  TiXmlNode* node, const Ogre::String& tag,
  const Ogre::Quaternion& def = Ogre::Quaternion::IDENTITY
);

/**
 * Retrieves a colour from an XMl tag.
 *
 * @param node[in] The XML node.
 * @param tag[in] The name of the tag.
 * @param def[in] Default value, in case the tag is not found.
 * @return Colour in the tag. If it's not found, def is returned.
 */
const Ogre::ColourValue GetColourValue(
  TiXmlNode* node, const Ogre::String& tag,
  const Ogre::ColourValue& def = Ogre::ColourValue::ZERO
);

/**
 * Parses percentage strings
 *
 * Accepts strings in the formats "A" "A%" "A%B", where A and B are real
 * values.
 *
 * @param value_percent[out] Percent value
 * @param value[out] String numeric value.
 * @param string[in] Input string.
 * @example "80.4%" -> value = 0, percent_value = 80.4
 * @example "80.4%20.6" -> value = 20.6, percent_value = 80.4
 * @example "80.4" -> value = 80.4, percent_value = 0
 *
 */
void ParsePercent(
  float& value_percent, float& value, const Ogre::String& string
);

/**
 * Parses a keyframe time string.
 *
 * It acceps input string of numeric values and numeric values followed by a
 * percentage sign
 *
 * @param length[in] Keyframe duration, used only for percentages.
 * @param string[in] Input string
 * @return Keyframe time.
 * @example "7.2" -> 7.2
 * @example "7.2%" -> length * 7.2
 */
float ParseKeyFrameTime(const float length, const Ogre::String& string);

/**
 * Creates a name.
 *
 * @prefix[in] Name prefix.
 * @return PREFIX + "0".
 * @todo This seems so simple, am I missing something?
 */
const Ogre::String CreateAutoName(const Ogre::String prefix);

/**
 * Obtains a name from a key code.
 *
 * @param key[in] Key code.
 * @return Human readable name assigned to the key code. "UNASSIGNED" if there
 * is no name for the key code.
 */
Ogre::String KeyToString(OIS::KeyCode key);

/**
 * Obtains a key code from a name.
 *
 * @param str[in] Key name.
 * @return Key code assigned to the name. OIS::KC_UNASSIGNED  if there is no
 * keycode with the specified name.
 */
OIS::KeyCode StringToKey(const Ogre::String& str);

/**
 * Tokenizes a string.
 *
 * @param str[in] Input string.
 * @param delimiters[in] Token delimiters. They will be stripped from the
 * string.
 * @param delimiters_preserve[in] More token delimiters. They will be returned
 * as individual tokens.
 * @param quote[in] @todo Understand and document.
 * @param esc[in] Accepted escape characters. Escaped characters won't be
 * considered delimiters of any kind.
 * @return Tokens of the original string.
 */
Ogre::StringVector StringTokenise(
  const Ogre::String& str, const Ogre::String& delimiters = "\t\n ",
  const Ogre::String& delimiters_preserve = "",
  const Ogre::String& quote = "\"", const Ogre::String& esc = "\\"
);

