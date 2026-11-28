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

#include "installer/formats/BinGZipFile.h"
#include "common/TypeDefine.h"
#include "installer/formats/DiskImage.h"
#include "installer/tables/Characters.h"
#include "installer/formats/SaveMap.h"
#include "installer/extractors/KernelAttackData.h"
#include "installer/extractors/KernelItemData.h"
#include "installer/extractors/KernelEquipmentData.h"
#include "installer/extractors/KernelMateriaData.h"
#include "installer/extractors/KernelCharacterData.h"

class KernelDataInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image Disk image to extract the data from.
         */
        KernelDataInstaller(DiskImage& disk_image);

        /**
         * Destructor.
         */
        ~KernelDataInstaller();

        /**
         * Reads items and materia prices from the executable.
         */
        void ReadPrices();

        /**
         * Writes the inventories of the shops, read from the executable.
         *
         * @param file_name Path of the Lua file to write.
         */
        void WriteShops(std::string file_name);

        /**
         * Reads the command data from the kernel.
         *
         * @return The number of commands read.
         */
        int ReadCommands();

        /**
         * Saves command data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteCommands(std::string file_name);

        /**
         * Reads the attack data from the kernel.
         *
         * @return The number of attacks read.
         */
        int ReadAttacks();

        /**
         * Saves command data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteAttacks(std::string file_name);

        /**
         * Reads character data from the kernel.
         *
         * @return The number of characters read.
         */
        int ReadCharacters();

        /**
         * Saves character data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteCharacters(std::string file_name);

        /**
         * Reads growth data from the kernel.
         */
        void ReadGrowth();

        /**
         * Saves growth data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteGrowth(std::string file_name);

        /**
         * Reads the items from the kernel.
         *
         * @return The number of items read.
         */
        int ReadItems();

        /**
         * Saves item data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteItems(std::string file_name);

        /**
         * Reads the weapon info from the kernel.
         *
         * @return The number of weapons read.
         */
        int ReadWeapons();

        /**
         * Saves weapon data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteWeapons(std::string file_name);

        /**
         * Reads the armor info from the kernel.
         *
         * @return The number of armor read.
         */
        int ReadArmors();

        /**
         * Saves armor data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteArmors(std::string file_name);

        /**
         * Reads the accessory info from the kernel.
         *
         * @return The number of accessories read.
         */
        int ReadAccessories();

        /**
         * Saves accessory data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteAccessories(std::string file_name);

        /**
         * Reads materia info from the kernel.
         *
         * @return The number of accessories read.
         */
        int ReadMateria();

        /**
         * Saves materia data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteMateria(std::string file_name);

        /**
         * Reads key item info from the kernel.
         *
         * @return The number of key items read.
         */
        int ReadKeyItems();

        /**
         * Saves key item data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteKeyItems(std::string file_name);

        /**
         * Reads summon name info from the kernel.
         *
         * @return The number of summons read.
         */
        int ReadSummonNames();

        /**
         * Saves summon names data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteSummonNames(std::string file_name);

        /**
         * Reads the initial savemap from the kernel.
         */
        void ReadInitialSaveMap();

        /**
         * Reads from ff7.exe the character data that Cait Sith and Vincent get when they join.
         *
         * Their slots of the initial savemap hold young Cloud and Sephiroth.
         *
         * @param[in] exe_path Path to ff7.exe.
         */
        void ReadRecruitRecords(const std::string& exe_path);

        /**
         * Saves the initial savemap to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteInitialSaveMap(std::string file_name);

    private:

        enum KERNEL_SECTIONS{

            /**
             * Command data section in KERNEL.BIN.
             */
            KERNEL_COMMAND_DATA = 0,

            /**
             * Attack data section in KERNEL.BIN.
             */
            KERNEL_ATTACK_DATA,

            /**
             * Battle and growth data section in KERNEL.BIN.
             */
            KERNEL_BATTLE_AND_GROWTH_DATA,

            /**
             * Initialization data section in KERNEL.BIN.
             */
            KERNEL_INITIALIZATION_DATA,

            /**
             * Item data section in KERNEL.BIN.
             */
            KERNEL_ITEM_DATA,

            /**
             * Weapon data section in KERNEL.BIN.
             */
            KERNEL_WEAPON_DATA,

            /**
             * Armor data section in KERNEL.BIN.
             */
            KERNEL_ARMOR_DATA,

            /**
             * Accessory data section in KERNEL.BIN.
             */
            KERNEL_ACCESSORY_DATA,

            /**
             * Materia data section in KERNEL.BIN.
             */
            KERNEL_MATERIA_DATA,

            /**
             * Command descriptions section in KERNEL.BIN.
             */
            KERNEL_COMMAND_DESCRIPTIONS,

            /**
             * Attack descriptions section in KERNEL.BIN.
             */
            KERNEL_ATTACK_DESCRIPTIONS,

            /**
             * Item descriptions section in KERNEL.BIN.
             */
            KERNEL_ITEM_DESCRIPTIONS,

            /**
             * Weapon descriptions section in KERNEL.BIN.
             */
            KERNEL_WEAPON_DESCRIPTIONS,

            /**
             * Armor descriptions section in KERNEL.BIN.
             */
            KERNEL_ARMOR_DESCRIPTIONS,

            /**
             * Accessory descriptions section in KERNEL.BIN.
             */
            KERNEL_ACCESSORY_DESCRIPTIONS,

            /**
             * Materia descriptions section in KERNEL.BIN.
             */
            KERNEL_MATERIA_DESCRIPTIONS,

            /**
             * Key Item descriptions section in KERNEL.BIN.
             */
            KERNEL_KEY_ITEM_DESCRIPTIONS,

            /**
             * Command Names section in KERNEL.BIN.
             */
            KERNEL_COMMAND_NAMES,

            /**
             * Atack Names section in KERNEL.BIN.
             */
            KERNEL_ATTACK_NAMES,

            /**
             * Item Names section in KERNEL.BIN.
             */
            KERNEL_ITEM_NAMES,

            /**
             * Weapon Names section in KERNEL.BIN.
             */
            KERNEL_WEAPON_NAMES,

            /**
             * Armor Names section in KERNEL.BIN.
             */
            KERNEL_ARMOR_NAMES,

            /**
             * Accessory Names section in KERNEL.BIN.
             */
            KERNEL_ACCESSORY_NAMES,

            /**
             * Materia Names section in KERNEL.BIN.
             */
            KERNEL_MATERIA_NAMES,

            /**
             * Key Item Names section in KERNEL.BIN.
             */
            KERNEL_KEY_ITEM_NAMES,

            /**
             * Battle and Battle-Screen Text section in KERNEL.BIN.
             */
            KERNEL_BATTLE_AND_BATTLE_SCREEN_TEXT,

            /**
             * Summon Attack Names section in KERNEL.BIN.
             */
            KERNEL_SUMMON_ATTACK_NAMES

        };

        /**
         * Items read from the kernel.
         */
        std::vector<KernelData::CommandData> commands_;

        /**
         * Attacks read from the kernel.
         */
        std::vector<KernelData::AttackData> attacks_;
        /** Limit techniques (attack IDs 128 and up). Only name and description are filled. */
        std::vector<KernelData::AttackData> limit_attacks_;

        /**
         * Character info read from the kernel.
         */
        std::vector<KernelData::CharacterData> characters_;

        KernelData::GrowthData growth_;

        /**
         * Items read from the kernel.
         */
        std::vector<KernelData::ItemData> items_;

        /**
         * Weapons read from the kernel.
         */
        std::vector<KernelData::WeaponData> weapons_;

        /**
         * Armors read from the kernel.
         */
        std::vector<KernelData::ArmorData> armors_;

        /**
         * Accessories read from the kernel.
         */
        std::vector<KernelData::AccessoryData> accessories_;

        /**
         * Materia read from the kernel.
         */
        std::vector<KernelData::MateriaData> materia_;

        /**
         * Key items read from the kernel.
         */
        std::vector<KernelData::KeyItemData> key_items_;

        /**
         * Summon names read from the kernel.
         */
        std::vector<KernelData::SummonNameData> summon_names_;

        /**
         * The initial savemap structure.
         */
        SaveMap savemap_;

        /**
         * Sets the colour of a materia and the type it belongs to.
         *
         * @param[in,out] data The materia. Its raw type has to be set.
         */
        static void SetMateriaType(KernelData::MateriaData& data);

        /**
         * Sets the stats that a materia changes.
         *
         * @param[in,out] data The materia. Its raw stat byte has to be set.
         */
        static void SetMateriaStats(KernelData::MateriaData& data);

        /**
         * Writes the attacks that a magic materia teaches.
         *
         * @param[in] file The file to write to.
         * @param[in] materia The materia.
         */
        void WriteMagicMateria(std::ostream& file, const KernelData::MateriaData& materia);

        /**
         * Writes the summon of a summon materia.
         *
         * @param[in] file The file to write to.
         * @param[in] materia The materia.
         */
        void WriteSummonMateria(std::ostream& file, const KernelData::MateriaData& materia);

        /**
         * Writes the commands of a command materia.
         *
         * @param[in] file The file to write to.
         * @param[in] materia The materia.
         */
        void WriteCommandMateria(std::ostream& file, const KernelData::MateriaData& materia);

        /**
         * Writes the list of the commands that a materia gives.
         *
         * @param[in] file The file to write to.
         * @param[in] materia The materia.
         */
        void WriteMateriaCommandList(std::ostream& file, const KernelData::MateriaData& materia);

        /**
         * Writes the effect of a support materia on the materia linked to it.
         *
         * @param[in] file The file to write to.
         * @param[in] materia The materia.
         */
        static void WriteSupportMateria(std::ostream& file, const KernelData::MateriaData& materia);

        /**
         * Writes the ability of an independent materia.
         *
         * @param[in] file The file to write to.
         * @param[in] materia The materia.
         */
        static void WriteIndependentMateria(std::ostream& file, const KernelData::MateriaData& materia);

        /**
         * Reads one character record of the savemap.
         *
         * @param[in,out] savemap_file Data, at the start of the record.
         * @param[out] character The character read.
         */
        void ReadSaveMapCharacter(File& savemap_file, SaveMap::Character& character);

        /**
         * Writes one character record as a Lua table.
         *
         * @param[in,out] file Target file.
         * @param[in] character The character to write.
         */
        void WriteSaveMapCharacter(std::ostream& file, const SaveMap::Character& character);

        /**
         * Character data of Cait Sith and Vincent when they join.
         */
        SaveMap::Character recruit_records_[2];

        /**
         * True if the recruit records could be read.
         */
        bool recruit_records_found_ = false;

        /**
         * The kernel file.
         */
        BinGZipFile kernel_;

        /**
         * Path to ff7.exe file.
         */
        std::string exe_path_;

        /**
         * Item and materia shop prices.
         *
         * Indexes 0 - 127: Items.
         * Indexes 128 - 255: Weapons.
         * Indexes 256 - 287: Armors.
         * Indexes 288 - 319: Accessories
         * Indxes 320 - 415: Materia.
         */
        u32 prices_[416];

        /**
         * Attack names, required for other sections.
         */
        std::string attack_names_[128];

        /**
         * Command names, required for other sections.
         */
        std::string command_names_[128];

};
