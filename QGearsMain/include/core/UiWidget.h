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

#include <OgreColourValue.h>
#include <OgreString.h>
#include <Ogre.h>
#include <vector>
#include "ScriptManager.h"
#include "UiAnimation.h"

/**
 * An UI widget.
 */
class UiWidget{

    public:

        /**
         * Constructor.
         *
         * @param name[in] Name for the widget.
         */
        UiWidget(const Ogre::String& name);

        /**
         * Constructor.
         *
         * @param name[in] Name for the widget.
         * @param path_name[in] @todo Understand and document.
         * @param parent[in] Widget to be made parent of the new one.
         */
        UiWidget(
          const Ogre::String& name, const Ogre::String& path_name,
          UiWidget* parent
        );

        /**
         * Destructor.
         */
        virtual ~UiWidget();

        /**
         * Initializes the widget.
         *
         * It sets the widget geometry.
         */
        void Initialise();

        /**
         * Updates the widget status.
         */
        virtual void Update();

        /**
         * Handles resizing events.
         *
         * Recalculates the widget geometry and calls OnResize for all the
         * widget children.
         */
        void OnResize();

        /**
         * Renders the widget and it's children.
         */
        virtual void Render();

        /**
         * Toggles the widget visibility.
         *
         * @param vivible[in] True to make the widget visible, false to make it
         * invisible.
         */
        void SetVisible(const bool visible);

        /**
         * Checks the widget visibility.
         *
         * @return True if the widget is visible, false if it is invisible.
         */
        bool IsVisible() const;

        /**
         * Retrieves the widget name.
         *
         * @return The widget name.
         */
        const Ogre::String& GetName() const;

        /**
         * Add other widget as the widget's child.
         *
         * @param widget[in] Widget to be made children of this one.
         */
        void AddChild(UiWidget* widget);

        /**
         * Retrieves a children by name.
         *
         * @param name[in] Name of the children widget to retrieve.
         * @return The children widget by the specified name. NULL if the
         * widgets has no direct children by that name.
         */
        UiWidget* GetChild(const Ogre::String& name);

        /**
         * Retrieves a children by ID.
         *
         * @param name[in] ID of the children widget to retrieve.
         * @return The children widget by the specified name. NULL if the
         * widgets has no direct children by that ID.
         */
        UiWidget* GetChild(const unsigned int id);

        /**
         * Counts the widget's children.
         *
         * @return The number of direct children.
         */
        unsigned int GetNumberOfChildren();

        /**
         * Removes all children widgets.
         */
        void RemoveAllChildren();

        /**
         * Adds an animation to the widget.
         *
         * @param animation[in] The animation to add.
         */
        void AddAnimation(UiAnimation* animation);

        /**
         * Retrieves the current animation name.
         *
         * @return The current animation name. A blankstring if there is no set
         * default animation.
         */
        const Ogre::String& GetCurrentAnimationName() const;

        /**
         * Retrieves the current animation state.
         *
         * @return The current animation state.
         */
        UiAnimation::State GetAnimationState() const;

        /**
         * Plays an animation.
         *
         * @param animation[in] Animation to play.
         * @param state[in] Initial animation state.
         * @param start[in] Seconds at which to start the animation.
         * @param start[in] Seconds at which to end the animation. -1 to play
         * it in full.
         */
        void PlayAnimation(
          const Ogre::String& animation, UiAnimation::State state,
          const float start, const float end
        );

        /**
         * Plays an animation.
         *
         * The animation will be played from the start for it's full length.
         *
         * @param animation[in] The name of the animation to play.
         * @todo It uses the parameter UiAnimation::DEFAULT. Whats does it do?
         * Does it play the animation in a loop, or uses a default value of the
         * specific animation.
         */
        void ScriptPlayAnimation(const char* name);

        /**
         * Plays an animation.
         *
         * The animation will be played from the start for it's full length,
         * only once.
         *
         * @param animation[in] The name of the animation to play.
         */
        void ScriptPlayAnimationStop(const char* name);

        /**
         * @param name[in] Name of the animation to play.
         * @param start[in] Seconds at which to start the animation.
         * @param start[in] Seconds at which to end the animation. -1 to play
         * it in full.
         * @todo It uses the parameter UiAnimation::DEFAULT. Whats does it do?
         * Does it play the animation in a loop, or uses a default value of the
         * specific animation.
         */
        void ScriptPlayAnimation(
          const char* name, const float start, const float end
        );

        /**
         * Plays an animation.
         *
         * The animation will be played only once.
         *
         * @param animation[in] The name of the animation to play.
         */
        void ScriptPlayAnimationStop(
          const char* name, const float start, const float end
        );

        /**
         * Sets the default animation for the widget.
         *
         * @param animation[in] The name of the default animation.
         */
        void ScriptSetDefaultAnimation(const char* animation);

        /**
         * Synchronizes an animation.
         *
         * @return Always -1.
         * @todo Understand and document better.
         */
        int ScriptAnimationSync();

        /**
         * Marks the widget as transformed.
         *
         * It means that it's geometry must be updated in the next pass. It
         * also marks the direct children as transformed.
         */
        void SetUpdateTransformation();

        /**
         * Recalculates the widget geometry.
         */
        virtual void UpdateTransformation();

        /**
         * Horizontal alignment of the widget.
         */
        enum Align{

            /**
             * Left alignment.
             */
            LEFT,

            /**
             * Right alignment.
             */
            RIGHT,

            /**
             * Centered horizontally.
             */
            CENTER
        };

        /**
         * Vertical alignment of the widget.
         */
        enum VerticalAlign{

            /**
             * Top alignment.
             */
            TOP,

            /**
             * Bottom alignment.
             */
            BOTTOM,

            /**
             * Centered vertically.
             */
            MIDDLE
        };

        /**
         * Sets the widget horizontal alignment.
         *
         * @poaram align[in] Widget alignment.
         */
        void SetAlign(const UiWidget::Align align);

        /**
         * Sets the widget vertical alignment.
         *
         * @poaram align[in] Widget alignment.
         */
        void SetVerticalAlign(const UiWidget::VerticalAlign valign);

        /**
         * Retrieves the final Z-index of the widget after a transformation.
         *
         * @return The final Z-index.
         */
        float GetFinalZ() const;

        /**
         * Retrieves the coordinates of the widget after a transformation.
         *
         * @return The final coordinater.
         */
        Ogre::Vector2 GetFinalOrigin() const;

        /**
         * Retrieves the final translation of the widget after a
         * transformation.
         *
         * @return The final translation.
         * @todo What is a translation here? Just a movement?
         */
        Ogre::Vector2 GetFinalTranslate() const;

        /**
         * Retrieves the final size of the widget after a transformation.
         *
         * @return The final size.
         */
        Ogre::Vector2 GetFinalSize() const;

        /**
         * Retrieves the final scale of the widget after a transformation.
         *
         * @return The final scale.
         */
        Ogre::Vector2 GetFinalScale() const;

        /**
         * Retrieves the final scissor of the widget after a transformation.
         *
         * @return The final scissor.
         * @todo What is a scissor?
         */
        Ogre::Vector4 GetFinalScissor(bool& scissor) const;
        float GetFinalRotation() const;

        /**
         * Sets the X coordinate origin for the widget.
         *
         * @param percent[in] @todo
         * @param x[in] Origin X coordinate.
         */
        void SetOriginX(const float percent, const float x);

        /**
         * Sets the Y coordinate origin for the widget.
         *
         * @param percent[in] @todo
         * @param y[in] Origin Y coordinate.
         */
        void SetOriginY(const float percent, const float y);

        /**
         * Sets the X coordinate for the widget.
         *
         * @param percent[in] @todo
         * @param x[in] The X coordinate.
         */
        void SetX(const float percent, const float x);

        /**
         * Retrieves the X coordinate for the widget.
         *
         * @param percent[out] @todo
         * @param x[out] The X coordinate will be loaded here.
         */
        void GetX(float& percent, float& x);

        /**
         * Sets the Y coordinate for the widget.
         *
         * @param percent[in] @todo
         * @param y[in] The Y coordinate.
         */
        void SetY(const float percent, const float y);

        /**
         * Retrieves the Y coordinate for the widget.
         *
         * @param percent[out] @todo
         * @param y[out] The Y coordinate will be loaded here.
         */
        void GetY(float& percent, float& y);

        /**
         * Sets the widget Z-index.
         *
         * @param z[in] The Z-index.
         */
        void SetZ(const float z);

        /**
         * Sets the width the widget.
         *
         * @param percent[in] @todo
         * @param width[in] The widget width.
         */
        void SetWidth(const float percent, const float width);

        /**
         * Retrieves the width of the widget.
         *
         * @param percent[out] @todo
         * @param width[out] The width will be loaded here.
         */
        void GetWidth(float& percent, float& width);

        /**
         * Sets the height the widget.
         *
         * @param percent[in] @todo
         * @param height[in] The widget height.
         */
        void SetHeight(const float percent, const float height);

        /**
         * Retrieves the height of the widget.
         *
         * @param percent[out] @todo
         * @param height[out] The height will be loaded here.
         */
        void GetHeight(float& percent, float& height);

        /**
         * Sets the widget scale.
         *
         * @param scale[in] The new widget scale.
         */
        void SetScale(const Ogre::Vector2& scale);

        /**
         * Sets the widget rotation.
         *
         * @param scale[in] The new widget rotation.
         */
        void SetRotation(const float degree);

        /**
         * Sets the widget scissor area.
         *
         * @param percent_x1[in] @todo Undersnd and document.
         * @param x1[in] @todo Undersnd and document.
         * @param percent_y1[in] @todo Undersnd and document.
         * @param y1[in] @todo Undersnd and document.
         * @param percent_x2[in] @todo Undersnd and document.
         * @param x2[in] @todo Undersnd and document.
         * @param percent_y2[in] @todo Undersnd and document.
         * @param y2[in] @todo Undersnd and document.
         */
        void SetScissorArea(
          const float percent_x1, const float x1, const float percent_y1,
          const float y1, const float percent_x2, const float x2,
          const float percent_y2, const float y2
        );

        /**
         * @todo Understand and document.
         *
         * @param global[in] @todo Understand and document.
         */
        void SetGlobalScissor(const bool global);

        /**
         * Sets the widget colour.
         *
         * It set the same colour for all corners.
         *
         * @param r[in] Colour red component (0-255).
         * @param g[in] Colour green component (0-255).
         * @param b[in] Colour blue component (0-255).
         */
        void SetColour(const float r, const float g, const float b);

        /**
         * Sets the widget colour.
         *
         * It set the same colours for each corner, and blends it in the
         * interior.
         *
         * @param r1[in] Colour red component for the top-left corner (0-255).
         * @param g1[in] Colour green component for the top-left corner
         * (0-255).
         * @param b1[in] Colour blue component for the top-left corner (0-255).
         * @param r2[in] Colour red component for the top-right corner (0-255).
         * @param g2[in] Colour green component for the top-right corner
         * (0-255).
         * @param b2[in] Colour blue component for the top-right corner (0-255).
         * @param r3[in] Colour red component for the bottom-right corner (0-255).
         * @param g3[in] Colour green component for the bottom-right corner
         * (0-255).
         * @param b3[in] Colour blue component for the bottom-right corner
         * (0-255).
         * @param r4[in] Colour red component for the bottom-left corner
         * (0-255).
         * @param g4[in] Colour green component for the bottom-left corner
         * (0-255).
         * @param b4[in] Colour blue component for the bottom-left corner
         * (0-255).
         */
        void SetColours(
          const float r1, const float g1, const float b1,
          const float r2, const float g2, const float b2,
          const float r3, const float g3, const float b3,
          const float r4, const float g4, const float b4
        );

        /**
         * Sets the widget transparency.
         *
         * @param a[in] Alpha value (0 opaque, 255 fully transparent).
         */
        void SetAlpha(const float a);

    protected:

        /**
         * The widget name.
         */
        Ogre::String name_;

        /**
         * @todo Understand and document.
         */
        Ogre::String path_name_;

        /**
         * The parent widget.
         */
        UiWidget* parent_;

        /**
         * The list of children widgets.
         */
        std::vector<UiWidget*> children_;

        /**
         * Width, in game screen units.
         *
         * @todo Understand and document.
         */
        float screen_width_;

        /**
         * Height, in game screen units.
         *
         * @todo Understand and document.
         */
        float screen_height_;

        /**
         * Indicates if the widget is visible.
         */
        bool visible_;

        /**
         * Top-left corner colour.
         */
        Ogre::ColourValue colour_1_;

        /**
         * Top-right corner colour.
         */
        Ogre::ColourValue colour_2_;

        /**
         * Bottom-right corner colour.
         */
        Ogre::ColourValue colour_3_;

        /**
         * Bottom-left corner colour.
         */
        Ogre::ColourValue colour_4_;

        /**
         * The vertical alignment.
         */
        Align align_;

        /**
         * The evrtical alignment.
         */
        VerticalAlign vertical_align_;

        /**
         * Indicates if the widget is marked as transformed.
         */
        bool update_transformation_;

        /**
         * Final origin point after a transformation.
         */
        Ogre::Vector2 final_origin_;

        /**
         * Final translation after a transformation.
         *
         * @todo Understand and document.
         */
        Ogre::Vector2 final_translate_;

        /**
         * Final Z-index after a transformation.
         */
        float final_z_;

        /**
         * Final size after a transformation.
         */
        Ogre::Vector2 final_size_;

        /**
         * Final scale after a transformation.
         */
        Ogre::Vector2 final_scale_;

        /**
         * Final rotation after a transformation.
         */
        float final_rotation_;

        /**
         * @todo Understand and document.
         */
        float origin_x_percent_;

        /**
         * The origin X coordinate.
         */
        float origin_x_;

        /**
         * @todo Understand and document.
         */
        float origin_y_percent_;

        /**
         * The origin Y coordinate.
         */
        float origin_y_;

        /**
         * @todo Understand and document.
         */
        float x_percent_;

        /**
         * The widget X coordinate.
         */
        float x_;

        /**
         * @todo Understand and document.
         */
        float y_percent_;

        /**
         * The widget Y coordinate.
         */
        float y_;

        /**
         * The widget Z coordinate.
         */
        float z_;

        /**
         * @todo Understand and document.
         */
        float width_percent_;

        /**
         * The widget width.
         */
        float width_;

        /**
         * @todo Understand and document.
         */
        float height_percent_;

        /**
         * The widget height.
         */
        float height_;

        /**
         * The widget scale.
         */
        Ogre::Vector2 scale_;

        /**
         * The widget rotation.
         */
        float rotation_;

        /**
         * @todo Understand and document.
         */
        bool scissor_;

        /**
         * @todo Understand and document.
         */
        bool local_scissor_;

        /**
         * @todo Understand and document.
         */
        bool global_scissor_;

        /**
         * @todo Understand and document.
         */
        int scissor_top_;

        /**
         * @todo Understand and document.
         */
        float scissor_x_percent_top_;

        /**
         * @todo Understand and document.
         */
        float scissor_x_top_;

        /**
         * @todo Understand and document.
         */
        int scissor_bottom_;

        /**
         * @todo Understand and document.
         */
        float scissor_x_percent_bottom_;

        /**
         * @todo Understand and document.
         */
        float scissor_x_bottom_;

        /**
         * @todo Understand and document.
         */
        int scissor_left_;

        /**
         * @todo Understand and document.
         */
        float scissor_y_percent_left_;

        /**
         * @todo Understand and document.
         */
        float scissor_y_left_;

        /**
         * @todo Understand and document.
         */
        int scissor_right_;

        /**
         * @todo Understand and document.
         */
        float scissor_y_percent_right_;

        /**
         * @todo Understand and document.
         */
        float scissor_y_right_;

        /**
         * The current animation.
         */
        UiAnimation* animation_current_;

        /**
         * The animation stack.
         *
         * @todo Understand and document better.
         */
        std::vector<ScriptId> animation_sync_;

        /**
         * The current animation state.
         */
        UiAnimation::State animation_state_;

        /**
         * The name of the default animation.
         */
        Ogre::String animation_default_;

        /**
         * Time for the current animation to end.
         */
        float animation_end_time_;

        /**
         * The list of animations.
         */
        std::vector<UiAnimation*> animations_;

    private:

        /**
         * Constructor.
         */
        UiWidget();
};

