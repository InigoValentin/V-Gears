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

#include <OgreStringConverter.h>
#include "core/Utilites.h"

bool
GetBool(TiXmlNode* node, const Ogre::String& tag, bool def){
    bool ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL) ret = Ogre::StringConverter::parseBool(*string);
    }
    return ret;
}

int GetInt(TiXmlNode* node, const Ogre::String& tag, int def){
    int ret = def;

    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL) ret = Ogre::StringConverter::parseInt(*string);
    }
    return ret;
}

float GetFloat(TiXmlNode* node, const Ogre::String& tag, float def){
    float ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL) ret = Ogre::StringConverter::parseReal(*string);
    }
    return ret;
}

const Ogre::String GetString(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::String& def
){
    Ogre::String ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL) ret = *string;
    }
    return ret;
}

const Ogre::UTFString GetUTFString(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::UTFString& def
){
    Ogre::UTFString ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL) ret = (*string).c_str();
    }
    return ret;
}

const Ogre::Vector2 GetVector2(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::Vector2& def
){
    Ogre::Vector2 ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL) ret = Ogre::StringConverter::parseVector2(*string);
    }
    return ret;
}

const Ogre::Vector3 GetVector3(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::Vector3& def
){
    Ogre::Vector3 ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL) ret = Ogre::StringConverter::parseVector3(*string);
    }
    return ret;
}

const Ogre::Vector4 GetVector4(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::Vector4& def
){
    Ogre::Vector4 ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL) ret = Ogre::StringConverter::parseVector4(*string);
    }
    return ret;
}

const Ogre::Matrix4 GetMatrix4(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::Matrix4& def
){
    Ogre::Matrix4 ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL) ret = Ogre::StringConverter::parseMatrix4(*string);
    }
    return ret;
}

const Ogre::Quaternion GetQuaternion(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::Quaternion& def
){
    Ogre::Quaternion ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL)
            ret = Ogre::StringConverter::parseQuaternion(*string);
    }
    return ret;
}

const Ogre::ColourValue GetColourValue(
  TiXmlNode* node, const Ogre::String& tag, const Ogre::ColourValue& def
){
    Ogre::ColourValue ret = def;
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT){
        const std::string* string = node->ToElement()->Attribute(tag);
        if (string != NULL)
            ret = Ogre::StringConverter::parseColourValue(*string);
    }
    return ret;
}

void ParsePercent(
  float& value_percent, float& value, const Ogre::String& string
){
    if (string.at(string.size() - 1) == '%'){
        value_percent = Ogre::StringConverter::parseReal(
          string.substr(0, string.size() - 1)
        );
        value = 0;
    }
    else{
        Ogre::StringVector param = Ogre::StringUtil::split(string, "%");
        if (param.size() > 1){
            value_percent = Ogre::StringConverter::parseReal(param[0]);
            value = Ogre::StringConverter::parseReal(param[1]);
        }
        else{
            value_percent = 0;
            value = Ogre::StringConverter::parseReal(string);
        }
    }
}

float ParseKeyFrameTime(const float length, const Ogre::String& string){
    float res = 0;
    if (string.at(string.size() - 1) == '%'){
        res
          = length
            * Ogre::StringConverter::parseReal(
              string.substr(0, string.size() - 1)
            )
            / 100;
    }
    else res = Ogre::StringConverter::parseReal(string);
    return res;
}

const Ogre::String CreateAutoName(const Ogre::String prefix){
    static int enumerator = 0;
    return prefix + Ogre::StringConverter::toString(enumerator++);
}

/**
 * Relates key codes and their names.
 */
struct KeyName {

    /**
     * Name assigned to the key code.
     */
    Ogre::String name;

    /**
     * The key code.
     */
    OIS::KeyCode key;
};


KeyName key_names[] = {
    {"Enter", OIS::KC_RETURN},
    {"Escape", OIS::KC_ESCAPE},
    {"Space", OIS::KC_SPACE},
    {"LShift", OIS::KC_LSHIFT},
    {"RShift", OIS::KC_RSHIFT},
    {"LCtrl", OIS::KC_LCONTROL},
    {"RCtrl", OIS::KC_RCONTROL},
    {"LAlt", OIS::KC_LMENU},
    {"RAlt", OIS::KC_RMENU},
    {"0", OIS::KC_0},
    {"1", OIS::KC_1},
    {"2", OIS::KC_2},
    {"3", OIS::KC_3},
    {"4", OIS::KC_4},
    {"5", OIS::KC_5},
    {"6", OIS::KC_6},
    {"7", OIS::KC_7},
    {"8", OIS::KC_8},
    {"9", OIS::KC_9},
    {"A", OIS::KC_A},
    {"B", OIS::KC_B},
    {"C", OIS::KC_C},
    {"D", OIS::KC_D},
    {"E", OIS::KC_E},
    {"F", OIS::KC_F},
    {"G", OIS::KC_G},
    {"H", OIS::KC_H},
    {"I", OIS::KC_I},
    {"J", OIS::KC_J},
    {"K", OIS::KC_K},
    {"L", OIS::KC_L},
    {"M", OIS::KC_M},
    {"N", OIS::KC_N},
    {"O", OIS::KC_O},
    {"P", OIS::KC_P},
    {"Q", OIS::KC_Q},
    {"R", OIS::KC_R},
    {"S", OIS::KC_S},
    {"T", OIS::KC_T},
    {"U", OIS::KC_U},
    {"V", OIS::KC_V},
    {"W", OIS::KC_W},
    {"X", OIS::KC_X},
    {"Y", OIS::KC_Y},
    {"Z", OIS::KC_Z},
    {"Left", OIS::KC_LEFT},
    {"Up", OIS::KC_UP},
    {"Down", OIS::KC_DOWN},
    {"Right", OIS::KC_RIGHT},
    {"Num0", OIS::KC_NUMPAD0},
    {"Num1", OIS::KC_NUMPAD1},
    {"Num2", OIS::KC_NUMPAD2},
    {"Num3", OIS::KC_NUMPAD3},
    {"Num4", OIS::KC_NUMPAD4},
    {"Num5", OIS::KC_NUMPAD5},
    {"Num6", OIS::KC_NUMPAD6},
    {"Num7", OIS::KC_NUMPAD7},
    {"Num8", OIS::KC_NUMPAD8},
    {"Num9", OIS::KC_NUMPAD9},
    {"NumLock", OIS::KC_NUMLOCK},
    {"NumDiv", OIS::KC_DIVIDE},
    {"NumMult", OIS::KC_MULTIPLY},
    {"NumSub", OIS::KC_SUBTRACT},
    {"NumAdd", OIS::KC_ADD},
    {"NumEnter", OIS::KC_NUMPADENTER},
    {"NumDec", OIS::KC_DECIMAL},
    {"F1", OIS::KC_F1},
    {"F2", OIS::KC_F2},
    {"F3", OIS::KC_F3},
    {"F4", OIS::KC_F4},
    {"F5", OIS::KC_F5},
    {"F6", OIS::KC_F6},
    {"F7", OIS::KC_F7},
    {"F8", OIS::KC_F8},
    {"F9", OIS::KC_F9},
    {"F10", OIS::KC_F10},
    {"F11", OIS::KC_F11},
    {"F12", OIS::KC_F12},
    {"UNASSIGNED", OIS::KC_UNASSIGNED},
};

Ogre::String KeyToString(OIS::KeyCode key){
    // Find string by key
    for (KeyName* kn = key_names; kn->name != "UNASSIGNED"; ++ kn)
        if (kn->key == key) return kn->name;
    return "UNASSIGNED";
}

OIS::KeyCode StringToKey(const Ogre::String& str){
    if (str == "") return OIS::KC_UNASSIGNED;

    // find key by string
    for (KeyName* kn = key_names; kn->name != "UNASSIGNED"; ++ kn)
        if (kn->name == str) return kn->key;
    return OIS::KC_UNASSIGNED;
}

Ogre::StringVector StringTokenise(
  const Ogre::String& str, const Ogre::String& delimiters,
  const Ogre::String& delimiters_preserve, const Ogre::String& quote,
  const Ogre::String& esc
){
    Ogre::StringVector ret;
    Ogre::String::size_type pos = 0; // Current position (char) in the string.
    char ch = 0; // Buffer for the current character.
    char delimiter = 0; // The buffer for the delimiter char which will be
                        // added to the tokens if the delimiter is preserved.
    char current_quote = 0; // The char of the currently open quote.
    bool quoted = false; // Indicator for an open quote.
    Ogre::String token;  // String buffer for the token.
    bool token_complete = false; // Indicates if the current token is ready to
                                 // be added to the result vector.
    Ogre::String::size_type len = str.length();  // Length of the input string.

    // For every char in the input string.
    while (len > pos){
        // Get the character of the string and reset the delimiter buffer.
        ch = str.at(pos);
        delimiter = 0;
        // Assume ch isn't a delimiter.
        bool add_char = true;
        // Check ...
        // ... if the delimiter is an escaped character.
        bool escaped = false; // Indicates if the next char is escaped.
        if (esc.empty() == false){ // Checks if esc-chars are provided.
            if (esc.find_first_of(ch) != std::string::npos){
                // Get the escaped char
                ++ pos;
                if (pos < len){ // If there are more chars left...
                    // ... get the next one...
                    ch = str.at(pos);

                    // ... and add the escaped character to the token.
                    add_char = true;
                }
                else{ // Can't get any more characters
                    // Don't add the esc-char.
                    add_char = false;
                }

                // Ignore the remaining delimiter checks
                escaped = true;
            }
        }

        // ... if the delimiter is a quote...
        if (quote.empty() == false && escaped == false){
            // ... if quote chars are provided and the char isn't escaped...
            if (quote.find_first_of(ch) != std::string::npos){
                // If not quoted, set to open quote and set the quote char.
                if (quoted == false){
                    quoted = true;
                    current_quote = ch;
                    // Don't add the quote-char to the token.
                    add_char = false;
                }
                else{ // If quote is open already.
                    // Check if it is the matching character to close it.
                    if (current_quote == ch){
                        // Close quote and reset the quote character
                        quoted = false;
                        current_quote = 0;
                        // Don't add the quote-char to the token
                        add_char = false;
                    }
                }
            }
        }

        // If the delimiter isn't preserved.
        if (delimiters.empty() == false && escaped == false && quoted == false){
            // If a delimiter is provided and the char isn't protected by quote
            // or escape char.
            if (delimiters.find_first_of(ch) != std::string::npos){
                // If the char  is a delimiter and the token string isn't empty
                // the token is complete.
                if (token.empty() == false) token_complete = true;
                // Don't add the delimiter to the token.
                add_char = false;
            }
        }

        // If the delimiter is preserved, add it as a token.
        bool add_delimiter = false;
        if (
          delimiters_preserve.empty() == false
          && escaped == false && quoted == false
        ){
            // If a delimiter which will be preserved is provided and the char
            // isn't protected by quote or escape char.
            if (delimiters_preserve.find_first_of(ch) != std::string::npos){
                // If the char is a delimiter and the token string isn't empty
                //  the token is complete.
                if (token.empty() == false) token_complete = true;
                // Don't add the delimiter to the token.
                add_char = false;
                // add the delimiter
                delimiter = ch;
                add_delimiter = true;
            }
        }

        // Add the character to the token.
        if (add_char == true) token.push_back(ch);

        // Add the token if it is complete.
        if (token_complete == true && token.empty() == false){
            // Add the token string.
            ret.push_back(token);
            // Clear the contents.
            token.clear();
            // Build the next token.
            token_complete = false;
        }

        // Add the delimiter
        if (add_delimiter == true){
            // The next token is the delimiter.
            Ogre::String delim_token;
            delim_token.push_back(delimiter);
            ret.push_back(delim_token);
        }
        // Repeat for the next character
        ++ pos;
    }

    // Add the final token
    if (token.empty() == false) ret.push_back(token);
    return ret;
}
