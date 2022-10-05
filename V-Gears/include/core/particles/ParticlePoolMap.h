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

/**
 * A particle pool map.
 *
 * @tparam T The type of particle.
 */
template <typename T>
class ParticlePoolMap{

    public:

        typedef std::multimap<Ogre::String, T*> PoolMapMap;

        // The 'typename' MUST be added, since T is not a fixed type.
        typedef typename PoolMapMap::iterator PoolMapIterator;

        /**
         * Constructor.
         */
        ParticlePoolMap(){};

        /**
         * Destructor.
         */
        virtual ~ParticlePoolMap(){};

        /**
         * Checks if the pool map is empty.
         *
         * It doesn't consider locked particles, only the released ones.
         *
         * @return True if the pool is empty, false if there is at least one
         * particle in the pool.
         */
        bool IsEmpty(){return released_.empty();};

        /**
         * Retrieves the pool map size.
         *
         * It doesn't consider locked particles, only the released ones.
         *
         * @return The number of particles in the pool.
         */
        size_t GetSize(){return released_.size();};

        /**
         * Resets the pool iterator.
         */
        void ResetIterator(){pool_map_iterator_ = released_.begin();};

        /**
         * Retrieves the first particle.
         *
         * Before doing so, it resets the pool map iterator. It doesn't
         * consider locked particles, only the released ones.
         *
         * @return A reference to the first particle in the pool map. NULL if
         * the pool map is empty.
         */
        T* GetFirst(){
            ResetIterator();
            if (End()) return NULL;
            T* t = pool_map_iterator_->second;
            return t;
        };

        /**
         * Retrieves the next particle in the pool map.
         *
         * Before doing so, it advances the pool map iterator. It doesn't
         * consider locked particles, only the released ones.
         *
         * @return A reference to the next particle in the pool map. NULL if
         * the pool map is empty or the iterator is at the end of it.
         */
        T* GetNext(){
            if (End()) return NULL;
            ++ pool_map_iterator_;
            if (End()) return NULL;
            T* t = pool_map_iterator_->second;
            return t;
        };

        /**
         * Checks if the iterator is at the end of the pool map.
         *
         * If it's at the end, {@see GetNext} will return null when called. It
         * doesn't consider locked particles, only the released ones.
         *
         * @return True if the iterator is at the end of the pool map.
         */
        bool End(){return pool_map_iterator_ == released_.end();};

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
         * Adds a particle to the pool map.
         *
         * A particle is added in locked state.
         *
         * @param[in] key PArticle key in the map.
         * @param[in] element Particle to add to the pool.
         */
        void AddElement(const Ogre::String& key, T* element){
            locked_.insert(make_pair(key, element));
        };

        /**
         * Releases a locked particle.
         *
         * @param[in] key The key of the particle to unlock.
         * @return A pointer to the previously locked and now released
         * particle, or 0 if there were no more locked particles.
         */
        T* ReleaseElement(const Ogre::String& key){
            // Return with 0 if no elements left
            if (locked_.empty()) return NULL;

            // Return the first element that is encountered
            T* t = 0;
            PoolMapIterator it;
            it = locked_.find(key);
            if (it != locked_.end()){
                // Get the element and move it to the released elements list
                t = it->second;
                released_.insert(make_pair(key, t));
                locked_.erase(it);
            }
            return t;
        };

        /**
         * Releases all locked particles.
         *
         * It also resets the pool map operator.
         */
        void ReleaseAllElements(){
            // Move all elements from locked elements to released elements
            PoolMapIterator it;
            for (it = locked_.begin(); it != locked_.end(); ++ it)
                released_.insert(make_pair(it->first, it->second));
            locked_.clear();
            ResetIterator();
        };

        /**
         * Locks the released particle pointed by the iterator.
         *
         * The particle gets added to the end of the locked particle list.
         */
        void LockLatestElement(){
            // Move element pointed by iterator from released elements to
            // locked elements
            locked_.insert(
              make_pair(pool_map_iterator_->first, pool_map_iterator_->second)
            );
            // Watch the ++ at the end to set mPoolMapIterator to the next
            // element
            released_.erase(pool_map_iterator_ ++);
        };

        /**
         * Locks all particles.
         *
         * The particles get added at the end of the locked list in the same
         * order they were in the released list. The pool iterator is reseted.
         */
        void LockAllElements() {
            // Move all elements from release elements to locked elements
            PoolMapIterator it;
            for (it = released_.begin(); it != released_.end(); ++ it)
                locked_.insert(make_pair(it->first, it->second));
            released_.clear();
            ResetIterator();
        };

    protected:
        /**
         * List with released particles.
         *
         * They can be operated upon.
         */
        PoolMapMap released_;

        /**
         * List with locked particles.
         *
         * They can't be operated upon until released.
         */
        PoolMapMap locked_;

        /**
         * The pool map iterator.
         *
         * Determines the current particle to be acted upon. Only works for
         * released particles.
         */
         PoolMapIterator pool_map_iterator_;
};




