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

// Add this include because this is only common file between everything that
// uses Input function.
#include <OIS/OIS.h>

namespace QGears{

    /**
     * Type of event.
     */
    enum EventType{

        /**
         * Null event.
         */
        ET_NULL = 0,

        /**
         * A key is pressed.
         */
        ET_KEY_PRESS,

        /**
         * A key is being held down.
         */
        ET_KEY_REPEAT,

        /**
         * @todo Understand and document.
         */
        ET_KEY_REPEAT_WAIT,

        /**
         * @todo Understand and document.
         */
        ET_KEY_IMPULSE,

        /**
         * A key is released.
         */
        ET_KEY_RELEASE,

        /**
         * A mouse button is pressed.
         */
        ET_MOUSE_PRESS,

        /**
         * A mouse button is released.
         */
        ET_MOUSE_RELEASE,

        /**
         * The mouse has moved.
         */
        ET_MOUSE_MOVE,

        /**
         * The mouse has scrolled.
         */
        ET_MOUSE_SCROLL
    };


    /**
     * An input event.
     */
    struct Event{

        /**
         * Constructor.
         *
         * Sets the type to {@see ET_NULL}, and the parameters to 0.
         */
        Event():
          type(ET_NULL),
          param1(0),
          param2(0)
        {};

        /**
         * Constructor.
         *
         * Sets the parameters to 0.
         *
         * @param n[in] The event type.
         */
        Event(EventType n):
          type(n),
          param1(0),
          param2(0)
        {};

        /**
         * Constructor.
         *
         * @param n[in] The event type.
         * @param p1[in] First parameter.
         * @param p1[in] Second parameter.
         */
        Event(EventType n, float p1, float p2) :
          type(n),
          param1(p1),
          param2(p2)
        {};

        /**
         * The type of the event.
         */
        EventType type;

        /**
         * First event parameter.
         */
        float param1;

        /**
         * Second event parameter.
         */
        float param2;

        /**
         * Event ID.
         */
        Ogre::String event;
    };
}
