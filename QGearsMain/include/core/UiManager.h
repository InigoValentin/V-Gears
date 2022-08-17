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
#include "UiFont.h"
#include "UiWidget.h"

/**
 * The UI manager.
 */
class UiManager
  : public Ogre::RenderQueueListener, public Ogre::Singleton<UiManager>
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
         * Loads all available fonts and screens from fonts.xml and
         * screens.xml, respectably.
         */
        void Initialise();

        /**
         * Updates the UI elements in the manager.
         */
        void Update();

        /**
         * Handles resizing events.
         */
        void OnResize();

        /**
         * Adds a font to the manager.
         *
         * @param font[in] The font to add.
         */
        void AddFont(UiFont* font);

        /**
         * Retrieves a font by name.
         *
         * @param name[in] Name of the font.
         * @return The font by the specified name, or NULL if there is none.
         */
        UiFont* GetFont(const Ogre::String& name);

        /**
         * Adds a prototype to the manager.
         *
         * @param name[in] The prototype manager.
         * @param prototype[in] The prototyme, as an XML node.
         * @todo What exactly is a prototype here?
         */
        void AddPrototype(const Ogre::String& name, TiXmlNode* prototype);

        /**
         * Retrieves a prototype by name.
         *
         * @param name[in] Name of the prototype.
         * @return The prototype by the specified name, or NULL if there is
         * none.
         * @todo What exactly is a prototype here?
         */
        TiXmlNode* GetPrototype(const Ogre::String& name) const;

        /**
         * Adds a UI widget to the manager.
         *
         * @param widget[in] The widget to add.
         */
        void AddWidget(UiWidget* widget);

        /**
         * Retrieves a UI widget by name.
         *
         * @param name[in] Name of the widget.
         * @return The widget by the specified name, or NULL if there is none.
         */
        UiWidget* GetWidget(const Ogre::String& name);

        /**
         * Retrieves a UI widget by name.
         *
         * @param name[in] Name of the widget.
         * @return The widget by the specified name, or NULL if there is none.
         */
        UiWidget* ScriptGetWidget(const char* name);

        /**
         * Updates the render queue.
         *
         * @param queueGroupId[in] The queue group ID.
         * @param invocation[in] Unused.
         * @param skipThisInvocation[in] Unused.
         */
        void renderQueueStarted(
          Ogre::uint8 queueGroupId, const Ogre::String& invocation,
          bool& skipThisInvocation
        );

    private:

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
