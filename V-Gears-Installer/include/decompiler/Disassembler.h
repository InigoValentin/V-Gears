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

#include <iostream>
#include <vector>
#include "common/BinaryReader.h"
#include "DecompilerException.h"
#include "instruction/Instruction.h"
#include "ObjectFactory.h"

/**
 * Base class for disassemblers.
 */
class Disassembler{

    public:

        /**
         * Constructor for Disassembler.
         *
         * @param insts[in] Where disassembled instructions will be placed.
         */
        Disassembler(InstVec &insts);

        /**
         * Destructor.
         */
        virtual ~Disassembler() = default;

        /**
         * Open a file for disassembly.
         *
         * @param filename[in] The file to Disassemble.
         */
        virtual void Open(const char *filename);

        /**
         * Request disassembled instructions.
         */
        void Disassemble();

        /**
         * Outputs the disassembled code.
         *
         * Disassembles code if this has not already been done.
         *
         * @param output[out] The output stream.
         */
        void DumpDisassembly(std::ostream &output);

    protected:

        /**
         * Performs disassembly.
         *
         * @throws UnknownOpcodeException on unknown opcode.
         * @throws std::exception on other failures.
         */
        virtual void DoDisassemble() = 0;

        /**
         * Outputs the disassembled code.
         *
         * @param output[out] The output stream.
         */
        virtual void DoDumpDisassembly(std::ostream &output);

        /**
         * Read parameters and associate them with an instruction.
         *
         * @param inst[in] The instruction to associate the parameters with.
         * @param types[in] NUL-terminated string describing the type of each
         * parameter.
         */
        void ReadParams(InstPtr inst, const char *types);

        /**
         * Read parameters but it doesn't associate them with an instruction.
         *
         * @param inst[in] The instruction to associate the parameters with.
         * Unused.
         * @param types[in] NUL-terminated string describing the type of each
         * parameter.
         * @param params[in] Unused.
         */
        void ReadParams(InstPtr inst, const char *types, const std::vector<std::string>& params);

        /**
         * Reads data for a single parameter.
         *
         * @param inst The instruction the parameter will belong to. Unused.
         * @param type Character describing the type of the parameter.
         * @return The read data.
         */
        virtual ValuePtr ReadParameter(InstPtr inst, std::string type);

        /**
         * Used to perform file I/O.
         */
        std::unique_ptr<BinaryReader> stream_;

        /**
         * List of disassembled instructions.
         */
        InstVec &insts_;

        /**
         * Base address where the script starts.
         */
        uint32 address_base_;

        /**
         * The current address.
         */
        uint32 address_;

};
