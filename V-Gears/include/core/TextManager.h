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

#include <OgreString.h>
#include <OgreSingleton.h>
#include <tinyxml.h>

/**
 * The text manager.
 */
class TextManager : public Ogre::Singleton<TextManager>{

    public:

        /**
         * Constructor.
         */
        TextManager();

        /**
         * Destructor.
         */
        virtual ~TextManager();

        /**
         * Initializes the commands for the text manager.
         */
        void InitCmd();

        /**
         * Sets the language for texts.
         *
         * @param language[in] Language identifier. Must be the name of one of
         * the folders in the data/text/.
         */
        void SetLanguage(const Ogre::String& language);

        /**
         * Retrieves the currently set language.
         *
         * @return The current language.
         */
        const Ogre::String& GetLanguage();

        /**
         * Adds text.
         *
         * @param name[in] Text name or identifier.
         * @param node[in] Text node in an XML file.
         */
        void AddText(const Ogre::String& name, TiXmlNode* node);

        /**
         * Adds a text dialog.
         *
         * @param name[in] Text name or identifier.
         * @param node[in] Text node in an XML file.
         * @param width[in] Text width.
         * @param height[in] Text height.
         */
        void AddDialog(
          const Ogre::String& name, TiXmlNode* node,
          const float width, const float height
        );

        /**
         * Retrieves a text by name.
         *
         * @param name[in] Text name or identifier.
         * @return Text node in an XML file, or NULL if there is none.
         */
        TiXmlNode* GetText(const Ogre::String& name) const;

        /**
         * Retrieves a dialog and it's dimensions by name.
         *
         * @param name[in] Text name or identifier.
         * @param width[out] The text width will be saved here. It won't change
         * if the dialog is not found.
         * @param height[out] The text height will be saved here. It won't
         * change if the dialog is not found.
         * @return Text node in an XML file, or NULL if there is none.
         */
        TiXmlNode* GetDialog(
          const Ogre::String& name, float &width, float& height
        ) const;

        /**
         * Deletes all texts in the manager.
         */
        void UnloadTexts();

    private:

        /**
         * The language for the texts.
         */
        Ogre::String language_;

        /**
         * A text.
         */
        struct Text{

            /**
             * Name or identifier of the text.
             */
            Ogre::String name;

            /**
             * Text node in an XML file.
             */
            TiXmlNode* node;
        };

        /**
         * The list of texts in the manager.
         */
        std::vector<Text> texts_;

        /**
         * A dialog.
         */
        struct Dialog{

            /**
             * Name or identifier of the text.
             */
            Ogre::String name;

            /**
             * Text node in an XML file.
             */
            TiXmlNode* node;

            /**
             * The text width.
             */
            float width;

            /**
             * The text height.
             */
            float height;
        };

        /**
         * The list of dialogs in the manager.
         */
        std::vector<Dialog> dialogs_;
};

