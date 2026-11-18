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

#include "../../src/core/TextHandler.h"

#include <boost/test/unit_test.hpp>
#include <tinyxml.h>

#pragma message("Unimplemented test file: " __FILE__)

BOOST_AUTO_TEST_CASE(TestTextManager){
    // TODO
    BOOST_WARN_MESSAGE(false, "Unimplemented test file: TextManager");
    BOOST_CHECK(0 == 0);
}


namespace{
    std::string TextOf(const char* xml){
        TiXmlDocument document;
        document.Parse(xml);
        return TextHandler::ExtractText(
          document.RootElement(), [](int id){return std::string("Name") + std::to_string(id);}
        );
    }
}

BOOST_AUTO_TEST_CASE(TestTextHandlerExtractTextPlain){
    BOOST_CHECK_EQUAL(TextOf("<dialog>Sector 5</dialog>"), "Sector 5");
    BOOST_CHECK_EQUAL(TextOf("<dialog></dialog>"), "");
}

BOOST_AUTO_TEST_CASE(TestTextHandlerExtractTextStartsWithACharacterName){
    // The map name of the house of Tifa. It starts with a tag, which crashed the game.
    BOOST_CHECK_EQUAL(TextOf("<dialog><character id=\"3\" />&apos;s House</dialog>"), "Name3's House");
}

BOOST_AUTO_TEST_CASE(TestTextHandlerExtractTextKeepsTheContentOfOtherTags){
    BOOST_CHECK_EQUAL(
      TextOf("<dialog>A<colour value=\"1 0 0\">[SAVE]</colour>B</dialog>"), "A[SAVE]B"
    );
}
