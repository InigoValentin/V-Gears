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

#include <map>
#include <set>
#include <OgreRenderQueueListener.h>
#include <OgreRoot.h>
#include "formats/field/Background2DFile.h"
#include "Background2DAnimation.h"
#include "BackgroundPalette.h"
#include "BackgroundScroll.h"
#include "BackgroundShake.h"
#include "BackgroundTileBuffer.h"
#include "engine/entity/Entity.h"
#include "engine/script/ScriptManager.h"

/**
 * A field background
 */
class Background2D : public Ogre::RenderQueueListener, public BackgroundScrollTypes{

    public:

        typedef VGears::Blending  Blending;

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
         * @param[in] event Event.
         */
        void InputDebug(const VGears::Event& event);

        /**
         * Hides the background.
         *
         * Once called, the background will not be rendered until {@see Show()} is called. If the
         * background is already hidden, it will do nothing.
         */
        void Hide();

        /**
         * Shows the background.
         *
         * Once called, the background will start rendered until {@see Hide()} is called.
         */
        void Show();

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
         * @param[in] entity The entity to track.
         */
        void ScriptAutoScrollToEntity(Entity* entity);

        /**
         * Retrieves the entity currently being tracked for autoscroll.
         *
         * @return The entity currently being tracked, or nullptr if the background is not
         * currently scrolling to any entity.
         */
        Entity* GetAutoScrollEntity() const;

        /**
         * Checks if the camera is following an entity, scrolling to one, or has been placed by a
         * script.
         *
         * @return False if no script has told the camera what to do.
         */
        bool HasScrollControl() const;

        /**
         * Scrolls the background to the position of the playable character.
         *
         * Background2D::NONE centers the view instantly and resumes following the character. Any
         * other type moves the view over time, and leaves it in place once it arrives.
         *
         * @param[in] type Scroll type.
         * @param[in] seconds Scroll duration.
         */
        void ScriptScrollToPlayer(const SCROLL_TYPE type, const float seconds);

        /**
         * Scrolls the background to an entity, and keeps following it once it arrives.
         *
         * @param[in] entity Entity to scroll to.
         * @param[in] type Scroll type.
         * @param[in] seconds Scroll duration.
         */
        void ScriptScrollToEntity(Entity* entity, const SCROLL_TYPE type, const float seconds);

        /**
         * Updates the destination of the current scroll, if it is moving towards an entity.
         */
        void UpdateScrollTarget();

        /**
         * Retrieves the scroll position that centers an entity in the view.
         *
         * @param[in] entity Entity to center.
         * @return Scroll position in game internal screen coordinates.
         */
        const Ogre::Vector2 GetEntityScroll(const Entity* entity) const;

        /**
         * Scrolls the background to a position.
         *
         * @param[in] x X coordinate to scroll to.
         * @param[in] y Y coordinate to scroll to.
         * @param[in] type Scroll type.
         * @param[in] seconds Duration of the scroll.
         */
        void ScriptScrollToPosition(
          const float x, const float y, const SCROLL_TYPE type, const float seconds
        );

        /**
         * Waits for the scroll to complete. If no scroll is active, the script just continues.
         *
         * @return -1.
         */
        int ScriptScrollSync();

        /**
         * Offsets the background to a position.
         *
         * @param[in] x X coordinate to scroll to.
         * @param[in] y Y coordinate to scroll to.
         */
        void ScriptOffset(const float x, const float y);

        /**
         * Shows the tiles of a background area that are in a layer.
         *
         * Tiles can belong to a scripted area, and to the layers of it given by a bit mask: layer
         * N is the bit 1 << N. They are only drawn while one of their layers is on. Tiles that
         * don't belong to an area are always drawn. Areas start with all their layers off.
         *
         * @param[in] area Area ID.
         * @param[in] layer Layer of the area to turn on, from 0 to 31.
         */
        void ScriptAreaOn(const int area, const int layer);

        /**
         * Hides the tiles of a background area that are in a layer.
         *
         * @param[in] area Area ID.
         * @param[in] layer Layer of the area to turn off, from 0 to 31.
         */
        void ScriptAreaOff(const int area, const int layer);

        /**
         * Hides all the tiles of a background area, whatever their layer.
         *
         * @param[in] area Area ID.
         */
        void ScriptAreaClear(const int area);

        /**
         * Shakes the view.
         *
         * The shake is added to the scroll, and doesn't change it. Each axis is a chain of random
         * bounces of the given amplitude, one every given number of frames. Changing the
         * parameters of a shake that is running takes effect when the current bounce ends.
         *
         * @param[in] type 0 stops the shake and brings the view back, 1 shakes the X axis, 2 the
         * Y axis and 3 both.
         * @param[in] x_amplitude Amplitude of the bounces in the X axis, in original screen pixels.
         * @param[in] x_frames Frames between bounces in the X axis.
         * @param[in] y_amplitude Amplitude of the bounces in the Y axis, in original screen pixels.
         * @param[in] y_frames Frames between bounces in the Y axis.
         */
        /**
         * Sets the parallax of a background layer, from the distance the scripts give it.
         *
         * The layer scrolls at the speed of the background multiplied by the distance divided by
         * 4096 (the distance of the layers that don't move with it is 4096), so a layer far away
         * moves less than the foreground. That relation comes from the values the field scripts
         * use and has not been verified against the original game.
         *
         * @param[in] layer Layer of the background, from 0 to 3.
         * @param[in] distance Parallax distance of the layer.
         */
        void ScriptSetLayerParallax(const int layer, const int distance);

        /**
         * Sets the speed a background layer scrolls by itself, on top of the scroll of the
         * background.
         *
         * The speeds are given in 1/256 of a pixel of the original screen for each frame of the
         * original game (30 per second). That unit has not been verified against the original game.
         *
         * @param[in] layer Layer of the background, from 0 to 3.
         * @param[in] x Speed on the X axis.
         * @param[in] y Speed on the Y axis.
         */
        void ScriptSetLayerScroll(const int layer, const int x, const int y);

        void ScriptShake(
          const int type, const int x_amplitude, const int x_frames, const int y_amplitude,
          const int y_frames
        );

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
        SCROLL_TYPE GetScrollType() const;

        /**
         * Retrieves the total duration the current scroll action.
         *
         * @return Duration of the current scroll action.
         */
        float GetScrollSeconds() const;

        /**
         * Sets the time taken by the current scroll action.
         *
         * It represents the time the current scroll action has been going on for.
         *
         * @param[in] seconds Time taken by the current scroll action.
         */
        void SetScrollCurrentSeconds(const float seconds);

        /**
         * Retrieves the time taken by the current scroll action.
         *
         * It represents the time the current scroll action has been going on for.
         *
         * @return Time taken by the current scroll action.
         */
        float GetScrollCurrentSeconds() const;

        /**
         * Retrieves the scroll position in screen coordinates.
         *
         * @return Screen coordinates of the scroll.
         */
        const Ogre::Vector2 GetScreenScroll() const;

        /**
         * Sets the scroll in game internal screen coordinates.
         *
         * @param[in] position Position to set the scroll.
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
         * @param[in] image Image name.
         */
        void SetImage( const Ogre::String& image );

        /**
         * Loads the palettes of the image, if a script animates them.
         *
         * @param[in] name Resource name of the palette file (palette.bin of the field).
         */
        void SetPalette(const Ogre::String& name);

        /**
         * Palette opcodes of the scripts, see BackgroundPalette.
         */
        void ScriptPaletteStore(int page, int slot, int count);
        void ScriptPaletteLoad(int slot, int page, int count);
        void ScriptPaletteCopy(int source, int destination, int count);
        void ScriptPaletteMultiply(
          int source, int destination, int start, int count, int red, int green, int blue
        );
        void ScriptPaletteAdd(
          int source, int destination, int red, int green, int blue, int count
        );

        /**
         * Set the background scrolling range.
         *
         * The range is applied in game internal screen coordinates.
         *
         * @param[in] min_x Min scrollabe x coordinate.
         * @param[in] min_y Min scrollabe y coordinate.
         * @param[in] max_x Max scrollabe x coordinate.
         * @param[in] max_y Max scrollabe y coordinate.
         */
        void SetRange(const int min_x, const int min_y, const int max_x, const int max_y);

        /**
         * Set the background scrolling range.
         *
         * The range is applied in game internal screen coordinates.
         *
         * @param[in] range Range bounds vector.
         */
        void SetRange(const Ogre::Vector4& range);

        /**
         * Adds a tile to the background
         *
         * @param[in] x X coordinate for the tile.
         * @param[in] y Y coordinate for the tile.
         * @param[in] width Tile width.
         * @param[in] height Tile height.
         * @param[in] depth Depth of the tile.
         * @param[in] u1
         * @param[in] v1
         * @param[in] u2
         * @param[in] v2
         * @param[in] blending Blending mode for the tile
         * @param[in] area Scripted area the tile belongs to, or 0 if it always has to be drawn.
         * @param[in] state Bit mask with the layers of the area in which the tile is drawn.
         * @param[in] layer Layer of the original background the tile is from.
         * @param[in] intensity Factor of the colour of the tile (0.25 for the additive tiles that
         * only add a quarter of their colour).
         * @todo Depth is z-index??
         * @todo What are v1, v2, u1 and u2?
         */
        void AddTile(
          const int x, const int y, const int width, const int height, const float depth,
          const float u1, const float v1, const float u2, const float v2, const Blending blending,
          const int area = 0, const int state = 0, const int layer = 0,
          const float intensity = 1.0f
        );

        /**
         * Adds a tile to the background
         *
         * @param[in] destination Coordinates for the tile.
         * @param[in] width Tile width.
         * @param[in] height Tile height.
         * @param[in] depth Depth of the tile.
         * @param[in] uv
         * @param[in] blending Blending mode for the tile
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
         * @param[in] tile The tile to add.
         */
        void AddTile(const VGears::Tile& tile);

        /**
         * Converts the depth of a tile to the value that is written to the depth buffer.
         *
         * The depth is projected with the camera. The tiles that are the farthest (the background
         * layers) are at depths so close to each other (as 127.938 and 128) that their projected
         * values are closer than what the depth buffer can tell apart, and the order they are
         * drawn in decided what was in front. For them the order is kept with a linear mapping
         * right behind all the rest of what is drawn.
         *
         * @param[in] depth Depth of the tile.
         * @param[in] projection Projection matrix of the camera, with the depth range of the
         * render system.
         * @return The value for the depth buffer.
         */
        static float TileDepthToBuffer(const float depth, const Ogre::Matrix4& projection);

        /**
         * Updates the UV vector of a tile
         *
         * @param[in] tile_id ID of the tile to update.
         * @param[in] u1
         * @param[in] v1
         * @param[in] u2
         * @param[in] v2
         * @todo What are v1, v2, u1 and u2?
         */
        void UpdateTileUV(
          const unsigned int tile_id, const float u1, const float v1, const float u2, const float v2
        );

        /**
         * Adds an animation to the background.
         *
         * @param[in] animation Animation to add.
         */
        void AddAnimation(Background2DAnimation* animation);

        /**
         * Plays an animation.
         *
         * @param[in] animation The animation to play.
         * @param[in] state Animation state.
         */
        void PlayAnimation(const Ogre::String& animation, const Background2DAnimation::State state);

        /**
         * Plays an animation in a loop.
         *
         * The animation is played asynchronously.
         *
         * @param[in] name Animation name.
         */
        void ScriptPlayAnimationLooped(const char* name);

        /**
         * Plays an animation once, then stops.
         *
         * The animation is played asynchronously.
         *
         * @param[in] name Animation name.
         */
        void ScriptPlayAnimationOnce(const char* name);

        /**
         * Plays an animation once, then stops.
         *
         * The animation is played synchronously, and the thread is locked
         * until it ends.
         *
         * @param[in] name Animation name.
         */
        int ScriptAnimationSync(const char* name);

        /**
         * Ends the render queue.
         *
         * @param[in] queueGroupId The group id of the queue to end.
         * @param[in] invocation
         * @param[in] repeatThisInvocation Indicates if the invocation must be repeated.
         * @todo Understand and document.
         */
        void renderQueueEnded(
          Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation
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

            /**
             * Scripted area the tile belongs to, or 0 if it is always drawn.
             */
            int area;

            /**
             * Bit mask with the layers of the area in which the tile is drawn.
             */
            int state;

            /**
             * Whether the tile is drawn.
             */
            bool visible;
        };

        typedef std::vector<Tile> TileList;

        /**
         * Checks if any of the layers in a bit mask is on for an area.
         *
         * @param[in] area Area ID.
         * @param[in] layers Bit mask with layers of the area.
         * @return True if one of the layers is on.
         */
        bool IsAreaStateOn(const int area, const int layers) const;

        /**
         * Moves the shake forward in time, and applies it to the view if it changed.
         *
         * @param[in] delta Seconds since the last frame.
         */
        void UpdateShake(const float delta);

        /**
         * Draws or hides the tiles of an area according to its states.
         *
         * @param[in] area Area ID.
         */
        void RefreshArea(const int area);

        /**
         * Writes the position of a tile in its vertex buffer.
         *
         * A tile that is not visible is written collapsed, so that nothing is rasterised.
         *
         * @param[in] tile Tile to write.
         */
        void WriteTilePosition(const Tile& tile);

        /**
         * Loads a background.
         *
         * @param[in] background The background to load.
         */
        virtual void load(const VGears::Background2DFilePtr &background);

    protected:

        /**
         * Loads a tile list to the background.
         *
         * @param[in] tiles The list of tiles to load.
         */
        virtual void load(const VGears::Background2DFile::TileList &tiles);

        /**
         * Loads an animation list to the background.
         *
         * @param[in] tile_index The index of the tile.
         * @param[in] animations List of animations to load.
         */
        virtual void load( const size_t tile_index, const VGears::AnimationMap &animations);

        /**
         * Apply the camera position to match the current scroll.
         */
        virtual void applyScroll();

        /**
         * Calculates the screen scale.
         *
         * The scale is calculated as the proportion between the viewport and
         * the virtual screen.
         */
        virtual void calculateScreenScale();


        /**
         * Sets the virtual screen to world space.
         *
         * @param[in] pos
         * @todo Understand and document.
         */
        virtual void virtualScreenToWorldSpace(Ogre::Vector2 &pos) const;

    private:

        /**
         * The number of layers of a background.
         */
        static const int LAYER_COUNT = BackgroundLayerRanges::LAYER_COUNT;

        /**
         * Number of alpha and of add tiles that fit. Some fields have more than 1600 tiles that add
         * their colour (the fire and the light of las0_2, clsin2_1 and ujunon4), and more than 2500
         * tiles in all.
         */
        static const unsigned int MAX_TILES = 4096;

        /**
         * Number of subtract tiles that fit.
         */
        static const unsigned int MAX_SUBTRACT_TILES = 2048;

        /**
         * The alpha blended tiles.
         */
        BackgroundTileBuffer alpha_tiles_;

        /**
         * The tiles that add their colour.
         */
        BackgroundTileBuffer add_tiles_;

        /**
         * The tiles that subtract their colour.
         */
        BackgroundTileBuffer subtract_tiles_;

        /**
         * The scroll of the view.
         */
        BackgroundScroll scroll_;

        /**
         * The shake of the view.
         */
        BackgroundShake shake_;

        /**
         * Retrieves the buffer of the tiles with a blending mode.
         *
         * @param[in] blending The blending mode.
         * @return The buffer, or null if the mode is unknown.
         */
        BackgroundTileBuffer* BufferFor(const Blending blending);

        /**
         * Calculates where the corners of a tile are.
         *
         * @param[in] tile The tile.
         * @return The corners, in world space.
         */
        QuadWriter::Corners TileCorners(const Tile& tile) const;

        /**
         * Moves forward the animations that are playing.
         *
         * @param[in] delta Seconds since the last frame.
         */
        void UpdateAnimations(const float delta);

        /**
         * Draws the tiles of a buffer, each layer scrolled as it has to be.
         *
         * @param[in,out] buffer Buffer to draw.
         * @param[in] rs_params Parameters of the render system.
         * @param[in] width Width of the viewport.
         * @param[in] height Height of the viewport.
         */
        void RenderBuffer(
          BackgroundTileBuffer& buffer, Ogre::GpuProgramParametersPtr& rs_params, const float width,
          const float height
        );

        /**
         * Moves the view the tiles are drawn with.
         *
         * @param[in] position Scroll of the view, in pixels of the screen.
         * @param[in] rs_params Parameters of the render system.
         * @param[in] width Width of the viewport.
         * @param[in] height Height of the viewport.
         */
        void SetView(
          const Ogre::Vector2& position, Ogre::GpuProgramParametersPtr& rs_params, const float width,
          const float height
        );

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
         * The layers that are on for each scripted area, as a bit mask.
         */
        std::map<int, unsigned int> area_states_on_;

        /**
         * Painting again of the tiles of the palettes the scripts animate.
         */
        void UpdatePalette();

        /**
         * The palettes the scripts animate.
         */
        BackgroundPalette palette_;

        /**
         * Name of the image of the background.
         */
        Ogre::String image_name_;

        /**
         * A copy of the image of the background, where the animated tiles are painted.
         */
        Ogre::Image palette_image_;

        /**
         * True if palette_image_ has the image.
         */
        bool palette_image_loaded_ = false;

        /**
         * The shake being applied to the scroll, in virtual screen units.
         */
        Ogre::Vector2 shake_offset_ = Ogre::Vector2::ZERO;

        /**
         * Alpha blending render operation.
         */
        /**
         * Scroll of each layer relative to the one of the background.
         */
        float layer_parallax_[LAYER_COUNT][2];

        /**
         * Speed at which each layer scrolls by itself, in pixels of the virtual screen per second.
         */
        float layer_scroll_speed_[LAYER_COUNT][2];

        /**
         * Distance each layer has scrolled by itself, in pixels of the virtual screen.
         */
        float layer_scroll_offset_[LAYER_COUNT][2];

        /**
         * Sets the layers to the state they have when a background is loaded.
         */
        void ResetLayers();

        /**
         * If any layer is not scrolled as the background is.
         */
        bool HasLayerScroll() const;

        /**
         * The current background offset, virtual screen size.
         */
        Ogre::Vector2 offset_;

        /**
         * The current scroll position, viewport size.
         *
         * Includes the offset.
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
