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

#include <memory>
#include <array>
#include <OgreResourceManager.h>
#include "VGearsPrerequisites.h"
#include "common/VGearsResource.h"
#include "VGearsSerializer.h"

namespace VGears{

    /**
     * A manager for trigger files.
     */
    class TriggersFileManager :
      public Ogre::ResourceManager, public Ogre::Singleton<TriggersFileManager>
    {

        // TODO: Move this to it's own file?

        public:

            /**
             * Constructor.
             */
            TriggersFileManager();

            /**
             * Destructor.
             */
            virtual ~TriggersFileManager();

            /**
             * Retrieves a singleton to the manager.
             */
            static TriggersFileManager& GetSingleton();

            /**
             * Retrieves a pointer to the manager singleton.
             */
            static TriggersFileManager* GetSingletonPtr();

        protected:

            /**
             * Loads the manager.
             *
             * @param[in] name The unique name of the manager.
             * @param[in] handle @todo Understand and document.
             * @param[in] group The name of the resource group to which this
             * resource belong.
             * @param[in] is_manual True if the resource is manually loaded,
             * false otherwise.
             * @param[in] loader Pointer to a ManualResourceLoader
             * implementation which will be called when the Resource wishes to
             * load (should be supplied if is_manual is set to true). It can be
             * null, but the Resource will never be able to reload if anything
             * ever causes it to unload. Therefore provision of a proper
             * ManualResourceLoader instance is strongly recommended.
             * @param[in] create_params Unused.
             */
            Ogre::Resource *createImpl(
              const Ogre::String &name, Ogre::ResourceHandle handle, const Ogre::String &group,
              bool is_manual, Ogre::ManualResourceLoader *loader,
              const Ogre::NameValuePairList *create_params
            );
    };

    /**
     * Handles trigger files.
     */
    class TriggersFile : public Resource{

        public:

            /**
             * Constructor.
             *
             * @param[in] creator Pointer to the ResourceManager that is
             * creating this resource.
             * @param[in] name The unique name of the resource.
             * @param[in] handle @todo Understand and document.
             * @param[in] group The name of the resource group to which this
             * resource belong.
             * @param[in] is_manual True if the resource is manually loaded,
             * false otherwise.
             * @param[in] loader Pointer to a ManualResourceLoader
             * implementation which will be called when the Resource wishes to
             * load (should be supplied if is_manual is set to true). It can be
             * null, but the Resource will never be able to reload if anything
             * ever causes it to unload. Therefore provision of a proper
             * ManualResourceLoader instance is strongly recommended.
             */
            TriggersFile(
              Ogre::ResourceManager* creator, const String &name, Ogre::ResourceHandle handle,
              const String& group, bool is_manual = false,
              Ogre::ManualResourceLoader* loader = nullptr
            );

            /**
             * Destructor.
             */
            virtual ~TriggersFile();

            /**
             * The type pf resource.
             */
            static const String RESOURCE_TYPE;

            /**
             * Trigger range.
             *
             * @todo What range?
             */
            struct Range{

                /**
                 * Left range.
                 */
                s16 left;

                /**
                 * Top range.
                 */
                s16 top;

                /**
                 * Right range.
                 */
                s16 right;

                /**
                 * Bottom range.
                 */
                s16 bottom;
            };

            /**
             * Each of the trigger vertices.
             */
            struct TriggerVertex{

                /**
                 * X coordinate.
                 */
                s16 x;

                /**
                 * Y coordinate.
                 */
                s16 y;

                /**
                 * Z coordinate.
                 */
                s16 z;
            };

            /**
             * A gateway.
             *
             * Gateways are a special kind of trigger.
             */
            struct Gateway{

                /**
                 * The gateway line, between two vertices.
                 */
                std::array<TriggerVertex, 2> exit_line;

                /**
                 * The destination point in the target map.
                 */
                TriggerVertex destination;

                /**
                 * ID of the destination field.
                 */
                u16 destination_field_id;

                /**
                 * @todo Understand and document.
                 */
                u8 dir;

                /**
                 * @todo Understand and document.
                 */
                u8 dir_copy1;

                /**
                 * @todo Understand and document.
                 */
                u8 dir_copy2;

                /**
                 * @todo Understand and document.
                 */
                u8 dir_copy3;
            };

            /**
             * A trigger.
             */
            struct Trigger{

                /**
                 * The trigger line, defined by two vertices.
                 */
                std::array<TriggerVertex, 2> trigger_line;

                /**
                 * @todo Understand and document.
                 */
                u8 background_parameter;

                /**
                 * @todo Understand and document.
                 */
                u8 background_state;

                /**
                 * The trigger behavior.
                 *
                 * 0 - OnTrigger - ON.
                 * 1 - OnTrigger - OFF.
                 * 2 - OnTrigger - ON, AwayFromTrigger - OFF.
                 * 3 - OnTrigger - OFF, AwayFromTrigger - ON.
                 * 4 - OnTrigger - ON, AwayFromTriggerOnPlusSide - OFF.
                 * 5 - OnTrigger - OFF, AwayFromTriggerOnPlusSide - ON.
                 */
                u8 behavior;

                /**
                 * Trigger sound.
                 */
                u8 sound_id;
            };

            /**
             * A map arrow.
             */
            struct Arrow{

                /**
                 * X coordinate.
                 */
                s32 x;

                /**
                 * Z coordinate.
                 */
                s32 z;

                /**
                 * Y coordinate.
                 */
                s32 y;

                /**
                 * Arrow type
                 *
                 * 0 - Invisible, 1 - Red, 2 - Green.
                 */
                u32 type;
            };

            /**
             * Trigger data.
             */
            struct TriggerData{

                /**
                 * Trigger name.
                 */
                std::array<char, 9> name;

                /**
                 * @todo Understand and document.
                 */
                u8 control;

                /**
                 * @todo Understand and document.
                 */
                s16 camera_focus_height;

                /**
                 * @todo Understand and document.
                 *
                 * 8 bytes.
                 */
                Range camera_range;

                /**
                 * Flag for the background layer 1.
                 */
                u8 bg_layer1_flag;

                /**
                 * Flag for the background layer 2.
                 */
                u8 bg_layer2_flag;

                /**
                 * Flag for the background layer 3.
                 */
                u8 bg_layer3_flag;

                /**
                 * Flag for the background layer 4.
                 */
                u8 bg_layer4_flag;

                /**
                 * @todo Understand and document.
                 */
                s16 bg_layer3_width;

                /**
                 * @todo Understand and document.
                 */
                s16 bg_layer3_height;

                /**
                 * @todo Understand and document.
                 */
                s16 bg_layer4_width;

                /**
                 * @todo Understand and document.
                 */
                s16 bg_layer4_height;

                /**
                 * Unknown data.
                 */
                std::array<u8, 24> unknown;

                /**
                 * Doors (gateways).
                 *
                 * A door is 24 * 12 bytes.
                 */
                std::array<Gateway, 12> doors;

                /**
                 * Triggers.
                 *
                 * A trigger is 16 * 12 bytes.
                 */
                std::array<Trigger, 12> triggers;

                /**
                 * The map arrows
                 *
                 * Only in occidental/international version.
                 */
                std::array<u8, 12> display_arrow;

                /**
                 * The map arrows
                 *
                 * Only in occidental/international version. 16 * 12 bytes.
                 */
                std::array<Arrow, 12> arrows;
            };


            /**
             * Retrieves the camera range.
             *
             * @return The camera range.
             */
            const Range& GetCameraRange() const{return trigger_data_->camera_range;}

            /**
             * Retrieves the movement rotation.
             *
             * The movement rotation is the angle in which the player moves
             * when "up" is pressed.
             *
             * @return The movement rotation.
             */
            float MovementRotation() const{
                return 180.0f * (static_cast<float>(trigger_data_->control) - 128.0f) / 128.0f;
            }

            /**
             * Retrieves the list of gateways.
             */
            const std::array<Gateway, 12>& GetGateways() const{return trigger_data_->doors;}

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
            virtual size_t calculateSize(void) const override;

        private:

            /**
             * The trigger data.
             */
            std::unique_ptr<TriggerData> trigger_data_;

            friend class TriggerFileSerializer;
    };
    
    /**
     * Handles the serialization of trigger files.
     */
    class TriggerFileSerializer : public Serializer{

        // TODO Move this to it's own file.

        public:

            /**
             * Constructor.
             */
            TriggerFileSerializer();

            /**
             * Imports a trigger file.
             *
             * @param[in] stream The contents of the trigger file.
             * @param[out] dest The formed trigger file.
             */
            void ImportTriggerFile(Ogre::DataStreamPtr &stream, TriggersFile *dest);

        private:

            /**
             * Reads trigger vertex data from the trigger file.
             *
             * @param[in] stream The contents of the trigger file.
             * @param[out] vertex The vertex data.
             */
            void ReadTriggerVertex(
              Ogre::DataStreamPtr& stream, TriggersFile::TriggerVertex& vertex
            );

            /**
             * Reads range data from the trigger file.
             *
             * @param[in] stream The contents of the trigger file.
             * @param[out] range The range data.
             */
            void ReadRange(Ogre::DataStreamPtr& stream, TriggersFile::Range& range);

            /**
             * Reads gateway data from the trigger file.
             *
             * @param[in] stream The contents of the trigger file.
             * @param[out] exit The gateway data.
             */
            void ReadGateway(Ogre::DataStreamPtr& stream, TriggersFile::Gateway& exit);

            /**
             * Reads arrow data from the trigger file.
             *
             * @param[in] stream The contents of the trigger file.
             * @param[out] arrow The arrow data.
             */
            void ReadArrow(Ogre::DataStreamPtr& stream, TriggersFile::Arrow& arrow);

            /**
             * Reads trigger data from the trigger file.
             *
             * @param[in] stream The contents of the trigger file.
             * @param[out] trigger The trigger data.
             */
            void ReadTrigger(Ogre::DataStreamPtr& stream, TriggersFile::Trigger& trigger);
    };

    typedef Ogre::SharedPtr<TriggersFile> TriggersFilePtr;
}
