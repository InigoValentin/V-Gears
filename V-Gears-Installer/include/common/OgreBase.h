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

#include <Ogre.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreOverlay.h>
#include <OgreOverlayManager.h>
#include <Bites/OgreWindowEventUtilities.h>
#include <OIS/OIS.h>
#include "common/FileSystem.h"
#include "Logger.h"

/**
 * The Ogre system root component.
 */
extern Ogre::Root* root;

/**
 * The Ogre render window.
 */
extern Ogre::RenderWindow* window;

/**
 * List of Ogre entities.
 */
extern std::vector<Ogre::Entity*> entities;

/**
 * The Ogre camera.
 */
extern Ogre::Camera* camera;

class DisplayFrameListener;

/**
 * The display frame listener.
 */
extern DisplayFrameListener* frame_listener;

/**
 * The display frame listener.
 *
 * It listens and responds to input events, such as window changes or mouse and
 * keyboard inputs.
 */
class DisplayFrameListener :
  public Ogre::FrameListener, public Ogre::WindowEventListener,
  public OIS::KeyListener, public OIS::MouseListener
{

    public:
        // Constructor takes a RenderWindow because it uses that to determine input context

        /**
         * Constructor.
         *
         * @param win[in] The render window. Used to determine input context.
         */
        DisplayFrameListener(Ogre::RenderWindow* win):
          window_(win), input_manager_(0), keyboard_(0),
          mouse_(0), mouse_rotate_(false), exit_(false)
        {

            using namespace OIS;
            Ogre::LogManager::getSingletonPtr()->logMessage("[OIS] Initializing...");
            ParamList pl;
            size_t window_hnd = 0; // TODO What does HND stand for?
            std::ostringstream window_hnd_str;
            win->getCustomAttribute("WINDOW", &window_hnd);
            window_hnd_str << window_hnd;
            pl.insert(std::make_pair(std::string("WINDOW"), window_hnd_str.str()));
            input_manager_ = InputManager::createInputSystem(pl);
            keyboard_ = static_cast<Keyboard*>(
              input_manager_->createInputObject(OIS::OISKeyboard, true)
            );
            keyboard_->setEventCallback(this);
            mouse_ = static_cast<OIS::Mouse*>(
              input_manager_->createInputObject(OIS::OISMouse, true)
            );
            mouse_->setEventCallback(this);
            //Register as a Window listener.
            Ogre::WindowEventUtilities::addWindowEventListener(window_, this);
            // Debug info.
            info_text = new Ogre::TextAreaOverlayElement("DebugText");
            info_text->setCaption("");
            info_text->setMetricsMode(Ogre::GMM_PIXELS);
            info_text->setPosition(5, 5);
           // info_text->setFontName("BlueHighway");
            info_text->setCharHeight(18);
            info_overlay = Ogre::OverlayManager::getSingleton().create("DebugOverlay");
            info_overlay->setZOrder(1);
            info_overlay->add2D((Ogre::OverlayContainer*) info_text);
            info_overlay->show();
        }

        virtual ~DisplayFrameListener(){
            input_manager_->destroyInputObject(keyboard_);
            OIS::InputManager::destroyInputSystem(input_manager_);
            //Remove self as a Window listener.
            Ogre::WindowEventUtilities::removeWindowEventListener(window_, this);
            windowClosed(window_);
        }

        /**
         * Called on window closing.
         *
         * Detaches OIS before window shutdown (very important under Linux).
         */
        virtual void windowClosed(Ogre::RenderWindow* rw){exit_ = true;}

        /**
         * Called when a frame is about to begin rendering.
         *
         * This event happens before any render targets have begun updating.
         * It captures input events and checks the state of entities.
         *
         * @param evt[in] Triggering event. Unused.
         * @return True to go ahead, false to abort rendering and drop out of
         * the rendering loop.
         */
        bool frameStarted(const Ogre::FrameEvent& evt){

            // TODO: Move implementation to cpp file.

            if (exit_ == true) return false;
            if (keyboard_) keyboard_->capture();
            if (mouse_) mouse_->capture();

            for (unsigned int i = 0; i < entities.size(); ++ i){
                if (entities[i]->isVisible() == true){
                    Ogre::AnimationStateIterator animations
                      = entities[i]->getAllAnimationStates()->getAnimationStateIterator();
                    while (animations.hasMoreElements() == true){
                        Ogre::AnimationState* state = animations.getNext();
                        if (state->getEnabled() == true){
                            state->addTime(evt.timeSinceLastFrame);
                            info_text->setCaption(
                              entities[i]->getName() + ": " + state->getAnimationName()
                            );
                        }
                    }
                    break;
                }
            }
            if (keyboard_->isKeyDown(OIS::KC_A)){
                assert(camera_);
                camera_->moveRelative(Ogre::Vector3(-0.005f, 0.0f, 0.0f));
            }
            if (keyboard_->isKeyDown(OIS::KC_D)){
                assert(camera_);
                camera_->moveRelative(Ogre::Vector3(0.005f, 0.0f, 0.0f));
            }
            if (keyboard_->isKeyDown(OIS::KC_W)){
                assert(camera_);
                camera_->moveRelative(Ogre::Vector3(0.0f, 0.0f, -0.005f));
            }
            if (keyboard_->isKeyDown(OIS::KC_S)){
                assert(camera_);
                camera_->moveRelative(Ogre::Vector3(0.0f, 0.0f,  0.005f));
            }
            if (mouse_rotate_ == true){
                assert(camera_);
                camera_->yaw(Ogre::Degree(-mouse_move_x_ * 0.13f));
                camera_->pitch(Ogre::Degree(-mouse_move_y_ * 0.13f));
                mouse_move_x_ = 0;
                mouse_move_y_ = 0;
            }
            return true;
        }

        /**
         * The key listener.
         *
         * Parses keyboard events.
         *
         * @param event[in] Triggering event.
         * @return Always true
         */
        bool keyPressed(const OIS::KeyEvent& e){
            switch(e.key){
                case OIS::KC_RIGHT:
                    {
                        bool change = false;
                        for (unsigned int i = 0; i < entities.size(); ++ i){
                            if (change == true){
                                entities[i]->setVisible(true);
                                break;
                            }
                            if (entities[i]->isVisible() == true && (i != entities.size() - 1)){
                                entities[i]->setVisible(false);
                                change = true;
                            }
                        }
                    }
                    break;
                case OIS::KC_LEFT:
                    {
                        bool change = false;
                        for(int i = entities.size() - 1; i >= 0; -- i){
                            if (change == true){
                                entities[i]->setVisible(true);
                                break;
                            }
                            if (entities[i]->isVisible() == true && i != 0){
                                entities[i]->setVisible(false);
                                change = true;
                            }
                        }
                    }
                    break;
                case OIS::KC_UP:
                    {
                        bool change = false;
                        for (unsigned int i = 0; i < entities.size(); ++ i){
                            if (entities[i]->isVisible() == true){
                                Ogre::AnimationStateIterator animations
                                  = entities[i]->getAllAnimationStates()
                                    ->getAnimationStateIterator();
                                Ogre::AnimationState* old_state;
                                Ogre::AnimationState* new_state = animations.getNext();
                                while (animations.hasMoreElements() == true){
                                    old_state = new_state;
                                    new_state = animations.getNext();
                                    if (new_state->getEnabled() == true){
                                        new_state->setEnabled(false);
                                        new_state->setLoop(false);
                                        old_state->setEnabled(true);
                                        old_state->setLoop(true);
                                    }
                                }
                                break;
                            }
                        }
                    }
                    break;
                case OIS::KC_DOWN:{
                    bool change = false;
                    for (unsigned int i = 0; i < entities.size(); ++ i){
                        if (entities[i]->isVisible() == true){
                            Ogre::AnimationStateIterator animations
                              = entities[i]->getAllAnimationStates()->getAnimationStateIterator();
                            while (animations.hasMoreElements() == true){
                                Ogre::AnimationState* state = animations.getNext();
                                if (change == true){
                                    state->setEnabled(true);
                                    state->setLoop(true);
                                    break;
                                }
                                if (
                                  state->getEnabled() == true
                                  && animations.hasMoreElements() != false
                                ){
                                    state->setEnabled(false);
                                    state->setLoop(false);
                                    change = true;
                                }
                            }
                            break;
                        }
                    }
                }
                break;
            }
            return true;
        }

        /**
         * Handles key releasing events.
         *
         * It does nothing.
         *
         * @param e[in] Triggering event.
         * @return Always true.
         */
        bool keyReleased(const OIS::KeyEvent& e){return true;}

        /**
         * Handles mouse movement events.
         *
         * @param e[in] Triggering event.
         * @return Always true.
         */
        bool mouseMoved(const OIS::MouseEvent& e){
            mouse_move_x_ = float(e.state.X.rel);
            mouse_move_y_ = float(e.state.Y.rel);
            return true;
        }

        /**
         * Handles mouse button-down events.
         *
         * @param e[in] Triggering event.
         * @param id[in] Button ID.
         * @return Always true.
         */
        bool mousePressed(const OIS::MouseEvent& e, OIS::MouseButtonID id){
            if (id == OIS::MB_Right) mouse_rotate_ = true;
            return true;
        }

        /**
         * Handles mouse button-up events.
         *
         * @param e[in] Triggering event.
         * @param id[in] Button ID.
         * @return Always true.
         */
        bool mouseReleased(const OIS::MouseEvent& e, OIS::MouseButtonID id){
            if (id == OIS::MB_Right) mouse_rotate_ = false;
            return true;
        }

        /**
         * The information text.
         */
        Ogre::TextAreaOverlayElement* info_text;

        /**
         * The information overlay.
         */
        Ogre::Overlay* info_overlay;

        /**
         * Sets the viewport camera
         */
        virtual void setCamera(Ogre::Camera *camera){camera_ = camera;}

    private:

        /**
         * The render window.
         */
        Ogre::RenderWindow* window_;

        /**
         * The OIS input manager.
         */
        OIS::InputManager* input_manager_;

        /**
         * The OIS keyboard.
         */
        OIS::Keyboard* keyboard_;

        /**
         * The OIS mouse.
         */
        OIS::Mouse* mouse_;

        /**
         * The camera.
         */
        Ogre::Camera *camera_;

        /**
         * Mouse horizontal movement.
         */
        float mouse_move_x_;

        /**
         * Mouse vertical movement.
         */
        float mouse_move_y_;

        /**
         * Indicates if the mouse is being rotated.
         */
        bool mouse_rotate_;

        /**
         * Flag for application exit.
         */
        bool exit_;
};

/**
 * Initializes the Ogre system.
 *
 * @param name[in] Application title.
 */
void InitializeOgreBase(const Ogre::String& name);

/**
 * Destructs the Ogre system.
 */
void DeinitializeOgreBase();
