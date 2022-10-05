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

#include "decompiler/Disassembler.h"

class WorldEngine;

class WorldDisassembler : public Disassembler{

    public:

        /**
         * CConstructor.
         *
         * @param engine[in] The engine to use to disassemble.
         * @param insts[in] The list of instructions.
         * @param script_number[in] The script number to process.
         */
        WorldDisassembler(WorldEngine* engine, InstVec& insts, int script_number);

        /**
         * Destructor.
         */
        ~WorldDisassembler();

        /**
         * Disassembles the instructions.
         */
        void DoDisassemble() override;

    private:

        /**
         * Parses an opcode.
         *
         * Adds properties to an instruction, and adds the instruction to the
         * list.
         *
         * @param opcode[in] The opcode code.
         * @param name[in] the opcode name.
         * @param instruction[in][out] A newly created instruction of the type
         * the opcode belongs to.
         * @param stack_change[in] Indicates how much the instruction changes
         * the stack pointer by.
         * @param argument_format[in] The opcode argument format.
         */
        template<typename T> void ParseOpcode(
          int opcode, std::string name, T instruction, int stack_change, const char* argument_format
        ){
            uint32 full_opcode = (full_opcode << 8) + opcode;
            this->insts_.push_back(instruction);
            this->insts_.back()->SetOpcode(full_opcode);
            this->insts_.back()->SetAddress(this->address_);
            this->insts_.back()->SetStackChange(0);
            this->insts_.back()->SetName(std::string(name));
            this->insts_.back()->SetCodeGenData("");
            this->ReadParams(this->insts_.back(), argument_format);
        }

        /**
         * The script number.
         */
        int script_number_ = 0;
};
