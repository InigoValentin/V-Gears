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
#include <OgreString.h>
#include <OIS/OIS.h>
#include <vector>
#include "Event.h"

typedef std::vector<VGears::Event> InputEventArray;
typedef std::vector<int> ButtonList;

class ConfigCmd;

/**
 * The input manager.
 *
 * It handles input events and creates {@see Event}s.
 */
class InputManager : public Ogre::Singleton<InputManager>{

    public:

        /**
         * Constructor.
         */
        InputManager();

        /**
         * Destructor.
         */
        virtual ~InputManager();

        /**
         * Triggered when a keyboard button is pressed or released.
         *
         * Creates an {@see Event}.
         *
         * @param[in] button Pressed button ID.
         * @param[in] text @todo The key code? It gets assigned to parameter 1
         * of the generated event.
         * @param[in] down True if the button has been pressed, false if it has
         * been released. It gets assigned to parameter 2 of the event.
         */
        void ButtonPressed(int button, char text, bool down);

        /**
         * Triggered when a mouse button is pressed or released.
         *
         * Creates an {@see Event}.
         *
         * @param[in] button Pressed button ID.
         * @param[in] down True if the button has been pressed, false if it has
         * been released. It gets assigned to parameter 1 of the event.
         */
        void MousePressed(int button, bool down);

        /**
         * Triggered when a mouse moves.
         *
         * Creates an {@see Event}.
         *
         * @param[in] x New mouse's X coordinate. It gets assigned to parameter
         * 1 of the event.
         * @param[in] y New mouse's Y coordinate. It gets assigned to parameter
         * 2 of the event.
         */
        void MouseMoved(int x, int y);

        /**
         * Triggered when a mouse scrolls.
         *
         * Creates an {@see Event}.
         *
         * @param[in] value Number of lines scrolled. Positive for scroll down,
         * negative for scroll up. It gets assigned to parameter 1 of the
         * event.
         */
        void MouseScrolled(int value);

        /**
         * Resets all keyboard and mouse events to their default state.
         *
         * Keyboard and mouse are considered to not pressed, the mouse movement
         * and scroll are considered 0.
         */
        void Reset();

        /**
         * Update keyboard and mouse buttons, movements and scroll status.
         */
        void Update();

        bool IsButtonPressed(int button) const;

        /**
         * Retrieves the current input events.
         *
         * The input event queue will be empty after calling this function.
         *
         * @param[out] input_events The current events will be loaded here.
         */
        void GetInputEvents(InputEventArray& input_events);

        /**
         * Initializes all command bindings.
         */
        void InitCmd();

        /**
         * Binds a command to an input event.
         *
         * @param[in] cmd Command to bind.
         * @param[in] params Command arguments.
         * @param[in] buttons Buttons to bind to the command.
         */
        void BindCommand(
          ConfigCmd* cmd, const Ogre::StringVector& params,
          const ButtonList& buttons
        );

        /**
         * Binds a game event to an input event.
         *
         * @param[in] event The game event to bind.
         * @param[in] buttons Buttons to bind to the event.
         */
        void BindGameEvent(
          const Ogre::String& event, const ButtonList& buttons
        );

        /**
         * Activates all bindings for a button.
         *
         * @param[in] button ID of the button.
         */
        void ActivateBinds(const int button);

        /**
         * Adds a game event associated to a button.
         *
         * @param[in] button ID of the button.
         * @param[in] type Event type.
         */
        void AddGameEvents(const int button, const VGears::EventType type);

    private:

        /**
         * The state of eahc button.
         */
        bool button_state_[256];

        /**
         * The text of each button.
         *
         * @todo The text is the keycode?
         */
        char button_text_[256];

        /**
         * Indicates if a key holded down is waiting to repeat events.
         *
         * @todo Verify this.
         */
        bool repeat_first_wait_;

        /**
         * The time a key has been pressed.
         */
        float repeat_timer_;

        /**
         * The event queue.
         */
        InputEventArray event_queue_;

        /**
         * A command binding.
         */
        struct BindInfo{

            /**
             * Constructor.
             */
            BindInfo(): cmd(NULL){}

            /**
             * The bind command.
             */
            ConfigCmd* cmd;

            /**
             * The command parameters.
             */
            Ogre::StringVector params;

            /**
             * The buttons bond to the command.
             */
            ButtonList buttons;
        };

        /**
         * List of command bindings.
         */
        std::vector<BindInfo> binds_;

        /**
         * A game event binding
         */
        struct BindGameEventInfo{

            /**
             * The game event.
             */
            Ogre::String event;

            /**
             * The buttons bond to the command.
             */
            ButtonList buttons;
        };

        /**
         * List of game event bindings.
         */
        std::vector<BindGameEventInfo> bind_game_events_;
};

