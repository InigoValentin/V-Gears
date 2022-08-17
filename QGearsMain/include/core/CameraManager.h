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

#include <OgreCamera.h>
#include <OgreSingleton.h>
#include "Event.h"

/**
 * The camera manager.
 */
class CameraManager : public Ogre::Singleton<CameraManager>{

    public:

        /**
         * Constructor.
         *
         * Initializes the camera with default parameters attached to the root scene
         * manager.
         */
        CameraManager();

        /**
         * Destructor.
         */
        virtual ~CameraManager();

        /**
         * Handles camera actions.
         *
         * Handles the scene manager camera actions based on events.
         *
         * @param event[in] Event that triggers the camera action.
         * @param time_since_last_frame[in] For speed calculation.
         */
        void Input(
          const QGears::Event& event , Ogre::Real time_since_last_frame
        );

        /**
         * Triggered when updated.
         *
         * Unused.
         */
        void Update();

        /**
         * Trigered when the viewport is resized.
         *
         * Resets the aspect ratio.
         */
        void OnResize();

        /**
         * Enables or disables the free camera.
         *
         * A free camera moves depending on player input, not according to a
         * script. For example, a camera following the PC is a free camera.
         *
         * @param enable[in] True to enable free camera, false to disable it.
         */
        void SetCameraFree(const bool enable);

        /**
         * Sets the camera properties.
         *
         * @param position[in] Camera coordinates.
         * @param orientation[in] Camera rotation, in quaternion format.
         * @param fov[in] Field of view, in radians.
         */
        void Set2DCamera(
          const Ogre::Vector3 position, const Ogre::Quaternion orientation,
          const Ogre::Radian fov
        );

        /**
         * Sets the camera scroll.
         *
         * Moves the camera to the desired position. If the camera is in free
         * mode, it sets the position, but it won't actually move the camera.
         *
         * @param position[in] Position to scroll the camera to.
         */
        void Set2DScroll(const Ogre::Vector2& position);

        /**
         * Retrieves the camera position.
         *
         * @return The camera current position, relative to absolute origin.
         */
        const Ogre::Vector2& Get2DScroll() const;

        /**
         * Calculates the position of a point in screen.
         *
         * The calculation is done using A map position and the camera scroll.
         *
         * @param point[in] Position of the map to be translated to screen
         * position.
         * @return Position of POINT in screen coordinates.
         */
        const Ogre::Vector3 ProjectPointToScreen(const Ogre::Vector3& point);

        /**
         * Retrieves the camera.
         *
         * @return The current camera.
         */
        Ogre::Camera* GetCurrentCamera();

        /**
         * Retrieves the viewport.
         *
         * @return The camera viewport.
         */
        Ogre::Viewport* getViewport();

        /**
         * Enables or disables the camera wireframe.
         *
         * In wireframe mode, faces will not be rendered, only edges
         *
         * @param enable[in] True to enable wireframe mode, false to disable.
         */
        void EnableWireFrame(bool enable);

    private:

        /**
         * Initializes the camera parameters.
         *
         * Must be called on construction.
         */
        void InitCommands();

        /**
         * The camera.
         */
        Ogre::Camera *camera_;

        /**
         * The viewport.
         */
        Ogre::Viewport *viewport_;

        /**
         * Flag to indicate a free camera.
         *
         * A free camera moves depending on player input, not according to a
         * script. For example, a camera following the PC is a free camera.
         */
        bool camera_free_;

        /**
         * Flag to indicate the free camera has rotated.
         */
        bool camera_free_rotate_;

        /**
         * Camera position.
         */
        Ogre::Vector3 d2_position_;

        /**
         * Camera orientation.
         */
        Ogre::Quaternion d2_orientation_;

        /**
         * The field of view.
         */
        Ogre::Radian d2_fov_;

        /**
         * The camera scroll.
         */
        Ogre::Vector2 d2_scroll_;
};

