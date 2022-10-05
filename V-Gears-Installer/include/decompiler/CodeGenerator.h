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

#include "DecompilerException.h"
#include "Graph.h"
#include "LuaLanguage.h"
#include "Value.h"


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
 *
 * This is to be overriden by each engine.
 */
class CodeGenerator {

    public:

        /**
         * Constructor
         *
         * @param[in] engine The engine used for the script.
         * @param[out] output The stream to output the code to.
         * @param[in] bin_order Order of arguments for binary operators.
         * @param[in] call_order Order of arguments for function calls.
         */
        CodeGenerator(
          Engine *engine, std::ostream &output, ARGUMENT_ORDER bin_order, ARGUMENT_ORDER call_order
        );

        /**
         * Destructor.
         *
         * Does nothing.
         */
        virtual ~CodeGenerator();

        /**
         * Retrieves the target language.
         */
        LuaLanguage& GetLanguage();

        /**
         * Writes a function call.
         *
         * @param[in] function_name The name of the function.
         * @param[in] param_format Characters indicating the parameter format.
         * 'b' for boolean parameters, 'n' for integers (treated as unsigned)
         * or 'f' for floats.
         * @param[in] params The list of parameters.
         */
        void WriteFunctionCall(
          std::string function_name, std::string param_format, const std::vector<ValuePtr>& params
        );

        /**
         * Generates code from the provided graph and outputs it to stdout.
         *
         * @param[in] insts The list of instructions.
         * @param[in] graph The annotated graph of the script.
         */
        virtual void Generate(InstVec& insts, const Graph &graph);

        /**
         * Adds a line of code to the current group.
         *
         * @param[in] line The line to add.
         * @param[in] unindent_before Whether or not to remove an indentation
         * level before the line. Defaults to false.
         * @param[in] indent_after Whether or not to add an indentation level
         * after the line. Defaults to false.
         */
        virtual void AddOutputLine(
          std::string line, bool unindent_before = false, bool indent_after = false
        );

        /**
         * Writes a comment line indicating an unimplemented opcode.
         *
         * @param[in] class_name The class where the instruction is. Unused.
         * @param[in] instruction The unimplemented instruction.
         */
        void WriteTodo(std::string class_name, std::string instruction);

        /**
         * Generate an assignment statement.
         *
         * @param[in] dst The variable being assigned to.
         * @param[in] src The value being assigned.
         */
        void WriteAssignment(ValuePtr dst, ValuePtr src);

        /**
         * Add an argument to the argument list.
         *
         * @param[in] arg The argument to add.
         */
        void AddArg(ValuePtr arg);

        /**
         * Process a single character of metadata.
         *
         * @param[in] inst The instruction being processed. Unused.
         * @param[in] c The character signifying the action to be taken. The
         * only valid one is 'p'.
         * @param[in] pos The position at which c occurred in the metadata.
         * Unused.
         */
        virtual void ProcessSpecialMetadata(const InstPtr inst, char c, int pos);

        /**
         * Retrieves the argument list.
         *
         * @return The argument list.
         */
        virtual ValueList GetArgList();

        /**
         * Retrieves the order of operands for binary operations.
         *
         * @return The order of operands
         */
        virtual ARGUMENT_ORDER GetBinaryOrder();

    protected:

        /**
         * Processes an instruction. Called by {@see Process()} for each
         * instruction. Call the base class implementation for opcodes not
         * handled by an implemented engine, or where the base class
         * implementation is preferable.
         *
         * @param[in] function The function the instruction is is.
         * @param[in] inst The instruction to process.
         * @param[in] insts Every instruction in the function.
         */
        void ProcessInst(Function& function, InstVec& insts, const InstPtr inst);

        /**
         * Processes an unconditional jump instruction. Called by
         * {@see ProcessInst()} for those instructions. Call the base class
         * implementation for opcodes not handled by an implemented engine,
         * or where the base class implementation is preferable.
         *
         * @param[in] function The function the instruction is is.
         * @param[in] inst The instruction to process.
         * @param[in] insts Every instruction in the function.
         */
        void ProcessUncondJumpInst(Function& function, InstVec& insts, const InstPtr inst);

        /**
         * Processes a conditional jump instruction. Called by
         * {@see ProcessInst()} for those instructions. Call the base class
         * implementation for opcodes not handled by an implemented engine,
         * or where the base class implementation is preferable.
         *
         * @param[in] inst The instruction to process.
         */
        void ProcessCondJumpInst(const InstPtr inst);

        /**
         * Indents a string according to the current indentation level.
         *
         * @param[in] s The string to indent.
         * @result The indented string.
         */
        std::string IndentString(std::string s);

        /**
         * Construct the signature for a function.
         *
         * @param[in] function Reference to the function to construct the
         * signature for.
         * @return For this base class, an empty string.
         */
        virtual std::string ConstructFuncSignature(const Function& function);

        /**
         * Adds lines to the script before a function.
         *
         * Called before writing a function start. For this base class, it
         * does nothing.
         *
         * @param[in] function The function about to start.
         */
        virtual void OnBeforeStartFunction(const Function& function);

        /**
         * Adds lines to the script at the end a function.
         *
         * Called after writing a function. For this base class, it adds a
         * closing bracer "}".
         *
         * @param[in] function The function about to end.
         */
        virtual void OnEndFunction(const Function& function);

        /**
         * Adds lines to the script before a function instructions.
         *
         * Called after writing a function start. For this base class, it
         * does nothing.
         *
         * @param[in] function The function starting.
         */
        virtual void OnStartFunction(const Function& function);

        /**
         * Checks if only required labels are to be written.
         *
         * @return Always false.
         */
        virtual bool OutputOnlyRequiredLabels() const;

        /**
         * Generates a pass.
         *
         * @param[in] insts The list of instructions.
         * @param[in] graph The code graph.
         * @todo Understand and explain.
         */
        void GeneratePass(InstVec& insts, const Graph& graph);

        /**
         * Indicates if a label is being processed.
         */
        bool is_label_pass_ = true;

        /**
         * The group currently being processed.
         */
        GroupPtr cur_group_;

        /**
         * The engine used for teh script.
         */
        Engine *engine_;

        /**
         * The stream to output the code to.
         */
        std::ostream &output_;

        /**
         * The stack currently being processed.
         */
        ValueStack stack_;

        /**
         * Current indentation level.
         */
        uint indent_level_;

        /**
         * Graph vertex currently being processed.
         */
        GraphVertex cur_vertex_;

        /**
         * The target language.
         */
        std::unique_ptr<LuaLanguage> target_lang_;

    private:

        /**
         * Processes a GraphVertex.
         *
         * @param[in] function The function to process.
         * @param[in] insts The list of instructions.
         * @param[in] vertex The vertex to process.
         */
        void Process(Function& function, InstVec& insts, GraphVertex vertex);

        /**
         * The annotated graph of the script.
         */
        Graph graph_;

        /**
         * Order of operands for binary operations.
         */
        const ARGUMENT_ORDER bin_order_;

        /**
         * Order of operands for call arguments.
         */
        const ARGUMENT_ORDER call_order_;

        /**
         * Lists of arguments to be built when processing function calls.
         */
        ValueList arg_list_;

};
