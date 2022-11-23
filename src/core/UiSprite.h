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

#include <OgreHardwareVertexBuffer.h>
#include <OgreRenderQueueListener.h>
#include <OgreRoot.h>
#include "UiWidget.h"

/**
 * A sprite.
 */
class UiSprite : public UiWidget{

    public:

        /**
         * Constructor.
         *
         * @param[in] name The sprite name.
         */
        UiSprite(const Ogre::String& name);

        /**
         * Constructor.
         *
         * @param[in] name The sprinte name.
         * @param[in] path_name Path to the sprite file.
         * @param[in] parent Widget to be set as parent of the sprite.
         * @todo path_name is relative to data? Filename is needed?
         */
        UiSprite(
          const Ogre::String& name, const Ogre::String& path_name,
          UiWidget* parent
        );

        /**
         * Destructor.
         */
        virtual ~UiSprite();

        /**
         * Loads an initializes the sprite.
         */
        void Initialise();

        /**
         * Updates the sprite state.
         */
        virtual void Update();

        /**
         * Renders the sprite.
         */
        virtual void Render();

        /**
         * Updates the sprite state.
         */
        virtual void UpdateTransformation();

        /**
         * Sets the sprite image.
         *
         * @param[in] image Path to the image.
         */
        void SetImage(const Ogre::String& image);

        /**
         * Sets the sprite image.
         *
         * @param[in] image Path to the image.
         */
        void SetImage(const char* image) override;

        /**
         * Sets the vertex shader for the sprite.
         *
         * @param[in] shader The shader to set.
         */
        void SetVertexShader(const Ogre::String& shader);

        /**
         * Sets the fragment shader for the sprite.
         *
         * @param[in] shader The shader to set.
         */
        void SetFragmentShader(const Ogre::String& shader);

        /**
         * Recalculates the sprite geometry.
         */
        void UpdateGeometry();

    private:

        /**
         * Constructor.
         */
        UiSprite();

        /**
         * Creates a vertex buffer for the sprite.
         */
        void CreateVertexBuffer();

        /**
         * Destroys a vertex buffer for the sprite.
         */
        void DestroyVertexBuffer();

        /**
         * The sprite material.
         */
        Ogre::MaterialPtr material_;

        /**
         * The scene manager.
         */
        Ogre::SceneManager* scene_manager_;

        /**
         * The render system.
         */
        Ogre::RenderSystem* render_system_;

        /**
         * The render operation.
         */
        Ogre::RenderOperation render_operation_;

        /**
         * The vertex buffer.
         */
        Ogre::HardwareVertexBufferSharedPtr vertex_buffer_;
};
