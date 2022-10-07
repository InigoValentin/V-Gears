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

class RefCounted;

inline void intrusive_ptr_add_ref(RefCounted *ptr);
inline void intrusive_ptr_release(RefCounted *ptr);

/**
 * Provides a base implementation of reference counting.
 *
 * To be used with boost::intrusive_ptr.
 */
class RefCounted{

    protected:
        RefCounted(): ref_count_(0){}
        virtual ~RefCounted(){}

    private:

        /**
         * Reference count used for boost::intrusive_ptr.
         */
        long ref_count_;

        friend void ::intrusive_ptr_add_ref(RefCounted *ptr);

        friend void ::intrusive_ptr_release(RefCounted *ptr);


};

/**
 * Add a reference to a pointer.
 *
 * @param[in] ptr Pointer to reference.
 */
inline void intrusive_ptr_add_ref(RefCounted *ptr){++ (ptr->ref_count_);}

/**
 * Remove a reference from a pointer.
 *
 * @param[in] ptr Pointer to de-reference.
 */
inline void intrusive_ptr_release(RefCounted *ptr){
    if (-- (ptr->ref_count_) == 0) delete ptr;
}
