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
 *
 * This is to be overriden by each engine.
 */
class CodeGenerator {

    public:

        /**
         * Constructor
         *
         * @param engine[in] The engine used for the script.
         * @param output[out] The stream to output the code to.
         * @param bin_order[in] Order of arguments for binary operators.
         * @param call_order[in] Order of arguments for function calls.
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
         * @param function_name[in] The name of the function.
         * @param param_format[in] Characters indicating the parameter format.
         * 'b' for boolean parameters, 'n' for integers (treated as unsigned)
         * or 'f' for floats.
         * @param params[in] The list of parameters.
         */
        void WriteFunctionCall(
          std::string function_name, std::string param_format, const std::vector<ValuePtr>& params
        );

        /**
         * Generates code from the provided graph and outputs it to stdout.
         *
         * @param insts[in] The list of instructions.
         * @param graph[in] The annotated graph of the script.
         */
        virtual void Generate(InstVec& insts, const Graph &graph);

        /**
         * Adds a line of code to the current group.
         *
         * @param line[in] The line to add.
         * @param unindent_before[in] Whether or not to remove an indentation
         * level before the line. Defaults to false.
         * @param indent_after[in] Whether or not to add an indentation level
         * after the line. Defaults to false.
         */
        virtual void AddOutputLine(
          std::string line, bool unindent_before = false, bool indent_after = false
        );

        /**
         * Writes a comment line indicating an unimplemented opcode.
         *
         * @param code_gen[in|out] The code generator.
         * @param class_name[in] The class where the instruction is. Unused.
         * @param instruction[in] The unimplemented instruction.
         */
        void WriteTodo(std::string class_name, std::string instruction);

        /**
         * Generate an assignment statement.
         *
         * @param dst[in] The variable being assigned to.
         * @param src[in] The value being assigned.
         */
        void WriteAssignment(ValuePtr dst, ValuePtr src);

        /**
         * Add an argument to the argument list.
         *
         * @param arg[in] The argument to add.
         */
        void AddArg(ValuePtr arg);

        /**
         * Process a single character of metadata.
         *
         * @param inst[in] The instruction being processed. Unused.
         * @param c[in] The character signifying the action to be taken. The
         * only valid one is 'p'.
         * @param pos[in] The position at which c occurred in the metadata.
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
         * @param function[in] The function the instruction is is.
         * @param inst[in] The instruction to process.
         * @param insts[in] Every instruction in the function.
         */
        void ProcessInst(Function& function, InstVec& insts, const InstPtr inst);

        /**
         * Processes an unconditional jump instruction. Called by
         * {@see ProcessInst()} for those instructions. Call the base class
         * implementation for opcodes not handled by an implemented engine,
         * or where the base class implementation is preferable.
         *
         * @param function[in] The function the instruction is is.
         * @param inst[in] The instruction to process.
         * @param insts[in] Every instruction in the function.
         */
        void ProcessUncondJumpInst(Function& function, InstVec& insts, const InstPtr inst);

        /**
         * Processes a conditional jump instruction. Called by
         * {@see ProcessInst()} for those instructions. Call the base class
         * implementation for opcodes not handled by an implemented engine,
         * or where the base class implementation is preferable.
         *
         * @param inst[in] The instruction to process.
         */
        void ProcessCondJumpInst(const InstPtr inst);

        /**
         * Indents a string according to the current indentation level.
         *
         * @param s[in] The string to indent.
         * @result The indented string.
         */
        std::string IndentString(std::string s);

        /**
         * Construct the signature for a function.
         *
         * @param function[in] Reference to the function to construct the
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
         * @param function[in] The function about to start.
         */
        virtual void OnBeforeStartFunction(const Function& function);

        /**
         * Adds lines to the script at the end a function.
         *
         * Called after writing a function. For this base class, it adds a
         * closing bracer "}".
         *
         * @param function[in] The function about to end.
         */
        virtual void OnEndFunction(const Function& function);

        /**
         * Adds lines to the script before a function instructions.
         *
         * Called after writing a function start. For this base class, it
         * does nothing.
         *
         * @param function[in] The function starting.
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
         * @param insts[in] The list of instructions.
         * @param graph[in] The code graph.
         * @todo Understand and explain.
         */
        void GeneratePass(InstVec& insts, const Graph& g);

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
         * @param vertex[in] The vertex to process.
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
