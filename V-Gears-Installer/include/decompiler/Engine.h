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
#include "CodeGenerator.h"
#include "Disassembler.h"
#include "Function.h"

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

        virtual std::unique_ptr<Disassembler> GetDisassembler(InstVec &, const std::vector<unsigned char>& )
        {
            throw NotImplementedException();
        }

        /**
         * Retrieve the disassembler for the engine.
         *
         * @param insts Reference to the std::vector to place the Instructions in.
         * @return Pointer to a Disassembler for the engine.
         */
        virtual std::unique_ptr<Disassembler> GetDisassembler(InstVec &insts) = 0;

        /**
         * Retrieve the code generator for the engine.
         *
         * @param output The std::ostream to output the code to.
         * @return Pointer to a CodeGenerator for the engine.
         */
        virtual std::unique_ptr<CodeGenerator> GetCodeGenerator(const InstVec& insts, std::ostream &output) = 0;

        /**
         * Post-processing step after CFG analysis.
         * @param insts Reference to the std::vector to place the Instructions in.
         * @param g Graph generated from the CFG analysis.
         */
        virtual void PostCFG(InstVec&, Graph) { }

        /**
         * Whether or not code flow analysis is supported for this engine.
         *
         * @return True if supported, false if not. If false is returned, code flow analysis should not take place, and -D should be implied.
         */
        virtual bool SupportsCodeFlow() const { return true; }

        /**
         * Whether or not code generation is supported for this engine.
         *
         * @return True if supported, false if not. If false is returned, code generation should not take place, and -G should be implied.
         */
        virtual bool SupportsCodeGen() const { return true; }



        /**
         * Fill a vector with the names of all variants supported for this engine.
         * If variants are not used by this engine, leave the vector empty (default implementation).
         *
         * @param variants Vector to add the supported variants to.
         */
        virtual void GetVariants(std::vector<std::string>&) const { };


        /**
         * Whether or not to use "pure" grouping during code flow analysis.
         * With pure grouping, code flow analysis only looks at branches when merging.
         * This method may be more appropriate for non-stack-based engines.
         *
         * @return True if pure grouping should be used, false if not.
         */
        virtual bool UsePureGrouping() const { return false; }

        virtual FuncMap GetFunctions() const{return _functions;}

        virtual void SetFunction(uint32 index, Function function){
            _functions[index] = function;
        }

        FuncMap _functions;

    protected:

        //FuncMap _functions; ///< Map to functions in the current script, indexed by starting address.
        std::string _variant; ///< Engine variant to use for the script.

};
