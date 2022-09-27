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

#include <OgreQuaternion.h>
#include <Ogre.h>
#include "common/VGearsResource.h"
#include "VGearsTile.h"

namespace VGears{

    /**
     * Handles 2D background files
     *
     * @todo Whats the difference with VGearsBackgroundFile.h?
     */
    class Background2DFile : public Resource{
        public:

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
            Background2DFile(
              Ogre::ResourceManager* creator, const String &name,
              Ogre::ResourceHandle handle, const String& group,
              bool is_manual = false,
              Ogre::ManualResourceLoader* loader = nullptr
            );

            /**
            * Destructor.
            */
            virtual ~Background2DFile();

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

            typedef std::vector<Tile> TileList;

            /**
             * Sets the texture name.
             *
             * @param[in] The name of the texture.
             */
            virtual void SetTextureName(const String &texture_name);

            /**
             * Retrieves the texture name.
             *
             * @return The texture name.
             */
            virtual String GetTextureName() const;

            /**
             * Sets the background range.
             *
             * @param[in] Range vector.
             * @todo What is this range?
             */
            virtual void SetRange(const Ogre::Vector4& range);

            /**
             * Retrieves the background range.
             *
             * @return 4-dimensional vector with the background range.
             * @todo What is this range?
             */
            virtual Ogre::Vector4 GetRange() const;

            /**
             * Sets the background clip.
             *
             * @param[in] Clip vector.
             * @todo What is this clip?
             */
            virtual void SetClip(const Ogre::Vector2& clip);

            /**
             * Retrieves the background clip.
             *
             * @return 4-dimensional vector with the background clip.
             * @todo What is this clip?
             */
            virtual Ogre::Vector2 GetClip() const;

            /**
             * Sets the background position.
             *
             * @param[in] Position vector.
             */
            virtual void SetPosition(const Ogre::Vector3 &position);

            /**
             * Retrieves the background clip.
             *
             * @return 4-dimensional vector with the background clip.
             * @todo What is this clip?
             */
            virtual Ogre::Vector3 GetPosition() const;

            /**
             * Sets the background orientation.
             *
             * @param orientation[in] The new orientation.
             */
            virtual void SetOrientation(const Ogre::Quaternion &orientation);

            /**
             * Retrieves the background orientation.
             *
             * @return The background orientation quaternion.
             */
            virtual Ogre::Quaternion GetOrientation() const;

            /**
             * Sets the background field of view.
             *
             * @param fow[in] Field of view angle.
             */
            virtual void SetFov(const Ogre::Radian &fov);

            /**
             * Retrieves the background field of view.
             *
             * @return The field of view angle.
             */
            virtual Ogre::Radian GetFov() const;

            /**
             * Retrieves the list of tiles.
             */
            virtual TileList& GetTiles();

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
             * Calculates the size of the background.
             *
             * @return The size of the background.
             * @todo Units?
             */
            virtual size_t calculateSize() const override;

        private:

            /**
             * The texture name.
             */
            String texture_name_;

            /**
             * The background clip.
             */
            Ogre::Vector2 clip_;

            /**
             * The background range.
             */
            Ogre::Vector4 range_;

            /**
             * The background position.
             */
            Ogre::Vector3 position_;

            /**
             * The background orientation.
             */
            Ogre::Quaternion orientation_;

            /**
             * The background field of view.
             */
            Ogre::Radian fov_;

            /**
             * The list of tiles.
             */
            TileList tiles_;
    };

    typedef Ogre::SharedPtr<Background2DFile> Background2DFilePtr;
}
