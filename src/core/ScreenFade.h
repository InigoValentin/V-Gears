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

#include <vector>
#include <OgreHardwareVertexBuffer.h>
#include <OgreMaterial.h>
#include <OgreRenderOperation.h>
#include <OgreRenderQueueListener.h>
#include <OgreRenderSystem.h>
#include <OgreSceneManager.h>
#include "ScreenFadeState.h"
#include "ScriptManager.h"

/**
 * A coloured fade of the screen, for flashes and fades to a colour.
 *
 * The colour is drawn over the field and its models, and below the dialogs and the rest of the
 * interface. The rules of the fades are in {@see ScreenFadeState}.
 */
class ScreenFade : public Ogre::RenderQueueListener{

    public:

        /**
         * Constructor.
         */
        ScreenFade();

        /**
         * Destructor.
         */
        virtual ~ScreenFade();

        /**
         * Run each frame. Moves the fade forward, and resumes the scripts that wait for its end.
         *
         * @param[in] delta Seconds since the last frame.
         */
        void Update(const float delta);

        /**
         * Removes the fade. Run when a field is unloaded.
         */
        void Clear();

        /**
         * Starts a fade like the FADE opcode.
         *
         * @param[in] type Fade type, from 1 to 10.
         * @param[in] r Red component, from 0 to 255.
         * @param[in] g Green component, from 0 to 255.
         * @param[in] b Blue component, from 0 to 255.
         * @param[in] speed Speed of the fade. 8 takes a second.
         */
        void ScriptFade(const int type, const int r, const int g, const int b, const int speed);

        /**
         * Starts a fade like the NFADE opcode.
         *
         * @param[in] type Fade type: 0, 11 or 12.
         * @param[in] r Red component, from 0 to 255.
         * @param[in] g Green component, from 0 to 255.
         * @param[in] b Blue component, from 0 to 255.
         * @param[in] frames Duration of the fade, in frames of 30 per second.
         */
        void ScriptNFade(const int type, const int r, const int g, const int b, const int frames);

        /**
         * Waits for the fade to end, like the FADEW opcode. It continues at once if there is no
         * fade running.
         *
         * @return 1 if the script doesn't have to wait, and -1 if it does.
         */
        int ScriptWait();

        /**
         * Draws the fade, over the field and below the interface.
         *
         * It's drawn when the overlay queue starts, which is when the interface is drawn, and not
         * after the main queue, where it would depend on the order of the listeners and be
         * covered by the background if the background registered itself again.
         */
        void renderQueueStarted(
          Ogre::uint8 queue_group_id, const Ogre::String& invocation, bool& skip_this_invocation
        ) override;

    private:

        /**
         * Writes the colour in the vertices of the screen quad.
         *
         * @param[in] colour The colour.
         * @param[in] alpha The alpha.
         */
        void WriteQuad(const FadeColour& colour, const float alpha);

        /**
         * The rules and the progress of the fade.
         */
        ScreenFadeState state_;

        /**
         * Whether the current fade has been logged as drawn.
         */
        bool draw_logged_ = false;

        /**
         * The scripts that wait for the fade to end.
         */
        std::vector<ScriptId> waiting_;

        /**
         * The scene manager.
         */
        Ogre::SceneManager* scene_manager_;

        /**
         * The render system.
         */
        Ogre::RenderSystem* render_system_;

        /**
         * Material that covers the field.
         */
        Ogre::MaterialPtr normal_material_;

        /**
         * Material that adds to the field.
         */
        Ogre::MaterialPtr add_material_;

        /**
         * Material that subtracts from the field.
         */
        Ogre::MaterialPtr subtract_material_;

        /**
         * Render operation of the screen quad.
         */
        Ogre::RenderOperation render_op_;

        /**
         * Vertex buffer of the screen quad.
         */
        Ogre::HardwareVertexBufferSharedPtr vertex_buffer_;
};
