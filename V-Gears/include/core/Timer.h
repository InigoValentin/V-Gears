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

#include <OgreSingleton.h>

/**
 * The game timer.
 */
class Timer : public Ogre::Singleton<Timer>{
    public:

        /**
         * Constructor.
         */
        Timer();

        /**
         * Retrieves the total system time.
         *
         * @return The total system time.
         * @todo What is the system time? What's the difference with game time?
         */
        float GetSystemTimeTotal();

        /**
         * Retrieves the system time difference.
         *
         * @return The system time difference.
         * @todo What is the system time? What's the difference with game time?
         */
        float GetSystemTimeDelta();

        /**
         * Retrieves the total game time.
         *
         * @return The total game time.
         * @todo What is the game time? What's the difference with system time?
         */
        float GetGameTimeTotal();

        /**
         * Retrieves the game time difference.
         *
         * @return The game time difference.
         * @todo What is the game time? What's the difference with system time?
         */
        float GetGameTimeDelta();

        /**
         * Adds time to the time.
         *
         * It can also be used to subtract time, but it will never get below 0.
         *
         * @param time[in] The time to add?
         * @todo What are the units? seconds?
         */
        void AddTime(const float time);

        /**
         * Sets the game timer.
         *
         * @param timer[in] The new game timer.
         * @todo Understand and document.
         */
        void SetGameTimer(const float timer);

        /**
         * Retrieves the game timer.
         *
         * @return The game timer.
         * @todo What is this timer? Why is it an int, but the setter accepts
         * float?
         */
        int GetGameTimer() const;

    private:

        /**
         * The total system time.
         */
        float system_time_total_;

        /**
         * The system time difference.
         */
        float system_time_delta_;

        /**
         * The total game time.
         */
        float game_time_total_;

        /**
         * The game time difference.
         */
        float game_time_delta_;

        /**
         * The game timer.
         */
        float game_timer_;
};

