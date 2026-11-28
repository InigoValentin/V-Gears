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
#include <tinyxml.h>
#include "engine/ui/UiTextLayout.h"
#include "engine/ui/UiTextParser.h"

namespace{

    /**
     * Sources with fixed names, a variable "money" and an included text "greeting".
     */
    struct TestSources{
        TiXmlDocument included;
        UiTextParser::Sources sources;
        TestSources(){
            // As in the game, where the XML files set it: spaces and line feeds are kept.
            TiXmlBase::SetCondenseWhiteSpace(false);
            included.Parse("<text>Hi <colour value=\"1 0 0\">you</colour>!</text>");
            sources.variable = [](const Ogre::String& name){
                return Ogre::UTFString(name == "money" ? "150" : "");
            };
            sources.character_name = [](int id){return "Name" + std::to_string(id);};
            sources.party_name = [](int position){return "Party" + std::to_string(position);};
            sources.text = [this](const Ogre::String& name){
                return name == "greeting" ? included.RootElement() : nullptr;
            };
            sources.image = [](const Ogre::String& name, TextChar& character){
                character.sprite_y = 7;
                return name == "circle";
            };
        }
    };

    std::vector<TextChar> Parse(const std::string& markup, bool* timer = nullptr){
        TestSources test;
        TiXmlDocument document;
        document.Parse(("<container>" + markup + "</container>").c_str());
        std::vector<TextChar> text;
        UiTextParser parser(test.sources, text);
        parser.Parse(document.RootElement(), Ogre::ColourValue::White);
        if (timer != nullptr) *timer = parser.UsesTimer();
        return text;
    }

    /**
     * The visible letters of a text.
     */
    std::string Letters(const std::vector<TextChar>& text){
        std::string letters;
        for (const TextChar& character : text){
            if (character.char_code != 0) letters += static_cast<char>(character.char_code);
        }
        return letters;
    }

    UiCharData Glyph(const int code){
        UiCharData data = {};
        data.char_code = code;
        data.width = 8;
        data.height = 10;
        data.pre = 1;
        data.post = 1;
        return data;
    }
}

BOOST_AUTO_TEST_CASE(TestUiTextReadsEveryElement){
    // All the text is read, not only the first element of each level.
    const auto text = Parse("HP <colour value=\"1 0 0\">-5</colour> of <include name=\"greeting\" />.");
    BOOST_CHECK_EQUAL(Letters(text), "HP -5 of Hi you!.");
    BOOST_CHECK(text[0].colour == Ogre::ColourValue::White);
    BOOST_CHECK(text[3].colour == Ogre::ColourValue(1, 0, 0));
    BOOST_CHECK(text[5].colour == Ogre::ColourValue::White);
}

BOOST_AUTO_TEST_CASE(TestUiTextNamesAndVariables){
    BOOST_CHECK_EQUAL(Letters(Parse("<character id=\"2\"/>,<party pos=\"1\"/>")), "Name2,Party1");
    const auto text = Parse("<variable name=\"money\"/>G");
    BOOST_REQUIRE_EQUAL(text.size(), 5u);
    BOOST_CHECK(text[0].skip);
    BOOST_CHECK_EQUAL(text[0].variable, "money");
    BOOST_CHECK_EQUAL(text[0].variable_len, 3u);
    BOOST_CHECK_EQUAL(Letters(text), "150G");
    bool timer = false;
    BOOST_CHECK_EQUAL(Parse("<timer/>", &timer)[0].variable, UiTextParser::TIMER_VARIABLE);
    BOOST_CHECK(timer);
}

BOOST_AUTO_TEST_CASE(TestUiTextCommands){
    const auto text = Parse("a<pause_ok/><pause time=\"60\"/><next_page/><image sprite=\"circle\"/>");
    BOOST_REQUIRE_EQUAL(text.size(), 5u);
    BOOST_CHECK(text[1].pause_ok);
    BOOST_CHECK_EQUAL(text[2].pause_time, 2.0f);
    BOOST_CHECK(text[3].next_page);
    BOOST_CHECK_EQUAL(text[4].sprite_y, 7.0f);
}

BOOST_AUTO_TEST_CASE(TestUiTextIgnoresIncompleteElements){
    // These crashed the game.
    BOOST_CHECK_EQUAL(
      Letters(Parse(
        "<colour>a</colour><character/><party pos=\"x\"/><variable/><pause/><include/><image/>b"
      )), "ab"
    );
    BOOST_CHECK(Parse("<include name=\"nothing\"/><image sprite=\"square\"/>").empty());
}

BOOST_AUTO_TEST_CASE(TestUiTextLayoutWrapsAndStopsAtTheBottom){
    // An area of 25 x 20 on a screen of the reference height: two letters per line, two lines.
    const UiTextLayout::Area area{{0, 0}, {25, 20}, {1, 1}, 720, 0, 0, 0, 0};
    UiTextLayout layout(area, 0, 0, 10);
    UiTextLayout::Box box;
    BOOST_REQUIRE(layout.Place(Glyph('a'), box));
    BOOST_CHECK_EQUAL(box.left, 1);
    BOOST_CHECK_EQUAL(box.right, 9);
    BOOST_REQUIRE(layout.Place(Glyph('b'), box));
    BOOST_CHECK_EQUAL(box.left, 11);
    BOOST_CHECK_EQUAL(box.top, 0);
    // The third letter doesn't fit: it goes to the next line. The space before it is not
    // repeated there.
    BOOST_REQUIRE(layout.Place(Glyph('c'), box));
    BOOST_CHECK_EQUAL(box.left, 0);
    BOOST_CHECK_EQUAL(box.top, 10);
    layout.NewLine();
    BOOST_CHECK(!layout.Place(Glyph('d'), box));
}

BOOST_AUTO_TEST_CASE(TestUiTextLayoutWidthAndCorners){
    const std::vector<TextChar> text = Parse("ab\nabc");
    BOOST_CHECK_EQUAL(UiTextLayout::TextWidth(text, Glyph), 30);
    const auto corners = UiTextLayout::ScreenCorners(
      {10, 20, 30, 40}, {0, 0}, 0, {100, 100}
    );
    BOOST_CHECK_CLOSE(corners[0].x, -0.8f, 0.001f);
    BOOST_CHECK_CLOSE(corners[0].y, 0.6f, 0.001f);
    BOOST_CHECK_CLOSE(corners[2].x, -0.4f, 0.001f);
}
