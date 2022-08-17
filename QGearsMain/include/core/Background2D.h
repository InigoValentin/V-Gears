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
#include "map/QGearsBackground2DFile.h"
#include "Background2DAnimation.h"
#include "Entity.h"
#include "ScriptManager.h"

/**
 * A field background
 */
class Background2D : public Ogre::RenderQueueListener{

    public:

        typedef QGears::Blending  Blending;

        /**
         * How to scroll the background.
         */
        enum ScrollType{

            /**
             * Don't scroll the background.
             */
            NONE,

            /**
             * Linearly scroll.
             *
             * May seem unnatural in scripted sequences, but it's OK for
             * character tracking.
             */
            LINEAR,
            /**
             * Smooth (soft in, soft out) scroll.
             *
             * Best for scripted sequences.
             */
            SMOOTH
        };

        /**
         * Constructor.
         */
        Background2D();

        /**
         * Destructor
         */
        virtual ~Background2D();

        /**
         * @todo Understand and document.
         * @param event[in] Event.
         */
        void InputDebug(const QGears::Event& event);

        /**
         * Run each frame.
         *
         * Updates animations and changes in the background.
         */
        void Update();

        /**
         * Debug inormation about changes in the background.
         */
        void UpdateDebug();

        /**
         * Called on window resize.
         *
         * Resizes the background, keeping the ratio.
         */
        void OnResize();

        /**
         * Removes the background and it's animations.
         */
        void Clear();

        /**
         * Flags the background to automatically scroll to an entity position.
         *
         * @param entity The entity to track.
         */
        void ScriptAutoScrollToEntity(Entity* entity);

        /**
         * Retrieves the entity currently being tracked for autoscroll.
         *
         * @return The entity currently being tracked, or nullptr if the
         * background is not currently scrolling to any entity.
         */
        Entity* GetAutoScrollEntity() const;

        /**
         * Scrolls the background to a position.
         *
         * @param x[in] X coordinate to scroll to.
         * @param y[in] Y coordinate to scroll to.
         * @param type[in] Scroll type.
         * @param seconds[in] Duration of the scroll.
         */
        void ScriptScrollToPosition(
          const float x, const float y,
          const ScrollType type, const float seconds
        );

        /**
         * Waits for the scroll to complete.
         *
         * @return -1.
         */
        int ScriptScrollSync();

        /**
         * Stops the current scrolling.
         */
        void UnsetScroll();

        /**
         * Retrieves the initial position of the current scroll action.
         *
         * @return The initial position of the current scroll action.
         */
        const Ogre::Vector2& GetScrollPositionStart() const;

        /**
         * Retrieves the final position of the current scroll action.
         *
         * @return The final position of the current scroll action.
         */
        const Ogre::Vector2& GetScrollPositionEnd() const;

        /**
         * Retrieves the type of the current scroll action.
         *
         * @return The type of the current scroll action.
         */
        ScrollType GetScrollType() const;

        /**
         * Retrieves the total duration the current scroll action.
         *
         * @return Duration of the current scroll action.
         */
        float GetScrollSeconds() const;

        /**
         * Sets the time taken by the current scroll action.
         *
         * It represents the time the current scroll action has been going on
         * for.
         *
         * @param seconds[in] Time taken by the current scroll action.
         */
        void SetScrollCurrentSeconds(const float seconds);

        /**
         * Retrieves the time taken by the current scroll action.
         *
         * It represents the time the current scroll action has been going on
         * for.
         *
         * @return Time taken by the current scroll action.
         */
        float GetScrollCurrentSeconds() const;

        /**
         * Scroll position in screen coordinates.
         *
         * @param position[in] Position to set the scroll.
         */
        void SetScreenScroll(const Ogre::Vector2& position);

        /**
         * Retrieves the scroll position in screen coordinates.
         *
         * @return Screen coordinates of the scroll.
         */
        const Ogre::Vector2 GetScreenScroll() const;

        /**
         * Sets the scroll in game internal screen coordinates.
         *
         * @param position[in] Position to set the scroll.
         */
        void SetScroll(const Ogre::Vector2& position);

        /**
         * Retrieves the scroll position in game internal screen coordinates.
         *
         * @return Screen coordinates of the scroll.
         */
        const Ogre::Vector2& GetScroll() const;

        /**
         * Sets the background image.
         *
         * @param image[in] Image name.
         */
        void SetImage( const Ogre::String& image );

        /**
         * Set the background scrolling range.
         *
         * The range is applied in game internal screen coordinates.
         *
         * @param min_x[in] Min scrollabe x coordinate.
         * @param min_y[in] Min scrollabe y coordinate.
         * @param max_x[in] Max scrollabe x coordinate.
         * @param max_y[in] Max scrollabe y coordinate.
         */
        void SetRange(
          const int min_x, const int min_y, const int max_x, const int max_y
        );

        /**
         * Set the background scrolling range.
         *
         * The range is applied in game internal screen coordinates.
         *
         * @param range[in] Range bounds vector.
         */
        void SetRange(const Ogre::Vector4& range);

        /**
         * Adds a tile to the background
         *
         * @param x[in] X coordinate for the tile.
         * @param y[in] Y coordinate for the tile.
         * @param width[in] Tile width.
         * @param height[in] Tile height.
         * @param depth[in] Depth of the tile.
         * @param u1[in]
         * @param v1[in]
         * @param u2[in]
         * @param v2[in]
         * @param blending[in] Blending mode for the tile
         * @todo Depth is z-index??
         * @todo What are v1, v2, u1 and u2?
         */
        void AddTile(
          const int x, const int y, const int width, const int height,
          const float depth, const float u1, const float v1,
          const float u2, const float v2, const Blending blending
        );

        /**
         * Adds a tile to the background
         *
         * @param destination[in] Coordinates for the tile.
         * @param width[in] Tile width.
         * @param height[in] Tile height.
         * @param depth[in] Depth of the tile.
         * @param uv[in]
         * @param blending[in] Blending mode for the tile
         * @todo Depth is z-index??
         * @todo What is uv?
         */
        void AddTile(
          const Ogre::Vector2& destination, const int width, const int height,
          const float depth, const Ogre::Vector4& uv, const Blending blending
        );

        /**
         * Adds a tile to the background
         *
         * @param tile[in] The tile to add.
         */
        void AddTile(const QGears::Tile& tile);

        /**
         * Updates the UV vector of a tile
         *
         * @param tile_id[in] ID of the tile to update.
         * @param u1[in]
         * @param v1[in]
         * @param u2[in]
         * @param v2[in]
         * @todo What are v1, v2, u1 and u2?
         */
        void UpdateTileUV(
          const unsigned int tile_id,
          const float u1, const float v1, const float u2, const float v2
        );

        /**
         * Adds an animation to the background.
         *
         * @param animation[in] Animation to add.
         */
        void AddAnimation(Background2DAnimation* animation);

        /**
         * Plays an animation.
         *
         * @param animation[in] The animation to play.
         * @param state[in] Animation state.
         */
        void PlayAnimation(
          const Ogre::String& animation, const Background2DAnimation::State state
        );

        /**
         * Plays an animation in a loop.
         *
         * The animation is played asynchronously.
         *
         * @param name[in] Animation name.
         */
        void ScriptPlayAnimationLooped(const char* name);

        /**
         * Plays an animation once, then stops.
         *
         * The animation is played asynchronously.
         *
         * @param name[in] Animation name.
         */
        void ScriptPlayAnimationOnce(const char* name);

        /**
         * Plays an animation once, then stops.
         *
         * The animation is played synchronously, and the thread is locked
         * until it ends.
         *
         * @param name[in] Animation name.
         */
        int ScriptAnimationSync(const char* name);

        /**
         * Ends the render queue.
         *
         * @param queueGroupId[in] The group id of the queue to end.
         * @param invocation[in]
         * @param repeatThisInvocation[in]
         * @todo Understand and document.
         */
        void renderQueueEnded(
          Ogre::uint8 queueGroupId, const Ogre::String& invocation,
          bool& repeatThisInvocation
        ) override;

        /**
         * Represents a tile.
         */
        struct Tile{

            /**
             * Tile X coordinate.
             */
            int x;

            /**
             * Tile Y coordinate.
             */
            int y;

            /**
             * Tile width.
             */
            int width;

            /**
             * Tile height.
             */
            int height;

            /**
             * @todo
             */
            size_t start_vertex_index;

            /**
             * Tile blending mode.
             */
            Blending blending;
        };

        typedef std::vector<Tile> TileList;

        /**
         * Loads a background.
         *
         * @param background[in] The background to load.
         */
        virtual void load(const QGears::Background2DFilePtr &background);

    protected:

        /**
         * Loads a tile list to the background.
         *
         * @param tiles[in] The list of tiles to load.
         */
        virtual void load(const QGears::Background2DFile::TileList &tiles);

        /**
         * Loads an animation list to the background.
         *
         * @param tiles_index[in]
         * @param animations[in] List of animations to load.
         */
        virtual void load(
          const size_t tile_index, const QGears::AnimationMap &animations
        );

        /**
         * Apply the camera position to match the current scroll.
         */
        virtual void applyScroll(void);

        /**
         * Calculates the screen scale.
         *
         * The scale is calculated as the proportion between the viewport and
         * the virtual screen.
         */
        virtual void calculateScreenScale(void);


        /**
         * Sets the virtual screen to world space.
         *
         * @param pos[in]
         * @todo Understand and document.
         */
        virtual void virtualScreenToWorldSpace(Ogre::Vector2 &pos) const;

        /**
         * Tile vertex properties.
         */
        enum{

            /**
             * Tile vertex count.
             *
             * @todo Understand and document.
             */
            TILE_VERTEX_COUNT = 6,

            /**
             * Tile vertex index size.
             */
            TILE_VERTEX_INDEX_SIZE = TILE_VERTEX_COUNT + 3
        };

    private:

        /**
         * Creates all vertex buffers.
         */
        void CreateVertexBuffers();

        /**
         * Destroys all vertex buffers.
         */
        void DestroyVertexBuffers();

        /**
         * The scene manager.
         */
        Ogre::SceneManager* scene_manager_;

        /**
         * The render system.
         */
        Ogre::RenderSystem* render_system_;

        /**
         * The list of tiles in the background.
         */
        TileList tiles_;

        /**
         * Alpha blending render operation.
         */
        Ogre::RenderOperation alpha_render_op_;

        /**
         * Alpha blending vertex buffer.
         */
        Ogre::HardwareVertexBufferSharedPtr alpha_vertex_buffer_;

        /**
         * Alpha blending max vertex count.
         */
        unsigned int alpha_max_vertex_count_;

        /**
         * Alpha blending material.
         */
        Ogre::MaterialPtr alpha_material_;

        /**
         * Add blending render operation.
         */
        Ogre::RenderOperation add_render_op_;

        /**
         * Add blending vertex buffer.
         */
        Ogre::HardwareVertexBufferSharedPtr add_vertex_buffer_;

        /**
         * Add blending max vertex count.
         */
        unsigned int add_max_vertex_count_;

        /**
         * Add blending material.
         */
        Ogre::MaterialPtr add_material;

        /**
         * Substract blending render operation.
         */
        Ogre::RenderOperation subtract_render_op_;

        /**
         * Substract blending vertex buffer.
         */
        Ogre::HardwareVertexBufferSharedPtr subtract_vertex_buffer_;

        /**
         * Substract blending max vertex count.
         */
        unsigned int subtract_max_vertex_count_;

        /**
         * Substract blending material.
         */
        Ogre::MaterialPtr subtract_material_;

        /**
         * The entity to keep track of with the scroll.
         */
        Entity* scroll_entity_;

        /**
         * Starting position of the current scroll action.
         */
        Ogre::Vector2 scroll_position_start_;

        /**
         * Final position of the current scroll action.
         */
        Ogre::Vector2 scroll_position_end_;

        /**
         * Type of the current scroll action.
         */
        ScrollType scroll_type_;

        /**
         * Total duration of the current scroll action, in seconds
         */
        float scroll_seconds_;

        /**
         * Secund the currens scroll action has taken so far.
         */
        float scroll_current_seconds_;

        /**
         * @todo Document.
         */
        std::vector<ScriptId> scroll_sync_;

        /**
         * The current scroll position, virtual screen size.
         */
        Ogre::Vector2 position_;

        /**
         * The current scroll position, viewport size.
         */
        Ogre::Vector2 position_real_;

        /**
         * The scale of the screen
         */
        Ogre::Real screen_scale_;

        /**
         * Screen aspect ration.
         */
        Ogre::Vector2 screen_proportion_;

        /**
         * The size of the virtual screen.
         */
        Ogre::Vector2 virtual_screen_size_;

        /**
         * @todo Document.
         */
        Ogre::AxisAlignedBox range_;

        /**
         * State of an animation.
         */
        struct AnimationPlayed{

            /**
             * Animation name.
             */
            Ogre::String name;

            /**
             * @todo Document
             */
            std::vector<ScriptId> sync;

            /**
             * Animation state.
             */
            Background2DAnimation::State state;
        };

        /**
         * @todo Document.
         */
        std::vector<AnimationPlayed> animation_played_;

        /**
         * List of animations.
         */
        std::vector<Background2DAnimation*> animations_;
};


