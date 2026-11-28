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
#include <string>
#include <vector>
#include "installer/extractors/KernelDataTypes.h"

/*
 * Reading and writing of the data that several kinds of kernel records have.
 */
namespace KernelData{

    /**
     * Reads who can be targeted, as in the records of attacks, items and weapons.
     *
     * @param[in] raw The target byte.
     * @param[in,out] target The target.
     */
    void ReadTarget(const u8 raw, Target& target);

    /**
     * Reads the elements of a record. A record with all of them has none.
     *
     * @param[in] raw A bit for each element.
     * @param[in,out] elements The elements are added here.
     */
    void ReadElements(const u16 raw, std::vector<int>& elements);

    /**
     * Reads the statuses of a record.
     *
     * @param[in] raw A bit for each status.
     * @param[in,out] statuses The statuses are added here.
     */
    void ReadStatuses(const u32 raw, std::vector<int>& statuses);

    /**
     * Reads the statuses that an attack or an item changes, how, and with what chance.
     *
     * @param[in] change The status change byte.
     * @param[in] raw A bit for each status.
     * @param[in,out] status The status effect.
     */
    void ReadStatusEffect(const u8 change, const u32 raw, StatusEffect& status);

    /**
     * Reads what an attack or an item restores.
     *
     * @param[in] condition The condition byte.
     * @return What it restores.
     */
    RESTORE_TYPE ReadRestoreType(const u8 condition);

    /**
     * Reads the characters that can equip a record.
     *
     * @param[in] raw A bit for each character.
     * @param[in,out] users The IDs of the characters are added here.
     */
    void ReadUsers(const u16 raw, std::vector<u8>& users);

    /**
     * Reads the stat bonuses of a piece of equipment.
     *
     * @param[in] stats The stats, 0xFF for none.
     * @param[in] amounts The bonus of each stat.
     * @param[in] count Number of bonuses.
     * @param[in,out] bonuses The bonuses are added here.
     */
    void ReadStatBonuses(
      const u8* stats, const u8* amounts, const int count, std::vector<StatBonus>& bonuses
    );

    /**
     * Reads the materia slots of a weapon or an armor.
     *
     * @param[in] raw The eight slot bytes.
     * @param[in,out] slots The slots are added here.
     */
    void ReadMateriaSlots(const u8 (&raw)[8], std::vector<u8>& slots);

    /**
     * Reads where an item can be used and if it can be sold or thrown.
     *
     * @param[in,out] data The record. Its restriction bits have to be set.
     */
    template<typename T> void ReadRestrictions(T& data){
        data.sellable = !(data.restrict_raw & 1);
        data.useable_battle = !(data.restrict_raw & 2);
        data.useable_menu = !(data.restrict_raw & 4);
        data.throwable = !(data.restrict_raw & 8);
    }

    /**
     * Writes a target as the field of a Lua table.
     *
     * @param[in] file The file to write to.
     * @param[in] target The target.
     */
    void WriteTarget(std::ostream& file, const Target& target);

    /**
     * Writes the stat bonuses of a piece of equipment as the field of a Lua table.
     *
     * @param[in] file The file to write to.
     * @param[in] bonuses The bonuses.
     */
    void WriteStatBonuses(std::ostream& file, const std::vector<StatBonus>& bonuses);

    /**
     * Retrieves the name of a stat as used in the Lua scripts.
     *
     * @param[in] id The stat.
     * @return The name of the stat.
     */
    std::string GetBasicStatName(const int id);
}
