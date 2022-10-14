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
#include <Overlay/OgreUTFString.h>
#include <tinyxml.h>

#include "UiFont.h"
#include "UiWidget.h"

class UiSprite;

/**
 * The textarea state.
 */
enum TextState{

    /**
     * The text is displayed.
     */
    TS_SHOW_TEXT,

    /**
     * The text is scrolling.
     */
    TS_SCROLL_TEXT,

    /**
     * The text is paused waiting for input.
     */
    TS_PAUSE_OK,

    /**
     * The text is paused waiting for time to pass.
     */
    TS_PAUSE_TIME,

    /**
     * The textarea is done with.
     */
    TS_DONE,

    /**
     * The text is overflowing.
     */
    TS_OVERFLOW,

    /**
     * The text is changing pages.
     */
    TS_NEXT_PAGE,
};

/**
 * A character in a text.
 */
struct TextChar{
    TextChar():
      char_code(0),
      colour(Ogre::ColourValue::White),
      skip(false),
      variable(""),
      variable_len(0),
      pause_ok(false),
      pause_time(0.0f),
      next_page(false),
      sprite(NULL),
      sprite_y(0)
    {}

    /**
     * The character code.
     *
     * Normal character code, no FF7 code tables.
     */
    int char_code;

    /**
     * Color for the character.
     */
    Ogre::ColourValue colour;

    /**
     * Indicates if the character is to be actually printed.
     */
    bool skip;

    /**
     * If the character is a variable, the variable name.
     */
    Ogre::String variable;

    /**
     * If the character is a variable, the variable length.
     */
    unsigned int variable_len;

    /**
     * Indicates if the OK button has been pressed during a text pause.
     */
    bool pause_ok;

    /**
     * For pause characters, the time.
     */
    float pause_time;

    /**
     * Indicates if the character is a new page character.
     */
    bool next_page;

    /**
     * If the character is a symbol or an image, this is the sprite.
     */
    UiSprite* sprite;

    /**
     * If the character is a symbol or an image, this is the Y position.
     */
    float sprite_y;
};

/**
 * A variable displayed in a text.
 */
struct TextVariable{

    /**
     * The variable name.
     */
    Ogre::String name;

    /**
     * The variable value, resolved as a string.
     */
    Ogre::UTFString value;
};


/**
 * An ingame textarea.
 *
 * It may be any window used to represent text: a diaog window, a menu panel,
 * a choice selection...
 */
class UiTextArea : public UiWidget{

    public:

        /**
         * Creates a UiTextArea.
         *
         * @param[in] name Name for the textarea.
         */
        UiTextArea(const Ogre::String& name);

        /**
         * Creates a UiTextArea.
         *
         * @param[in] name Name for the textarea.
         * @param[in] path_name Path for the widget.
         * @param[in] parent Pointer to the widget that will parent the
         * UiTextArea.
         */
        UiTextArea(const Ogre::String& name, const Ogre::String& path_name, UiWidget* parent);

        /**
         * Destroys the UiTextArea.
         */
        virtual ~UiTextArea();

        /**
         * Sets default values for the UiTextArea.
         *
         * It's automatically called when the UiTextArea is created.
         */
        void Initialise();

        /**
         * Updates the UiTextArea.
         */
        virtual void Update();

        /**
         * Renders the UiTextArea on the screen.
         */
        virtual void Render();

        /**
         * Updates the UiTextArea.
         */
        virtual void UpdateTransformation();

        /**
         * Updates the UiTextArea geometry.
         */
        void UpdateGeometry();

        /**
         * Handles a keypress.
         */
        void InputPressed();

        /**
         * Handles a key being hold.
         */
        void InputRepeated();

        /**
         * Text alignment in the text area.
         */
        enum TextAlign{

            /**
             * Left aligned text.
             */
            LEFT,

            /**
             * Right aligned text.
             */
            RIGHT,

            /**
             * Horizontally centered text.
             */
            CENTER
        };

        /**
         * Sets the text alignment in the text area.
         *
         * @param[in] align The text alignment.
         */
        void SetTextAlign(const TextAlign align);

        /**
         * Sets the paddings in the text area.
         *
         * @param[in] top Padding from the top, in pixels.
         * @param[in] right Padding from the right, in pixels.
         * @param[in] bottom Padding from the bottom, in pixels.
         * @param[in] left Padding from the left, in pixels.
         */
        void SetPadding(const float top, const float right, const float bottom, const float left);

        /**
         * Sets the text from a string.
         *
         * @param[in] text Text to set.
         */
        void SetText(const Ogre::UTFString& text);

        /**
         * Sets the text from a string.
         *
         * @param[in] text Text to set.
         */
        void SetText(const char* text) override;

        /**
         * Sets the text from an XML node.
         *
         * @param[in] text XML node with the text to set.
         */
        void SetText(TiXmlNode* text);

        /**
         * Removes the text from the text area.
         */
        void TextClear();

        /**
         * @todo Understand and document.
         *
         * @param[in] end @todo Understand and document.
         */
        void RemoveSpritesFromText(const unsigned int end);

        /**
         * Set the font for the text.
         *
         * @param[in] font The font to set.
         */
        void SetFont(const Ogre::String& font);

        /**
         * Retrieves the font.
         *
         * @return The font used for the text in the textarea.
         */
        const UiFont* GetFont() const;

        /**
         * Sets the printing speed.
         *
         * @param[in] speed The text speed. -1 for instant text.
         * @todo Explain units or references.
         */
        void SetTextPrintSpeed(const float speed);

        /**
         * Sets the scroll duration.
         *
         * @param[in] time Time to scroll a line.
         * @todo Units or references.
         */
        void SetTextScrollTime(const float time);

        /**
         * Sets a variable in the text.
         *
         * @param[in] name Variable name.
         * @param[in] value Variable value.
         */
        void SetVariable(const Ogre::String& name, const Ogre::UTFString& value);

        /**
         * Gets the value of a variable in the text.
         *
         * @param[in] name The variable name.
         * @return The variable value, in string format, or an empty string if
         * there is no such variable.
         */
        Ogre::UTFString GetVariable(const Ogre::String& name) const;

        /**
         * Checks the text state.
         *
         * @return The text state.
         */
        TextState GetTextState() const;

        /**
         * Gets the text limit.
         *
         * @return The text limit.
         * @todo The limit is the max number of letters per text area? Does it
         * include multiple pages?
         */
        float GetTextLimit() const;

        /**
         * Gets the text size.
         *
         * @return The number of characters in the text.
         */
        unsigned int GetTextSize() const;

        /**
         * Retrieves the pause time of the text.
         *
         * @return The time the text must still remain paused, in second.
         */
        float GetPauseTime() const;

    private:

        /**
         * Retrieves the text width.
         *
         * @return The text width, in pixels.
         */
        float GetTextWidth() const;

        /**
         * Prepares text from a XML node.
         *
         * @param[in] node The XML node to get the text from.
         * @param[in] colour The text colour.
         */
        void PrepareTextFromNode(TiXmlNode* node, const Ogre::ColourValue& colour);

        /**
         * Prepares text from a string.
         *
         * @param[in] text The text to prepare.
         * @param[in] colour The text colour.
         */
        void PrepareTextFromText(const Ogre::UTFString& text, const Ogre::ColourValue& colour);

        /**
         * Constructor.
         */
        UiTextArea();

        /**
         * Creates a vertex buffer for the textarea.
         */
        void CreateVertexBuffer();

        /**
         * Destroys a vertex buffer for the textarea.
         */
        void DestroyVertexBuffer();

        /**
         * Material for the text area.
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
         * Max letter per textarea.
         */
        unsigned int max_letters_;

        /**
         * The render operation.
         */
        Ogre::RenderOperation render_operation_;

        /**
         * The text area vertext buffer.
         */
        Ogre::HardwareVertexBufferSharedPtr vertex_buffer_;

        /**
         * The font for the text.
         */
        UiFont* font_;

        /**
         * The text alignment.
         */
        TextAlign text_align_;

        /**
         * The text.
         */
        std::vector<TextChar> text_;

        /**
         * The text limit.
         */
        float text_limit_;

        /**
         * The text printing speed.
         */
        float text_print_speed_;

        /**
         * @todo Understand and document.
         */
        float text_print_speed_mod_;

        /**
         * The state of the text.
         */
        TextState text_state_;

        /**
         * Variables in the text.
         */
        std::vector<TextVariable> text_variable_;

        /**
         * Time to scroll the text.
         */
        float text_scroll_time_;

        /**
         * @todo Understand and document.
         */
        float text_y_offset_;

        /**
         * @todo Understand and document.
         */
        float text_y_offset_target_;

        /**
         * The time to pause the text.
         */
        float pause_time_;

        /**
         * @todo Understand and document.
         */
        unsigned int next_page_start_;

        /**
         * Indicates if the 'next' button has been pressed.
         */
        bool next_pressed_;

        /**
         * Indicates if the 'next' button is being held down.
         */
        bool next_repeated_;

        /**
         * The top padding.
         */
        float padding_top_;

        /**
         * The right padding.
         */
        float padding_right_;

        /**
         * The bottom padding.
         */
        float padding_bottom_;

        /**
         * The left padding.
         */
        float padding_left_;

        /**
         * @todo Understand and document.
         */
        bool timer_;

        /**
         * @todo Understand and document.
         */
        int timer_time_;
};

