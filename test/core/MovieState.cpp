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

#include <boost/test/unit_test.hpp>
#include "core/MovieState.h"

BOOST_AUTO_TEST_CASE(TestMovieStateReportsAHighFrameIfThereIsNoMovie){
    MovieState movie;
    BOOST_CHECK(!movie.IsPlaying());
    BOOST_CHECK_EQUAL(movie.GetFrame(), MovieState::NO_MOVIE_FRAME);
    // A movie without frames is a movie that isn't installed.
    movie.Start(0, 15.0f);
    BOOST_CHECK(!movie.IsPlaying());
    BOOST_CHECK_EQUAL(movie.GetFrame(), MovieState::NO_MOVIE_FRAME);
}

BOOST_AUTO_TEST_CASE(TestMovieStateShowsTheFrameOfTheElapsedTime){
    MovieState movie;
    movie.Start(30, 15.0f);
    BOOST_CHECK(movie.IsPlaying());
    BOOST_CHECK_EQUAL(movie.GetFrame(), 0);
    BOOST_CHECK(!movie.Advance(0.5f));
    BOOST_CHECK_EQUAL(movie.GetFrame(), 7);
    BOOST_CHECK(!movie.Advance(0.5f));
    BOOST_CHECK_EQUAL(movie.GetFrame(), 15);
}

BOOST_AUTO_TEST_CASE(TestMovieStateEndsAfterTheLastFrame){
    MovieState movie;
    movie.Start(30, 15.0f);
    BOOST_CHECK(!movie.Advance(1.9f));
    BOOST_CHECK(movie.IsPlaying());
    BOOST_CHECK(movie.Advance(0.2f));
    BOOST_CHECK(!movie.IsPlaying());
    // The frame stays at the number of frames, and the end is only reported once.
    BOOST_CHECK_EQUAL(movie.GetFrame(), 30);
    BOOST_CHECK(!movie.Advance(5.0f));
    BOOST_CHECK_EQUAL(movie.GetFrame(), 30);
}

BOOST_AUTO_TEST_CASE(TestMovieStateStopKeepsTheFrame){
    MovieState movie;
    movie.Start(300, 15.0f);
    movie.Advance(2.0f);
    movie.Stop();
    BOOST_CHECK(!movie.IsPlaying());
    BOOST_CHECK_EQUAL(movie.GetFrame(), 30);
    BOOST_CHECK(!movie.Advance(1.0f));
    BOOST_CHECK_EQUAL(movie.GetFrame(), 30);
}

BOOST_AUTO_TEST_CASE(TestMovieStateStartRestarts){
    MovieState movie;
    movie.Start(30, 15.0f);
    movie.Advance(1.0f);
    movie.Start(60, 30.0f);
    BOOST_CHECK(movie.IsPlaying());
    BOOST_CHECK_EQUAL(movie.GetFrame(), 0);
    movie.Advance(0.5f);
    BOOST_CHECK_EQUAL(movie.GetFrame(), 15);
}

BOOST_AUTO_TEST_CASE(TestMovieStatePreparedMovieIsAtFrameZeroUntilItStarts){
    MovieState movie;
    movie.Prepare();
    BOOST_CHECK(!movie.IsPlaying());
    BOOST_CHECK_EQUAL(movie.GetFrame(), 0);
    BOOST_CHECK(!movie.Advance(5.0f));
    BOOST_CHECK_EQUAL(movie.GetFrame(), 0);
    movie.Start(30, 15.0f);
    movie.Advance(1.0f);
    BOOST_CHECK_EQUAL(movie.GetFrame(), 15);
    // Choosing another movie afterwards starts again from frame 0.
    movie.Stop();
    movie.Prepare();
    BOOST_CHECK_EQUAL(movie.GetFrame(), 0);
}
