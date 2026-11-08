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
#include "installer/decompiler/field/FieldDecompiler.h"

BOOST_AUTO_TEST_CASE(TestFieldDecompiler){
    std::vector<unsigned char> script(104, 0);
    script[0] = 0x02;
    script[1] = 0x05;
    script[2] = 1;
    script[9] = 2;
    script[32] = 'D';
    script[33] = 'o';
    script[34] = 'o';
    script[35] = 'r';
    for (size_t index = 0; index < 32; ++index) script[40 + index * 2] = 104;

    for (unsigned char type = 0; type <= 2; ++type){
        const unsigned char instruction[] = {0xc3, 0, 0, type, 128, 0, 0, 0, 0, 0, 30, 0};
        script.insert(script.end(), std::begin(instruction), std::end(instruction));
    }
    script.push_back(0);
    script[4] = static_cast<unsigned char>(script.size());
    script[5] = static_cast<unsigned char>(script.size() >> 8);

    FieldScriptFormatter formatter;
    const std::string output = FieldDecompiler::Decompile("synthetic", script, formatter).luaScript;
    BOOST_CHECK(output.find("offset_to_position(1, 0, 0, Entity.NONE, 1)") != std::string::npos);
    BOOST_CHECK(output.find("offset_to_position(1, 0, 0, Entity.LINEAR, 1)") != std::string::npos);
    BOOST_CHECK(output.find("offset_to_position(1, 0, 0, Entity.SMOOTH, 1)") != std::string::npos);
}

static std::string DecompileInstructions(const std::vector<unsigned char>& instructions){
    std::vector<unsigned char> script(104, 0);
    script[0] = 0x02;
    script[1] = 0x05;
    script[2] = 1;
    script[9] = 2;
    script[32] = 'D';
    script[33] = 'o';
    script[34] = 'o';
    script[35] = 'r';
    for (size_t index = 0; index < 32; ++index) script[40 + index * 2] = 104;
    script.insert(script.end(), instructions.begin(), instructions.end());
    script.push_back(0); // RET
    script[4] = static_cast<unsigned char>(script.size());
    script[5] = static_cast<unsigned char>(script.size() >> 8);
    FieldScriptFormatter formatter;
    return FieldDecompiler::Decompile("synthetic", script, formatter).luaScript;
}

namespace{
    class DefaultAnimationFormatter : public FieldScriptFormatter{
        public:
            std::string GetDefaultAnimationName(int char_id) override{
                return char_id == 2 ? "stand" : "";
            }
    };
}

static FieldDecompiler::DecompiledScript DecompileFully(
  const std::vector<unsigned char>& instructions, FieldScriptFormatter& formatter
){
    std::vector<unsigned char> script(104, 0);
    script[0] = 0x02;
    script[1] = 0x05;
    script[2] = 1;
    script[9] = 2;
    script[32] = 'D';
    script[33] = 'o';
    script[34] = 'o';
    script[35] = 'r';
    for (size_t index = 0; index < 32; ++index) script[40 + index * 2] = 104;
    script.insert(script.end(), instructions.begin(), instructions.end());
    script.push_back(0); // RET
    script[4] = static_cast<unsigned char>(script.size());
    script[5] = static_cast<unsigned char>(script.size() >> 8);
    return FieldDecompiler::Decompile("synthetic", script, formatter);
}

static std::string DecompileWith(
  const std::vector<unsigned char>& instructions, FieldScriptFormatter& formatter
){return DecompileFully(instructions, formatter).luaScript;}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerModelWithoutIdleGetsDefaultAnimation){
    DefaultAnimationFormatter formatter;
    // CHAR 2: the formatter has an animation for it.
    const std::string with_animation = DecompileWith({0xA1, 2}, formatter);
    BOOST_CHECK(
      with_animation.find("self.Door:set_default_animation(\"stand\") -- No Idle animation")
      != std::string::npos
    );
    BOOST_CHECK(with_animation.find("self.Door:play_animation(\"stand\")") != std::string::npos);
    // CHAR 1: the formatter has none, as when the model already has an Idle animation.
    const std::string without_animation = DecompileWith({0xA1, 1}, formatter);
    BOOST_CHECK(without_animation.find("self.Door = entity_manager") != std::string::npos);
    BOOST_CHECK(without_animation.find("set_default_animation") == std::string::npos);
    // The default formatter never adds one.
    FieldScriptFormatter plain_formatter;
    BOOST_CHECK(
      DecompileWith({0xA1, 2}, plain_formatter).find("set_default_animation") == std::string::npos
    );
}

namespace{
    class NamedAnimationFormatter : public FieldScriptFormatter{
        public:
            std::string GetFriendlyAnimationName(int char_id, int id) override{
                return "anim" + std::to_string(id);
            }
    };
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerNonWaitingAnimationsDontWait){
    NamedAnimationFormatter formatter;
    // ANIM!1 3, 1 - ANIME2 2, 1 - CANIM1 4, 0, 14, 1 - CANM!1 5, 15, 29, 1.
    const std::string output = DecompileWith(
      {0xAF, 3, 1, 0xAE, 2, 1, 0xB0, 4, 0, 14, 1, 0xB1, 5, 15, 29, 1}, formatter
    );
    BOOST_CHECK(output.find("self.Door:play_animation_stop(\"anim3\") -- speed 1") != std::string::npos);
    BOOST_CHECK(output.find("self.Door:play_animation(\"anim2\") -- speed 1") != std::string::npos);
    BOOST_CHECK(
      output.find("self.Door:play_animation(\"anim4\", 0, 0.466667) -- speed 1") != std::string::npos
    );
    BOOST_CHECK(
      output.find("self.Door:play_animation_stop(\"anim5\", 0.5, 0.966667) -- speed 1")
      != std::string::npos
    );
    BOOST_CHECK(output.find("UNIMPLMENTED") == std::string::npos);
    // None of them waits.
    BOOST_CHECK(output.find("animation_sync") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerMovaWalksToTheEntityAndWaits){
    FieldScriptFormatter formatter;
    // MOVA 0: the only entity of the synthetic field.
    const std::string output = DecompileWith({0xAA, 0}, formatter);
    const size_t walk = output.find("self.Door:move_to_entity(entity_manager:get_entity(\"Door\"))");
    const size_t wait = output.find("self.Door:move_sync()");
    BOOST_REQUIRE(walk != std::string::npos && wait != std::string::npos);
    BOOST_CHECK(walk < wait);
    BOOST_CHECK(output.find("UNIMPLMENTED") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerScreenFades){
    FieldScriptFormatter formatter;
    // FADE 255, 0, 0, speed 8, type 5 - NFADE type 11, 10, 20, 30, 15 frames - FADEW.
    const std::string output = DecompileWith(
      {0x6B, 0, 0, 255, 0, 0, 8, 5, 0, 0x25, 0, 0, 11, 10, 20, 30, 15, 0, 0x6C}, formatter
    );
    BOOST_CHECK(output.find("screen_fade:fade(5, 255, 0, 0, 8)") != std::string::npos);
    BOOST_CHECK(output.find("screen_fade:nfade(11, 10, 20, 30, 15)") != std::string::npos);
    BOOST_CHECK(output.find("screen_fade:wait()") != std::string::npos);
    // They used to be comments.
    BOOST_CHECK(output.find("-- fade:") == std::string::npos);
    BOOST_CHECK(output.find("UNIMPLMENTED") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerWindowCloseTalkRangeAndFacing){
    FieldScriptFormatter formatter;
    // WCLS 2 - TALKR 64 - TLKR2 128 - DIRA 0.
    const std::string output = DecompileWith(
      {0x2E, 2, 0xC5, 0, 64, 0xD6, 0, 128, 0, 0xB6, 0}, formatter
    );
    BOOST_CHECK(output.find("dialog:dialog_close(\"2\")") != std::string::npos);
    BOOST_CHECK(output.find("self.Door:set_talk_radius(0.5)") != std::string::npos);
    BOOST_CHECK(output.find("self.Door:set_talk_radius(1)") != std::string::npos);
    BOOST_CHECK(
      output.find("self.Door:face_entity(entity_manager:get_entity(\"Door\"))") != std::string::npos
    );
    BOOST_CHECK(output.find("UNIMPLMENTED") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerAspedSetsTheAnimationSpeed){
    FieldScriptFormatter formatter;
    // ASPED 16 (normal speed), ASPED 32 (double) and ASPED 8 (half).
    const std::string output = DecompileWith(
      {0xBD, 0, 16, 0, 0xBD, 0, 32, 0, 0xBD, 0, 8, 0}, formatter
    );
    BOOST_CHECK(output.find("self.Door:set_animation_speed(1)") != std::string::npos);
    BOOST_CHECK(output.find("self.Door:set_animation_speed(2)") != std::string::npos);
    BOOST_CHECK(output.find("self.Door:set_animation_speed(0.5)") != std::string::npos);
    BOOST_CHECK(output.find("UNIMPLMENTED") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerShakeKeepsItsParameters){
    FieldScriptFormatter formatter;
    // SHAKE 0, 0, 2, 0, 0, 5, 2 (shake the Y axis) and SHAKE 0, 0, 0, 0, 1, 0, 1 (stop).
    const std::string output = DecompileWith(
      {0x5E, 0, 0, 2, 0, 0, 5, 2, 0x5E, 0, 0, 0, 0, 1, 0, 1}, formatter
    );
    BOOST_CHECK(output.find("background2d:shake(2, 0, 0, 5, 2)") != std::string::npos);
    BOOST_CHECK(output.find("background2d:shake(0, 0, 1, 0, 1)") != std::string::npos);
    BOOST_CHECK(output.find("UNIMPLMENTED") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerAnimwWaitsForTheAnimation){
    NamedAnimationFormatter formatter;
    // ANIM!1 3, 1 and then ANIMW: the script waits after starting the animation.
    const std::string output = DecompileWith({0xAF, 3, 1, 0xAC}, formatter);
    const size_t start = output.find("self.Door:play_animation_stop(\"anim3\")");
    const size_t wait = output.find("self.Door:animation_sync()");
    BOOST_REQUIRE(start != std::string::npos && wait != std::string::npos);
    BOOST_CHECK(start < wait);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerBackgroundAreas){
    FieldScriptFormatter formatter;
    // BGON 0, 0, 2, 0 - BGOFF 0, 0, 8, 2 - BGCLR 0, 8.
    const std::string output = DecompileWith(
      {0xE0, 0, 2, 0, 0xE1, 0, 8, 2, 0xE4, 0, 8}, formatter
    );
    BOOST_CHECK(output.find("background2d:area_on(2, 0)") != std::string::npos);
    BOOST_CHECK(output.find("background2d:area_off(8, 2)") != std::string::npos);
    BOOST_CHECK(output.find("background2d:area_clear(8)") != std::string::npos);
    // They used to be comments.
    BOOST_CHECK(output.find("-- field:background_") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerReportsTheBackgroundAreasItControls){
    FieldScriptFormatter formatter;
    const auto literal = DecompileFully({0xE0, 0, 2, 0, 0xE1, 0, 8, 2, 0xE4, 0, 5}, formatter);
    BOOST_CHECK(literal.background_areas == std::set<int>({2, 5, 8}));
    BOOST_CHECK(!literal.background_areas_dynamic);
    // BGON with the area read from bank 1, address 3: any area could be controlled.
    const auto from_variable = DecompileFully({0xE0, 0x10, 3, 0}, formatter);
    BOOST_CHECK(from_variable.background_areas_dynamic);
    // A script that doesn't touch the background controls no area.
    const auto none = DecompileFully({}, formatter);
    BOOST_CHECK(none.background_areas.empty());
    BOOST_CHECK(!none.background_areas_dynamic);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerFmoveWaitsAndKeepsTheAnimation){
    FieldScriptFormatter formatter;
    // FMOVE 0, 0, 128, -256: both bank nibbles zero, then X and Y as signed shorts.
    const std::string output = DecompileWith({0xAD, 0, 0x80, 0x00, 0x00, 0xFF}, formatter);
    BOOST_CHECK(output.find("UNIMPLMENTED INSTRUCTION: \"FMOVE\"") == std::string::npos);
    const size_t off = output.find("self.Door:move_auto_animation(false)");
    const size_t move = output.find("self.Door:move_to_position(");
    const size_t sync = output.find("self.Door:move_sync()");
    const size_t on = output.find("self.Door:move_auto_animation(true)");
    BOOST_REQUIRE(off != std::string::npos && move != std::string::npos);
    BOOST_REQUIRE(sync != std::string::npos && on != std::string::npos);
    // The move must end before the automatic animation comes back.
    BOOST_CHECK(off < move);
    BOOST_CHECK(move < sync);
    BOOST_CHECK(sync < on);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerBattleKeepsItsId){
    FieldScriptFormatter formatter;
    // BATTLE 0, 0, 328 and BATTLE 0, 0, 7.
    const std::string output = DecompileWith({0x70, 0, 0x48, 0x01, 0x70, 0, 0x07, 0x00}, formatter);
    BOOST_CHECK(output.find("Battle.start(328)") != std::string::npos);
    BOOST_CHECK(output.find("Battle.start(7)") != std::string::npos);
    BOOST_CHECK(output.find("Battle.start(0)") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerMessageInUndefinedWindow){
    // MESSAGE 0, 0 and MESSAGE 1, 0, where no WINDOW defines any of them.
    const std::string output = DecompileInstructions({0x40, 0, 0, 0x40, 1, 0});
    BOOST_CHECK(
      output.find("dialog:dialog_open(\"0\", 16, 176, 288, 56) -- Default, no WINDOW")
      != std::string::npos
    );
    BOOST_CHECK(
      output.find("dialog:dialog_open(\"1\", 16, 176, 288, 56) -- Default, no WINDOW")
      != std::string::npos
    );
    // The window must be opened before the text is set.
    BOOST_CHECK(
      output.find("dialog_open(\"0\"") < output.find("dialog:dialog_set_text(\"0\", \"synthetic_0\")")
    );
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerMessageInDefinedWindow){
    // WINDOW 1, 83, 120, 138, 25 and then MESSAGE 1, 0. Window 0 is not defined.
    const std::string output = DecompileInstructions(
      {0x50, 1, 83, 0, 120, 0, 138, 0, 25, 0, 0x40, 1, 0, 0x40, 0, 0}
    );
    BOOST_CHECK(output.find("dialog:dialog_open(\"1\", 83, 120, 138, 25)") != std::string::npos);
    BOOST_CHECK(output.find("dialog_open(\"1\", 16, 176") == std::string::npos);
    BOOST_CHECK(output.find("dialog_open(\"0\", 16, 176, 288, 56)") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerOnUpdateRunsOnce){
    std::vector<unsigned char> script(104, 0);
    script[0] = 0x02;
    script[1] = 0x05;
    script[2] = 1;
    script[9] = 2;
    script[32] = 'D';
    script[33] = 'o';
    script[34] = 'o';
    script[35] = 'r';
    for (size_t index = 0; index < 32; ++index) script[40 + index * 2] = 104;
    // The first RET ends the init script (on_start), the code after it is the main script
    // (on_update).
    script.push_back(0); // RET
    script.push_back(0); // RET
    script[4] = static_cast<unsigned char>(script.size());
    script[5] = static_cast<unsigned char>(script.size() >> 8);

    FieldScriptFormatter formatter;
    const std::string output = FieldDecompiler::Decompile("synthetic", script, formatter).luaScript;
    const size_t on_update = output.find("on_update = function(self)");
    BOOST_REQUIRE(on_update != std::string::npos);
    const std::string body = output.substr(on_update);
    // The engine runs on_update again once it ends, but a script that returns is over.
    const size_t guard = body.find("if self.on_update_finished then");
    const size_t mark = body.find("self.on_update_finished = true");
    BOOST_CHECK(guard != std::string::npos);
    BOOST_CHECK(mark != std::string::npos);
    BOOST_CHECK(guard < mark);
    // Other functions must keep running normally.
    BOOST_CHECK(output.substr(0, on_update).find("on_update_finished") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDecompilerScrollInstructions){
    std::vector<unsigned char> script(104, 0);
    script[0] = 0x02;
    script[1] = 0x05;
    script[2] = 1;
    script[9] = 2;
    script[32] = 'D';
    script[33] = 'o';
    script[34] = 'o';
    script[35] = 'r';
    for (size_t index = 0; index < 32; ++index) script[40 + index * 2] = 104;

    const unsigned char instructions[] = {
      0x63, 0, 128, 0, 0, 3, // SCRLA: Smooth scroll to entity 0, in 128 frames.
      0x63, 0, 0, 0, 0, 1, // SCRLA: Instant scroll to entity 0.
      0x67, // SCRLW
      0x62, 0, 30, 0, 2, // SCRLC: Linear scroll to the player, in 30 frames.
      0x62, 0, 0, 0, 1, // SCRLC: This type has no effect.
      0x65 // SCRCC
    };
    script.insert(script.end(), std::begin(instructions), std::end(instructions));
    script.push_back(0);
    script[4] = static_cast<unsigned char>(script.size());
    script[5] = static_cast<unsigned char>(script.size() >> 8);

    FieldScriptFormatter formatter;
    const std::string output = FieldDecompiler::Decompile("synthetic", script, formatter).luaScript;
    const std::string get_door = "entity_manager:get_entity(\"Door\")";
    BOOST_CHECK(
      output.find("scroll_to_entity(" + get_door + ", Background2D.SMOOTH, 4.266667)")
      != std::string::npos
    );
    BOOST_CHECK(
      output.find("scroll_to_entity(" + get_door + ", Background2D.NONE, 0.000000)")
      != std::string::npos
    );
    BOOST_CHECK(output.find("background2d:scroll_sync()") != std::string::npos);
    BOOST_CHECK(
      output.find("scroll_to_player(Background2D.LINEAR, 1.000000)") != std::string::npos
    );
    BOOST_CHECK(output.find("-- SCRLC without effect") != std::string::npos);
    BOOST_CHECK(output.find("scroll_to_player(Background2D.NONE, 0)") != std::string::npos);
}

