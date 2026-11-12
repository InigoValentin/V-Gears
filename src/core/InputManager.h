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
#include <vector>
#include <OgreSingleton.h>
#include <OgreString.h>
#include <OgreStringVector.h>
#include <OIS/OIS.h>
#include "Event.h"
#include "Manager.h"

typedef std::vector<VGears::Event> InputEventArray;

typedef std::vector<int> ButtonList;

class ConfigCmd;

/**
 * The input manager.
 *
 * It handles input events and creates {@see Event}s.
 */
class InputManager : public Manager, public Ogre::Singleton<InputManager>{

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
         * Makes the input manager itself handles an input event.
         *
         * @param[in] event Event to handle.
         */
        void Input(const VGears::Event& event) override;

        /**
         * Update keyboard and mouse buttons, movements and scroll status.
         */
        void Update();

        /**
         * Updates the input manager with debug information.
         */
        void UpdateDebug() override;

        /**
         * Handles resizing events
         */
        void OnResize() override;

        /**
         * Clears all field information in the input manager.
         *
         * It does nothing.
         */
        void ClearField() override;

        /**
         * Clears all battle information in the input manager.
         *
         * It does nothing.
         */
        void ClearBattle() override;

        /**
         * Clears all world map information in the input manager.
         *
         * It does nothing.
         */
        void ClearWorld() override;

        /**
         * Triggered when a keyboard button is pressed or released.
         *
         * Creates an {@see Event}.
         *
         * @param[in] button Pressed button ID.
         * @param[in] text @todo The key code? It gets assigned to parameter 1 of the generated
         * event.
         * @param[in] down True if the button has been pressed, false if it has been released. It
         * gets assigned to parameter 2 of the event.
         */
        void ButtonPressed(int button, char text, bool down);

        /**
         * Triggered when a mouse button is pressed or released.
         *
         * Creates an {@see Event}.
         *
         * @param[in] button Pressed button ID.
         * @param[in] down True if the button has been pressed, false if it has been released. It
         * gets assigned to parameter 1 of the event.
         */
        void MousePressed(int button, bool down);

        /**
         * Triggered when a mouse moves.
         *
         * Creates an {@see Event}.
         *
         * @param[in] x New mouse's X coordinate. It gets assigned to parameter 1 of the event.
         * @param[in] y New mouse's Y coordinate. It gets assigned to parameter 2 of the event.
         */
        void MouseMoved(int x, int y);

        /**
         * Triggered when a mouse scrolls.
         *
         * Creates an {@see Event}.
         *
         * @param[in] value Number of lines scrolled. Positive for scroll down, negative for scroll
         * up. It gets assigned to parameter 1 of the event.
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
         * Checks if a button is being pressed.
         *
         * @param[in] button Button code.
         * @return true if the button is being pressed, false otherwise.
         */
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
          ConfigCmd* cmd, const Ogre::StringVector& params, const ButtonList& buttons
        );

        /**
         * Binds a game event to an input event.
         *
         * @param[in] event The game event to bind.
         * @param[in] buttons Buttons to bind to the event.
         */
        void BindGameEvent(const Ogre::String& event, const ButtonList& buttons);

        /**
         * Replaces all the keys bound to a game event.
         *
         * To be called from Lua scripts (controller setting of the config menu) and from the
         * console command "set_game_event_keys".
         *
         * @param[in] event The game event.
         * @param[in] keys Key names separated by commas (for example "Enter,Space"). Each one can
         * be a combination of keys joined with '+' (for example "LCtrl+X"). If empty, the event is
         * left with no keys.
         */
        void ScriptSetGameEventKeys(const char* event, const char* keys);

        /**
         * Gets the keys bound to a game event.
         *
         * @param[in] event The game event.
         * @return Key names separated by commas, like the ones taken by ScriptSetGameEventKeys.
         * Empty if the event has no keys.
         */
        std::string ScriptGetGameEventKeys(const char* event) const;

        /**
         * Makes the next key that is pressed be reported to the scripts as the game event
         * "KEY_CAPTURE" (the name of the key is the third argument of on_button), instead of
         * doing what it is bound to. To be called from Lua scripts, to let the player choose a
         * key.
         */
        void ScriptCaptureNextKey();

        /**
         * Binds the directions of the menus (game events "Up", "Down", "Left" and "Right") to the
         * arrow keys, if the configuration doesn't bind them.
         */
        void EnsureDefaultGameEvents();

        /**
         * Remembers the keys that every game event has now as its default keys.
         *
         * To be called once the configuration file was read, and before the keys chosen by the
         * player are applied. See ScriptGetDefaultGameEventKeys.
         */
        void SnapshotDefaultGameEvents();

        /**
         * Gets the default keys of a game event: the ones it had when SnapshotDefaultGameEvents was
         * called. To be called from Lua scripts, to restore the default keys.
         *
         * @param[in] event The game event.
         * @return Key names separated by commas, like the ones of ScriptGetGameEventKeys. Empty if
         * the event had no keys.
         */
        std::string ScriptGetDefaultGameEventKeys(const char* event) const;

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
         * Updates the input manager while on the fields.
         *
         * It just calls the generic {@see Update()}.
         */
        void UpdateField() override;

        /**
         * Updates the input manager while on a battle.
         *
         * It just calls the generic {@see Update()}.
         */
        void UpdateBattle() override;

        /**
         * Updates the input manager while on the world map.
         *
         * It just calls the generic {@see Update()}.
         */
        void UpdateWorld() override;

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
         * Indicates if a key held down is waiting to repeat events.
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

        /**
         * Indicates if the next key pressed must be captured for the scripts.
         */
        bool capture_next_key_ = false;

        /**
         * Default keys of the game events (see SnapshotDefaultGameEvents), by event.
         */
        std::map<Ogre::String, std::string> default_game_event_keys_;
};

