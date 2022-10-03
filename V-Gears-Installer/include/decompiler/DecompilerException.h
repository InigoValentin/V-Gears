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

#include <exception>
#include <string>

class DecompilerException : public std::exception{};

/**
 * Wrong type exception.
 *
 * Signals that an operation was performed on a Value that doesn't support the
 * operation.
 */
class WrongTypeException : public DecompilerException{};

/**
 * Not implemented exception.
 *
 * Signals that a function is not implemented in the decompiler.
 */
class NotImplementedException : public DecompilerException{

    public:

        /**
         * Constructor.
         */
        NotImplementedException() = default;
};

/**
 * Invalid header exception.
 *
 * Signals that a script has an invalid header.
 */
class ScriptHeaderInvalidException : public DecompilerException{

    public:

        /**
         * Constructor.
         */
        ScriptHeaderInvalidException() = default;
};

/**
 * Unknown bank exception.
 *
 * Signals that an unknown bank has been tried to access.
 */
class UnknownBankException : public DecompilerException{

    public:

        /**
         * Constructor.
         */
        UnknownBankException() = default;
};

/**
 * Unknown opcode parameter exception.
 *
 * Signals that an opcode has an invalid parameter in a script.
 */
class UnknownOpcodeParameterException : public DecompilerException{

    public:

        /**
         * Constructor.
         *
         * @param param[in] The unknown parameter.
         */
        UnknownOpcodeParameterException(std::string param);

        /**
         * Produces an error message.
         *
         * @return The error message.
         */
        virtual const char *what() const noexcept override;

    private:

        /**
         * The error message.
         */
        std::string what_;
};

/**
 * Unknown conditional operator exception.
 *
 * Signals that an unknown conditional operator has been found in a script.
 */
class UnknownConditionalOperatorException : public DecompilerException{

    public:

        /**
         * Constructor.
         *
         * @param address[in] Address at which the operator has been found.
         * @param op[in] The unknown operator.
         */
        UnknownConditionalOperatorException(unsigned int address, unsigned int op);

        /**
         * Produces an error message.
         *
         * @return The error message.
         */
        virtual const char *what() const noexcept override;

    private:

        /**
         * The error message.
         */
        std::string what_;
};

/**
 * Unknown opcode exception.
 *
 * Signals that an unknown opcode has been found in a script.
 */
class UnknownOpcodeException : public DecompilerException{

    public:

        /**
         * Constructor.
         *
         * @param address[in] Address where the invalid opcode was found.
         * @param opcode[in] The value of the invalid opcode.
         */
        UnknownOpcodeException(unsigned int address, unsigned int opcode);

        /**
         * Produces an error message.
         *
         * @return The error message.
         */
        virtual const char* what() const noexcept override;

    private:

        /**
         * Retrieves an exception identifier.
         *
         * @return "Opcode".
         */
        virtual const char* Type() const;

        /**
         * Address where the invalid opcode was found.
         */
        unsigned int address_;

        /**
         * The value of the invalid opcode.
         */
        unsigned int opcode_;

        /**
         * Buffer for formatting the error message.
         */
        mutable char buffer_[255];
};

/**
 * Unknown jump type exception.
 *
 * Signals that a jump is not a conditional jump neither an unconditional
 * jump.
 */
class UnknownJumpTypeException : public DecompilerException{

    public:

        /**
         * Constructor.
         *
         * @param address[in] Address where the jump opcode was found.
         * @param opcode[in] The value of the jump opcode.
         */
        UnknownJumpTypeException(unsigned int address, unsigned int opcode);

        /**
         * Produces an error message.
         *
         * @return The error message.
         */
        virtual const char* what() const noexcept override;

    private:

        /**
         * The error message.
         */
        std::string what_;
};

/**
 * Unknown subopcode exception.
 *
 * Signals that an unknown subopcode has been found in a script.
 */
class UnknownSubOpcodeException : public UnknownOpcodeException{

    public:

        /**
         * Constructor.
         *
         * @param address[in] Address where the invalid subopcode was found.
         * @param opcode[in] The value of the invalid subopcode.
         */
        UnknownSubOpcodeException(unsigned int address, unsigned int opcode);

    private:

        /**
         * Retrieves an exception identifier.
         *
         * @return "Subopcode".
         */
        virtual const char* Type() const override;

};
