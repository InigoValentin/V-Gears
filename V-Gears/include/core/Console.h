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
#include <OgreLog.h>
#include <OgreSingleton.h>
#include <OgreStringVector.h>
#include <OIS/OIS.h>
#include <list>
#include <vector>
#include "Event.h"

/**
 * The game console.
 */
class Console : public Ogre::Singleton<Console>, public Ogre::LogListener{

    public:

        /**
         * Constructor.
         */
        Console();

        /**
         * Destructor.
         */
        ~Console();

        /**
         * Processes an input event.
         *
         * @param[in] event Event to process.
         */
        void Input(const VGears::Event& event);

        /**
         * Updates the console.
         */
        void Update();

        /**
         * Redraws the console.
         */
        void UpdateDraw();

        /**
         * Notifies the console to update itself.
         */
        void UpdateNotification();

        /**
         * Processes a console resizing event.
         */
        void OnResize();

        /**
         * Shows the console.
         */
        void SetToVisible();

        /**
         * Hides the console.
         */
        void SetToHide();

        /**
         * Checks if the console is currently visible.
         *
         * @return TRue if the console is visible, false otherwise.
         */
        bool IsVisible() const;

        /**
         * Writes text to the console.
         *
         * @param[in] text Text to write.
         * @param[in] colour Color for the text. Default is white.
         */
        void AddTextToOutput(
          const Ogre::String& text, const Ogre::ColourValue& colour = Ogre::ColourValue::White
        );

        /**
         * Executes a command in the console
         *
         * @param[in] command Command to execute.
         */
        void ExecuteCommand(const Ogre::String& command);

        /**
         * @todo Understand and document.
         */
        void ExecuteScript();

        /**
         * Autocompletes the current input.
         */
        void CompleteInput();

        /**
         * Clears the autocompletion list.
         */
        void ResetAutoCompletion();

        /**
         * Adds a console input to the console history.
         */
        void AddInputToHistory();

        /**
         * Sets a line from the history as the current input.
         */
        void SetInputLineFromHistory();

        /**
         * Logs a message to the console.
         *
         * @param[in] message The message to log.
         * @param[in] lml Log level for the message.
         * @param[in] maskDebug Indicates if the mesage is being printed to
         * the console or not.
         * @param[in] logName Name of the log.
         * @param[in] skipThisMessage If true, the message will not be logged.
         */
        virtual void messageLogged(
          const Ogre::String& message, Ogre::LogMessageLevel lml,
          bool maskDebug, const Ogre::String &logName, bool& skipThisMessage
        );

    private:

        /**
         * Loads the console input history.
         */
        void LoadHistory();

        /**
         * Saves the console input history.
         */
        void SaveHistory();

        /**
         * Adds a string to the console input history.
         */
        void AddToHistory(const Ogre::String& history);

        /**
         * Translates numpad key events to regular number events.
         *
         * @param[in] event Event to translate.
         */
        char TranslateNumpad(const VGears::Event& event);

        /**
         * The console width.
         */
        int console_width_;

        /**
         * The console height.
         */
        int console_height_;

        /**
         * The width of each line in the console.
         */
        unsigned int line_width_;

        /**
         * The width of each letter in the console.
         */
        int letter_width_;

        /**
         * Indicates if the console is being made visible.
         */
        bool to_visible_;

        /**
         * Indicates if the console is currently visible.
         */
        bool visible_;

        /**
         * Line height of the console.
         */
        float height_;

        /**
         * An ouptut line of the console.
         */
        struct OutputLine{

            /**
             * Text in the line.
             */
            Ogre::String text;

            /**
             * Color for the text in the line.
             */
            Ogre::ColourValue colour;

            /**
             * Time at which it was printed.
             */
            float time;
        };

        /**
         * Listof lines in the console.
         */
        std::list<OutputLine> output_line_;

        /**
         * Max number of lines in output list.
         */
        unsigned int max_output_line_;

        /**
         * The bottom line of the console, the active one.
         */
        unsigned int display_line_;

        /**
         * Currently typed text.
         */
        Ogre::String input_line_;

        /**
         * Position of the cursor in the active line.
         */
        unsigned int cursor_position_;

        /**
         * Cursor blink frequency, in seconds.
         */
        float cursor_blink_time_;

        /**
         * The console input history.
         */
        std::list<Ogre::String> history_;

        /**
         * Currently selected line of the input history.
         */
        int history_line_cycle_index_;

        /**
         * Maximum number of inputs to save to the history.
         */
        unsigned int max_history_size_;

        /**
         * List of strings available for autocompletion.
         */
        Ogre::StringVector auto_completition_;

        /**
         * Index of the currently displayed autocompletion option.
         */
        unsigned int auto_completition_line_;
};

