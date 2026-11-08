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
 * A colour whose components go from 0 (none) to 1 (full intensity).
 */
struct FadeColour{

    /**
     * Red component.
     */
    float r = 0;

    /**
     * Green component.
     */
    float g = 0;

    /**
     * Blue component.
     */
    float b = 0;
};

/**
 * The rules of the screen fades of the FADE and NFADE opcodes.
 *
 * A fade is a linear tween of a colour that is drawn over the field, blended with it. Depending
 * on the fade type the colour is added to the field, subtracted from it, or mixed with it. Some
 * fades keep their last colour when the tween ends, and the others go back to showing the field.
 * A black colour in an additive or subtractive fade changes nothing.
 *
 * This class has no dependency on the rendering engine, so the rules can be tested.
 */
class ScreenFadeState{

    public:

        /**
         * How the colour is combined with the field.
         */
        enum Blend{

            /**
             * Nothing is drawn.
             */
            BLEND_NONE,

            /**
             * The colour covers the field in proportion to the alpha.
             */
            BLEND_NORMAL,

            /**
             * The colour is added to the field.
             */
            BLEND_ADD,

            /**
             * The colour is subtracted from the field.
             */
            BLEND_SUBTRACT
        };

        /**
         * Starts a fade of the FADE opcode, replacing the one that is running.
         *
         * @param[in] type Fade type, from 1 to 10. 3 is not used.
         * @param[in] r Red component, from 0 to 255.
         * @param[in] g Green component, from 0 to 255.
         * @param[in] b Blue component, from 0 to 255.
         * @param[in] speed Speed of the fade. 8 takes a second, and double speed halves the time.
         * @return False if the type is unknown, and then nothing changes.
         */
        bool StartFade(const int type, const int r, const int g, const int b, const int speed);

        /**
         * Starts a fade of the NFADE opcode, replacing the one that is running.
         *
         * @param[in] type Fade type: 0 clears the fade, 11 adds and 12 subtracts.
         * @param[in] r Red component, from 0 to 255.
         * @param[in] g Green component, from 0 to 255.
         * @param[in] b Blue component, from 0 to 255.
         * @param[in] frames Duration of the fade, in frames of 30 per second.
         * @return False if the type is unknown, and then nothing changes.
         */
        bool StartNFade(const int type, const int r, const int g, const int b, const int frames);

        /**
         * Moves the fade forward in time.
         *
         * @param[in] seconds Time since the last call.
         * @return True if the tween of the fade ended with this call.
         */
        bool Advance(const float seconds);

        /**
         * Removes any fade, including the colour it holds.
         */
        void Reset();

        /**
         * Checks if a tween is running. A script waiting for the fade waits while it is.
         *
         * @return True if the fade is changing.
         */
        bool IsFading() const;

        /**
         * Checks if something has to be drawn: a tween, or the colour it holds.
         *
         * @return True if the fade covers the field.
         */
        bool IsVisible() const;

        /**
         * Retrieves how the colour has to be combined with the field.
         *
         * @return The blending.
         */
        Blend GetBlend() const;

        /**
         * Retrieves the colour to draw now, with all its components between 0 and 1.
         *
         * @return The colour.
         */
        FadeColour GetColour() const;

        /**
         * Retrieves how much the colour covers the field. Only the normal blending uses it.
         *
         * @return The alpha, between 0 and 1.
         */
        float GetAlpha() const;

        /**
         * Converts the speed of the FADE opcode to the seconds the fade takes.
         *
         * @param[in] speed The speed. 1 is 8 seconds, 2 is 4, 4 is 2 and 8 is 1.
         * @return The seconds. 0 for a speed that is not positive, meaning instant.
         */
        static float SpeedToSeconds(const int speed);

    private:

        /**
         * Starts a fade.
         *
         * @param[in] blend How the colour is combined with the field.
         * @param[in] from Colour at the start.
         * @param[in] to Colour at the end.
         * @param[in] alpha_from Alpha at the start.
         * @param[in] alpha_to Alpha at the end.
         * @param[in] seconds Duration. If it is 0 the fade is instant.
         * @param[in] hold Whether the final colour stays on the screen.
         */
        void Begin(
          const Blend blend, const FadeColour& from, const FadeColour& to, const float alpha_from,
          const float alpha_to, const float seconds, const bool hold
        );

        /**
         * How the colour is combined with the field.
         */
        Blend blend_ = BLEND_NONE;

        /**
         * Colour at the start of the tween.
         */
        FadeColour from_;

        /**
         * Colour at the end of the tween.
         */
        FadeColour to_;

        /**
         * Alpha at the start of the tween.
         */
        float alpha_from_ = 1;

        /**
         * Alpha at the end of the tween.
         */
        float alpha_to_ = 1;

        /**
         * Duration of the tween.
         */
        float duration_ = 0;

        /**
         * Time since the start of the tween.
         */
        float elapsed_ = 0;

        /**
         * Whether the tween is running.
         */
        bool fading_ = false;

        /**
         * Whether the final colour stays on the screen when the tween ends.
         */
        bool hold_ = false;

        /**
         * Whether the final colour is on the screen after the tween.
         */
        bool holding_ = false;
};
