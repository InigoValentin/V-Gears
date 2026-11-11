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

/**
 * The clock of a movie: which frame is shown at each moment, and when it ends.
 *
 * It has no knowledge of the video or the screen. It's separated from {@see MoviePlayer} so its
 * rules can be tested.
 */
class MovieState{

    public:

        /**
         * Frame reported when there is no movie to play. It's high so the scripts that wait for a
         * frame of the movie to continue don't wait for ever.
         */
        static const int NO_MOVIE_FRAME = 32767;

        /**
         * Starts the movie from its first frame.
         *
         * @param[in] frame_count Number of frames of the movie.
         * @param[in] frames_per_second Frames shown per second.
         */
        void Start(const int frame_count, const float frames_per_second);

        /**
         * Chooses a movie without playing it. Until it starts the frame is 0, which is what the
         * scripts that wait for a frame of the movie expect before it starts.
         */
        void Prepare();

        /**
         * Stops the movie. The frame it reaches is kept.
         */
        void Stop();

        /**
         * Moves the clock forward.
         *
         * @param[in] seconds Seconds since the last update.
         * @return True if the movie ended in this update.
         */
        bool Advance(const float seconds);

        /**
         * @return True while the movie is being played.
         */
        bool IsPlaying() const;

        /**
         * Retrieves the frame that must be shown.
         *
         * @return The frame, starting at 0. When the movie ended, the number of frames. If there
         * was no movie, {@see NO_MOVIE_FRAME}.
         */
        int GetFrame() const;

    private:

        /**
         * Whether the movie is being played.
         */
        bool playing_ = false;

        /**
         * Whether a movie has been started.
         */
        bool started_ = false;

        /**
         * Number of frames of the movie.
         */
        int frame_count_ = 0;

        /**
         * Frames per second.
         */
        float frames_per_second_ = 15.0f;

        /**
         * Seconds played.
         */
        float elapsed_ = 0;
};
