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

#include "common/QGearsResource.h"
#include "core/EntityPoint.h"
#include "core/EntityTrigger.h"

namespace QGears{

    /**
     * Handles map files.
     */
    class MapFile : public Resource{
        
        public:

            /**
             * The type of resource
             */
            static const String RESOURCE_TYPE;

            /**
             * Constructor.
             *
             * @param creator[in] Pointer to the ResourceManager that is
             * creating this resource.
             * @param name[in] The unique name of the resource.
             * @param handle[in] @todo Understand and document.
             * @param group[in] The name of the resource group to which this
             * resource belong.
             * @param is_manual[in] True if the resource is manually loaded,
             * false otherwise.
             * @param loader[in] Pointer to a ManualResourceLoader
             * implementation which will be called when the Resource wishes to
             * load (should be supplied if is_manual is set to true). It can be
             * null, but the Resource will never be able to reload if anything
             * ever causes it to unload. Therefore provision of a proper
             * ManualResourceLoader instance is strongly recommended.
             */
            MapFile(
              Ogre::ResourceManager* creator, const String &name,
              Ogre::ResourceHandle handle, const String& group,
              bool is_manual = false,
              Ogre::ManualResourceLoader* loader = NULL
            );

            /**
            * Destructor.
            */
            virtual ~MapFile();

            typedef EntityPoint Point;

            typedef std::vector<Point> PointList;

            typedef EntityTrigger Trigger;

            typedef std::vector<Trigger> TriggerList;

            /**
             * Retrieves the map script name.
             *
             * @return The script name.
             */
            virtual const String& GetScriptName() const;

            /**
             * Retrieves the map background name.
             *
             * @return The background name.
             */
            virtual const String& GetBackground2dName() const;

            /**
             * Retrieves the map walkmesh name.
             *
             * @return The walkmesh name.
             */
            virtual const String& GetWalkmeshName() const;

            /**
             * Retrieves the map forward direction.
             *
             * The forward direction is the direction the playable character
             * moves when 'up' is pressed.
             *
             * @return The forward direction.
             */
            virtual const Ogre::Real& GetForwardDirection () const;

            /**
             * Retrieves the list of entity points in the map.
             *
             * @return The list of entity points in the map.
             */
            virtual PointList& GetPoints();

            /**
             * Retrieves the map triggers.
             *
             * @return The list of triggers.
             */
            virtual TriggerList& GetTriggers();

            /**
             * Sets the map script name.
             *
             * @param[in] The script name.
             */
            virtual void SetScriptName(const String &script_name);

            /**
             * Sets the map background name.
             *
             * @param[in] The background name.
             */
            virtual void SetBackground2dName(const String &background2d_name);

            /**
             * Sets the map walkmesh name.
             *
             * @param[in] The walkmesh name.
             */
            virtual void SetWalkmeshName(const String &walkmesh_name);

            /**
             * Sets the map forward direction.
             *
             * The forward direction is the direction the playable character
             * moves when 'up' is pressed.
             *
             * @param forward_direction[in] The forward direction.
             */
            virtual void SetForwardDirection(
              const Ogre::Real &forward_direction
            );

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
            virtual size_t calculateSize() const;

        private:

            /**
             * The script name.
             */
            String script_name_;

            /**
             * The background name.
             */
            String background_2d_name;

            /**
             * The walkmesh name.
             */
            String walkmesh_name_;

            /**
             * The forward direction.
             */
            Ogre::Real forward_direction_;

            /**
             * The list of entity points.
             */
            PointList points_;

            /**
             * The list of triggers.
             */
            TriggerList triggers_;
    };
    
    typedef Ogre::SharedPtr<MapFile> MapFilePtr;
}

