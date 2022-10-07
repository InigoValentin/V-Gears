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
#include "map/VGearsTile.h"

class Background2D;

/**
 * An animation in the background of a field.
 */
class Background2DAnimation{

    public:

        /**
         * Constructor.
         *
         * @param[in] name The animation name.
         * @param[in] background The background to animate.
         * @param[in] tile_index The tile the animation applies to.
         */
        Background2DAnimation(
          const Ogre::String& name, Background2D* background,
          const int tile_index
        );

        /**
         * Destructor.
         */
        virtual ~Background2DAnimation();

        /**
         * Repetition status of the animation.
         */
        enum State{

            /**
             * The animation must be played once.
             */
            ONCE,

            /**
             * The animation must loop indefinitely.
             */
            LOOPED
        };

        /**
         * Adds time, so the animation state is changed according to it.
         *
         * @param[in] time The time passed
         * @todo time is in seconds?
         */
        void AddTime(const float time);

        /**
         * Retrieves the animation name.
         *
         * @return The animation name.
         */
        const Ogre::String& GetName() const;

        /**
         * Sets the time the animation has been running.
         *
         * @param[in] time The time the animation has been running.
         * @todo time is in seconds?
         */
        void  SetTime( const float time );

        /**
         * Retrieves the time the animation has been running.
         *
         * @return The time the animation has been running.
         * @todo time is in seconds?
         */
        float GetTime() const;

        /**
         * Sets the duration of the animation.
         *
         * @param[in] time The duration of the animation.
         * @todo time is in seconds?
         */
        void  SetLength(const float time);

        /**
         * Retrieves the duration of the animation.
         *
         * @return The duration of the animation.
         * @todo time is in seconds?
         */
        float GetLength() const;

        /**
         * Adds a keyframe to the animation.
         *
         * @param[in] key_frame The keyframe to add.
         */
        virtual void AddUVKeyFrame(const VGears::KeyFrame key_frame);

        /**
         * Adds a keyframe to the animation.
         *
         * @param[in] time Time to add the keyframe at.
         * @param[in] uv Animation vector.
         * @todo time is in seconds?
         */
        virtual void AddUVKeyFrame(const float time, const Ogre::Vector4& uv);

        /**
         * Adds a keyframe to the animation.
         *
         * @param[in] time Time to add the keyframe at.
         * @param[in] u1 Animation vector component.
         * @param[in] v1 Animation vector component.
         * @param[in] u2 Animation vector component.
         * @param[in] v2 Animation vector component.
         * @todo time is in seconds?
         */
        virtual void AddUVKeyFrame(
          const float time,
          const float u1, const float v1, const float u2, const float v2
        );

    private:

        /**
         * Constructor.
         */
        Background2DAnimation();

        /**
         * The animation name.
         */
        Ogre::String name_;

        /**
         * The background the animation belongs to.
         */
        Background2D* background_;

        /**
         * The index of the tile tha animation applies to.
         */
        int tile_index_;

        /**
         * The animation current time.
         */
        float time_;

        /**
         * The duration of the animation.
         */
        float length_;

        /**
         * A keyframe.
         */
        struct Background2DKeyFrameUV{

            /**
             * Keyframe time.
             */
            float time;

            /**
             * Keyframe vector component.
             */
            float u1;

            /**
             * Keyframe vector component.
             */
            float v1;

            /**
             * Keyframe vector component.
             */
            float u2;

            /**
             * Keyframe vector component.
             */
            float v2;
        };

        /**
         * Keyframe list.
         */
        std::vector<Background2DKeyFrameUV> uv_;
};

