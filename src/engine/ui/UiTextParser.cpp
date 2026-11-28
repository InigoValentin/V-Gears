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
#include "UiTextParser.h"

const char* const UiTextParser::TIMER_VARIABLE = "UITextAreaTimer";

UiTextParser::UiTextParser(const Sources& sources, std::vector<TextChar>& text):
  sources_(sources), text_(text), uses_timer_(false)
{}

void UiTextParser::Parse(TiXmlNode* node, const Ogre::ColourValue& colour){
    for (; node != nullptr; node = node->NextSibling()){
        if (node->ToText() != nullptr) AddString(node->ToText()->Value(), colour);
        else if (node->ToElement() != nullptr){
            const Ogre::ColourValue content_colour = ParseElement(*node->ToElement(), colour);
            Parse(node->FirstChild(), content_colour);
        }
    }
}

Ogre::ColourValue UiTextParser::ParseElement(
  const TiXmlElement& element, const Ogre::ColourValue& colour
){
    const std::string& name = element.ValueStr();
    if (name == "colour"){
        const char* value = element.Attribute("value");
        if (value != nullptr) return Ogre::StringConverter::parseColourValue(value);
    }
    else if (name == "pause_ok"){
        TextChar character;
        character.pause_ok = true;
        text_.push_back(character);
    }
    else if (name == "pause"){
        const char* time = element.Attribute("time");
        if (time != nullptr){
            TextChar character;
            // The time of the pause is in frames of the game (30 per second), as in the original
            // game.
            character.pause_time = Ogre::StringConverter::parseReal(time) / 30.0f;
            text_.push_back(character);
        }
    }
    else if (name == "next_page"){
        TextChar character;
        character.next_page = true;
        text_.push_back(character);
    }
    else if (name == "timer"){
        uses_timer_ = true;
        AddVariable(TIMER_VARIABLE, colour);
    }
    else if (name == "variable"){
        const char* variable = element.Attribute("name");
        if (variable != nullptr) AddVariable(variable, colour);
    }
    else if (name == "character"){
        int id = 0;
        if (element.QueryIntAttribute("id", &id) == TIXML_SUCCESS)
            AddName(sources_.character_name(id), colour);
    }
    else if (name == "party"){
        int position = 0;
        if (element.QueryIntAttribute("pos", &position) == TIXML_SUCCESS)
            AddName(sources_.party_name(position), colour);
    }
    else if (name == "include"){
        const char* text = element.Attribute("name");
        if (text != nullptr) Parse(sources_.text(text), colour);
    }
    else if (name == "image"){
        const char* sprite = element.Attribute("sprite");
        TextChar character;
        if (sprite != nullptr && *sprite != '\0' && sources_.image(sprite, character))
            text_.push_back(character);
    }
    return colour;
}

void UiTextParser::AddVariable(const Ogre::String& name, const Ogre::ColourValue& colour){
    const Ogre::UTFString value = sources_.variable(name);
    TextChar mark;
    mark.skip = true;
    mark.variable = name;
    mark.variable_len = value.size();
    mark.colour = colour;
    text_.push_back(mark);
    AddString(value, colour);
}

void UiTextParser::AddName(const std::string& name, const Ogre::ColourValue& colour){
    for (const char letter : name){
        TextChar character;
        character.char_code = letter;
        character.colour = colour;
        text_.push_back(character);
    }
}

void UiTextParser::AddString(const Ogre::UTFString& string, const Ogre::ColourValue& colour){
    for (size_t i = 0; i < string.size(); ++ i){
        TextChar character;
        character.char_code = string[i];
        character.colour = colour;
        text_.push_back(character);
    }
}
