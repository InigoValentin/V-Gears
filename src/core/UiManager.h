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

#include <OgreRenderQueueListener.h>
#include <OgreSingleton.h>
#include <OgreUTFString.h>
#include <tinyxml.h>
#include "Manager.h"
#include "UiFont.h"
#include "UiWidget.h"

/**
 * The UI manager.
 */
class UiManager :
  public Manager, public Ogre::RenderQueueListener, public Ogre::Singleton<UiManager>
{

    public:

        /**
         * Constructor.
         */
        UiManager();

        /**
         * Destructor.
         */
        virtual ~UiManager();

        /**
         * Initializes the manager.
         *
         * Loads all available fonts and screens from _fonts.xml and
         * screens.xml, respectably.
         */
        void Initialise();

        /**
         * Handles an input event.
         *
         * @param[in] event Event to handle.
         */
        void Input(const VGears::Event& event) override;

        /**
         * Updates the elements in the manager with debug information.
         */
        void UpdateDebug() override;

        /**
         * Handles resizing events.
         */
        void OnResize();

        /**
         * Clears all field UI elements in the manager.
         */
        void ClearField() override;

        /**
         * Clears all battle UI elements in the manager.
         */
        void ClearBattle() override;

        /**
         * Clears all world map UI elements in the manager.
         */
        void ClearWorld() override;

        /**
         * Adds a font to the manager.
         *
         * @param[in] font The font to add.
         */
        void AddFont(UiFont* font);

        /**
         * Retrieves a font by name.
         *
         * @param[in] name Name of the font.
         * @return The font by the specified name, or NULL if there is none.
         */
        UiFont* GetFont(const Ogre::String& name);

        /**
         * Adds a prototype to the manager.
         *
         * @param[in] name The prototype manager.
         * @param[in] prototype The prototyme, as an XML node.
         * @todo What exactly is a prototype here?
         */
        void AddPrototype(const Ogre::String& name, TiXmlNode* prototype);

        /**
         * Retrieves a prototype by name.
         *
         * @param[in] name Name of the prototype.
         * @return The prototype by the specified name, or NULL if there is
         * none.
         * @todo What exactly is a prototype here?
         */
        TiXmlNode* GetPrototype(const Ogre::String& name) const;

        /**
         * Adds a UI widget to the manager.
         *
         * @param[in] widget The widget to add.
         */
        void AddWidget(UiWidget* widget);

        /**
         * Retrieves a UI widget by name.
         *
         * @param[in] name Name of the widget.
         * @return The widget by the specified name, or NULL if there is none.
         */
        UiWidget* GetWidget(const Ogre::String& name);

        /**
         * Retrieves a UI widget by name.
         *
         * @param[in] name Name of the widget.
         * @return The widget by the specified name, or NULL if there is none.
         */
        UiWidget* ScriptGetWidget(const char* name);

        /**
         * Sets the colours of the background of every window.
         *
         * They are applied to all the existing window backgrounds (widgets marked with
         * UiWidget::SetWindowBackground), and to the ones created later.
         *
         * @param[in] r1 Red component for the top-left corner (0-1).
         * @param[in] g1 Green component for the top-left corner (0-1).
         * @param[in] b1 Blue component for the top-left corner (0-1).
         * @param[in] r2 Red component for the top-right corner (0-1).
         * @param[in] g2 Green component for the top-right corner (0-1).
         * @param[in] b2 Blue component for the top-right corner (0-1).
         * @param[in] r3 Red component for the bottom-right corner (0-1).
         * @param[in] g3 Green component for the bottom-right corner (0-1).
         * @param[in] b3 Blue component for the bottom-right corner (0-1).
         * @param[in] r4 Red component for the bottom-left corner (0-1).
         * @param[in] g4 Green component for the bottom-left corner (0-1).
         * @param[in] b4 Blue component for the bottom-left corner (0-1).
         */
        void SetWindowColours(
          const float r1, const float g1, const float b1,
          const float r2, const float g2, const float b2,
          const float r3, const float g3, const float b3,
          const float r4, const float g4, const float b4
        );

        /**
         * Applies the window colours to a widget if it is a window background, and to its
         * children. Does nothing until the colours have been set.
         *
         * @param[in] widget The widget.
         */
        void ApplyWindowColours(UiWidget* widget);

        /**
         * Updates the render queue.
         *
         * @param[in] queueGroupId The queue group ID.
         * @param[in] invocation Unused.
         * @param[in] skipThisInvocation Unused.
         */
        void renderQueueStarted(
          Ogre::uint8 queueGroupId, const Ogre::String& invocation,
          bool& skipThisInvocation
        );

        /**
         * Draws the widgets when the overlay queue ends.
         *
         * The screen fade and the movies are drawn when the overlay queue starts, and the order
         * the listeners are called in is the order they were added in. The UI was added first,
         * so it was drawn before them, and they were drawn over the dialogs. At the end of the
         * queue it is drawn over all of them.
         *
         * @param[in] queueGroupId The queue group ID.
         * @param[in] invocation Unused.
         * @param[in] repeatThisInvocation Unused.
         */
        void renderQueueEnded(
          Ogre::uint8 queueGroupId, const Ogre::String& invocation,
          bool& repeatThisInvocation
        );

    private:

        /**
         * Updates the field UI elements in the manager.
         */
        void UpdateField() override;

        /**
         * Updates the battle UI elements in the manager.
         */
        void UpdateBattle() override;

        /**
         * Updates the world map UI elements in the manager.
         */
        void UpdateWorld() override;

        /**
         * List of fonts in the manager.
         */
        std::vector<UiFont*> fonts_;

        /**
         * A UI prototype.
         *
         * @todo What exactly is a prototype here?
         */
        struct UiPrototype{

            /**
             * The prototype node.
             */
            Ogre::String name;

            /**
             * The prototype structure as an XML node.
             */
            TiXmlNode* node;
        };

        /**
         * List of prototypes.
         */
        std::vector<UiPrototype> prototypes_;

        /**
         * List of widgets.
         */
        std::vector<UiWidget*> widgets_;

        /**
         * Colours of the windows (RGB of the top-left, top-right, bottom-right and bottom-left
         * corners). Only used if window_colours_set_.
         */
        float window_colours_[12];

        /**
         * Indicates if the window colours were set.
         */
        bool window_colours_set_ = false;
};
