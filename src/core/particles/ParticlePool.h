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

#include <list>

/**
 * A particle pool
 *
 * There are actually two pools, one for locked particles and one for released
 * ones. Locked particles can't be used until they are released.
 *
 * @tparam T The type of particle.
 */
template <typename T>
class ParticlePool{

    public:

        typedef std::list<T*> PoolList;

        // The 'typename' MUST be added, since T is not a fixed type
        typedef typename PoolList::iterator PoolIterator;

        /**
         * Constructor.
         */
        ParticlePool(){};

        /**
         * Destructor.
         */
        virtual ~ParticlePool(){};

        /**
         * Checks if the pool is empty.
         *
         * It doesn't consider locked particles, only the released ones.
         *
         * @return True if the pool is empty, false if there is at least one
         * particle in the pool.
         */
        bool IsEmpty(){return released_.empty();};

        /**
         * Retrieves the pool size.
         *
         * It doesn't consider locked particles, only the released ones.
         *
         * @return The number of particles in the pool.
         */
        size_t GetSize(){return released_.size();};

        /**
         * Resets the pool iterator.
         */
        void ResetIterator(){pool_iterator_ = released_.begin();};

        /**
         * Retrieves the first particle.
         *
         * Before doing so, it resets the pool iterator. It doesn't consider
         * locked particles, only the released ones.
         *
         * @return A reference to the first particle in the pool. NULL if the
         * pool is empty.
         */
        T* GetFirst() {
            ResetIterator();
            if (End()) return NULL;
            T* t = *pool_iterator_;
            return t;
        };

        /**
         * Retrieves the next particle in the pool.
         *
         * Before doing so, it advances the pool iterator. It doesn't consider
         * locked particles, only the released ones.
         *
         * @return A reference to the next particle in the pool. NULL if the
         * pool is empty or the iterator is at the end of it.
         */
        T* GetNext(){
            if (End()) return NULL;
            ++ pool_iterator_;
            if (End()) return NULL;
            T* t = *pool_iterator_;
            return t;
        };

        /**
         * Checks if the iterator is at the end of the pool.
         *
         * If it's at the end, {@see GetNext} will return null when called. It
         * doesn't consider locked particles, only the released ones.
         *
         * @return True if the iterator is at the end of the pool.
         */
        bool End(){
            return pool_iterator_ == released_.end();
        };

        /**
         * Removes all particles.
         *
         * Removes both locked and released particles.
         */
        void Clear(){
            locked_.clear();
            released_.clear();
        };

        /**
         * Adds a particle to the pool.
         *
         * A particle is added in locked state.
         *
         * @param[in] element Particle to add to the pool.
         */
        void AddElement(T* element) {
            locked_.push_back(element);
        };

        /**
         * Releases a locked particle.
         *
         * The first locked particle will be the one unlocked.
         *
         * @return A pointer to the previously locked and now released
         * particle, or 0 if there were no more locked particles.
         */
        T* ReleaseElement() {
            // Return with 0 if no elements left
            if (locked_.empty()) return 0;

            // Move element from locked elements to released elements and return it
            T* t = locked_.front();
            released_.splice(released_.end(), locked_, locked_.begin());
            return t;
        };

        /**
         * Releases all locked particles.
         *
         * It also resets the pool operator.
         */
        void ReleaseAllElements(){
            // Move all elements from locked elements to released elements
            released_.splice(released_.end(), locked_);
            ResetIterator();
        };

        /**
         * Locks the released particle pointed by the iterator.
         *
         * The particle gets added to the end of the locked particle list.
         */
        void LockLatestElement(){
            if (End() == false){
                locked_.push_back(*pool_iterator_);
                pool_iterator_ = released_.erase(pool_iterator_);
            }
        };

        /**
         * Locks all particles.
         *
         * The particles get added at the end of the locked list in the same
         * order they were in the released list. The pool iterator is reseted.
         */
        void LockAllElements(){
            // Move all elements from released elements to locked elements
            locked_.splice(locked_.end(), released_);
            ResetIterator();
        };

        /**
         * Retrieves the list of released particles.
         *
         * @return List of released particles.
         */
        std::list<T*>& GetActiveElementsList(){
            return released_;
        };

    protected:

        /**
         * List with released particles.
         *
         * They can be operated upon.
         */
        PoolList released_;

        /**
         * List with locked particles.
         *
         * They can't be operated upon until released.
         */
        PoolList locked_;

        /**
         * The pool iterator.
         *
         * Determines the current particle to be acted upon. Only works for
         * released particles.
         */
        PoolIterator pool_iterator_;
};

