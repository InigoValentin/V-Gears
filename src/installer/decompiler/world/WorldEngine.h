/*
 * V-Gears
 * Copyright (C) 2022 V-Gears Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include <vector>

#include "decompiler/Engine.h"

/**
 * Represents the FF7 world map engine.
 */
class WorldEngine : public Engine{

    public:

        /**
         * A bank value.
         *
         * @todo Understand and document.
         */
        class BankValue : public VarValue{

            public:

                /**
                 * Constructor.
                 *
                 * @param[in] name Variable name.
                 */
                BankValue(std::string name);
        };

        /**
         * A value that is a Lua boolean, the result of a comparison or a logic operation.
         *
         * The script machine only has numbers, and in Lua 0 is true, so a boolean can't be used as
         * a number nor a number as a condition without a conversion. This value prints as the
         * number that the machine has (1 or 0), and has the condition for the places that want a
         * boolean.
         */
        class BoolValue : public VarValue{

            public:

                /**
                 * Constructor.
                 *
                 * @param[in] condition Lua expression that is a boolean.
                 */
                explicit BoolValue(std::string condition);

                /**
                 * Prints the value as a number.
                 *
                 * @param[out] output The stream to write to.
                 * @return The stream used for output.
                 */
                virtual std::ostream& Print(std::ostream &output) const override;

                /**
                 * Retrieves the Lua boolean expression.
                 *
                 * @return The expression.
                 */
                std::string Condition() const;
        };

        /**
         * Gets a value as a Lua boolean expression: a boolean as it is, and a number as "different
         * from 0".
         *
         * @param[in] value The value.
         * @return The expression.
         */
        static std::string AsCondition(const ValuePtr& value);

        /**
         * Constructor.
         *
         * @param[in] script_number The script number.
         */
        WorldEngine(int script_number);

        /**
         * Retrieves the disassembler.
         *
         * @param[in] insts List of instructions.
         * @return Pointer to the disassembler.
         */
        std::unique_ptr<Disassembler> GetDisassembler(InstVec &insts) override;

        /**
         * Retrieves the code generator.
         *
         * @param[in] insts List of instructions. Unused.
         * @param[in] output Pointer to the output (file, stream...).
         * @return Pointer to the generator.
         */
        std::unique_ptr<CodeGenerator> GetCodeGenerator(
          const InstVec& insts, std::ostream &output
        ) override;

        /**
         * Post-processing actions to apply to the scripts.
         *
         * It actually does nothing. CFG stands for control flow group.
         *
         * @param[in] insts Instruction list.
         * @param[in] graph Code graph.
         */
        virtual void PostCFG(InstVec &insts, Graph graph) override;

        /**
         * Retrieves the variants.
         *
         * @todo Variants of what? What is this supposed to do?
         */
        virtual void GetVariants(std::vector<std::string> &variants) const override;

        /**
         * Indicates if instructions are purely grouped.
         *
         * @return True if instructions are purely grouped. Always false.
         */
        virtual bool UsePureGrouping() const override;

    private:

        /**
         * The script number.
         */
        int script_number_;

        /**
         * Container for strings from the TEXT chunk.
         */
        std::vector<std::string> text_strings_;
};
