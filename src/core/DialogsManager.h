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

#include <OgreSingleton.h>
#include "Manager.h"
#include "UiManager.h"
#include "UiTextArea.h"
#include "InputManager.h"


/**
 * Possible states for messages.
 */
enum MessageState{

    /**
     * The message is now closed.
     */
    MS_CLOSED,

    /**
     * The message window is shown, but there is no text.
     */
    MS_SHOW_WINDOW,

    /**
     * The message window and text are shown.
     */
    MS_SHOW_TEXT,

    /**
     * The message window is open.
     */
    MS_OPENED,

    /**
     * The message window has been hidden.
     */
    MS_HIDE_WINDOW
};

/**
 * Styles for message boxes.
 */
enum MessageStyle{

    /**
     * Regular message box, solid background.
     */
    MSL_SOLID,

    /**
     * Translucent message box.
     */
    MSL_TRANSPARENT,

    /**
     * No message box.
     */
    MSL_NONE
};

/**
 * Message data.
 */
struct MessageData{
    MessageData():
      widget(NULL),
      window(NULL),
      scissor(NULL),
      text_area(NULL),
      cursor(NULL),
      state(MS_CLOSED),
      clickable(true),
      show_window(true),
      show_cursor(false),
      auto_close(false),
      cursor_percent_y(0),
      cursor_y(0),
      cursor_row_selected(0),
      cursor_row_current(0),
      cursor_row_first(0),
      cursor_row_last(0),
      closeable(true),
      visible(true),
      translucent(false),
      timer(false),
      numeric(false)
    {}

    /**
     * The widget where the message is displayed.
     */
    UiWidget* widget;

    /**
     * The window where the message is displayed.
     */
    UiWidget* window;

    /**
     * @todo Understand and document.
     */
    UiWidget* scissor;

    /**
     * The message text area.
     */
    UiTextArea* text_area;

    /**
     * The message cursor.
     */
    UiWidget* cursor;

    /**
     * The state of the message.
     */
    MessageState state;

    /**
     * The message window X coordinate.
     */
    int x = 0;

    /**
     * The message window Y coordinate.
     */
    int y = 0;

    /**
     * The message width, in pixels.
     */
    int w = 0;

    /**
     * The message height, in pixels.
     */
    int h = 0;

    /**
     * @todo Understand and document.
     */
    std::vector<ScriptId> sync;

    /**
     * Indicates if the text is 'clickable', i.e., if it requieres a keypress to advance.
     */
    bool clickable;

    /**
     * Indicates if the dialog window is shown.
     */
    bool show_window;

    /**
     * Indicates if the choice selection cursor must be shown.
     */
    bool show_cursor;

    /**
     * Indicates if the dialog must be closed automatically.
     */
    bool auto_close;

    /**
     * @todo Understand and document.
     */
    float cursor_percent_y;

    /**
     * Y position of the cursor in the message box.
     */
    float cursor_y;

    /**
     * Selected choice line.
     */
    int cursor_row_selected;

    /**
     * Currently selected choice line.
     */
    int cursor_row_current;

    /**
     * First chooseable line.
     */
    int cursor_row_first;

    /**
     * Last chooseable line.
     */
    int cursor_row_last;

    /**
     * Indicates if the window is closeable.
     */
    bool closeable;

    /**
     * Indicates if the window background is visible.
     */
    bool visible;

    /**
     * Indicates if the window background is translucent.
     */
    bool translucent;

    /**
     * Indicates if the window is for a timer.
     */
    bool timer;

    /**
     * Indicates if the window background is for a number.
     */
    bool numeric;
};

/**
 * The dialog manager.
 */
class DialogsManager : public Manager, public Ogre::Singleton<DialogsManager>{

    public:

        /**
         * Constructor.
         */
        DialogsManager();

        /**
         * Destructor.
         */
        virtual ~DialogsManager();

        /**
         * Initializes the dialog manager.
         */
        void Initialise();

        /**
         * Processes an input event.
         *
         * @param[in] event Event to process.
         */
        void Input(const VGears::Event& event) override;

        /**
         * Updates the messagemanager with debug information.
         */
        void UpdateDebug();

        /**
         * Handles resizing events
         */
        void OnResize() override;

        /**
         * Clears all field messages.
         */
        void ClearField() override;

        /**
         * Clears all battle messages.
         */
        void ClearBattle() override;

        /**
         * Clears all world map maeeages.
         */
        void ClearWorld() override;

        /**
         * Opens a dialog.
         *
         * Equivalent to dialog_open in the field scripts.
         *
         * @param[in] d_name The dialog name. Can be used as ID.
         * @param[in] x X coordinate for the top left corner of the dialog.
         * @param[in] y Y coordinate for the top left corner of the dialog.
         * @param[in] w Width of the dialog, in pixels.
         * @param[in] h Height of the dialog, in pixels.
         */
        void OpenDialog(const char* d_name, int x, int y, int w, int h);

        /**
         * Sets the text of a dialog.
         *
         * Equivalent to dialog_set_text in the field scripts.
         *
         * @param[in] d_name Name of the dialog.
         * @param[in] text Text to set in the dialog.
         */
        void SetText(const char* d_name, const char* text);

        /**
         * Sets or unsets a dialog window as numeric
         *
         * @param[in] d_name Name of the dialog.
         * @param[in] numeric True to make it numeric, false otherwise.
         * @param[in] timer True to use the window for times, false for regular numbers. If used
         * for timer, the window will be used when calling {@see UpdateTimer}
         */
        void SetNumeric(const char* d_name, const bool numeric, const bool timer);

        /**
         * Updates the timer.
         *
         * Displays the formatted time in the timer window. To do so, a timer window must have been
         * set as timer with {@see SetNumeric}
         */
        void UpdateTimer(const unsigned int seconds);

        /**
         * Sets the window mode.
         *
         * @param[in] d_name Name of the dialog.
         * @param[in] bg Background mode. 1: No background/border. 2: Translucent background.
         * Anything else: Normal.
         * @param[in] closeable True if the window can be closed manually, false to lock it.
         */
        void SetMode(const char* d_name, const int bg, const bool closeable);

        /**
         * Syncs the dialog and makes the script wait until it is closed.
         *
         * Equivalent to dialog_wait_for_close in the field scripts.
         *
         * @param[in] d_name Name of the dialog.
         * @return 1 if the dialog doesn't exist, -1 otherwise.
         */
        int Sync(const char* d_name);

        /**
         * Closes and hides a dialog window.
         *
         * Equivalent to dialog_close in the field scripts.
         *
         * @param[in] d_name Name of the dialog to close.
         */
        void Hide(const char* d_name);

        /**
         * Sets a variable in a dialog
         *
         * @param[in] d_name Name of the dialog.
         * @param[in] name Name of the variable.
         * @param[in] value Value for the variable.
         */
        void SetVariable(const char* d_name, const char* name, const char* value);

        /**
         * Makes the dialog clickable or not.
         *
         * A clickable dialog presents a choice for the user that must be selected with a cursor.
         *
         * @param[in] d_name Name of the dialog.
         * @param[in] clickable True to make the dialog clickable, false to make it unclickable
         */
        void SetClickable(const char* d_name, const bool clickable);

        /**
         * Sets a choice sursor in the dialog.
         *
         * @param[in] d_name Name of the dialog.
         * @param[in] first_row First selectable line.
         * @param[in] last_row Last selectable line.
         */
        void SetCursor(const char* d_name, const int first_row, const int last_row);

        /**
         * Gets the cursor position in a dialog.
         *
         * @param[in] d_name Dialog name.
         * @return Position of the cursor in the dialog, including unselectable
         * lines (0-index). 0 if the dialog doesn't exist.
         */
        int GetCursor(const char* d_name) const;

        /**
         * Sets the map name from a text ID.
         *
         * @param[in] text_id The ID of the text to set as map name.
         */
        void ScriptSetMapName(const char* text_id);

        /**
         * Sets the map name from a text ID.
         *
         * @param[in] name The new map name.
         */
        void SetMapName(std::string name);

        /**
         * Retrieves the current map name.
         *
         * @return The map name.
         */
        std::string GetMapName();

    private:

        /**
         * Updates the dialogs while in a field.
         */
        void UpdateField() override;

        /**
         * Updates the dialogs during a battle.
         */
        void UpdateBattle() override;

        /**
         * Updates the dialogs while on the world map.
         */
        void UpdateWorld() override;

        /**
         * Shows a message in a dialog.
         *
         * @param[in] id Message id.
         * @param[in] x X coordinate for the text in the dialog.
         * @param[in] y Y coordinate for the text in the dialog.
         * @param[in] width Width of the text, in pixels.
         * @param[in] height Height of the text, in pixels.
         * @todo Is this a window-limit-aware version of SetText?
         */
        void ShowMessage(const int id, const int x, const int y, const int width, const int height);

        /**
         * Closes and hides a message.
         *
         * @param[in] id ID of the message to close.
         */
        void HideMessage(const int id);

        /**
         * Retrieves a message Id from it's name.
         *
         * @param[in] d_name Dialog name.
         * @return The dialog ID. -1 if there is no dialog by that name.
         */
        int GetMessageId(const char* d_name) const;

        /**
         * Checks if a message is set to close automatically.
         *
         * @param[in] id Message ID.
         * @return True if the message is set to auto close, false otherwise.
         */
        bool AutoCloseCheck(const unsigned int id);

        /**
         * The limit area for message boxes.
         */
        UiWidget* limit_area_;

        /**
         * The list of messages for the field or the world map.
         */
        std::vector<MessageData*> messages_;

        /**
         * The list of battle messages.
         */
        std::vector<MessageData*> battle_messages_;

        /**
         * Indicates if the 'next' button has been pressed.
         */
        bool next_pressed_;

        /**
         * Indicates if the 'next' button is being hold.
         */
        bool next_repeated_;

        /**
         * Indicates if the 'up' button has been pressed.
         */
        bool up_pressed_;

        /**
         * Indicates if the 'down' button has been pressed.
         */
        bool down_pressed_;

        /**
         * Current map name.
         */
        std::string map_name_;

        /**
         * The ID of the window that contains the timer, is any
         */
        int timer_window_id_;

        /**
         * The number of seconds in the timer.
         *
         * Stored solely to not re-set the text if the time is the same.
         */
        unsigned int timer_seconds_;
};

