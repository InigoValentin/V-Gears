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
};
