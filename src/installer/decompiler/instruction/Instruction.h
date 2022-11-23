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

#include <string>
#include <vector>
#include <boost/format.hpp>
#include <boost/intrusive_ptr.hpp>
#include "installer/decompiler/RefCounted.h"
#include "installer/decompiler/Value.h"
#include "installer/common/scummsys.h"
#include "installer/decompiler/DecompilerException.h"

class CodeGenerator;

class Engine;

/**
 * Changes whether or not to output the stack effect for an instruction.
 *
 * @param[in] value True to output the effect, false not to.
 */
void SetOutputStackEffect(bool value);

class Instruction;

/**
 * Pointer to an Instruction.
 */
typedef boost::intrusive_ptr<Instruction> InstPtr;

class Function;

/**
 * Structure for representing an instruction.
 */
class Instruction : public RefCounted {

    public:

        /**
         * Binary operation (e.g. +, &&, etc.), including comparisons.
         */
        static int INST_TYPE_BINARY_OP;

        /**
         * Boolean negation.
         */
        static int INST_TYPE_BOOL_NEGATE;

        /**
         * Regular function call.
         */
        static int INST_TYPE_CALL;

        /**
         * Conditional jump.
         */
        static int INST_TYPE_COND_JUMP;

        /**
         * Instruction duplicates the most recent stack entry.
         */
        static int INST_TYPE_DUP;

        /**
         * Unconditional jump.
         */
        static int INST_TYPE_JUMP;

        /**
         * Kernel functions.
         */
        static int INST_TYPE_KERNEL_CALL;

        /**
         * Load value from memory.
         */
        static int INST_TYPE_LOAD;

        /**
         * Return from regular function call.
         */
        static int INST_TYPE_RETURN;

        /**
         * Stack allocation or deallocation (altering stack pointer).
         */
        static int INST_TYPE_STACK;

        /**
         * Store value in memory.
         */
        static int INST_TYPE_STORE;

        /**
         * Unary operation (e.g. !) with operator placed before the operator.
         */
        static int INST_TYPE_UNARY_OP_PRE;

        /**
         * Unary operation with operator placed after the operator.
         */
        static int INST_TYPE_UNARY_OP_POST;

        /**
         * Operator overload to output an Instruction to a stream.
         *
         * @param[out] output The stream to output to.
         * @param[in] inst The Instruction to output.
         * @return The stream used for output.
         */
        friend std::ostream& operator<<(std::ostream &output, const Instruction *inst) {
            return inst->Print(output);
        }

        /**
         * Print the instruction to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const;

        /**
         * Checks if the instruction is a jump of some sort.
         *
         * @return True if the instruction is a jump, otherwise false.
         */
        virtual bool IsJump() const;

        /**
         * Checks if the instruction is a conditional jump.
         *
         * @return True if the instruction is a conditional jump, false
         * otherwise.
         */
        virtual bool IsCondJump() const;

        /**
         * Checks if the instruction is an unconditional jump.
         *
         * @return True if the instruction is an unconditional jump, false
         * otherwise.
         */
        virtual bool IsUncondJump() const;

        /**
         * Checks if the instruction is a stack operation.
         *
         * @return True if the instruction is a stack operation, false
         * otherwise.
         */
        virtual bool IsStackOp() const;

        /**
         * Checks if the instruction is a call to a script function.
         *
         * @return True if the instruction is a script function call, false
         * otherwise.
         */
        virtual bool IsFuncCall() const;

        /**
         * Checks if the instruction is a return statement.
         *
         * @return True if the instruction is a return statement, false
         * otherwise.
         */
        virtual bool IsReturn() const;

        /**
         * Checks if the instruction is a call to a kernel function.
         *
         * @return True if the instruction is a kernel function call, false
         * otherwise.
         */
        virtual bool IsKernelCall() const;

        /**
         * Checks if the instruction is a load operation.
         *
         * @return True if the instruction is a load operation, false
         * otherwise.
         */
        virtual bool IsLoad() const;

        /**
         * Checks if the instruction is a store operation.
         *
         * @return True if the instruction is a store operation, false
         * otherwise.
         */
        virtual bool IsStore() const;

        /**
         * Returns the destination address of a jump instruction.
         *
         * @return Destination address of a jump instruction.
         * @throws WrongTypeException if instruction is not a jump.
         */
        virtual uint32 GetDestAddress() const;

        /**
         * Process an instruction for code generation.
         *
         * @param[in] function The function the instruction is in.
         * @param[in] stack The current stack.
         * @param[in] engine The Engine used for code generation.
         * @param[in] code_gen The CodeGenerator used for code generation.
         */
        virtual void ProcessInst(
          Function& function, ValueStack& stack, Engine* engine, CodeGenerator* code_gen
        ) = 0;

        /**
         * Retrieves the instruction opcode.
         *
         * @return The opcode.
         */
        uint32 GetOpcode() const;

        /**
         * Sets the instruction opcode.
         *
         * @param[in] opcode The opcode.
         */
        void SetOpcode(uint32 opcode);

        /**
         * Retrieves the instruction address.
         *
         * @return The address.
         */
        uint32 GetAddress() const;

        /**
         * Sets the instruction address.
         *
         * @param[in] address The address.
         */
        void SetAddress(uint32 address);

        /**
         * Retrieves the instruction name (the opcode name).
         *
         * @return The name.
         */
        std::string GetName() const;

        void SetName(std::string name);

        /**
         * Checks how much the instruction changes the stack pointer.
         *
         * @return The stack pointer change, in bytes.
         */
        int16 GetStackChange() const;

        /**
         * Defines how much the instruction changes the stack pointer.
         *
         * @param[in] stack_change Bytes the instruction changes the stack
         * pointer by.
         */
        void SetStackChange(int16 stack_change);

        /**
         * Retrieves the list of instruction parameters.
         *
         * @return The list of parameters.
         */
        std::vector<ValuePtr> GetParams() const;

        /**
         * Retrieves a instruction parameter.
         *
         * @param[in] index Index of the parameter to retrieve.
         * @return The parameter at the specified index, or nullptr if there
         * is not that many parameters.
         */
        ValuePtr GetParam(uint32 index) const;

        /**
         * Sets the instructions parameters.
         *
         * @param[in] params The list of instruction parameters.
         */
        void SetParams(std::vector<ValuePtr> params);

        /**
         * Adds a parameter to the instructions.
         *
         * @param[in] value The parameter to add.
         */
        void AddParam(ValuePtr value);

        /**
         * Retrieves metadata for code generation.
         *
         * See the extended documentation for details.
         *
         * @return Code generator metadata.
         */
        std::string GetCodeGenData() const;

        /**
         * Sets metadata for code generation.
         *
         * See the extended documentation for details.
         *
         * @param[in] code_gen_data Code generator metadata.
         */
        void SetCodeGenData(std::string code_gen_data);

        /**
         * Checks if the instruction requires a label.
         */
        bool LabelRequired() const;

        /**
         * Indicates if the instruction needs a label.
         */
        void SetLabelRequired(bool required);

    protected:

        /**
         * The instruction opcode.
         */
        uint32 opcode_;

        /**
         * The instruction address.
         */
        uint32 address_;

        /**
         * The instruction name (opcode name).
         */
        std::string name_;

        /**
         * How much this instruction changes the stack pointer by.
         */
        int16 stack_change_;

        /**
         * Array of parameters used for the instruction.
         */
        std::vector<ValuePtr> params_;

        /**
         * String containing metadata for code generation.
         *
         * See the extended documentation for details.
         */
        std::string code_gen_data_;

        /**
         * Indicates if a label is required.
         */
        bool label_required_ = false;
};

/**
 * Type representing a vector of InstPtrs.
 */
typedef std::vector<InstPtr> InstVec;

/**
 * Type representing an iterator over InstPtrs.
 */
typedef InstVec::iterator InstIterator;

/**
 * Type representing a const_iterator over InstPtrs.
 */
typedef InstVec::const_iterator ConstInstIterator;
