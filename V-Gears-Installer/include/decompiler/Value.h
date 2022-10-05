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

#include <deque>
#include <exception>
#include <ostream>
#include <string>
#include <boost/intrusive_ptr.hpp>
#include "common/scummsys.h"
#include "RefCounted.h"
#include "Stack.h"
#include "DecompilerException.h"

class Value;

/**
 * Precedence value for individual values with no operations.
 */
const int PRECEDENCE_NO = 0;

/**
 * Precedence value for a unary operation. (!, -, ~, etc.).
 */
const int PRECEDENCE_UNARY = 1;

/**
 * Precedence value for multiplication, division, modulus (*, /, %).
 */
const int PRECEDENCE_MULT = 2;

/**
 * Precedence value for addition and subtraction (+, -).
 */
const int PRECEDENCE_ADD = 3;

/**
 * Precedence value for bit shifting (<<, >>).
 */
const int PRECEDENCE_SHIFT = 4;

/**
 * Precedence value for relative comparison (<, <=, >=, >).
 */
const int PRECEDENCE_RELATION = 5;

/**
 * Precedence value for equality comparisons (==, !=).
 */
const int PRECEDENCE_EQUALITY = 6;

/**
 * Precedence value for bitwise AND (&).
 */
const int PRECEDENCE_BIT_AND = 7;

/**
 * Precedence value for bitwise XOR (^).
 */
const int PRECEDENCE_BIT_XOR = 8;

/**
 * Precedence value for bitwise OR (|).
 */
const int PRECEDENCE_BIT_OR = 9;

/**
 * Precedence value for logical AND (&&).
 */
const int PRECEDENCE_LOGIC_AND = 10;

/**
 * Precedence value for logical OR (||).
 */
const int PRECEDENCE_LOGIC_OR = 11;

/**
 * Pointer to a Value.
 */
typedef boost::intrusive_ptr<Value> ValuePtr;

/**
 * Type representing a list of values, e.g. for indexes used to access an array.
 */
typedef std::deque<ValuePtr> ValueList;

/**
 * Type representing a stack.
 */
typedef Stack<ValuePtr> ValueStack;

/**
 * Class representing a value (stack entry, parameter, etc.)
 */
class Value : public RefCounted{

    public:

        /**
         * Destructor.
         */
        virtual ~Value();

        /**
         * Return whether or not the Value is an integer.
         *
         * @return True if the Value is an integer, otherwise false.
         */
        virtual bool IsInteger();

        /**
         * Return whether or not the Value is an address.
         *
         * @return True if the Value is an address, otherwise false.
         */
        virtual bool IsAddress();

        /**
         * Returns whether or not any stored integer value is signed.
         *
         * @return True if the integer value is signed, false if it is not.
         * @throws WrongTypeException if the value is not an integer.
         */
        virtual bool IsSignedValue();

        /**
         * Retrieves a signed integer representing the value, if possible.
         *
         * @return A signed integer representing the value, if possible.
         * @throws WrongTypeException if the value is not an integer.
         */
        virtual int32 GetSigned();

        /**
         * Retrieves an unsigned integer representing the value, if possible.
         *
         * @return An unsigned integer representing the value, if possible.
         * @throws WrongTypeException if the value is not an integer.
         */
        virtual uint32 GetUnsigned();

        /**
         * Print the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const = 0;

        /**
         * Retrieves the string representation of the value.
         *
         * @return The string representation of the value.
         */
        virtual std::string GetString() const;

        /**
         * Duplicates a value.
         *
         * @param[out] output The stream to output any necessary assignment.
         * @return A Value corresponding to a duplicate of this entry.
         */
        virtual ValuePtr Dup(std::ostream &output);

        /**
         * Negates a value.
         *
         * @return The current Value, negated.
         * @throws WrongTypeException if negation is not possible.
         */
        virtual ValuePtr Negate();

        /**
         * Operator precedence for this value.
         *
         * Lower values bind stronger, i.e. they are resolved earlier.
         * If an operand has a higher precedence value than the operator,
         * parentheses are not required for that operand.
         *
         * @return The order of precedence.
         */
        virtual int GetPrecedence() const;

        /**
         * Output a value to a stream.
         *
         * @param[out] output The  stream to output to.
         * @param[in] value Reference counted pointer to the value to output.
         * @return The stream used for output.
         */
        friend std::ostream &operator<<(std::ostream &output, Value *value) {
            return value->Print(output);
        }

};

/**
 * Value containing an integer.
 */
class IntValue : public Value {

    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        IntValue(const IntValue& value) = delete;

        /**
         * Copy constructor, disabled
         *
         * @param[in] value The value to copy.
         */
        IntValue& operator = (const IntValue& value) = delete;

        /**
         * Constructor for IntValue.
         *
         * @param[in] val The integer value to be contained.
         * @param[in] is_signed Whether or not the value is signed. This will affect output.
         */
        IntValue(int32 val, bool is_signed);

        /**
         * Constructor for IntValue.
         *
         * @param[in] val The integer value to be contained.
         * @param[in] is_signed Whether or not the value is signed. This will affect output.
         */
        IntValue(uint32 val, bool is_signed);

        /**
         * Return whether or not the Value is an integer.
         *
         * @return True.
         */
        bool IsInteger() override;

        /**
         * Returns whether or not the stored integer value is signed.
         *
         * @return True if the integer value is signed, false if it is not.
         */
        bool IsSignedValue() override;

        /**
         * Retrieves a signed integer representing the value, if possible.
         *
         * @return A signed integer representing the value, if possible.
         */
        int32 GetSigned() override;

        /**
         * Retrieves an unsigned integer representing the value, if possible.
         *
         * @return An unsigned integer representing the value, if possible.
         * @throws WrongTypeException if the value is not an integer.
         */
        uint32 GetUnsigned() override;

        /**
         * Duplicates the value.
         *
         * @param[out] output The stream to output any necessary assignment.
         * @return A Value corresponding to a duplicate of the value.
         */
        ValuePtr Dup(std::ostream &output) override;

        /**
         * Print the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

    protected:

        /**
         * The value of the integer.
         */
        const int32 val_;

        /**
         * True if the value is signed, false if it's not.
         */
        const bool signed_;
};

/**
 * Value containing an absolute address.
 */
class AddressValue: public IntValue{

    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        AddressValue(const AddressValue& value) = delete;

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        AddressValue& operator = (const AddressValue& value) = delete;

        /**
         * Constructor for AddressValue.
         *
         * @param[in] addr The absolute address represented by the value.
         */
        explicit AddressValue(uint32 addr);

        /**
         * Return whether or not the Value is an address.
         *
         * @return True.
         */
        bool IsAddress() override;

        /**
         * Always throws {@see WrongTypeException}.
         *
         * A memory address can't ever be signed.
         *
         * @return Nothing.
         * @throws WrongTypeException always.
         */
        int32 GetSigned() override;

        /**
         * Duplicates the value.
         *
         * @param[out] output The stream to output any necessary assignment.
         * @return A Value corresponding to a duplicate of the value.
         */
        ValuePtr Dup(std::ostream &output) override;

        /**
         * Print the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;
};

/**
 * Value containing a signed, relative address.
 *
 * When asking for unsigned integer value, exact address is returned; when
 * printing or getting signed value, relative address is used.
 */
class RelAddressValue : public IntValue{

    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        RelAddressValue(const RelAddressValue& value) = delete;

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        RelAddressValue& operator = (const RelAddressValue& value) = delete;

        /**
         * Constructor.
         *
         * @param[in] base_addr The base address for the offset.
         * @param[in] offset The relative offset to the base address.
         */
        RelAddressValue(uint32 base_addr, int32 offset);

        /**
         * Return whether or not the Value is an address.
         *
         * @return True.
         */
        bool IsAddress() override;

        /**
         * Retrieves the exact address.
         *
         * @return The exact address.
         * @throws WrongTypeException if the value is not an integer.
         */
        uint32 GetUnsigned() override;

        /**
         * Duplicates the value.
         *
         * @param[out] output The stream to output any necessary assignment.
         * @return A Value corresponding to a duplicate of the value.
         */
        ValuePtr Dup(std::ostream &output) override;

        /**
         * Print the relative address to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

    protected:

        /**
         * The base address for the offset.
         */
        const uint32 base_addr_;
};

/**
 * Duplicated value.
 */
class DupValue : public Value {

    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        DupValue(const DupValue& value) = delete;

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        DupValue& operator = (const DupValue& value) = delete;

        /**
         * Constructor.
         *
         * @param idx Index to distinguish multiple duplicated entries.
         */
        explicit DupValue(int idx);

        /**
         * Duplicates the value.
         *
         * @param[out] output The stream to output any necessary assignment.
         * @return A Value corresponding to a duplicate of the value.
         */
        ValuePtr Dup(std::ostream &output) override;

        /**
         * Print the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

    protected:

        /**
         * Index to distinguish multiple duplicated entries.
         */
        const int index_;
};

/**
 * String value.
 */
class StringValue : public Value {

    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        StringValue(const StringValue& value) = delete;

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        StringValue& operator = (const StringValue& value) = delete;

        /**
         * Constructor.
         *
         * @param[in] str The string value.
         */
        explicit StringValue(std::string str);

        /**
         * Print the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

    protected:

        /**
         * The string value.
         */
        const std::string str_;
};

/**
 * A string value, unquoted.
 */
class UnquotedStringValue : public StringValue{

    public:

        /**
         * Constructor.
         *
         * @param[in] str The string value.
         */
        explicit UnquotedStringValue(std::string str);

        /**
         * Prints the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;
};

/**
 * Value representing a variable.
 */
class VarValue : public Value {

    public:
        /**
         * Constructor for VarValue.
         *
         * @param name The variable name.
         */
        explicit VarValue(std::string name);
        /**
         * Print the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

    protected:

        /**
         * The variable name.
         */
        std::string name_;
};

/**
 * Value representing array access.
 */
class ArrayValue : public VarValue {

    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        ArrayValue(const ArrayValue& value) = delete;

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        ArrayValue& operator = (const ArrayValue& value) = delete;

        /**
         * Constructor for ArrayValue.
         *
         * @param name The name of the array.
         * @param indexes List of stack entries representing the indexes used
         * (left-to-right).
         */
        ArrayValue(const std::string name, const ValueList indexes);

        /**
         * Print the value to a stream.
         *
         * Every item in the array will be printed.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

    protected:

        /**
         * Values representing the indexes used (left-to-right).
         */
        const ValueList indexes_;
};

/**
 * Value representing the result of a binary operation.
 */
class BinaryOpValue : public Value {


    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        BinaryOpValue(const BinaryOpValue& value) = delete;

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        BinaryOpValue& operator = (const BinaryOpValue& value) = delete;

        /**
         * Constructor.
         *
         * @param[in] left Value representing the left side of the operator.
         * @param[in] right Value representing the right side of the operator.
         * @param[in] oper The operator for this value.
         */
        BinaryOpValue(ValuePtr left, ValuePtr right, std::string oper);

        /**
         * Print the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

        /**
         * Negates a value.
         *
         * @return The value, negated.
         */
        virtual ValuePtr Negate() override;

        /**
         * Retrieves the operator precedence for this operation.
         *
         * Lower values bind stronger, i.e. they are resolved earlier.
         * If an operand has a higher precedence value than the operator,
         * parentheses are not required for that operand.
         *
         * @return the order of precedence for the operation.
         */
        virtual int GetPrecedence() const override;

    protected:

        /**
         * Value at the left side of the operator.
         */
        const ValuePtr left_val_;

        /**
         * Value at the right side of the operator.
         */
        const ValuePtr right_val_;

        /**
         * The operator.
         */
        const std::string oper_;
};

/**
 * Value representing the result of a unary operation.
 *
 * Used as base class for prefix and postfix variants.
 */
class UnaryOpValue : public Value{

    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        UnaryOpValue(const UnaryOpValue& value) = delete;

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        UnaryOpValue& operator = (const UnaryOpValue& value) = delete;

        /**
         * Constructor.
         *
         * @param[in] operand Value representing the operand of the operation.
         * @param[in] oper The operator for this value.
         * @param[in] postfix Whether or not the operator should be postfixed
         * to the operand.
         */
        UnaryOpValue(ValuePtr operand, std::string oper, bool postfix);

        /**
         * Print the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

        /**
         * Retrieves the operator precedence for this operation.
         *
         * Lower values bind stronger, i.e. they are resolved earlier.
         * If an operand has a higher precedence value than the operator,
         * parentheses are not required for that operand.
         *
         * @return {@see PRECEDENCE_UNARY_OP}.
         */
        virtual int GetPrecedence() const override;

    protected:

        /**
         * The operand of the operation
         */
        const ValuePtr operand_;

        /**
         * The operator for this value
         */
        const std::string oper_;

        /**
         * True if the operator is postfixed to the operand, false otherwise.
         */
        const bool postfix_;
};

/**
 * Negated value.
 */
class NegatedValue : public UnaryOpValue{

    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        NegatedValue(const NegatedValue& value) = delete;

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        NegatedValue& operator = (const NegatedValue& value) = delete;

        /**
         * Constructor.
         *
         * @param[in] val The value to negate.
         */
        explicit NegatedValue(ValuePtr val);

        /**
         * Negates the value.
         *
         * @return The value, negated.
         */
        virtual ValuePtr Negate() override;
};

/**
 * Value representing a function call.
 */
class CallValue : public Value {

    public:

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        CallValue(const CallValue& value) = delete;

        /**
         * Copy constructor, disabled.
         *
         * @param[in] value The value to copy.
         */
        CallValue& operator = (const CallValue& value) = delete;

        /**
         * Constructor for CallValue.
         *
         * @param[in] function The name of the function.
         * @param[in] args List of values representing the arguments used.
         */
        CallValue(std::string function, ValueList args);

        /**
         * Print the value to a stream.
         *
         * @param[out] output The stream to write to.
         * @return The stream used for output.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

    protected:

        /**
         * The name of the function.
         */
        const std::string function_;

        /**
         * List of values used as function arguments.
         */
        const ValueList args_;

};
