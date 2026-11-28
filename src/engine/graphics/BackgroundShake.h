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

#include <cstddef>
#include <OgreVector2.h>

/**
 * The shake of the view of a field: a chain of random bounces in each axis.
 */
class BackgroundShake{

    public:

        /**
         * Starts, changes or stops the shake, as the scripts ask.
         *
         * @param[in] type 0 to stop, 1 to shake horizontally, 2 vertically and 3 both.
         * @param[in] x_amplitude Horizontal amplitude, in original screen pixels.
         * @param[in] x_frames Duration of the horizontal bounces, at 30 frames per second.
         * @param[in] y_amplitude Vertical amplitude, in original screen pixels.
         * @param[in] y_frames Duration of the vertical bounces, at 30 frames per second.
         * @return False if the type is unknown.
         */
        bool Set(
          const int type, const int x_amplitude, const int x_frames, const int y_amplitude,
          const int y_frames
        );

        /**
         * Moves the shake forward in time.
         *
         * @param[in] delta Seconds since the last frame.
         * @return True if the position changed.
         */
        bool Update(const float delta);

        /**
         * Retrieves the position of the view.
         *
         * @return The displacement from the center, in original screen pixels.
         */
        Ogre::Vector2 Offset() const{return Ogre::Vector2(x_.value, y_.value);}

        /**
         * Checks if the view is moving in any axis.
         */
        bool IsRunning() const{return x_.running || y_.running;}

    private:

        /**
         * A chain of random bounces in one axis of the view.
         */
        struct Axis{

            /** Amplitude of the next bounces, in original screen pixels. */
            float amplitude = 0;

            /** Duration of the next bounces, in seconds. */
            float seconds = 0;

            /** Whether the axis is moving. */
            bool running = false;

            /** Whether the axis is going back to the center to stop. */
            bool stopping = false;

            /** Duration of the movement back to the center, in seconds. */
            float stop_seconds = 0;

            /** Position at the start of the current bounce. */
            float from = 0;

            /** Position at the end of the current bounce. */
            float to = 0;

            /** Duration of the current bounce, in seconds. */
            float duration = 0;

            /** Time since the start of the current bounce, in seconds. */
            float elapsed = 0;

            /** Index of the next random factor. */
            size_t step = 0;

            /** Current position, in original screen pixels. */
            float value = 0;

            /**
             * Changes the bounces of the axis, and starts it if it is stopped.
             */
            void Configure(const float new_amplitude, const float new_seconds);

            /**
             * Makes the axis go back to the center and stop after the current bounce.
             */
            void Stop(const float seconds_to_center);

            /**
             * Moves the axis forward in time.
             *
             * @return True if the position changed.
             */
            bool Advance(const float delta);

            /**
             * Starts the next bounce.
             */
            void StartBounce();
        };

        /**
         * The horizontal axis.
         */
        Axis x_;

        /**
         * The vertical axis.
         */
        Axis y_;
};
