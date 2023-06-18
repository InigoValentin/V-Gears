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
         * @param[in] event Event that triggers the camera action.
         * @param[in] time_since_last_frame For speed calculation.
         */
        void Input(
          const VGears::Event& event , Ogre::Real time_since_last_frame
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
         * @param[in] enable True to enable free camera, false to disable it.
         */
        void SetCameraFree(const bool enable);

        /**
         * Sets the camera properties.
         *
         * @param[in] position Camera coordinates.
         * @param[in] orientation Camera rotation, in quaternion format.
         * @param[in] fov Field of view, in radians.
         */
        void Set2DCamera(
          const Ogre::Vector3 position, const Ogre::Quaternion orientation,
          const Ogre::Radian fov
        );

        /**
         * Starts the battle camera
         *
         * Saves the position and orientation of the current camera to return once the battle is
         * over.
         *
         * @param[in] position Initial position of the battle camera.
         * @param[in] orientation Initial orientation of the battle camera. It indicates the point
         * the camera will look at.
         */
        void StartBattleCamera(const Ogre::Vector3 position, const Ogre::Vector3 orientation);

        /**
         * Ends the battle camera
         *
         * Returns the camera to the state it was when {@see StartBattleCamera} was called.
         */
        void EndBattleCamera();

        /**
         * Sets the camera scroll.
         *
         * Moves the camera to the desired position. If the camera is in free
         * mode, it sets the position, but it won't actually move the camera.
         *
         * @param[in] position Position to scroll the camera to.
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
         * @param[in] point Position of the map to be translated to screen
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
         * @param[in] enable True to enable wireframe mode, false to disable.
         */
        void EnableWireFrame(bool enable);

        /**
         * Sets the camera position and orientation.
         *
         * It's only meand to be used for 3D cameras (battle, world map...), and using it while on
         * a field can have unexpected results.
         *
         * @param[in] x X coordinate for the camera position.
         * @param[in] y Y coordinate for the camera position.
         * @param[in] z Z coordinate for the camera position.
         * @param[in] d_x X coordinate of the point the camera looks at.
         * @param[in] d_y Y coordinate of the point the camera looks at.
         * @param[in] d_y Z coordinate of the point the camera looks at.
         */
        void ScriptSetCamera(
          const int x, const int y, const int z, const int d_x, const int d_y, const int d_z
        );

    private:

        /**
         * Initializes the camera parameters.
         *
         * Must be called on construction.
         */
        void InitCommands();

        /**
         * Indicates if the camera is in battle mode.
         */
        bool battle_;

        /**
         * The camera.
         */
        Ogre::Camera *camera_;

        /**
         * The viewport.
         */
        Ogre::Viewport *viewport_;

        /**
         * The initial position of the camera, saved when created.
         */
        Ogre::Vector3 position_initial_;

        /**
         * The initial orientation of the camera, saved when created.
         */
        Ogre::Quaternion orientation_initial_;

        /**
         * A backup of the field or world camera position for when the battle camera is activated.
         */
        Ogre::Vector3 position_backup_;

        /**
         * A backup of the field or world camera orientation for when the battle camera is active.
         */
        Ogre::Quaternion orientation_backup_;

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

