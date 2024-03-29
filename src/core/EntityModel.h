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

#include "Entity.h"

/**
 * A 3D model for an entity.
 */
class EntityModel : public Entity{

    public:

        /**
         * Constructor. Creates an entity model.
         *
         * The models will be looked for in the resource groups FIELD_MODELS, BATTLE_MODELS or
         * WORLD_MODELS, depending on the module the {@see EntityManager} is currerntly set to.
         *
         * @param[in] name Entity name.
         * @param[in] file_name File with the model data.
         * @param[in] node Scene node to attach the model to.
         */
        EntityModel(
          const Ogre::String& name, const Ogre::String file_name,
          Ogre::SceneNode* node
        );
        virtual ~EntityModel();

        /**
         * Updates the model status.
         */
        virtual void Update();

        /**
         * Toggles the model visibility.
         *
         * @param[in] visible True to make the model visible, false to turn it
         * invisible.
         */
        virtual void SetVisible(const bool visible);

        /**
         * Checks the model visibility.
         *
         * @return True if the model is visible, false if it's invisible.
         */
        virtual bool IsVisible() const;

        Ogre::Entity* GetModel();

        /**
         * Plays an animation of the model.
         *
         * @param[in] animation Name of the animation to play.
         * @param[in] state Initial state of the animation.
         * @param[in] play_type Play mode, for single or looped playbacks.
         * @param[in] start Start point in time of the animation, in seconds.
         * @param[in] end End point in time of the animation, in seconds.
         */
        virtual void PlayAnimation(
          const Ogre::String& animation, AnimationState state,
          AnimationPlayType play_type, const float start, const float end
        );

        /**
         * Resumes an animation.
         *
         * @param[in] animation The name of the animation to resume.
         */
        virtual void PlayAnimationContinue(const Ogre::String& animation);

        /**
         * Updates the animation.
         *
         * @param[in] delta Ellapsed time.
         */
        virtual void UpdateAnimation(const float delta);

    private:

        /**
         * Constructor.
         */
        EntityModel();

        /**
         * The model.
         */
        Ogre::Entity* model_;

        /**
         * The current animation state.
         */
        Ogre::AnimationState* animation_current_;
};

