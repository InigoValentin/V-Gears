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

#include <boost/algorithm/string.hpp>
#include <deque>
#include <unordered_map>

#include "FieldScriptFormatter.h"
#include "decompiler/CodeGenerator.h"

/**
 * Parses an stores function metadata.
 *
 * Function metadata can indicate the entity and character of a function, and
 * if the function is the first or last in a class.
 */
class FunctionMetaData{

    public:

        /**
         * Constructor. Parses function metadata.
         *
         * Meta data format can be:
         * - start_end_entityname
         * - start_entityname
         * - end_entity_name
         *
         * @param meta_data[in] Metadata to parse.
         */
        FunctionMetaData(std::string meta_data);

        /**
         * Checks if the function is the first of a class.
         *
         * @return True if the metadata is the first of a class, false
         * otherwise.
         */
        bool IsStart() const;

        /**
         * Checks if the function is the last of a class.
         *
         * @return True if the metadata is the last of a class, false
         * otherwise.
         */
        bool IsEnd() const;

        /**
         * Retrieves the entity name.
         *
         * @return The entity name.
         */
        std::string GetEntityName();

        /**
         * Retrieves the character ID.
         *
         * @return The character ID.
         */
        int GetCharacterId();

    private:

        /**
         * Delimiter for metadata tokens.
         */
        static const std::string DELIMITER;

        /**
         * Start tag for metadata.
         */
        static const std::string START;

        /**
         * End tag for metadata.
         */
        static const std::string END;

        /**
         * Parses metadata.
         *
         * @param meta_data[in] Metadata to parse.
         */
        void Parse(std::string meta_data);

        /**
         * Parses a metadata item, looking for a start mark.
         *
         * If it finds it, marks the metadata as the first function of a
         * class. It also * calls {@see ParseEnd} with the next token.
         *
         * @param item[in] The item to check for a start mark.
         * @param item[in] All the metadata tokens.
         */
        void ParseStart(const std::string& item, std::deque<std::string>& strs);

        /**
         * Parses a metadata item, looking for an end mark.
         *
         * If it finds it, marks the metadata as the last function of a class.
         * It also calls {@see ParseCharId}.
         *
         * @param item[in] The item to check for a start mark.
         * @param item[in] Metadata tokens, first removed.
         */
        void ParseEnd(const std::string& item, std::deque<std::string>& strs);

        /**
         * Parses a metadata item, looking for a character ID.
         *
         * If it finds it, sets the character ID. It also calls
         * {@see ParseEntity}.
         *
         * @param item[in] The item to check for a start mark.
         * @param item[in] Metadata tokens, first two removed.
         */
        void ParseCharId(const std::string& item, std::deque<std::string>& strs);

        /**
         * Parses a metadata item, looking for an entity name.
         *
         * It sets the entity name.
         *
         * @param item[in] The item to check for a start mark.
         * @param item[in] Metadata tokens, first three removed.
         */
        void ParseEntity(const std::string& item, std::deque<std::string>& strs);

        /**
         * Indicates if the function is the last of a class.
         */
        bool end_ = false;

        /**
         * Indicates if the function is the first of a class.
         */
        bool start_ = false;

        /**
         * The entity name.
         */
        std::string entity_name_;

        /**
         * The character ID.
         */
        int character_id_ = -1;
};

/**
 * The code generator.
 *
 * Generates code for a field map function.
 */
class FieldCodeGenerator : public CodeGenerator{

    public:

        /**
         * Type of data.
         */
        enum struct ValueType : int{

            /**
             * Integer.
             */
            Integer = 0,

            /**
             * Float.
             */
            Float = 1
        };

        /**
         * Generates code for an unimplemented instruction.
         *
         * The code will actually be a comment, indicating where the function
         * call must have been.
         *
         * @param entity[in] The entity name.
         * @param address[in] The address of the instruction.
         * @param opcode[in] The opcode not implemented.
         */
        static const std::string FormatInstructionNotImplemented(
          const std::string& entity, uint32 address, uint32 opcode
        );

        /**
         * Generates code for an unimplemented instruction.
         *
         * The code will actually be a comment, indicating where the function
         * call must have been.
         *
         * @param entity[in] The entity name.
         * @param address[in] The address of the instruction.
         * @param opcode[in] The unimplemented instruction.
         */
        static const std::string FormatInstructionNotImplemented(
          const std::string& entity, uint32 address, const Instruction& instruction
        );

        /**
         * Formats a value as a boolean.
         *
         * @param value[in] The value to format.
         * @return "false" if the value is 0, "true" otherwise.
         */
        static const std::string FormatBool(uint32 value);

        /**
         * Formats a value as a boolean.
         *
         * @param value[in] The value to format.
         * @return "true" if the value is 0, "false" otherwise.
         */
        static const std::string FormatInvertedBool(uint32 value);

        /**
         * Formats a data access for a map script.
         *
         * If possible, it will look for friendly names for variables.
         *
         * @param formatter[in] Formatter to look up variable friendly names.
         * @param bank[in] The memory bank to use.
         * @param value_or_address[in] The value or memory address to use.
         * When bank is 0, it will be considered as a value. When bank is non
         * 0, it will be considered an address of the bank.
         * @param value_type[in] Data type to use. Used only when getting a
         * value, not a bank address.
         * @param scale[in] Scale to scale values to. Used only when using
         * float values, unused when type is integer or when using a bank
         * address.
         * @return String with the friendly variable name, value, or bank
         * address.
         */
        template<typename TValue>static const std::string FormatValueOrVariable(
          FieldScriptFormatter& formatter, uint32 bank, TValue value_or_address,
          ValueType type = ValueType::Integer, float scale = 1.0f
        ){
            switch (bank){
                case 0:
                    switch (type){
                        // TODO: check for zero
                        case ValueType::Float: return std::to_string(value_or_address / scale);
                        case ValueType::Integer: return std::to_string(value_or_address);
                        default: return std::to_string(value_or_address);
                    }
                case 1:
                case 2:
                case 3:
                case 13:
                case 15:
                    {
                        const auto address = static_cast<uint32>(value_or_address) & 0xFF;
                        const auto friendly_name = formatter.GetFriendlyVarName(
                          bank, value_or_address
                        );
                        if (friendly_name.empty())
                            return (boost::format("FFVII.Banks[%1%][%2%]") % bank % address).str();
                        return (boost::format("FFVII.Data.%1%") % friendly_name).str();
                    }
                case 5:
                case 6:
                    {
                        const auto address = static_cast<uint32>(value_or_address)& 0xFF;
                        const  auto friendly_name = formatter.GetFriendlyVarName(bank, address);
                        if (friendly_name.empty())
                            return (boost::format("FFVII.Banks[%1%][%2%]") % bank % address).str();
                        return "FFVII.Data." + friendly_name;
                    }
                default:
                    return (
                      boost::format(
                        "FFVII.Banks[%1%][%2%]"
                      ) % bank % (static_cast<uint32>(value_or_address) & 0xFF)
                    ).str();
            }
        }

        /**
         * Constructor.
         *
         * @param engine[in] The engine.
         * @param insts[in] The list of instructions to parse.
         * @param output[out] The generated script.
         * @param formatter[in] The code formatter.
         */
        FieldCodeGenerator(
          Engine *engine, const InstVec& insts, std::ostream &output,
          FieldScriptFormatter& formatter
        ) :
          CodeGenerator(engine, output, FIFO_ARGUMENT_ORDER, LIFO_ARGUMENT_ORDER),
          insts_(insts), formatter_(formatter)
        {target_lang_ = std::make_unique<LuaLanguage>();}

        /**
         * Generates the script from the instructions.
         *
         * @param insts[in] The list of instructions.
         * @param graph[in] Code graph, unused.
         */
        virtual void Generate(InstVec& insts, const Graph &graph) override;

        /**
         * Adds a line to the script.
         *
         * @param line[in] The line to add.
         * @param unindent_before[in] If true, the current script indentation
         * will be moved back one position before the line.
         * @param indent_after[in] If true, the current script indentation
         * will be moved forward one position after the line.
         */
        virtual void AddOutputLine(
          std::string line, bool unindent_before = false, bool indent_after = false
        ) override;

        /**
         * Retrieves the engine scale factor.
         *
         * @return The scale factor
         */
        float GetScaleFactor() const;

        /**
         * Retrieves the formatter.
         */
        FieldScriptFormatter& GetFormatter();



    protected:

        /**
         * Constructs the function signature.
         *
         * The function signature is the LUA function declaration, and it
         * looks like this:
         *
         * <function_name> = function(self)
         *
         * @param function[in] The function to construct the signature for.
         * @return The function signature.
         */
        virtual std::string ConstructFuncSignature(const Function &function) override;

        /**
         * Finalizes a function.
         *
         * It appends an "end" to finalize the function. If the function is
         * the last of a class, it also closes the class braces.
         *
         * @param function[in] The function to end.
         */
        virtual void OnEndFunction(const Function& function) override;

        /**
         * Adds lines to the script before a function.
         *
         * If the function is the first in a class, it initializes the class
         * and, if the function belongs to a character, creates the class
         * variable for the character. If the function has a comment
         * associated, it will also be added before the function.
         *
         * @param function[in] The function about to start.
         */
        virtual void OnBeforeStartFunction(const Function& function) override;

        /**
         * Initializes a function.
         *
         * It adds a comment with the function opcodes.
         *
         * @param function[in] The function to start.
         */
        virtual void OnStartFunction(const Function& function) override;

        /**
         * Checks if only required labels are to be written.
         *
         * @return Always true.
         */
        virtual bool OutputOnlyRequiredLabels() const override;

    private:

        /**
         * The list of instructions in the function to generate.
         */
        const InstVec& insts_;

        /**
         * Generated lines.
         */
        std::vector<CodeLine> lines_;

        /**
         * The formatter.
         */
        FieldScriptFormatter& formatter_;

};
