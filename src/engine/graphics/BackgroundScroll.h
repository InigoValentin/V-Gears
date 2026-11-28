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

#include <functional>
#include <vector>
#include <OgreAxisAlignedBox.h>
#include <OgreVector2.h>
#include "engine/script/ScriptManager.h"

class Entity;

/**
 * The kinds of scroll of a field.
 */
struct BackgroundScrollTypes{

    /**
     * Types of scroll.
     */
    enum SCROLL_TYPE{

        /**
         * No scroll.
         */
        NONE,

        /**
         * Linear, smooth scroll.
         */
        LINEAR,

        /**
         * Accelerated scroll.
         */
        SMOOTH
    };
};

/**
 * The scroll of a field: where the view is, where it is going, what entity it follows, and the
 * scripts that wait for it.
 *
 * The movement of a scroll is done by the caller, which sets the position as time passes.
 */
class BackgroundScroll : public BackgroundScrollTypes{

    public:

        /**
         * Function that lets a script that was waiting go on.
         */
        typedef std::function<void(const ScriptId&)> Resume;

        /**
         * Constructor.
         *
         * @param[in] resume Called for each waiting script when a scroll ends.
         */
        explicit BackgroundScroll(const Resume& resume);

        /**
         * Retrieves the position of the view.
         */
        const Ogre::Vector2& Position() const{return position_;}

        /**
         * Moves the view, within the limits unless the scroll is free.
         *
         * @param[in] position The new position.
         */
        void SetPosition(const Ogre::Vector2& position);

        /**
         * Sets the limits of the view.
         *
         * @param[in] range The limits.
         */
        void SetRange(const Ogre::AxisAlignedBox& range){range_ = range;}

        /**
         * Limits a position, unless the scroll is free.
         *
         * @param[in] position The position.
         * @return The position within the limits.
         */
        Ogre::Vector2 Clamp(const Ogre::Vector2& position) const;

        /**
         * Limits a position.
         *
         * @param[in] position The position.
         * @return The position within the limits.
         */
        Ogre::Vector2 ClampToRange(const Ogre::Vector2& position) const;

        /**
         * Makes the view follow an entity.
         *
         * @param[in] entity The entity.
         */
        void Follow(Entity* entity);

        /**
         * Retrieves the entity the view follows, or null.
         */
        Entity* FollowedEntity() const{return followed_entity_;}

        /**
         * Checks if something controls the view: an entity or a scroll of a script.
         */
        bool HasControl() const;

        /**
         * Scrolls to an entity, and follows it when the scroll ends.
         *
         * @param[in] entity The entity.
         * @param[in] position Position of the view that shows the entity.
         * @param[in] type Type of scroll.
         * @param[in] seconds Duration of the scroll.
         * @return True if the view has already moved: it was an instant scroll.
         */
        bool ToEntity(
          Entity* entity, const Ogre::Vector2& position, const SCROLL_TYPE type,
          const float seconds
        );

        /**
         * Scrolls to a position, which is not limited. The view follows nothing after it.
         *
         * An instant scroll is done in the next {@see ApplyPending}.
         *
         * @param[in] position The position.
         * @param[in] type Type of scroll.
         * @param[in] seconds Duration of the scroll.
         */
        void ToPosition(const Ogre::Vector2& position, const SCROLL_TYPE type, const float seconds);

        /**
         * Does the instant scroll to a position that is waiting, if there is one.
         *
         * @return True if the view moved.
         */
        bool ApplyPending();

        /**
         * Retrieves the entity the current scroll goes to, or null.
         */
        Entity* TargetEntity() const{return target_entity_;}

        /**
         * Changes where the current scroll ends, for scrolls to an entity that moves.
         *
         * @param[in] position The new end, which is limited.
         */
        void SetTargetPosition(const Ogre::Vector2& position){end_ = ClampToRange(position);}

        /**
         * Makes a script wait for the current scroll to end.
         *
         * @param[in] script The script.
         * @return False if there is no scroll to wait for.
         */
        bool Wait(const ScriptId& script);

        /**
         * Ends the current scroll: the view follows its target, and the waiting scripts go on.
         */
        void Stop();

        /**
         * Ends the current scroll and forgets everything.
         */
        void Clear();

        /**
         * Retrieves where the current scroll started.
         */
        const Ogre::Vector2& Start() const{return start_;}

        /**
         * Retrieves where the current scroll ends.
         */
        const Ogre::Vector2& End() const{return end_;}

        /**
         * Retrieves the type of the current scroll, NONE if there is none.
         */
        SCROLL_TYPE Type() const{return type_;}

        /**
         * Retrieves the duration of the current scroll.
         */
        float Seconds() const{return seconds_;}

        /**
         * Retrieves how long the current scroll has been going on.
         */
        float CurrentSeconds() const{return current_seconds_;}

        /**
         * Sets how long the current scroll has been going on.
         */
        void SetCurrentSeconds(const float seconds){current_seconds_ = seconds;}

    private:

        /**
         * Starts a scroll.
         *
         * @param[in] position Where it ends.
         * @param[in] type Type of scroll.
         * @param[in] seconds Duration.
         * @param[in] target Entity the view follows when the scroll ends, or null.
         * @return True if the view has already moved: it was an instant scroll.
         */
        bool Begin(
          const Ogre::Vector2& position, const SCROLL_TYPE type, const float seconds,
          Entity* target
        );

        /**
         * Ends the current scroll without following its target.
         */
        void Abandon();

        /** Lets the waiting scripts go on. */
        Resume resume_;

        /** The position of the view. */
        Ogre::Vector2 position_;

        /** The limits of the view. */
        Ogre::AxisAlignedBox range_;

        /** The entity the view follows. */
        Entity* followed_entity_;

        /** The entity the current scroll goes to. */
        Entity* target_entity_;

        /** Where the current scroll started. */
        Ogre::Vector2 start_;

        /** Where the current scroll ends. */
        Ogre::Vector2 end_;

        /** Type of the current scroll. */
        SCROLL_TYPE type_;

        /** Duration of the current scroll. */
        float seconds_;

        /** How long the current scroll has been going on. */
        float current_seconds_;

        /** True if a script moved the view to a position, which is not limited. */
        bool free_;

        /** Position of the instant scroll that is waiting. */
        Ogre::Vector2 pending_position_;

        /** True if there is an instant scroll waiting. */
        bool has_pending_;

        /** The scripts waiting for the current scroll. */
        std::vector<ScriptId> waiting_;
};
