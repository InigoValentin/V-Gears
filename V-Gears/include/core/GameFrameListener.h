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

#include <OgreFrameListener.h>
#include <OgreRenderWindow.h>
#include <OgreWindowEventUtilities.h>
#include <OIS/OIS.h>

/**
 * The game frame listener.
 *
 * Handles the game window and input events.
 */
class GameFrameListener :
  public Ogre::FrameListener, public Ogre::WindowEventListener,
  public OIS::KeyListener, public OIS::MouseListener
{
    public:

        /**
         * Constructor.
         *
         * @param[in] win Render window.
         */
        GameFrameListener(Ogre::RenderWindow* win);

        /**
         * Destructor.
         */
        virtual ~GameFrameListener();

        /**
         * Called when a frame is about to begin rendering.
         *
         * This event happens before any render targets have begun updating.
         *
         * @return True to go ahead, false to abort rendering and drop out of
         * the rendering loop.
         */
        bool frameStarted(const Ogre::FrameEvent& evt);

        /**
         * Called just after a frame has been rendered.
         *
         * This event happens after all render targets have been fully updated
         * and the buffers switched.
         *
         * @return True to continue with the next frame, false to drop out of
         * the rendering loop.
         */
        bool frameEnded(const Ogre::FrameEvent& evt);

        /**
         * Event triggered when the window moves.
         *
         * @param[in] rw The render window.
         */
        virtual void windowMoved(Ogre::RenderWindow *rw);

        /**
         * Event triggered when the window changes it's size.
         *
         * @param[in] rw The render window.
         */
        virtual void windowResized(Ogre::RenderWindow *rw);

        /**
         * Event triggered when the window is closed.
         *
         * @param[in] rw The render window.
         */
        virtual void windowClosed(Ogre::RenderWindow* rw);

        /**
         * Event triggered when the window losses or gains the focus.
         *
         * @param[in] rw The render window.
         */
        virtual void windowFocusChange(Ogre::RenderWindow *rw);

        /**
         * Triggered when a key is pressed.
         *
         * @param[in] e Keyboard event.
         */
        bool keyPressed(const OIS::KeyEvent& e);

        /**
         * Triggered when a key is released.
         *
         * @param[in] e Keyboard event.
         */
        bool keyReleased(const OIS::KeyEvent& e);

        /**
         * Triggered when the mouse moves.
         *
         * @param[in] e Mouse event.
         */
        bool mouseMoved(const OIS::MouseEvent &e);

        /**
         * Triggered when a mouse button is pressed.
         *
         * @param[in] e Mouse event.
         * @param[in] id ID of the presses mouse button.
         */
        bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);

        /**
         * Triggered when a mouse button is released.
         *
         * @param[in] e Mouse event.
         * @param[in] id ID of the presses mouse button.
         */
        bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);

    protected:

        /**
         * The render window.
         */
        Ogre::RenderWindow* window_;

        /**
         * The input manager.
         */
        OIS::InputManager*  input_manager_;

        /**
         * The keyboard.
         */
        OIS::Keyboard*      keyboard_;

        /**
         * The mouse.
         */
        OIS::Mouse*         mouse_;
};

