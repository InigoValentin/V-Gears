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
#include <OgreSingleton.h>
#include <OgreFont.h>

class DebugDraw :
  public Ogre::RenderQueueListener, public Ogre::Singleton<DebugDraw>
{
    public:

        /**
         * Constructor.
         */
        DebugDraw();

        /**
         * Destructor.
         */
        virtual ~DebugDraw();

        /**
         * Sets the colour of the element to draw.
         *
         * @param[in] colour The color to apply.
         */
        void SetColour(const Ogre::ColourValue& colour);

        /**
         * Sets the screen space.
         *
         * @param[in] screen_space The screen space.
         * @todo Understand and document properly.
         */
        void SetScreenSpace(const bool screen_space);

        /**
         * Set the Z coordinate.
         *
         * @param[in] z The Z coordinate.
         */
        void SetZ(const float z);

        /**
         * Sets the fade distance for text.
         *
         * @param[in] fade_s The text starts fading at this distance.
         * @param[in] fade_e The text has faded completely at this distance.
         */
        void SetFadeDistance(const float fade_s, const float fade_e);

        /**
         * Text alignment modes.
         */
        enum TextAlignment{

            /**
             * Left aligned text.
             */
            LEFT,

            /**
             * Right aligned text.
             */
            RIGHT,

            /**
             * Center aligned text.
             */
            CENTER
        };
        void SetTextAlignment(TextAlignment alignment);

        /**
         * Draws a line in 2D space.
         *
         * If the debug vertex limit has been reached or if it would be
         * exceeded by drawing the line, a warning message will be printed and
         * nothing will be done.
         *
         * @param[in] x1 X coordinate of the starting point.
         * @param[in] y1 Y coordinate of the starting point.
         * @param[in] x2 X coordinate of the ending point.
         * @param[in] y2 Y coordinate of the ending point.
         */
        void Line(
          const float x1, const float y1, const float x2, const float y2
        );

        /**
         * Draws a line in 3D space.
         *
         * If the debug vertex limit has been reached or if it would be
         * exceeded by drawing the line, a warning message will be printed and
         * nothing will be done.
         *
         * @param[in] point1 Starting point.
         * @param[in] point2 Ending point.
         */
        void Line3d(const Ogre::Vector3& point1, const Ogre::Vector3& point2);

        /**
         * Draws a triangle in 3D space.
         *
         * If the debug vertex limit has been reached or if it would be
         * exceeded by drawing the triangle, a warning message will be printed
         * and nothing will be done.
         *
         * @param[in] point1 A triangle vertex point.
         * @param[in] point2 A triangle vertex point.
         * @param[in] point3 A triangle vertex point.
         */
        void Triangle3d(
          const Ogre::Vector3& point1, const Ogre::Vector3& point2,
          const Ogre::Vector3& point3
        );

        /**
         * Draws a quad in 2D space.
         *
         * If the debug vertex limit has been reached or if it would be
         * exceeded by drawing the quad, a warning message will be printed and
         * nothing will be done.
         *
         * @param[in] x1 X coordinate of the first point.
         * @param[in] y1 Y coordinate of the first point.
         * @param[in] x2 X coordinate of the second point.
         * @param[in] y2 Y coordinate of the second point.
         * @param[in] x3 X coordinate of the third point.
         * @param[in] y3 Y coordinate of the third point.
         * @param[in] x4 X coordinate of the fourth point.
         * @param[in] y4 Y coordinate of the fourth point.
         */
        void Quad(
          const float x1, const float y1, const float x2, const float y2,
          const float x3, const float y3, const float x4, const float y4
        );

        /**
         * Writes debug text on the game screen.
         *
         * The text won't be warped automatically. If the maximum number of
         * debug letters has been written, or if it would be exceeded by
         * writing, a warning message will be printed to console and nothing
         * will be done.
         *
         * @param[in] x Left position of the text in the screen.
         * @param[in] y Left position of the text in the screen.
         * @param[in] text The text to write.
         */
        void Text(const float x, const float y, const Ogre::String& text);

        /**
         * Writes debug text on the game screen.
         *
         * The text will be warped automatically. If the maximum number of
         * debug letters has been written, or if it would be exceeded by
         * writing, a warning message will be printed to console and nothing
         * will be done.
         *
         * @param[in] point Top left point of the screen where the text will be
         * written.
         * @param[in] x Text width.
         * @param[in] y Text height.
         * @param[in] text The text to write.
         */
        void Text(
          const Ogre::Vector3& point, const float x, const float y,
          const Ogre::String& text
        );

        /**
         * Ends the render queue.
         *
         * @param[in] queueGroupId The group id of the queue to end.
         * @param[in] invocation
         * @param[in] repeatThisInvocation
         * @todo Understand and document.
         */
        void renderQueueEnded(
          Ogre::uint8 queueGroupId, const Ogre::String& invocation,
          bool& repeatThisInvocation
        );

    private:

        /**
         * Creates a 2D line vertex buffer (2 vertices).
         */
        void CreateLineVertexBuffer();

        /**
         * Destroys a 2D line vertex buffer (2 vertices).
         */
        void DestroyLineVertexBuffer();

        /**
         * Creates a 3D line vertex buffer (2 vertices).
         */
        void CreateLine3dVertexBuffer();

        /**
         * Destroys a 3D line vertex buffer (2 vertices).
         */
        void DestroyLine3dVertexBuffer();

        /**
         * Creates a 3D triangle vertex buffer (3 vertices).
         */
        void CreateTriangle3dVertexBuffer();

        /**
         * Destroys a 3D triangle vertex buffer (3 vertices).
         */
        void DestroyTriangle3dVertexBuffer();

        /**
         * Creates a quad vertex buffer (4 vertices).
         */
        void CreateQuadVertexBuffer();

        /**
         * Destroys a quad vertex buffer (4 vertices).
         */
        void DestroyQuadVertexBuffer();

        /**
         * Creates a text vertex buffer.
         */
        void CreateTextVertexBuffer();

        /**
         * Destroys a text vertex buffer.
         */
        void DestroyTextVertexBuffer();

        /**
         * The scene manager.
         */
        Ogre::SceneManager* scene_manager_;

        /**
         * The render system.
         */
        Ogre::RenderSystem* render_system_;

        /**
         * A 2D line render operation.
         */
        Ogre::RenderOperation line_render_operation_;

        /**
         * A 2D line vertex buffer (2 vertices).
         */
        Ogre::HardwareVertexBufferSharedPtr line_vertex_buffer_;

        /**
         * Maximum number of vertices for debug 2D lines.
         */
        unsigned int line_max_vertex_;

        /**
         * A 3D line render operation.
         */
        Ogre::RenderOperation line_3d_render_operation_;

        /**
         * A 3D line vertex buffer (2 vertices).
         */
        Ogre::HardwareVertexBufferSharedPtr line_3d_vertex_buffer_;

        /**
         * Maximum number of vertices for debug 3D lines.
         */
        unsigned int line_3d_max_vertex_;

        /**
         * A 3D triangle render operation.
         */
        Ogre::RenderOperation triangle_3d_render_operation_;

        /**
         * A 3D triangle vertex buffer (3 vertices).
         */
        Ogre::HardwareVertexBufferSharedPtr triangle_3d_vertex_buffer_;

        /**
         * Maximum number of vertices for debug 3D triangles.
         */
        unsigned int triangle_3d_max_vertex_;

        /**
         * A quad render operation.
         */
        Ogre::RenderOperation quad_render_operation_;

        /**
         * A quad vertex buffer (4 vertices).
         */
        Ogre::HardwareVertexBufferSharedPtr quad_vertex_buffer_;

        /**
         * Maximum number of vertices for debug quads.
         */
        unsigned int quad_max_vertex_;

        /**
         * A text render operation.
         */
        Ogre::RenderOperation text_render_operation_;

        /**
         * A text vertex buffer.
         *
         * @todo Confirm if it's 6 vertices per letter.
         */
        Ogre::HardwareVertexBufferSharedPtr text_vertex_buffer_;

        /**
         * Maximum number of vertices for debug text.
         *
         * @todo Confirm if it's 6 vertices per letter.
         */
        unsigned int text_max_vertex_;

        /**
         * Font to use for debug text.
         */
        Ogre::FontPtr font_;

        /**
         * Font height for the debug text.
         */
        int font_height_;

        /**
         * Alignment for debug text.
         */
        TextAlignment text_alignment_;

        /**
         * Material for a 2D object.
         */
        Ogre::MaterialPtr material_;

        /**
         * Material for a 3D object.
         */
        Ogre::MaterialPtr material_3d_;

        /**
         * Color for the debug object or text.
         */
        Ogre::ColourValue colour_;

        /**
         * The screen space.
         */
        bool screen_space_;

        /**
         * Z coordinate for the debug object.
         */
        float z_coordinate_;

        /**
         * For text fading. The text start fading from this distance.
         */
        float fade_start_square_;

        /**
         * For text fading. The text fully fades at this distance.
         */
        float fade_end_square_;
};

/**
 * Shortcut for a DebugDraw singleton.
 */
#define DEBUG_DRAW DebugDraw::getSingleton()

