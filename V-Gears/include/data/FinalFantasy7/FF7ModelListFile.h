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
#include "common/VGearsResource.h"

namespace VGears{

    /**
     * Handles model list files.
     */
    class ModelListFile : public Resource{

        public:

            /**
             * Constructor.
             *
             * @param creator[in] Pointer to the ResourceManager that is
             * this resource.
             * @param name[in] The unique name of the resource.
             * @param handle[in] @todo Understand and document.
             * @param group[in] The name of the resource group to which this
             * resource belong.
             * @param is_manual[in] True if the resource is manually loaded,
             * false otherwise.
             * @param loader[in] Pointer to a ManualResourceLoader
             * implementation which will be called when the Resource wishes to
             * load (should be supplied if is_manual is set to true). It can
             * be null, but the Resource will never be able to reload if
             * anything ever causes it to unload. Therefore provision of a
             * proper ManualResourceLoader instance is strongly recommended.
             */
            ModelListFile(
              Ogre::ResourceManager* creator, const String &name,
              Ogre::ResourceHandle handle, const String& group,
              bool is_manual = false, Ogre::ManualResourceLoader* loader = NULL
            );

            /**
            * Destructor.
            */
            virtual ~ModelListFile();

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

            /**
             * An animation description.
             */
            struct AnimationDescription{

                /**
                 * The animation name.
                 */
                String  name;

                /**
                 * Unknown data.
                 */
                uint16  unknown;
            };

            typedef std::vector<AnimationDescription> AnimationList;

            /**
             * Types of models.
             */
            enum ModelType{

                /**
                 * Playable character.
                 */
                PLAYER = 0,

                /**
                 * Non playable character.
                 */
                NPC = 1,

                /**
                 * Unknown character type
                 */
                UNKNOWN
            };

            /**
             * A model.
             */
            struct ModelDescription{

                /**
                 * The model name.
                 */
                String name;

                /**
                 * The model type.
                 */
                ModelType type;

                /**
                 * HRC file name.
                 *
                 * HRC files describe bone hierarchy.
                 */
                String hrc_name;

                /**
                 * Scale for the model.
                 */
                String scale;


                Ogre::ColourValue   light_colors[10];

                /**
                 * List of animations assigned to the model.
                 */
                AnimationList animations;
            };

            typedef std::vector<ModelDescription>   ModelList;

            /**
             * Retrieves the scale for the models.
             *
             * @return The scale.
             */
            virtual uint16 GetScale() const;

            /**
             * Sets the scale for the models.
             *
             * @param scale[in] The scale.
             */
            virtual void SetScale(uint16 scale);

            /**
             * Retrieves the model list.
             */
            virtual ModelList& GetModels();

        protected:

            /**
             * Loads the file.
             */
            virtual void loadImpl() override;

            /**
             * Unloads the file.
             */
            virtual void unloadImpl() override;

            /**
             * Calculates the size of the palette.
             *
             * @return The size of the palette.
             * @todo Units?
             */
            virtual size_t calculateSize( void ) const;

        private:

            /**
             * The models scale.
             *
             * @todo Each model has it's own scale. Is this some kind of
             * global scale? If so, is it to be applied over each model
             * scale, or instead of it?
             */
            uint16 scale_;

            /**
             * The list of models.
             */
            ModelList models_;
    };

    typedef Ogre::SharedPtr<ModelListFile> ModelListFilePtr;
}
