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

#include <OgreTexture.h>
#include "common/VGearsResource.h"
#include "common/TypeDefine.h"
#include "map/VGearsBackground2DFile.h"
#include "map/VGearsWalkmeshFile.h"
#include "FinalFantasy7/FF7ModelListFile.h"
#include "VGearsBackgroundFile.h"
#include "VGearsCameraMatrixFile.h"
#include "VGearsPaletteFile.h"
#include "VGearsHRCFile.h"
#include "VGearsTriggersFile.h"

namespace VGears{
    // TODO: move flevel stuff to ff7 as it is ff7 related!?

    using FF7::ModelListFile;

    using FF7::ModelListFilePtr;

    class FLevelTextureLoader;

    class FLevelBackground2DLoader;

    /**
     * A flevel file.
     *
     * Flevel files are LXS compressed files containing level data in the game
     * PC version.
     */
    class FLevelFile : public Resource{

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
            FLevelFile(
              Ogre::ResourceManager *creator, const String &name,
              Ogre::ResourceHandle handle, const String &group,
              bool is_manual = false, Ogre::ManualResourceLoader *loader = NULL
            );

            /**
             * Destructor.
             */
            virtual ~FLevelFile();

            /**
             * The resource type.
             */
            static const String RESOURCE_TYPE;

            /**
             * Retrieves the scripts in the level.
             *
             * @return The scripts in the level.
             */
            const std::vector<u8>& GetRawScript() const;

            /**
             * Retrieves the level background.
             *
             * @return The background file in the level.
             */
            const BackgroundFilePtr& GetBackground() const;

            /**
             * Retrieves the level cameara matrix.
             *
             * @return The camera matrix file in the level.
             */
            const CameraMatrixFilePtr& GetCameraMatrix() const;

            /**
             * Retrieves the level model list.
             *
             * @return The level model list file.
             */
            const ModelListFilePtr& GetModelList() const;

            /**
             * Retrieves the color palette for the level.
             *
             * @return The color palette file for the level.
             */
            const PaletteFilePtr& GetPalette() const;

            /**
             * Retrieves the level walkmesh.
             *
             * @return The level walkmesh file.
             */
            const WalkmeshFilePtr& GetWalkmesh() const;

            /**
             * Retrieves the level triggers.
             *
             * @return The level triggers file.
             */
            const TriggersFilePtr& GetTriggers() const;

            /**
             * Sets the scripts for the level.
             *
             * @param script_data[in] The scripts for the level.
             */
            void SetRawScript(const std::vector<u8>& script_data);

            /**
             * Sets the background for the level.
             *
             * @param background[in] The background for the level.
             */
            void SetBackground(const BackgroundFilePtr &background);

            /**
             * Sets the camera matrix for the level.
             *
             * @param camera_matrix[in] The camera matrix for the level.
             */
            void SetCameraMatrix(const CameraMatrixFilePtr &camera_matrix);

            /**
             * Sets the model list for the level.
             *
             * @param model_list[in] The model list for the level.
             */
            void SetModelList(const ModelListFilePtr &model_list);

            /**
             * Sets the color palette for the level.
             *
             * @param palette[in] The color palette for the level.
             */
            void SetPalette(const PaletteFilePtr &palette);

            /**
             * Sets the level walkmesh.
             *
             * @param walkmesh[in] The walkmesh for the level.
             */
            void SetWalkmesh(const WalkmeshFilePtr &walkmesh);

            /**
             * Sets the triggers for the level.
             *
             * @param triggers[in] The triggers for the level.
             */
            void SetTriggers(const TriggersFilePtr& triggers);

            /**
             * Retrieves the level background name.
             *
             * @return The background name.
             */
            String GetBackground2DName(void) const;

            /**
             * Retrieves the level background texture name.
             *
             * @return The background texture name.
             */
            String GetBackgroundTextureName(void) const;

        protected:

            typedef std::vector<HRCFilePtr> HRCList;

            typedef ModelListFile::ModelList ModelList;

            typedef ModelListFile::AnimationList AnimationList;

            /**
             * Background texture file names suffix.
             */
            static const String SUFFIX_BACKGROUND_TEXTURE;

            /**
             * Background file names suffix.
             */
            static const String SUFFIX_BACKGROUND_2D;

            /**
             * Loads the file.
             */
            virtual void loadImpl(void) override;

            /**
             * Loads the level models.
             */
            void LoadModels(void);

            /**
             * Loads the level animations
             */
            void LoadAnimations(
              const HRCFilePtr &model, const AnimationList &animations
            );

            /**
             * Unloads the file.
             */
            virtual void unloadImpl(void) override;

            /**
             * Calculates the size of the level.
             *
             * @return Always 0.
             */
            virtual size_t calculateSize(void) const override;

            /**
             * Retrieves the resource type.
             */
            virtual const String& GetResourceType(void) const;

        private:

            /**
             * The level background.
             */
            BackgroundFilePtr background_;

            /**
             * The level camera matrix.
             */
            CameraMatrixFilePtr camera_matrix_;

            /**
             * The level model list.
             */
            ModelListFilePtr model_list_;

            /**
             * The level color palette.
             */
            PaletteFilePtr palette_;

            /**
             * The level walkmesh.
             */
            WalkmeshFilePtr walkmesh_;

            /**
             * The level triggers.
             */
            TriggersFilePtr triggers_;

            /**
             * The level script data.
             */
            std::vector<u8> raw_script_;

            /**
             * The background texture loader.
             */
            FLevelTextureLoader *background_texture_loader_;

            /**
             * The background texture.
             */
            Ogre::TexturePtr background_texture_;

            /**
             * The background loader.
             */
            FLevelBackground2DLoader *background_2d_loader_;

            /**
             * The background loader.
             */
            Background2DFilePtr background_2d_;

            /**
             * The list of HRC files.
             *
             * HRC files describe bone hierarchy.
             */
            HRCList hrc_files_;
    };

    typedef Ogre::SharedPtr<FLevelFile> FLevelFilePtr;
}
