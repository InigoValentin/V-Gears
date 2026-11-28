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

#include <boost/test/unit_test.hpp>
#include <vector>
#include "installer/extractors/KernelDataParts.h"
#include "installer/extractors/KernelTextList.h"
#include "installer/tables/Characters.h"

/**
 * Retrieves the byte that stands for a character in the texts of the kernel.
 */
static u8 Encode(const char character){
    for (int code = 0; code < 256; code ++)
        if (ENGLISH_CHARS[code] == static_cast<unsigned char>(character)) return code;
    BOOST_FAIL("Character without a code.");
    return 0;
}

BOOST_AUTO_TEST_CASE(TestKernelTextListReadsNamesInOrderAndChangesQuotes){
    // Two offsets, and the texts "AB" and a double quote.
    const std::vector<u8> section = {
      4, 0, 7, 0, Encode('A'), Encode('B'), 0xFF, Encode('"'), 0xFF
    };
    KernelTextList names(File(section.data(), 0, section.size()));
    BOOST_CHECK_EQUAL(names.NextName(), "AB");
    BOOST_CHECK_EQUAL(names.NextName(), "'");
}

BOOST_AUTO_TEST_CASE(TestKernelTextListExpandsCompressedDescriptions){
    // "ABCD", and a reference to the four bytes that start four bytes before the 0xF9 byte.
    const std::vector<u8> section = {
      2, 0, Encode('A'), Encode('B'), Encode('C'), Encode('D'), 0xF9, 3, 0xFF
    };
    KernelTextList descriptions(File(section.data(), 0, section.size()));
    BOOST_CHECK_EQUAL(descriptions.NextDescription(), "ABCDABCD");
}

BOOST_AUTO_TEST_CASE(TestKernelDataBitsOfElementsAndUsers){
    std::vector<int> elements;
    KernelData::ReadElements(0x8001, elements);
    BOOST_REQUIRE_EQUAL(elements.size(), 2u);
    BOOST_CHECK_EQUAL(elements[0], static_cast<int>(KernelData::FIRE));
    BOOST_CHECK_EQUAL(elements[1], static_cast<int>(KernelData::HIDDEN));
    // All of them means none.
    elements.clear();
    KernelData::ReadElements(0xFFFF, elements);
    BOOST_CHECK(elements.empty());
    // Only eleven characters.
    std::vector<u8> users;
    KernelData::ReadUsers(0xFFFF, users);
    BOOST_CHECK_EQUAL(users.size(), 11u);
}

BOOST_AUTO_TEST_CASE(TestKernelDataStatusEffects){
    KernelData::StatusEffect cure;
    KernelData::ReadStatusEffect(0x40 | 63, 0x8, cure);
    BOOST_CHECK_EQUAL(cure.mode, KernelData::CURE);
    BOOST_CHECK_EQUAL(cure.chance, 63);
    BOOST_REQUIRE_EQUAL(cure.status.size(), 1u);
    BOOST_CHECK_EQUAL(cure.status[0], static_cast<int>(KernelData::POISONED));
    // Records that don't change statuses have all of them marked.
    KernelData::StatusEffect none;
    KernelData::ReadStatusEffect(0xFF, 0xFFFFFFFF, none);
    BOOST_CHECK(none.status.empty());
}

BOOST_AUTO_TEST_CASE(TestKernelDataTargets){
    KernelData::Target target;
    // All rows, and not the enemies by default.
    KernelData::ReadTarget(0x40, target);
    BOOST_CHECK(target.all_rows);
    BOOST_CHECK(!target.default_enemy);
    KernelData::ReadTarget(0x02, target);
    BOOST_CHECK(!target.all_rows);
    BOOST_CHECK(target.default_enemy);
}
