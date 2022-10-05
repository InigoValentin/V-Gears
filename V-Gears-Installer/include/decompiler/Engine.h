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

#include <set>
#include <string>
#include <vector>
#include "Function.h"
#include "Graph.h"
#include "CodeGenerator.h"
#include "Disassembler.h"

/**
 * Type representing a map of functions, indexed by starting address.
 */
typedef std::map<uint32, Function> FuncMap;

/**
 * Base class for engines.
 */
class Engine {

    public:

        virtual ~Engine() = default;

        virtual std::unique_ptr<Disassembler> GetDisassembler(
          InstVec& insts, const std::vector<unsigned char>& c
        );

        /**
         * Retrieve the disassembler for the engine.
         *
         * @param[out] insts Vector to place the Instructions in.
         * @return Pointer to a disassembler for the engine.
         */
        virtual std::unique_ptr<Disassembler> GetDisassembler(InstVec &insts) = 0;

        /**
         * Retrieve the code generator for the engine.
         *
         * @param[in] insts The list of instructions.
         * @param[out] output Stream to output the code to.
         * @return Pointer to a CodeGenerator for the engine.
         */
        virtual std::unique_ptr<CodeGenerator> GetCodeGenerator(
          const InstVec& insts, std::ostream &output
        ) = 0;

        /**
         * Post-processing step after CFG analysis.
         * @param[out] insts Vector to place the Instructions in.
         * @param[in] graph Graph generated from the CFG analysis.
         */
        virtual void PostCFG(InstVec& insts, Graph graph);

        /**
         * Whether or not code flow analysis is supported for this engine.
         *
         * @return True if supported, false if not. If false is returned, code
         * flow analysis should not take place.
         */
        virtual bool SupportsCodeFlow() const;

        /**
         * Whether or not code generation is supported for this engine.
         *
         * @return True if supported, false if not. If false is returned, code
         * generation should not take place.
         */
        virtual bool SupportsCodeGen() const;

        /**
         * Retrieves the names of all variants supported for this engine.
         *
         * If variants are not used by this engine, it will be empty (default
         * implementation).
         *
         * @param[out] variants Vector with the supported variants.
         */
        virtual void GetVariants(std::vector<std::string>& variants) const;


        /**
         * Whether or not to use "pure" grouping during code flow analysis.
         *
         * With pure grouping, code flow analysis only looks at branches when
         * merging. This method may be more appropriate for non-stack-based
         * engines.
         *
         * @return True if pure grouping should be used, false if not.
         */
        virtual bool UsePureGrouping() const;

        // TODO: functions must probably be set to private, and have accessors, but...
        // too much depends on it being public right now.
        // Maybe at some point in the future...

        /**
         * Map to functions in the current script, indexed by start address.
         */
        FuncMap functions;

    protected:

        /**
         * Engine variant to use for the script.
         */
        std::string variant_;

};
