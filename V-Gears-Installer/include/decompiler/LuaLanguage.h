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

#include "Value.h"

/**
 * Provides formatters and keywords for the LUA language.
 */
class LuaLanguage{

    public:

        enum CONTEXT{

            /**
             * End of if/elseif block and about to start a final else.
             */
            TO_ELSE_BLOCK,

            /**
             * Begin of else block.
             */
            BEGIN_ELSE,

            /**
             * End of if block.
             */
            END_OF_IF,

            /**
             * End of while block.
             */
            END_OF_WHILE,

            /**
             * End of an if/elseif/else block.
             */
            END_OF_IF_ELSE_CHAIN,

            /**
             * Begin of a while block.
             */
            BEGIN_WHILE,

            /**
             * End of a while block.
             */
            END_WHILE
        };

        /**
         * The Lua "break" keyword.
         *
         * @return "break".
         */
        virtual const std::string LoopBreak();

        /**
         * The Lua "continue" keyword.
         *
         * Continue is not implemented in Lua.
         *
         * @return A commented string.
         */
        virtual const std::string LoopContinue();

        /**
         * A Lua "goto" instruction.
         *
         * @param target[in] A memory address label.
         * @return "goto label_0x<target>X".
         */
        virtual std::string Goto(uint32 target);

        /**
         * The Lua "repeat" keyword.
         *
         * @return "repeat".
         */
        virtual const std::string DoLoopHeader();

        /**
         * The Lua parts of a do-loop footer
         *
         * @param before_expr[in] True to fetch the initial part of the
         * footer, false for the ending part.
         * @return "until (" if before_expr is true, ")" if not.
         */
        virtual std::string DoLoopFooter(bool before_expr);

        /**
         * The Lua parts of a if control sentence
         *
         * @param before_expr[in] True to fetch the initial part of the
         * sentence, false for the ending part.
         * @return "if (" if before_expr is true, ") then" if not.
         */
        virtual std::string If(bool before_expr);

        /**
         * The Lua parts of a while control sentence
         *
         * @param before_expr[in] True to fetch the initial part of the
         * sentence, false for the ending part.
         * @return "while (" if before_expr is true, ") do" if not.
         */
        virtual std::string WhileHeader(bool before_expr);

        /**
         * The Lua argument separator.
         *
         * @return ",".
         */
        virtual const std::string FunctionCallArgumentSeperator();

        /**
         * The Lua function argument list opener.
         *
         * @return "(".
         */
        virtual const std::string FunctionCallBegin();

        /**
         * The Lua function argument list closer.
         *
         * @return ")".
         */
        virtual const std::string FunctionCallEnd();

        /**
         * Generates a Lua label to use with goto.
         *
         * @param addr[in] Target address label.
         * @return "::label_0x<target>X::"
         */
        virtual std::string Label(uint32 addr);

        /**
         * The Lua "else" keyword.
         *
         * @return "else".
         */
        virtual const std::string Else();


        /**
         * The Lua block starter (none).
         *
         * @param context[in] Unused.
         * @return An empty string.
         */
        virtual std::string StartBlock(CONTEXT context);

        /**
         * A Lua block ender.
         *
         * @param context[in] The current context.
         * @return "end", unles the current context is {@see TO_ELSE_BLOCK},
         * in which case it will return an empty string, because "end" is not
         * needed before an else.
         */
        virtual std::string EndBlock(CONTEXT context);

        /**
         * The Lua line terminator (none).
         *
         * @return An empty string.
         */
        virtual const std::string LineTerminator();
};
