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

#include <ostream>
#include <utility>
#include <memory>
#include <boost/intrusive_ptr.hpp>
#include "LuaLanguage.h"
#include "graph.h"
#include "value.h"
#include "unknown_opcode_exception.h"


class Engine;

class Function;

const int INDENT_SPACES = 4; ///< How many spaces to use for each indent.

/**
 * Different argument/operand orderings.
 */
enum ARGUMENT_ORDER{

    /**
     * First argument is pushed to stack first.
     */
    FIFO_ARGUMENT_ORDER,

    /**
     * First argument is pushed to stack last.
     */
    LIFO_ARGUMENT_ORDER
};



/**
 * Base class for code generators.
 */
class CodeGenerator {

    public:
        LuaLanguage& TargetLang()
        {
            assert(target_lang_);
            return *target_lang_;
        }

        void writeFunctionCall(std::string functionName, std::string paramsFormat, const std::vector<ValuePtr>& params);

        const ARGUMENT_ORDER _binOrder;  ///< Order of operands for binary operations.
        const ARGUMENT_ORDER _callOrder; ///< Order of operands for call arguments.
        ValueList _argList;        ///< Storage for lists of arguments to be built when processing function calls.
        GroupPtr cur_group_;     ///< Pointer to the group currently being processed.

        virtual ~CodeGenerator() { }

        /**
         * Constructor for CodeGenerator.
         *
         * @param engine Pointer to the Engine used for the script.
         * @param output The std::ostream to output the code to.
         * @param binOrder Order of arguments for binary operators.
         * @param callOrder Order of arguments for function calls.
         */
        CodeGenerator(Engine *engine, std::ostream &output, ARGUMENT_ORDER binOrder, ARGUMENT_ORDER callOrder);

        /**
         * Generates code from the provided graph and outputs it to stdout.
         *
         * @param g The annotated graph of the script.
         */
        virtual void Generate(InstVec& insts, const Graph &g);

        /**
         * Adds a line of code to the current group.
         *
         * @param s The line to add.
         * @param unindentBefore Whether or not to remove an indentation level before the line. Defaults to false.
         * @param indentAfter Whether or not to add an indentation level after the line. Defaults to false.
         */
        virtual void AddOutputLine(std::string s, bool unindentBefore = false, bool indentAfter = false);

        /**
         * Writes a comment line indicating an unimplemented opcode.
         *
         * @param code_gen[in|out] The code generator.
         * @param class_name[in] The class where the instruction is. Unused.
         * @param instruction[in] The unimplemented instruction.
         */
        void WriteTodo(std::string class_name, std::string instruction){
            AddOutputLine("-- UNIMPLMENTED INSTRUCTION: \"" + instruction + "\")");
        }

        /**
         * Generate an assignment statement.
         *
         * @param dst The variable being assigned to.
         * @param src The value being assigned.
         */
        void writeAssignment(ValuePtr dst, ValuePtr src);

        /**
         * Add an argument to the argument list.
         *
         * @param p The argument to add.
         */
        void addArg(ValuePtr p);

        /**
         * Process a single character of metadata.
         *
         * @param inst The instruction being processed.
         * @param c The character signifying the action to be taken.
         * @param pos The position at which c occurred in the metadata.
         */
        virtual void ProcessSpecialMetadata(const InstPtr inst, char c, int pos);

    protected:
        Engine *_engine;        ///< Pointer to the Engine used for the script.
        std::ostream &_output;  ///< The std::ostream to output the code to.
        ValueStack _stack;      ///< The stack currently being processed.
        uint _indentLevel;      ///< Indentation level.
        GraphVertex _curVertex; ///< Graph vertex currently being processed.
        std::unique_ptr<LuaLanguage> target_lang_;

        /**
         * Processes an instruction. Called by process() for each instruction.
         * Call the base class implementation for opcodes you cannot handle yourself,
         * or where the base class implementation is preferable.
         *
         * @param inst The instruction to process.
         */
        void ProcessInst(Function& func, InstVec& insts, const InstPtr inst);
        void ProcessUncondJumpInst(Function& func, InstVec& insts, const InstPtr inst);
        void ProcessCondJumpInst(const InstPtr inst);

        /**
         * Indents a string according to the current indentation level.
         *
         * @param s The string to indent.
         * @result The indented string.
         */
        std::string indentString(std::string s);

        /**
         * Construct the signature for a function.
         *
         * @param func Reference to the function to construct the signature for.
         */
        virtual std::string ConstructFuncSignature(const Function& func);
        virtual void OnBeforeStartFunction(const Function& func);
        virtual void OnEndFunction(const Function& func);
        virtual void OnStartFunction(const Function&) { }
        virtual bool OutputOnlyRequiredLabels() const { return false; }

        void generatePass(InstVec& insts, const Graph& g);
        bool mIsLabelPass = true;

    private:
        Graph _g;                  ///< The annotated graph of the script.

        /**
         * Processes a GraphVertex.
         *
         * @param v The vertex to process.
         */
        void process(Function& func, InstVec& insts, GraphVertex v);

};
