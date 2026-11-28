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
#include <OgrePrerequisites.h>

class UiSprite;

/**
 * A character in a text.
 */
struct TextChar{
    TextChar():
      char_code(0),
      colour(Ogre::ColourValue::White),
      skip(false),
      variable(""),
      variable_len(0),
      pause_ok(false),
      pause_time(0.0f),
      next_page(false),
      sprite(nullptr),
      sprite_y(0)
    {}

    /**
     * The character code.
     *
     * Normal character code, no FF7 code tables.
     */
    int char_code;

    /**
     * Color for the character.
     */
    Ogre::ColourValue colour;

    /**
     * Indicates if the character is to be actually printed.
     */
    bool skip;

    /**
     * If the character is a variable, the variable name.
     */
    Ogre::String variable;

    /**
     * If the character is a variable, the variable length.
     */
    unsigned int variable_len;

    /**
     * Indicates if the OK button has been pressed during a text pause.
     */
    bool pause_ok;

    /**
     * For pause characters, the time.
     */
    float pause_time;

    /**
     * Indicates if the character is a new page character.
     */
    bool next_page;

    /**
     * If the character is a symbol or an image, this is the sprite.
     */
    UiSprite* sprite;

    /**
     * If the character is a symbol or an image, this is the Y position.
     */
    float sprite_y;
};
