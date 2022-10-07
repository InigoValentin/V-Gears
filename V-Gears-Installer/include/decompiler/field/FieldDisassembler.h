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

#include "decompiler/Disassembler.h"
#include <array>
#include "FieldScriptFormatter.h"

class BinaryReader;
class Function;

/**
 * Field opcodes.
 */
enum OPCODES{
    RET = 0x0,
    REQ = 0x01,
    REQSW = 0x02,
    REQEW = 0x03,
    PREQ = 0x04,
    PRQSW = 0x05,
    PRQEW = 0x06,
    RETTO = 0x07,
    JOIN = 0x08,
    SPLIT = 0x09,
    SPTYE = 0x0A,
    GTPYE = 0x0B,
    DSKCG = 0x0E,
    SPECIAL = 0x0F,
    JMPF = 0x10,
    JMPFL = 0x11,
    JMPB = 0x12,
    JMPBL = 0x13,
    IFUB = 0x14,
    IFUBL = 0x15,
    IFSW = 0x16,
    IFSWL = 0x17,
    IFUW = 0x18,
    IFUWL = 0x19,
    MINIGAME = 0x20,
    TUTOR = 0x21,
    BTMD2 = 0x22,
    BTRLD = 0x23,
    WAIT = 0x24,
    NFADE = 0x25,
    BLINK = 0x26,
    BGMOVIE = 0x27,
    KAWAI = 0x28,
    KAWIW = 0x29,
    PMOVA = 0x2A,
    SLIP = 0x2B,
    BGPDH = 0x2C,
    BGSCR = 0x2D,
    WCLS = 0x2E,
    WSIZW = 0x2F,
    IFKEY = 0x30,
    IFKEYON = 0x31,
    IFKEYOFF = 0x32,
    UC = 0x33,
    PDIRA = 0x34,
    PTURA = 0x35,
    WSPCL = 0x36,
    WNUMB = 0x37,
    STTIM = 0x38,
    GOLDU = 0x39,
    GOLDD = 0x3A,
    CHGLD = 0x3B,
    HMPMAX1 = 0x3C,
    HMPMAX2 = 0x3D,
    MHMMX = 0x3E,
    HMPMAX3 = 0x3F,
    MESSAGE = 0x40,
    MPARA = 0x41,
    MPRA2 = 0x42,
    MPNAM = 0x43,
    MPU = 0x45,
    MPD = 0x47,
    ASK = 0x48,
    MENU = 0x49,
    MENU2 = 0x4A,
    BTLTB = 0x4B,
    HPU = 0x4D,
    HPD = 0x4F,
    WINDOW = 0x50,
    WMOVE = 0x51,
    WMODE = 0x52,
    WREST = 0x53,
    WCLSE = 0x54,
    WROW = 0x55,
    GWCOL = 0x56,
    SWCOL = 0x57,
    STITM = 0x58,
    DLITM = 0x59,
    CKITM = 0x5A,
    SMTRA = 0x5B,
    DMTRA = 0x5C,
    CMTRA = 0x5D,
    SHAKE = 0x5E,
    NOP = 0x5F,
    MAPJUMP = 0x60,
    SCRLO = 0x61,
    SCRLC = 0x62,
    SCRLA = 0x63,
    SCR2D = 0x64,
    SCRCC = 0x65,
    SCR2DC = 0x66,
    SCRLW = 0x67,
    SCR2DL = 0x68,
    MPDSP = 0x69,
    VWOFT = 0x6A,
    FADE = 0x6B,
    FADEW = 0x6C,
    IDLCK = 0x6D,
    LSTMP = 0x6E,
    SCRLP = 0x6F,
    BATTLE = 0x70,
    BTLON = 0x71,
    BTLMD = 0x72,
    PGTDR = 0x73,
    GETPC = 0x74,
    PXYZI = 0x75,
    PLUS_ = 0x76, // PLUS!
    PLUS2_ = 0x77, // PLUS2!
    MINUS_ = 0x78, // MINUS!
    MINUS2_ = 0x79, // MINUS2!
    INC_ = 0x7A, //  INC!
    INC2_ = 0x7B, // INC2!
    DEC_ = 0x7C, // DEC!
    DEC2_ = 0x7D, // DEC2!
    TLKON = 0x7E,
    RDMSD = 0x7F,
    SETBYTE = 0x80,
    SETWORD = 0x81,
    BITON = 0x82,
    BITOFF = 0x83,
    BITXOR = 0x84,
    PLUS = 0x85,
    PLUS2 = 0x86,
    MINUS = 0x87,
    MINUS2 = 0x88,
    MUL = 0x89,
    MUL2 = 0x8A,
    DIV = 0x8B,
    DIV2 = 0x8C,
    MOD = 0x8D,
    MOD2 = 0x8E,
    AND = 0x8F,
    AND2 = 0x90,
    OR = 0x91,
    OR2 = 0x92,
    XOR = 0x93,
    XOR2 = 0x94,
    INC = 0x95,
    INC2 = 0x96,
    DEC = 0x97,
    DEC2 = 0x98,
    RANDOM = 0x99,
    LBYTE = 0x9A,
    HBYTE = 0x9B,
    TWOBYTE = 0x9C,
    SETX = 0x9D,
    GETX = 0x9E,
    SEARCHX = 0x9F,
    PC = 0xA0,
    opCodeCHAR = 0xA1,
    DFANM = 0xA2,
    ANIME1 = 0xA3,
    VISI = 0xA4,
    XYZI = 0xA5,
    XYI = 0xA6,
    XYZ = 0xA7,
    MOVE = 0xA8,
    CMOVE = 0xA9,
    MOVA = 0xAA,
    TURA = 0xAB,
    ANIMW = 0xAC,
    FMOVE = 0xAD,
    ANIME2 = 0xAE,
    ANIM_1 = 0xAF, // ANIM!1
    CANIM1 = 0xB0,
    CANM_1 = 0xB1, // CANM!1
    MSPED = 0xB2,
    DIR = 0xB3,
    TURNGEN = 0xB4,
    TURN = 0xB5,
    DIRA = 0xB6,
    GETDIR = 0xB7,
    GETAXY = 0xB8,
    GETAI = 0xB9,
    ANIM_2 = 0xBA, // ANIM!2
    CANIM2 = 0xBB,
    CANM_2 = 0xBC, // CANM!2
    ASPED = 0xBD,
    CC = 0xBF,
    JUMP = 0xC0,
    AXYZI = 0xC1,
    LADER = 0xC2,
    OFST = 0xC3,
    OFSTW = 0xC4,
    TALKR = 0xC5,
    SLIDR = 0xC6,
    SOLID = 0xC7,
    PRTYP = 0xC8,
    PRTYM = 0xC9,
    PRTYE = 0xCA,
    IFPRTYQ = 0xCB,
    IFMEMBQ = 0xCC,
    MMBUD = 0xCD,
    MMBLK = 0xCE,
    MMBUK = 0xCF,
    LINE = 0xD0,
    LINON = 0xD1,
    MPJPO = 0xD2,
    SLINE = 0xD3,
    SIN = 0xD4,
    COS = 0xD5,
    TLKR2 = 0xD6,
    SLDR2 = 0xD7,
    PMJMP = 0xD8,
    PMJMP2 = 0xD9,
    AKAO2 = 0xDA,
    FCFIX = 0xDB,
    CCANM = 0xDC,
    ANIMB = 0xDD,
    TURNW = 0xDE,
    MPPAL = 0xDF,
    BGON = 0xE0,
    BGOFF = 0xE1,
    BGROL = 0xE2,
    BGROL2 = 0xE3,
    BGCLR = 0xE4,
    STPAL = 0xE5,
    LDPAL = 0xE6,
    CPPAL = 0xE7,
    RTPAL = 0xE8,
    ADPAL = 0xE9,
    MPPAL2 = 0xEA,
    STPLS = 0xEB,
    LDPLS = 0xEC,
    CPPAL2 = 0xED,
    RTPAL2 = 0xEE,
    ADPAL2 = 0xEF,
    MUSIC = 0xF0,
    SOUND = 0xF1,
    AKAO = 0xF2,
    MUSVT = 0xF3,
    MUSVM = 0xF4,
    MULCK = 0xF5,
    BMUSC = 0xF6,
    CHMPH = 0xF7,
    PMVIE = 0xF8,
    MOVIE = 0xF9,
    MVIEF = 0xFA,
    MVCAM = 0xFB,
    FMUSC = 0xFC,
    CMUSC = 0xFD,
    CHMST = 0xFE,
    GAMEOVER = 0xFF
};

/**
 * SPECIAL opcodes.
 *
 * SPECIAL opcode is a special one. It's a multi-purpose opcode, used to
 * perform game-specific tasks. It has a sub-opcode, in the next address,
 * that determines the operation to perform. This is a list of those
 * sub-opcodes.
 */
enum OPCODES_SPECIAL{
    ARROW = 0xF5,
    PNAME = 0xF6,
    GMSPD = 0xF7,
    SMSPD = 0xF8,
    FLMAT = 0xF9,
    FLITM = 0xFA,
    BTLCK = 0xFB,
    MVLCK = 0xFC,
    SPCNM = 0xFD,
    RSGLB = 0xFE,
    CLITM = 0xFF
};

/**
 * KAWAI opcodes.
 *
 * KAWAI opcode is a special one. It's a multi-purpose opcode, used to
 * perform graphics related tasks. It has a sub-opcode, in the next
 * address, that determines the operation to perform. This is a list of
 * those sub-opcodes.
 */
enum OPCODES_KAWAI{
    EYETX = 0x00,
    TRNSP = 0x01,
    AMBNT = 0x02,
    Unknown03 = 0x03, // Makou Reactor can write these unknowns.
    Unknown04 = 0x04,
    Unknown05 = 0x05,
    LIGHT = 0x06,
    Unknown07 = 0x07,
    Unknown08 = 0x08,
    Unknown09 = 0x09,
    SBOBJ = 0x0A,
    Unknown0B = 0x0B,
    Unknown0C = 0x0C,
    SHINE = 0x0D,
    RESET = 0xFF
};

class FieldEngine;

/**
 * A disassebler for field maps.
 */
class FieldDisassembler : public Disassembler{

    public:

        /**
         * Information about an instruction.
         */
        struct InstructionRecord{

            /**
             * The size, in bytes, of the opcode.
             */
            unsigned char opcode_size;

            /**
             * The opcode code.
             */
            unsigned int opcode;

            /**
             * The opcode short name (mnemonic).
             */
            const char* opcode_name;

            /**
             * Format of the opcode arguments.
             */
            const char* argument_format;

            /**
             * @todo Document.
             */
            std::function<InstPtr()> factory_function;
        };

        /**
         * A list of flow control opcodes needed by the disassembler.
         */
        static InstructionRecord FLOW_OPCODES[];

        /**
         * Generates a map of control flow instructions.
         *
         * @return A map of control flow instructions, by opcode name.
         */
        static std::map<std::string, const InstructionRecord*> FieldInstructions();

        /**
         * Finds the ID of the character a function refers to.
         *
         * @param[in] start_addr The starting address of the function.
         * @param[in] end_addr The ending address of the instruction.
         * @param[in] insts The list of instructions to search.
         * @return The ID of the character the function refers to, or -1 if
         * the function doesn't belong to a character.
         */
        static int FindId(uint32 start_addr, uint32 end_addr, const InstVec& insts);

        /**
         * Constructor.
         *
         * @param[in] formatter The code formatter.
         * @param[in] engine The engine to use to disassemble.
         * @param[in] insts The list of instructions.
         */
        FieldDisassembler(FieldScriptFormatter& formatter, FieldEngine* engine, InstVec& insts);

        /**
         * Constructor.
         *
         * @param[in] formatter The code formatter.
         * @param[in] engine The engine to use to disassemble.
         * @param[in] insts The list of instructions.
         * @param[in] raw_script_data @todo Understand and document.
         */
        FieldDisassembler(
          FieldScriptFormatter& formatter, FieldEngine* engine,
          InstVec& insts, const std::vector<unsigned char>& raw_script_data
        );

        /**
         * Destructor.
         */
        ~FieldDisassembler();

        /**
         * Retrieves the scale factor for the field.
         */
        float GetScaleFactor() const;


        /**
         * Disassembles the instructions.
         */
        virtual void DoDisassemble() override;

    private:

        /**
         * Field file identification magic number.
         */
        static const int MAGIC;

        /**
         * The number of sections in a field.
         */
        static const int NUM_SECTIONS;

        /**
         * The sections of a field.
         */
        enum SECTIONS{

            /**
             * The scripts section.
             */
            SCRIPT = 0,

            /**
             * The walkmesh section.
             */
            WALKMESH = 1,

            /**
             * The tiles section.
             */
            TILE_MAP = 2,

            /**
             * The camera section.
             */
            CAMERA_MATRIX = 3,

            /**
             * The triggers section.
             */
            TRIGGERS = 4,

            /**
             * Encounter data section.
             */
            ENCOUNTERS = 5,

            /**
             * Field models section.
             */
            MODELS = 6
        };

        /**
         * Info about a script.
         */
        struct ScriptInfo{

            /**
             * Address of the script entry point.
             */
            uint16 entry_point;

            /**
             * Address of the next script entry point.
             *
             * I.e. The last address of the script, plus one.
             */
            uint32 next_entry_point;

            /**
             * Script index.
             */
            size_t index;
        };

        /**
         * A script header.
         */
        struct ScriptHeader{

            /**
             * Field file identification magic number.
             */
            uint16 magic;

            /**
             * The number of entities in the field.
             */
            uint8 number_of_entities;

            /**
             * The number of models in the field.
             */
            uint8 number_of_models;

            /**
             * Offset at which the field text strings begin.
             */
            uint16 offset_to_strings;

            /**
             * The number of Akao/tuto blocks/offsets.
             */
            uint16 number_of_akao_offsets;

            /**
             * Scale of field.
             *
             * For move and talk calculation (9bit fixed point).
             */
            uint16 scale;

            /**
             * Blank data, unused.
             */
            std::array<uint16, 3> blank;

            /**
             * Field creator name.
             */
            std::array<char, 8> creator;

            /**
             * The field name.
             */
            std::array<char, 8> name;

            /**
             * The list of entity names.
             *
             * There will be {@see number_of_entities} entities.
             */
            std::vector<std::array<char, 8>> field_entity_names;

            /**
             * Akao/Tuto block offsets.
             *
             * There will be {@see number_of_akao_offsets} offsets.
             */
            std::vector<uint32> akao_offsets;

            /**
             * Entity script entry points.
             *
             * More explicitly, subroutine offsets. There will be
             * {@see number_of_entities} entity scripts.
             */
            std::vector<std::array<uint16, 32>> entity_scripts;

            /**
             * Reads the header.
             *
             * @param[in] reader The reader used to read the header.
             * @throws ScriptHeaderInvalidException If the magic number for
             * the field is wrong.
             */
            void Read(BinaryReader& reader){
                magic = reader.ReadU16();
                if (magic != MAGIC) throw ScriptHeaderInvalidException();
                number_of_entities = reader.ReadU8();
                number_of_models = reader.ReadU8();
                offset_to_strings = reader.ReadU16();
                number_of_akao_offsets = reader.ReadU16();
                scale = reader.ReadU16();
                for (int i = 0; i < 3; i ++) blank[i] = reader.ReadU16();
                for (int i = 0; i < 8; i ++) creator[i] = reader.ReadU8();
                for (int i = 0; i < 8; i ++) name[i] = reader.ReadU8();
                for (int i = 0; i < number_of_entities; i ++){
                    std::array<char, 8> name;
                    for (int j = 0; j < 8; j ++) name[j] = reader.ReadU8();
                    field_entity_names.emplace_back(name);
                }
                for (int i = 0; i < number_of_akao_offsets; i ++)
                    akao_offsets.emplace_back(reader.ReadU32());
                entity_scripts.reserve(number_of_entities);
                for (int i = 0; i < number_of_entities; i ++){
                    std::array<uint16, 32> scripts;
                    for (int j = 0; j < 32; j ++) scripts[j] = reader.ReadU16();
                    entity_scripts.push_back(scripts);
                }
            }
        };

        /**
         * Open a file for disassembly.
         *
         * @param[in] filename The file to disassemble.
         * @deprecated Do not use. Load the instructions from the constructor.
         */
        virtual void Open(const char *filename) override;

        /**
         * Disassembles a script (function).
         *
         * @param[in] entity_name Name of the entity the script belongs to.
         * @param[in] entity_index Index of the entity the script belongs to.
         * @param[in] script_index Index of the script, relative to the entity
         * scripts.
         * @param[in] script_entry_point Address of the first instruction of
         * the script to disassemble.
         * @param[in] next_script_entry_point Address of the first instruction
         * of the script next to the one to disassemble (i.e. the last address
         * of the script to disassemble, plus one).
         * @param[in] is_start Indicates if the script is the first one for
         * it's entity.
         * @param[in] is_end Indicates if the script is the last one for it's
         * entity.
         * @throws DecompilerException for malformed scripts.
         */
        void DisassembleIndivdualScript(
          std::string entity_name, size_t entity_index, size_t script_index,
          size_t script_entry_point, uint32 next_script_entry_point, bool is_start, bool is_end
        );

        /**
         * Adds a function to the engine.
         *
         * @param[in] entity_name Name of the entity that owns the function.
         * @param[in] entity_index Index of the entity that owns the function.
         * @param[in] script_index Index of the script within the entity.
         * @param[in] next_script_entry_point Start position of the next
         * script (or the ending position of this one, plus one).
         * @param[in] is_start @todo Understand and document.
         * @param[in] is_end @todo Understand and document.
         * @param[in] to_return_only True to read the script only until the
         * first return.
         * @param[in] func_name Name of the function. If the entity is a line,
         * the provided name will be overridden and set to the standard line
         * entity script names.
         */
        void AddFunc(
          std::string entity_name, size_t entity_index, size_t script_index,
          uint32 next_script_entry_point, const bool is_start, const bool is_end,
          bool to_return_only, std::string func_name
        );

        /**
         * Reads opcodes from a script, and detects lines.
         *
         * It stops when it reaches the ending position or a RET opcode,
         * whatever comes first. It also detects if one of the read opcodes
         * LINE, which means that the entity is a line
         *
         * @param[in] end_pos The last position to read. Opcodes will be read
         * until this, or until a RET is found.
         * @param[out] point_a If a LINE opcode is found, the first point will
         * be saved here.
         * @param[out] point_b If a LINE opcode is found, the second point
         * will be saved here.
         * @return True if LINE opcode found, false if not.
         */
        bool ReadOpCodesToPositionOrReturn(
          size_t end_pos, std::vector<float>& point_a, std::vector<float>& point_b
        );

        /**
         * Initializes a function for an entity.
         *
         * @param[in] script_index Index of the script.
         */
        std::unique_ptr<Function> StartFunction(size_t script_index);

        /**
         * Reads the header.
         */
        void ReadHeader();

        /**
         * Reads the header.
         *
         * @param[in] reader Reader to use to read.
         */
        void ReadHeader(BinaryReader& reader);

        /**
         * Retrieves the offset of the end of a script.
         *
         * @param[in] cur_entry_point @todo Understand and document.
         * @param[in] entity_index Index of the entity the script belongs to.
         * @param[in] script_index The script index.
         * @return The address of the end of the script.
         */
        uint32 GetEndOfScriptOffset(
          uint16 cur_entry_point, size_t entity_index, size_t script_index
        );

        /**
         * Parses an opcode.
         *
         * Adds properties to an instruction, and adds the instruction to the
         * list.
         *
         * @param[in] opcode The opcode code.
         * @param[in] name the opcode name.
         * @param[in] instruction A newly created instruction of the type the
         * opcode belongs to.
         * @param[in] stack_change Indicates how much the instruction changes
         * the stack pointer by.
         * @param[in] argument_format The opcode argument format.
         */
        template<typename T> void ParseOpcode(
          int opcode, std::string name, T instruction, int stack_change, const char* argument_format
        ){
            uint32 full_opcode = (full_opcode << 8) + opcode;
            this->insts_.push_back(instruction);
            this->insts_.back()->SetOpcode(full_opcode);
            this->insts_.back()->SetAddress(this->address_);
            this->insts_.back()->SetStackChange(0);
            this->insts_.back()->SetName(std::string(name));
            this->insts_.back()->SetCodeGenData("");
            this->ReadParams(this->insts_.back(), argument_format);
        }

        /**
         * The engine.
         */
        FieldEngine* engine_;

        /**
         * Last address of the header.
         */
        uint32 header_end_position_ = 0;

        /**
         * The list of sections.
         */
        std::array<uint32, 7> sections_;

        /**
         * Size of the pointers to sections.
         *
         * @todo Understand and document.
         */
        int16 section_pointers_size_;

        /**
         * The field header.
         */
        ScriptHeader header_;

        /**
         * Indicates if the field has been loaded from raw data.
         */
        bool loaded_from_raw_data_ = false;

        /**
         * The formatter.
         */
        FieldScriptFormatter& formatter_;

        /**
         * The scale factor of the field.
         */
        float scale_factor_ = 1.0f;
};
