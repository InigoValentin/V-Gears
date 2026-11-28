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

#include "installer/extractors/KernelDataParts.h"

namespace KernelData{

    void ReadTarget(const u8 raw, Target& target){
        target.selection_enabled = raw & 0x01;
        target.default_enemy = raw & 0x02;
        target.default_multiple = raw & 0x04;
        target.toggle_multiple = raw & 0x08;
        target.fixed_row = raw & 0x10;
        target.short_range = raw & 0x20;
        target.all_rows = raw & 0x40;
        target.random = raw & 0x80;
    }

    void ReadElements(const u16 raw, std::vector<int>& elements){
        for (int element = FIRE; element <= HIDDEN; element ++)
            if (raw & (1 << element)) elements.push_back(element);
        // In KERNEL.BIN, non elemental records actually have all the elements marked.
        if (elements.size() == 16) elements.clear();
    }

    void ReadStatuses(const u32 raw, std::vector<int>& statuses){
        for (int status = DEATH; status <= IMPRISONED; status ++)
            if (raw & (1u << status)) statuses.push_back(status);
    }

    void ReadStatusEffect(const u8 change, const u32 raw, StatusEffect& status){
        // The lower 6 bits are the chance out of 63, and the upper bits the mode (0x80 toggles the
        // statuses, 0x40 cures them, otherwise they are inflicted).
        status.chance = change & 0x3F;
        if ((change & 0x80) != 0) status.mode = TOGGLE;
        else if ((change & 0x40) != 0) status.mode = CURE;
        else status.mode = INFLICT;
        // In KERNEL.BIN, records that don't change statuses actually have all of them marked, and
        // a chance of 255. The status IDs are the bit positions of the original game.
        if (change != 0xFF && raw != 0xFFFFFFFF) ReadStatuses(raw, status.status);
    }

    RESTORE_TYPE ReadRestoreType(const u8 condition){
        switch (condition){
            case 0: return RESTORE_HP;
            case 1: return RESTORE_MP;
            case 2: return RESTORE_STATUS;
            default: return RESTORE_NONE;
        }
    }

    void ReadUsers(const u16 raw, std::vector<u8>& users){
        for (int character = 0; character <= 10; character ++)
            if (raw & (1 << character)) users.push_back(character);
    }

    void ReadStatBonuses(
      const u8* stats, const u8* amounts, const int count, std::vector<StatBonus>& bonuses
    ){
        for (int s = 0; s < count; s ++){
            if (stats[s] < 0XFF && amounts[s] > 0){
                StatBonus stat_bonus;
                stat_bonus.stat = stats[s];
                stat_bonus.bonus = amounts[s];
                bonuses.push_back(stat_bonus);
            }
        }
    }

    void ReadMateriaSlots(const u8 (&raw)[8], std::vector<u8>& slots){
        for (const u8 slot : raw){
            if (slot == 1 || slot == 5) slots.push_back(SLOT_UNLINKED);
            else if (slot == 2 || slot == 3 || slot == 6 || slot == 7) slots.push_back(SLOT_LINKED);
        }
    }

    void WriteTarget(std::ostream& file, const Target& target){
        file
          << "    target = {selectable = " << target.selection_enabled
          << ", default_enemy = " << target.default_enemy
          << ", default_multiple = " << target.default_multiple
          << ", toggle_multiple = " << target.toggle_multiple
          << ", fixed_row = " << target.fixed_row
          << ", short_range = " << target.short_range
          << ", all_rows = " << target.all_rows
          << ", random = " << target.random << "},\n";
    }

    void WriteStatBonuses(std::ostream& file, const std::vector<StatBonus>& bonuses){
        file << "    stats = {";
        for (const StatBonus& s : bonuses){
            file << "\n        " << GetBasicStatName(s.stat) << " = "
              << static_cast<int>(s.bonus) << ",";
        }
        if (bonuses.size() > 0) file << "\n    },\n";
        else file << "},\n";
    }

    std::string GetBasicStatName(const int id){
        std::string name = "stat";
        switch (id){
            case 0: name = "str"; break;
            case 1: name = "vit"; break;
            case 2: name = "mag"; break;
            case 3: name = "spr"; break;
            case 4: name = "dex"; break;
            case 5: name = "lck"; break;
        }
        return name;
    }
}
