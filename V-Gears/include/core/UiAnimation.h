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

#include <OgreString.h>
#include <Ogre.h>
#include <vector>

class UiWidget;

/**
 * A keyframe value.
 */
struct UiKeyFrameFloat{

    /**
     * Keyframe time, in seconds.
     */
    float time;

    /**
     * Value for the keyframe.
     */
    float value;
};

/**
 * Coordinate keyframe.
 */
struct UiKeyFrameVector2{

    /**
     * Keyframe time, in seconds.
     */
    float time;

    /**
     * Position for the keyframe.
     */
    Ogre::Vector2 value;
};

/**
 * An UI element animation.
 */
class UiAnimation{

    public:

        /**
         * Constructor.
         *
         * @param[in] name Animation name.
         * @param[in] widget Widget to animate.
         */
        UiAnimation(const Ogre::String& name, UiWidget* widget);

        /**
         * Destructor.
         */
        virtual ~UiAnimation();

        /**
         * Animation state.
         */
        enum State{

            /**
             * @todo Understand and document.
             */
            DEFAULT,

            /**
             * Animate only once.
             */
            ONCE
        };

        /**
         * Adds time to the animation and updates it.
         *
         * @param[in] time Time to add, in seconds.
         */
        void AddTime(const float time);

        /**
         * Retrieves the animation name.
         *
         * @return The animation name.
         */
        const Ogre::String& GetName() const;

        /**
         * Sets the time of the animation and updates it.
         *
         * @param[in] time The time of the animation.
         */
        void SetTime(const float time);

        /**
         * Retrieves the animation current time.
         *
         * @return The animation current time.
         */
        float GetTime() const;

        /**
         * Sets the animation length.
         *
         * @param[in] time The animation length, in seconds.
         */
        void SetLength(const float time);

        /**
         * Retrieves the animation length.
         *
         * @return The animation length, in seconds.
         */
        float GetLength() const;

        /**
         * Adds a scale keyframe to the animation.
         *
         * @param[in] key_frame The keyframe to add.
         */
        void AddScaleKeyFrame(const UiKeyFrameVector2& key_frame);

        /**
         * Adds a X position keyframe to the animation.
         *
         * @param[in] key_frame The keyframe to add.
         */
        void AddXKeyFrame(const UiKeyFrameVector2& key_frame);

        /**
         * Adds a Y position keyframe to the animation.
         *
         * @param[in] key_frame The keyframe to add.
         */
        void AddYKeyFrame(const UiKeyFrameVector2& key_frame);

        /**
         * Adds a width keyframe to the animation.
         *
         * @param[in] key_frame The keyframe to add.
         */
        void AddWidthKeyFrame(const UiKeyFrameVector2& key_frame);

        /**
         * Adds a height keyframe to the animation.
         *
         * @param[in] key_frame The keyframe to add.
         */
        void AddHeightKeyFrame(const UiKeyFrameVector2& key_frame);

        /**
         * Adds a rotation keyframe to the animation.
         *
         * @param[in] key_frame The keyframe to add.
         */
        void AddRotationKeyFrame(const UiKeyFrameFloat& key_frame);

        /**
         * Adds an alpha keyframe to the animation.
         *
         * @param[in] key_frame The keyframe to add.
         */
        void AddAlphaKeyFrame(const UiKeyFrameFloat& key_frame);

        /**
         * Adds an scissor keyframe to the animation.
         *
         * @param[in] x1 @todo Understand and document.
         * @param[in] y1 @todo Understand and document.
         * @param[in] x2 @todo Understand and document.
         * @param[in] y2 @todo Understand and document.
         * @todo What is an scissor keyframe?.
         */
        void AddScissorKeyFrame(
          const UiKeyFrameVector2& x1, const UiKeyFrameVector2& y1,
          const UiKeyFrameVector2& x2, const UiKeyFrameVector2& y2
        );

    private:

        /**
         * Constructor.
         */
        UiAnimation();

        /**
         * Gets the value from a @{see UiKeyFrameFloat}.
         *
         * @return The value of the keyframe.
         */
        float KeyFrameGetValue(std::vector<UiKeyFrameFloat>& data);

        /**
         * Gets the value from a @{see UiKeyFrameVector2}.
         *
         * @return The value of the keyframe.
         */
        Ogre::Vector2 KeyFrameGetValue(std::vector<UiKeyFrameVector2>& data);

        /**
         * The UI animation name.
         */
        Ogre::String name_;

        /**
         * The UI widget.
         */
        UiWidget* widget_;

        /**
         * The animation's current time.
         */
        float time_;

        /**
         * The animation length.
         */
        float length_;

        /**
         * List of scale keyframes.
         */
        std::vector<UiKeyFrameVector2> scale_;

        /**
         * List of X position keyframes.
         */
        std::vector<UiKeyFrameVector2> x_;

        /**
         * List of Y position keyframes.
         */
        std::vector<UiKeyFrameVector2> y_;

        /**
         * List of width keyframes.
         */
        std::vector<UiKeyFrameVector2> width_;

        /**
         * List of height keyframes.
         */
        std::vector<UiKeyFrameVector2> height_;

        /**
         * List of rotation keyframes.
         */
        std::vector<UiKeyFrameFloat> rotation_;

        /**
         * List of alpha keyframes.
         */
        std::vector<UiKeyFrameFloat> alpha_;

        /**
         * @todo Understand and document.
         */
        std::vector<UiKeyFrameVector2> scissor_x_top_;

        /**
         * @todo Understand and document.
         */
        std::vector<UiKeyFrameVector2> scissor_y_left_;

        /**
         * @todo Understand and document.
         */
        std::vector<UiKeyFrameVector2> scissor_x_bottom_;

        /**
         * @todo Understand and document.
         */
        std::vector<UiKeyFrameVector2> scissor_y_right_;
};
