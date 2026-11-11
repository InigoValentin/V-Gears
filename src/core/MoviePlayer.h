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

#include <cstdint>
#include <string>
#include <vector>
#include <OgreHardwareVertexBuffer.h>
#include <OgreMaterial.h>
#include <OgreRenderOperation.h>
#include <OgreRenderQueueListener.h>
#include <OgreRenderSystem.h>
#include <OgreSceneManager.h>
#include <OgreTexture.h>
#include "MovieCameraFrame.h"
#include "MovieState.h"
#include "ScriptManager.h"

/**
 * Plays the movies that the installer converted: a folder of JPEG frames and an ogg file with the
 * sound, in data/video/NAME/.
 *
 * The movie is drawn over the field and below the interface and the screen fades.
 */
class MoviePlayer : public Ogre::RenderQueueListener{

    public:

        /**
         * Constructor.
         */
        MoviePlayer();

        /**
         * Destructor.
         */
        virtual ~MoviePlayer();

        /**
         * Run each frame. Moves the movie forward.
         *
         * @param[in] delta Seconds since the last frame.
         */
        void Update(const float delta);

        /**
         * Chooses the movie to play next, like the PMVIE opcode.
         *
         * @param[in] name Name of the movie, without extension.
         * @param[in] field_scale Scale factor of the field, to place the models over the movie.
         */
        void ScriptSet(const char* name, const float field_scale);

        /**
         * Starts the chosen movie, like the MOVIE opcode. The script that starts it waits until
         * the movie ends. The other scripts go on while it's played.
         *
         * @return 1 if the script doesn't have to wait, and -1 if it does.
         */
        int ScriptPlay();

        /**
         * Stops the movie.
         */
        void ScriptStop();

        /**
         * Retrieves the frame being shown, like the MVIEF opcode.
         *
         * @return The frame. If the chosen movie isn't installed, a number high enough to be
         * after any frame the scripts wait for.
         */
        int ScriptGetFrame() const;

        /**
         * @return True while a movie is being played.
         */
        bool ScriptIsPlaying() const;

        /**
         * Checks if the movie is drawn behind the field, which is drawn over it with the camera of
         * the movie. If not, the movie covers the field.
         *
         * @return True if a movie with a camera is being played.
         */
        bool IsDrawnBehind() const;

        /**
         * Draws the movie. If it has a camera it's drawn behind the models, and the models are
         * only drawn in the frames that have a camera. If not, it's drawn over the field and below
         * the interface.
         */
        void renderQueueStarted(
          Ogre::uint8 queue_group_id, const Ogre::String& invocation, bool& skip_this_invocation
        ) override;

    private:

        /**
         * Reads the information of the chosen movie.
         *
         * @param[out] frame_count Number of frames.
         * @param[out] frames_per_second Frames per second.
         * @param[out] camera_frame First frame with a camera for the models, or -1.
         * @param[out] release_frame Frame at which the script that plays the movie goes on, or -1
         * for the end.
         * @return True if the movie is installed.
         */
        bool ReadInfo(
          int& frame_count, float& frames_per_second, int& camera_frame, int& release_frame
        ) const;

        /**
         * Content of the camera file of the movie. Empty if it hasn't got one.
         */
        std::vector<uint8_t> camera_data_;

        /**
         * Scale factor of the field.
         */
        float field_scale_ = 1.0f;

        /**
         * Whether the current frame of the movie has a camera.
         */
        bool camera_valid_ = false;

        /**
         * Whether the movie camera is in use.
         */
        bool camera_in_use_ = false;

        /**
         * Frame at which the script that played the movie goes on, or -1 for the end of the movie.
         */
        int release_frame_ = -1;

        /**
         * Places the camera of the field in the one of the current frame of the movie, or puts it
         * back if the movie has ended.
         */
        void UpdateCamera();

        /**
         * Hides or shows everything the viewport draws as an object of the scene, the models.
         *
         * @param[in] hidden True to hide them.
         */
        void SetModelsHidden(const bool hidden);

        /**
         * Whether the models are hidden because the frame has no camera.
         */
        bool models_hidden_ = false;

        /**
         * Draws the quad with the current frame.
         */
        void DrawQuad();

        /**
         * Loads a frame in the texture.
         *
         * @param[in] frame The frame to load.
         */
        void LoadFrame(const int frame);

        /**
         * Writes the corners of the screen quad.
         */
        void WriteQuad();

        /**
         * The scripts that wait for the end of the movie.
         */
        std::vector<ScriptId> waiting_;

        /**
         * Resumes the scripts that wait for the end of the movie.
         */
        void ReleaseWaiting();

        /**
         * The clock of the movie.
         */
        MovieState state_;

        /**
         * Name of the chosen movie.
         */
        std::string name_;

        /**
         * Frame in the texture, or -1.
         */
        int loaded_frame_ = -1;

        /**
         * Scene manager.
         */
        Ogre::SceneManager* scene_manager_;

        /**
         * Render system.
         */
        Ogre::RenderSystem* render_system_;

        /**
         * Texture the frames are copied to.
         */
        Ogre::TexturePtr texture_;

        /**
         * Material of the quad.
         */
        Ogre::MaterialPtr material_;

        /**
         * Render operation of the quad.
         */
        Ogre::RenderOperation render_op_;

        /**
         * Vertex buffer of the quad.
         */
        Ogre::HardwareVertexBufferSharedPtr vertex_buffer_;
};
