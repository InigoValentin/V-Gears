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

#include "decompiler/CodeGenerator.h"

class WorldCodeGenerator : public CodeGenerator{

    public:

        /**
         * Constructor.
         *
         * @param[in] engine The engine.
         * @param[out] output The generated script.
         */
        WorldCodeGenerator(Engine *engine, std::ostream &output);

        /**
         * Finds the first function call.
         *
         * @return A pointer to the first call instruction.
         */
        const InstPtr FindFirstCall();

        /**
         * Finds the last function call.
         *
         * @return A pointer to the last call instruction.
         */
        const InstPtr FindLastCall();

        /**
         * Processes special metadata in an instruction.
         *
         * @param[in] inst The instruction to process.
         * @param[in] c
         * @param[in] pos
         * @todo understand and document.
         */
        virtual void ProcessSpecialMetadata(const InstPtr inst, char c, int pos);

        /**
         * Outputs a jump if false as "if not (condition) then goto label end".
         *
         * The scripts of the world map are not analyzed into ifs and loops, so every conditional
         * jump is a goto. A label is valid in Lua if it is in the same block, and the generated
         * functions are flat.
         *
         * @param[in] insts All the instructions of the script.
         * @param[in] inst The jump.
         */
        virtual void ProcessUnstructuredCondJump(InstVec& insts, const InstPtr inst) override;

    protected:

        bool OutputOnlyRequiredLabels() const override;
        void OnEndFunction(const Function& function) override;

        /**
         * Constructs the function signature.
         *
         * The function signature is the LUA function declaration, and it
         * looks like this:
         *
         * function_name = function(self)
         *
         * @param[in] function The function to construct the signature for.
         * @return The function signature.
         */
        std::string ConstructFuncSignature(const Function &function);
};
