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
#include "core/ScreenFadeState.h"

BOOST_AUTO_TEST_CASE(TestScreenFadeStateSpeedToSeconds){
    BOOST_CHECK_CLOSE(ScreenFadeState::SpeedToSeconds(1), 8.0f, 0.01f);
    BOOST_CHECK_CLOSE(ScreenFadeState::SpeedToSeconds(2), 4.0f, 0.01f);
    BOOST_CHECK_CLOSE(ScreenFadeState::SpeedToSeconds(4), 2.0f, 0.01f);
    BOOST_CHECK_CLOSE(ScreenFadeState::SpeedToSeconds(8), 1.0f, 0.01f);
    BOOST_CHECK_CLOSE(ScreenFadeState::SpeedToSeconds(16), 0.5f, 0.01f);
    BOOST_CHECK_CLOSE(ScreenFadeState::SpeedToSeconds(32), 0.25f, 0.01f);
    // A speed that is not positive is instant.
    BOOST_CHECK_EQUAL(ScreenFadeState::SpeedToSeconds(0), 0.0f);
    BOOST_CHECK_EQUAL(ScreenFadeState::SpeedToSeconds(-3), 0.0f);
}

BOOST_AUTO_TEST_CASE(TestScreenFadeStateNothingByDefault){
    ScreenFadeState fade;
    BOOST_CHECK(!fade.IsFading());
    BOOST_CHECK(!fade.IsVisible());
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_NONE);
    BOOST_CHECK(!fade.Advance(1.0f));
}

BOOST_AUTO_TEST_CASE(TestScreenFadeStateRedFlashAddsAndGoesBack){
    // FADE type 5: the colour is added, and goes down to nothing. It's the red flash.
    ScreenFadeState fade;
    BOOST_REQUIRE(fade.StartFade(5, 255, 0, 0, 8));
    BOOST_CHECK(fade.IsFading());
    BOOST_CHECK(fade.IsVisible());
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_ADD);
    BOOST_CHECK_CLOSE(fade.GetColour().r, 1.0f, 0.01f);
    BOOST_CHECK_SMALL(fade.GetColour().g, 0.0001f);

    BOOST_CHECK(!fade.Advance(0.5f));
    BOOST_CHECK_CLOSE(fade.GetColour().r, 0.5f, 0.01f);
    // It ends once, and then the field is shown again.
    BOOST_CHECK(fade.Advance(0.5f));
    BOOST_CHECK(!fade.IsFading());
    BOOST_CHECK(!fade.IsVisible());
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_NONE);
    BOOST_CHECK(!fade.Advance(1.0f));
}

BOOST_AUTO_TEST_CASE(TestScreenFadeStateFadeToColourHoldsIt){
    // FADE type 10: the colour is added for the time of the fade, and stays.
    ScreenFadeState fade;
    BOOST_REQUIRE(fade.StartFade(10, 40, 80, 120, 16));
    BOOST_CHECK(fade.Advance(0.5f));
    BOOST_CHECK(!fade.IsFading());
    BOOST_CHECK(fade.IsVisible());
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_ADD);
    BOOST_CHECK_CLOSE(fade.GetColour().b, 120.0f / 255.0f, 0.01f);

    // Type 9 is the same, but it goes back to the field.
    BOOST_REQUIRE(fade.StartFade(9, 40, 80, 120, 16));
    BOOST_CHECK(fade.Advance(0.5f));
    BOOST_CHECK(!fade.IsVisible());
}

BOOST_AUTO_TEST_CASE(TestScreenFadeStateSubtractiveTypes){
    // Type 2 goes from nothing to the inverse of the colour times four, and holds it.
    ScreenFadeState fade;
    BOOST_REQUIRE(fade.StartFade(2, 255, 255, 255, 8));
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_SUBTRACT);
    BOOST_CHECK(fade.Advance(1.0f));
    // White inverted is black: nothing is subtracted.
    BOOST_CHECK_SMALL(fade.GetColour().r, 0.0001f);

    // Black inverted four times is brighter than the maximum: it is limited, and takes it all.
    BOOST_REQUIRE(fade.StartFade(2, 0, 0, 0, 8));
    BOOST_CHECK(fade.Advance(1.0f));
    BOOST_CHECK_CLOSE(fade.GetColour().r, 1.0f, 0.01f);
    BOOST_CHECK(fade.IsVisible());

    // Type 1 goes the other way, and ends with the field.
    BOOST_REQUIRE(fade.StartFade(1, 0, 0, 0, 8));
    BOOST_CHECK_CLOSE(fade.GetColour().r, 1.0f, 0.01f);
    BOOST_CHECK(fade.Advance(1.0f));
    BOOST_CHECK(!fade.IsVisible());
}

BOOST_AUTO_TEST_CASE(TestScreenFadeStateInstantBlackAndNormalFade){
    // Type 4 makes the screen black at once, and nobody has to wait for it.
    ScreenFadeState fade;
    BOOST_REQUIRE(fade.StartFade(4, 12, 34, 56, 8));
    BOOST_CHECK(!fade.IsFading());
    BOOST_CHECK(fade.IsVisible());
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_NORMAL);
    BOOST_CHECK_CLOSE(fade.GetAlpha(), 1.0f, 0.01f);
    BOOST_CHECK_SMALL(fade.GetColour().r, 0.0001f);

    // Type 6 covers the field with the colour little by little, and keeps it.
    BOOST_REQUIRE(fade.StartFade(6, 255, 255, 255, 8));
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_NORMAL);
    BOOST_CHECK_SMALL(fade.GetAlpha(), 0.0001f);
    fade.Advance(0.5f);
    BOOST_CHECK_CLOSE(fade.GetAlpha(), 0.5f, 0.01f);
    BOOST_CHECK(fade.Advance(0.5f));
    BOOST_CHECK(fade.IsVisible());
    BOOST_CHECK_CLOSE(fade.GetAlpha(), 1.0f, 0.01f);
}

BOOST_AUTO_TEST_CASE(TestScreenFadeStateNFade){
    // NFADE type 11 adds the colour in 30 frames, which is a second, and keeps it.
    ScreenFadeState fade;
    BOOST_REQUIRE(fade.StartNFade(11, 200, 100, 0, 30));
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_ADD);
    BOOST_CHECK_SMALL(fade.GetColour().r, 0.0001f);
    BOOST_CHECK(!fade.Advance(0.5f));
    BOOST_CHECK_CLOSE(fade.GetColour().r, 0.5f * 200.0f / 255.0f, 0.01f);
    BOOST_CHECK(fade.Advance(0.5f));
    BOOST_CHECK(fade.IsVisible());
    BOOST_CHECK_CLOSE(fade.GetColour().r, 200.0f / 255.0f, 0.01f);

    // Type 12 does the same, taking the colour from the field.
    BOOST_REQUIRE(fade.StartNFade(12, 200, 100, 0, 30));
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_SUBTRACT);

    // Type 0 clears whatever there is.
    BOOST_REQUIRE(fade.StartNFade(0, 0, 0, 0, 0));
    BOOST_CHECK(!fade.IsVisible());
    BOOST_CHECK(!fade.IsFading());
}

BOOST_AUTO_TEST_CASE(TestScreenFadeStateUnknownTypesAndReplacement){
    ScreenFadeState fade;
    // Unknown types change nothing.
    BOOST_CHECK(!fade.StartFade(3, 255, 0, 0, 8));
    BOOST_CHECK(!fade.StartFade(99, 255, 0, 0, 8));
    BOOST_CHECK(!fade.StartNFade(5, 255, 0, 0, 8));
    BOOST_CHECK(!fade.IsVisible());

    // A new fade replaces the one that is running.
    BOOST_REQUIRE(fade.StartFade(5, 255, 0, 0, 8));
    fade.Advance(0.5f);
    BOOST_REQUIRE(fade.StartFade(10, 0, 255, 0, 8));
    BOOST_CHECK_EQUAL(fade.GetBlend(), ScreenFadeState::BLEND_ADD);
    BOOST_CHECK_SMALL(fade.GetColour().r, 0.0001f);
    BOOST_CHECK_CLOSE(fade.GetColour().g, 1.0f, 0.01f);

    // An unknown type doesn't stop the one that is running.
    BOOST_CHECK(!fade.StartFade(3, 0, 0, 0, 8));
    BOOST_CHECK(fade.IsFading());

    fade.Reset();
    BOOST_CHECK(!fade.IsVisible());
    BOOST_CHECK(!fade.IsFading());
}
