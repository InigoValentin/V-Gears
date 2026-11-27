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

#include <deque>
#include <functional>
#include <string>

namespace VGears {

    /**
     * What is special in the snow map (WM3): the blizzard, the camera that turns by itself and the
     * poles that the player can place.
     */
    class WorldmapSnow{

        public:

            /**
             * Creates the model of a pole. It receives the name of the entity, without the prefix "WorldMap".
             */
            typedef std::function<void(const std::string& name)> PoleCreator;

            /**
             * Shows or hides the blizzard, which is drawn over the screen. It is created the first time.
             *
             * @param[in] visible True to show it.
             */
            void SetVisible(const bool visible);

            /**
             * Forgets the poles and starts the wait for the next turn of the camera. For a map that is loaded.
             */
            void Reset();

            /**
             * Turns the camera by itself every few seconds.
             *
             * @param[in] delta_time Seconds since the last frame.
             * @return What the camera turns in this frame, in radians.
             */
            float UpdateCamera(const float delta_time);

            /**
             * Places a pole. Only 4 poles exist, the oldest one goes away.
             *
             * @param[in] create Creates the model of the new pole.
             */
            void PlacePole(const PoleCreator& create);

        private:

            /// Seconds until the next automatic turn of the camera.
            float snow_camera_timer_ = 0.0f;
            /// Radians left of the turn that is going on, with the sign of its direction.
            float snow_camera_remaining_ = 0.0f;
            /// The poles that exist (names of the entities), the oldest first.
            std::deque<std::string> snow_poles_;
            unsigned int snow_pole_counter_ = 0;
    };
}
