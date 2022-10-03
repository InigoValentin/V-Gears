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

#include <deque>
#include <iostream>

/**
 * Stack class based on a deque.
 */
template<typename T> class Stack {

    public:

        /**
         * Returns whether or not the stack is empty.
         *
         * @return True if the stack is empty, false if it is not.
         */
        bool IsEmpty() const{return stack_.empty();}

        /**
         * Push an item onto the stack.
         *
         * @param item[in] The item to push.
         */
        void Push(const T &item){stack_.push_front(item);}

        /**
         * Pop an item from the stack and return it.
         *
         * @return The value popped from the stack.
         */
        T Pop(){
            T retval = stack_.front();
            stack_.pop_front();
            return retval;
        }

        /**
         * Return the topmost item on the stack without removing it.
         *
         * @return The topmost item on the stack.
         */
        T &Peek(){return stack_.front();}

        /**
         * Return the topmost item on the stack without removing it.
         *
         * @return The topmost item on the stack.
         */
        const T &Peek() const{return stack_.front();}

        /**
         * Gets item on a specified stack position without removing it.
         *
         * @param pos[in] The number of items to skip on the stack.
         * @return The desired item from the stack.
         */
        T &PeekPos(size_t pos){
            if (pos >= stack_.size()) std::cerr << "WARNING: Looking outside stack\n";
            return stack_.at(pos);
        }

        /**
         * Gets item on a specified stack position without removing it.
         *
         * @param pos[in] The number of items to skip on the stack.
         * @return The desired item from the stack.
         */
        const T &PeekPos(size_t pos) const{
            if (pos >= stack_.size()) std::cerr << "WARNING: Looking outside stack\n";
            return stack_.at(pos);
        }

    private:

        /**
         * The stack.
         */
        std::deque<T> stack_;
};

