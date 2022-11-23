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
         * @param[in] name Name for the widget.
         */
        UiWidget(const Ogre::String& name);

        /**
         * Constructor.
         *
         * @param[in] name Name for the widget.
         * @param[in] path_name @todo Understand and document.
         * @param[in] parent Widget to be made parent of the new one.
         */
        UiWidget(const Ogre::String& name, const Ogre::String& path_name, UiWidget* parent);

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
         * Recalculates the widget geometry and calls OnResize for all the widget children.
         */
        void OnResize();

        /**
         * Renders the widget and it's children.
         */
        virtual void Render();

        /**
         * Toggles the widget visibility.
         *
         * @param[in] visible True to make the widget visible, false to make it invisible.
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
         * @param[in] widget Widget to be made children of this one.
         */
        void AddChild(UiWidget* widget);

        /**
         * Retrieves a children by name.
         *
         * @param[in] name Name of the children widget to retrieve.
         * @return The children widget by the specified name. NULL if the widgets has no direct
         * children by that name.
         */
        UiWidget* GetChild(const Ogre::String& name);

        /**
         * Retrieves a children by ID.
         *
         * @param[in] id ID of the children widget to retrieve.
         * @return The children widget by the specified name. NULL if the widgets has no direct
         * children by that ID.
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
         * @param[in] animation The animation to add.
         */
        void AddAnimation(UiAnimation* animation);

        /**
         * Retrieves the current animation name.
         *
         * @return The current animation name. A blankstring if there is no animation.
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
         * @param[in] animation Animation to play.
         * @param[in] state Initial animation state.
         * @param[in] start Seconds at which to start the animation.
         * @param[in] end Seconds at which to end the animation. -1 to play it in full.
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
         * @param[in] name The name of the animation to play.
         * @todo It uses the parameter UiAnimation::DEFAULT. What does it do? Does it play the
         * animation in a loop, or uses a default value of the specific animation.
         */
        void ScriptPlayAnimation(const char* name);

        /**
         * Plays an animation.
         *
         * The animation will be played from the start for it's full length, only once.
         *
         * @param[in] name The name of the animation to play.
         */
        void ScriptPlayAnimationStop(const char* name);

        /**
         * @param[in] name Name of the animation to play.
         * @param[in] start Seconds at which to start the animation.
         * @param[in] end Seconds at which to end the animation. -1 to play it in full.
         * @todo It uses the parameter UiAnimation::DEFAULT. Whats does it do? Does it play the
         * animation in a loop, or uses a default value of the specific animation.
         */
        void ScriptPlayAnimation(const char* name, const float start, const float end);

        /**
         * Plays an animation.
         *
         * The animation will be played only once.
         *
         * @param[in] name The name of the animation to play.
         * @param[in] start Seconds at which to start the animation.
         * @param[in] end Seconds at which to end the animation. -1 to play
         * it in full.
         */
        void ScriptPlayAnimationStop(const char* name, const float start, const float end);

        /**
         * Sets the default animation for the widget.
         *
         * @param[in] animation The name of the default animation.
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
         * @param[in] align Widget alignment.
         */
        void SetAlign(const UiWidget::Align align);

        /**
         * Sets the widget vertical alignment.
         *
         * @param[in] valign Widget alignment.
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
         * @return The final coordinates.
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

        /**
         * Retrieves the final rotation of the widget.
         *
         * @return The final orientation.
         */
        float GetFinalRotation() const;

        /**
         * Sets the X coordinate origin for the widget.
         *
         * @param[in] percent @todo
         * @param[in] x Origin X coordinate.
         */
        void SetOriginX(const float percent, const float x);

        /**
         * Sets the Y coordinate origin for the widget.
         *
         * @param[in] percent @todo
         * @param[in] y Origin Y coordinate.
         */
        void SetOriginY(const float percent, const float y);

        /**
         * Sets the X coordinate for the widget.
         *
         * @param[in] percent @todo
         * @param[in] x The X coordinate.
         */
        void SetX(const float percent, const float x);

        /**
         * Retrieves the X coordinate for the widget.
         *
         * @param[out] percent @todo
         * @param[out] x The X coordinate will be loaded here.
         */
        void GetX(float& percent, float& x);

        /**
         * Sets the Y coordinate for the widget.
         *
         * @param[in] percent @todo
         * @param[in] y The Y coordinate.
         */
        void SetY(const float percent, const float y);

        /**
         * Retrieves the Y coordinate for the widget.
         *
         * @param[out] percent @todo
         * @param[out] y The Y coordinate will be loaded here.
         */
        void GetY(float& percent, float& y);

        /**
         * Sets the widget Z-index.
         *
         * @param[in] z The Z-index.
         */
        void SetZ(const float z);

        /**
         * Sets the width the widget.
         *
         * @param[in] percent @todo
         * @param[in] width The widget width.
         */
        void SetWidth(const float percent, const float width);

        /**
         * Retrieves the width of the widget.
         *
         * @param[out] percent @todo
         * @param[out] width The width will be loaded here.
         */
        void GetWidth(float& percent, float& width);

        /**
         * Sets the height the widget.
         *
         * @param[in] percent @todo
         * @param[in] height The widget height.
         */
        void SetHeight(const float percent, const float height);

        /**
         * Retrieves the height of the widget.
         *
         * @param[out] percent @todo
         * @param[out] height The height will be loaded here.
         */
        void GetHeight(float& percent, float& height);

        /**
         * Sets the widget scale.
         *
         * @param[in] scale The new widget scale.
         */
        void SetScale(const Ogre::Vector2& scale);

        /**
         * Sets the widget rotation.
         *
         * @param[in] degree The new widget rotation.
         */
        void SetRotation(const float degree);

        /**
         * Sets the widget scissor area.
         *
         * @param[in] percent_x1 @todo Understand and document.
         * @param[in] x1 @todo Understand and document.
         * @param[in] percent_y1 @todo Understand and document.
         * @param[in] y1 @todo Understand and document.
         * @param[in] percent_x2 @todo Understand and document.
         * @param[in] x2 @todo Understand and document.
         * @param[in] percent_y2 @todo Understand and document.
         * @param[in] y2 @todo Understand and document.
         */
        void SetScissorArea(
          const float percent_x1, const float x1, const float percent_y1, const float y1,
          const float percent_x2, const float x2, const float percent_y2, const float y2
        );

        /**
         * @todo Understand and document.
         *
         * @param[in] global @todo Understand and document.
         */
        void SetGlobalScissor(const bool global);

        /**
         * Sets the widget colour.
         *
         * @param[in] r Colour red component (0-255).
         * @param[in] g Colour green component (0-255).
         * @param[in] b Colour blue component (0-255).
         */
        void SetColour(const float r, const float g, const float b);

        /**
         * Sets the widget colour.
         *
         * It set the same colours for each corner, and blends it in the interior.
         *
         * @param[in] r1 Colour red component for the top-left corner (0-255).
         * @param[in] g1 Colour green component for the top-left corner (0-255).
         * @param[in] b1 Colour blue component for the top-left corner (0-255).
         * @param[in] r2 Colour red component for the top-right corner (0-255).
         * @param[in] g2 Colour green component for the top-right corner (0-255).
         * @param[in] b2 Colour blue component for the top-right corner (0-255).
         * @param[in] r3 Colour red component for the bottom-right corner (0-255).
         * @param[in] g3 Colour green component for the bottom-right corner (0-255).
         * @param[in] b3 Colour blue component for the bottom-right corner (0-255).
         * @param[in] r4 Colour red component for the bottom-left corner (0-255).
         * @param[in] g4 Colour green component for the bottom-left corner (0-255).
         * @param[in] b4 Colour blue component for the bottom-left corner (0-255).
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
         * @param[in] a Alpha value (0 opaque, 255 fully transparent).
         */
        void SetAlpha(const float a);

        /**
         * Sets the text of the widget.
         *
         * If the widget doesn't have text, it won't do anything.
         *
         * @param[in] text The text to set.
         */
        virtual void SetText(const char* text);

        /**
         * Sets the text of the widget.
         *
         * If the widget doesn't have an image, it won't do anything.
         *
         * @param[in] image Path to the image.
         */
        virtual void SetImage(const char* image);

        /**
         * Retrieves the absolute width of the widget.
         *
         * Doesn't consider percentages.
         *
         * @return Absolute width.
         */
        float ScriptGetWidth();

        /**
         * Sets the absolute width of the widget.
         *
         * Doesn't modify percentages.
         *
         * @param[in] width Absolute width.
         */
        void ScriptSetWidth(float width);

        /**
         * Retrieves the absolute height of the widget.
         *
         * Doesn't consider percentages.
         *
         * @return Absolute height.
         */
        float ScriptGetHeight();

        /**
         * Sets the absolute height of the widget.
         *
         * Doesn't modify percentages.
         *
         * @param[in] height Absolute height.
         */
        void ScriptSetHeight(float height);

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

