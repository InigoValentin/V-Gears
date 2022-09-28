/*
 * V-Gears
 * Copyright (C) 2022 V-Gears Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "decompiler/decompiler_engine.h"
#include "decompiler/field/FieldEngine.h"

/**
 * A model instruction.
 */
class FieldModelInstruction : public KernelCallInstruction{

    public:

        /**
         * Processes the instruction.
         *
         * @param func[in] Function to process.
         * @param stack[out] Function stack.
         * @param engine[in] Engine.
         * @param code_gen[in|out] Code generator.
         */
        virtual void ProcessInst(
          Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
        ) override;

    private:

        /**
         * Processes a JOIN opcode.
         *
         * Opcode: 0x08
         * Short name: JOIN
         * Long name: Party Field Join
         * Memory layout (2 bytes)
         * |0x08|S|
         *
         * Arguments
         * - const UByte S: Speed that the characters join back together.
         *
         * Causes seperated party characters that have previously been SPLIT
         * onto the field, to be joined back together again; that is, only the
         * party leader becomes visible on the field. This should be called if
         * a previous SPLIT has completed (the party members have finished
         * speaking, or performing their actions, for example). As with SPLIT,
         * the speed of the join is specified, from a scale of 1 (almost
         * instant) to FF (very slow walk), and must be non-zero. In contrast
         * to most MOVE related opcodes, the speed is this setting is actually
         * the total number of frames required. Depending on the distance from
         * the player character and the number of frames required, the entity
         * plays a run or walk animation. Also, all characters take the same
         * time irrespective of distance. Calling JOIN without having
         * previously SPLIT the characters will cause the party members to
         * appear at the walkmesh origin and attempt to JOIN from there. This
         * is not normally the required behaviour and should be avoided.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         */
        void ProcessJOIN(CodeGenerator* code_gen);

        /**
         * Processes a SPLIT opcode.
         *
         * Opcode: 0x09
         * Short name: SPLIT
         * Long name: Party Field Split
         *
         * Memory layout (15 bytes)
         * |0x20|B1/B2|B3/B4|B5/B6|XA|XA|YA|YA|DA|XB|XB|YB|YB|DB|S|
         *
         * Arguments
         * - const Bit[4] B1: Bank for XA, or zero if XA is specified as a
         * literal value.
         * - const Bit[4] B2: Bank for YA, or zero if YA is specified as a
         * literal value.
         * - const Bit[4] B3: Bank for DA, or zero if DA is specified as a
         * literal value.
         * - const Bit[4] B4: Bank for XB, or zero if XB is specified as a
         * literal value.
         * - const Bit[4] B5: Bank for YB, or zero if YB is specified as a
         * literal value.
         * - const Bit[4] B6: Bank for DB, or zero if DB is specified as a
         * literal value.
         * - const Short XA: X-coordinate of the second character in the party
         * after the split, or address for the value if B1 is non-zero.
         * - const Short YA: Y-coordinate of the second character in the party
         * after the split, or address for the value if B2 is non-zero.
         * - const UByte DA: Direction the second character faces after the
         * split, or address for the value if B3 is non-zero.
         * - const Short XB: X-coordinate of the third character in the party
         * after the split, or address for the value if B4 is non-zero.
         * - const Short YB: Y-coordinate of the third character in the party
         * after the split, or address for the value if B5 is non-zero.
         * - const UByte DB: Direction the third character faces after the
         * split, or address for the value if B6 is non-zero.
         * - const UByte S: Speed that the characters split.
         *
         * Causes the common 'split effect' whereby the second and third
         * characters in the current party 'come out' from the party leader.
         * That is, they become visible in the field, starting from the center
         * of the party leader, and move out to the coordinates specified in
         * the argument list. This is commonly used when the other characters
         * in the current party have an action or dialog to perform and must
         * be individually visible in the field. As well as specifying final
         * coordinates for the two other party characters, the directions each
         * character faces after the split are specified as a byte, using the
         * common direction values found throughout the game. Speed is also
         * given and is used to specify the rate at which the characters leave
         * the party leader, using a scale from 1 (almost instant) to FF
         * (extremely slow walk); this must be non-zero. In contrast to most
         * MOVE related op codes, the speed is this setting is actually the
         * total number of frames required. Depending on the distance from the
         * player character and the number of frames required, the entity
         * plays a run or walk animation. Also, all * characters take the same
         * time irrespective of distance.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         */
        void ProcessSPLIT(CodeGenerator* code_gen);
        void ProcessTLKON(CodeGenerator* code_gen, const std::string& entity);
        void ProcessPC(CodeGenerator* code_gen, const std::string& entity);
        void ProcessCHAR(CodeGenerator* code_gen, const std::string& entity);
        void ProcessDFANM(CodeGenerator* code_gen, const std::string& entity, int char_id);
        void ProcessANIME1(CodeGenerator* code_gen, const std::string& entity, int char_id);
        void ProcessVISI(CodeGenerator* code_gen, const std::string& entity);
        void ProcessXYZI(CodeGenerator* code_gen, const std::string& entity);
        void ProcessMOVE(CodeGenerator* code_gen, const std::string& entity);
        void ProcessMSPED(CodeGenerator* code_gen, const std::string& entity);
        void ProcessDIR(CodeGenerator* code_gen, const std::string& entity);
        void ProcessTURNGEN(CodeGenerator* code_gen, const std::string& entity);
        void ProcessGETAI(CodeGenerator* code_gen, const FieldEngine& engine);
        void ProcessANIM_2(CodeGenerator* code_gen, const std::string& entity, int char_id);
        void ProcessCANIM2(CodeGenerator* code_gen, const std::string& entity, int char_id);
        void ProcessCANM_2(CodeGenerator* code_gen, const std::string& entity, int char_id);
        void ProcessCC(CodeGenerator* code_gen, const FieldEngine& engine);

        /**
         * Processes a JUMP opcode.
         *
         * Opcode: 0xC0
         * Short name: JUMP
         * Long name: Jump
         *
         * Memory layout (7 bytes)
         * |0xC2|B1/B2|B3/B4|X|Y|I|Steps|
         *
         * Arguments
         * - const Bit[4] B1: Bank to retrieve X-coordinate, or zero if
         * specifying X as a literal value.
         * - const Bit[4] B2: Bank to retrieve Y-coordinate, or zero if
         * specifying Y as a literal value.
         * - const Bit[4] B3: Bank to retrieve triangle ID, or zero if
         * specifying Z as a literal value.
         * - const Bit[4] B4: Bank to retrieve jump height, or zero if
         * specifying H as a literal value.
         * - const Short X: X-coordinate of the target to jump to, or lower
         * byte specifying address if B1 is non-zero.
         * - const Short Y: Y-coordinate of the target to jump to, or lower
         * byte specifying address if B2 is non-zero.
         * - const Short I: Triangle ID of the target to jump to, or lower
         * byte specifying address if B3 is non-zero.
         * - const UShort Steps: Steps in jump. Must be non-zero if a literal
         * value. Alternatively, lower byte specifies address if B4 is
         * non-zero.
         *
         * Causes the character to jump to the specified point and triangle
         * ID, with the jump curve peaking at a height which is increased by
         * using a larger value for the H argument. In addition, the larger
         * the number, the longer the jump will take to complete. A "normal"
         * value is around 0x15, 0x01 is fast and instantaneous; the argument
         * must not be zero or the game will crash. Whilst this is an unsigned
         * two-byte number, a large value (beyond around 0x60) will not only
         * cause a vast jump height, but also cause the screen to scroll
         * erratically (the larger the number, the more erratic). Main update
         * function go through all entity with JUMP state and if stage is 0 it
         * calculates final Z point according to triangle id. It sets current
         * coords as start coords. The main thing this function does is set B
         * coefficient for later calculation. It defines as follows:
         *   B = (Z_final - Z_start) / steps - steps * 1.45;
         * Then it set current step to 0 and stage to 1. On next update other
         * part of function works. It's calculate real position. First it
         * increment current step number. Then it calculate X and Y. They
         * change linear so nothing interesting here. The Z calculation is as
         * follows:
         *   Z_current = - step^2 * 1.45 + step * B + Z_start;
         * If current substep equal number of steps then we set current
         * triangle to final triangle and set stage to 2. Which finalizes the
         * routine on next opcode call. Neither animation nor sound is
         * specified in this opcode. An animation is played by using an
         * animation opcode such as DFANM, and a SOUND played, before the
         * jump.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         * @param entity[in] The name of the entity.
         */
        void ProcessJUMP(CodeGenerator* code_gen, const std::string& entity);

        /**
         * Processes a AXYZI opcode
         *
         * Opcode: 0xC1
         * Short name: AXYZI
         * Long name: Entity Get Position
         *
         * Memory layout (8 bytes)
         * |0xC1|B1/B2|B3/B4|A|X|Y|Z|I|
         *
         * Arguments
         * - const Bit[4] B1: Bank to store X.
         * - const Bit[4] B2: Bank to store Y.
         * - const Bit[4] B3: Bank to store Z.
         * - const Bit[4] B4: Bank to store I.
         * - const UByte A: Entity ID whose field object will have its
         * position retrieved from.
         * - const UByte X: Address to store the X-coordinate.
         * - const UByte Y: Address to store the Y-coordinate.
         * - const UByte Z: Address to store the Z-coordinate.
         * - const UByte I: Address to store the ID of the walkmesh triangle
         * the object is standing on.
         *
         * Retrieves the coordinates of the field object that the entity,
         * whose ID specified in A, is associated with. This opcode uses an
         * entity ID, not a field object offset; as such, if an entity ID is
         * given that does not have a field object, this opcode will store
         * zero in each of the four address specified.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         */
        void ProcessAXYZI(CodeGenerator* code_gen);

        /**
         * Processes a LADER opcode.
         *
         * Opcode: 0xC2
         * Short name: LADER
         * Long name: Ladder
         *
         * Memory layout (15 bytes)
         * |0xC2|B1/B2|B3/B4|X|X|Y|Y|Z|Z|I|I|K|A|D|S|
         *
         * Arguments
         * - const Bit[4] B1: Bank to retrieve X-coordinate, or zero if X is
         * specified as a literal value.
         * - const Bit[4] B2: Bank to retrieve Y-coordinate, or zero if Y is
         * specified as a literal value.
         * - const Bit[4] B3: Bank to retrieve Z-coordinate, or zero if Z is
         * specified as a literal value.
         * - const Bit[4] B4: Bank to retrieve ID, or zero if I is specified
         * as a literal value.
         * - const Short X: X-coordinate of the end of the ladder, or address
         * to find X-coordinate if B1 is non-zero.
         * - const Short Y: Y-coordinate of the end of the ladder, or address
         * to find Y-coordinate if B2 is non-zero.
         * - const Short Z: Z-coordinate of the end of the ladder, or address
         * to find Z-coordinate if B3 is non-zero.
         * - const UShort I: ID of the walkmesh triangle found at the end of
         * the ladder, or address to find ID if B4 is non-zero.
         * - const UByte K: The keys used to move the character on the ladder.
         * - const UByte A: Animation ID for the field object's movement
         * animation.
         * - const UByte D: Direction the character faces when climbing the
         * ladder.
         * - const UByte S: Speed of the animation whilst climbing the ladder.
         *
         * Causes the character to climb a ladder; that is, switching from
         * standard walkmesh movement, to climbing along a line connecting two
         * points on the walkmesh. If B1, B2, B3 or B4 is non-zero, then the
         * value for that particular component is taken from memory using the
         * corresponding bank and address specified, rather than as a literal
         * value. Both retrieved values and literals can be used for different
         * components. If using X, Y, Z or I as addresses, the lower byte
         * should hold the address whilst the higher byte should be zero. The
         * coordinates specify the end-point of the ladder; the current
         * position of the character is used as the start point. The ID of the
         * walkmesh triangle must be specified; this is the triangle the
         * character will step onto after reaching the end point of the
         * ladder. The K value specifies the keys used to move the character
         * across the ladder; keys outside the range found in the table will
         * cause unpredictable behavior. The animation ID specifies an offset
         * into the field object's animation list; this animation is played at
         * the speed specified by S whilst the character climbs. Finally, the
         * D argument is a direction value in the game's standard direction
         * format, which orients the character on the ladder. This opcode is
         * used as part of the character's entity, rather than in a separate
         * entity, as with a LINE. A LINE is used to set the start point of
         * the ladder on the walkmesh. When this LINE is crossed by the
         * player, a script in the LINE then uses a PREQ (or one of its
         * variants), calling the script in the party leader that defines the
         * LADER, causing the character to switch to 'climbing mode'. To set
         * up a two-way ladder, two LINEs are used at either end, with
         * different values for the LADER arguments, such as differing end
         * points. If this opcode is used as part of a non-playable character
         * entity, the NPC object will automatically climb from the start to
         * the end point without need for player interaction.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         * @param entity[in] The name of the entity.
         */
        void ProcessLADER(CodeGenerator* code_gen, const std::string& entity);

        void ProcessSOLID(CodeGenerator* code_gen, const std::string& entity);

        /**
         * Processes an OFST opcode.
         *
         * Opcode: 0xC3
         * Short name: OFST
         * Long name: Offset Object
         *
         * Memory layout (8 bytes)
         * |0xC3|B1/B2|B3/B4|T|X|Y|Z|S|
         *
         * Arguments:
         * - const Bit[4] B1: Bank to retrieve X offset, or zero if X is
         * specified as a literal.
         * - const Bit[4] B2: Bank to retrieve Y offset, or zero if Y is
         * specified as a literal.
         * - const Bit[4] B3: Bank to retrieve Z offset, or zero if Z is
         * specified as a literal.
         * - const Bit[4] B4: Bank to retrieve speed, or zero if S is
         * specified as a literal.
         * - const UByte T: Type of movement.
         * - const Short X: X offset amount, relative to current position, or
         * address to find X offset, if B1 is non-zero.
         * - const Short Y: Y offset amount, relative to current position, or
         * address to find Y offset, if B2 is non-zero.
         * - const Short Z: Z offset amount, relative to current position, or
         * address to find Z offset, if B3 is non-zero.
         * - const UShort S: Speed of the offset movement, if type is
         * non-zero, or address to find speed, if B4 is non-zero.
         *
         * Offsets the field object, belonging to the entity whose script this
         * opcode resides in, by a certain amount. After being offset, the
         * character continues to be constrained in movement as defined by the
         * walkmesh's shape, but at a certain distance away from the normal
         * walkmesh position. Other field objects are unaffected, and their
         * position or movements are maintained on the walkmesh's original
         * position. If B1, B2, B3 or B4 is non-zero, then the value for that
         * particular component is taken from memory using the corresponding
         * bank and address specified, rather than as a literal value. Both
         * retrieved values and literals can be used for different components.
         * If using T, X, Y or S as addresses, the lower byte should hold the
         * address whilst the higher byte should be zero. The amount to offset
         * is specified relative to the current position. If Type is
         * specified, the object moves gradually from its current point to the
         * offset position; this can be used to simulate movements such as
         * elevators. Any type outside the range in the table will cause the
         * offset not to occur. If the object is set to move gradually, then
         * the speed of offset can be set; the greater the number, the slower
         * the object moves to its target offset. Script execution may also be
         * halted until the gradual offset has been completed. For this, see
         * OFSTW.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         * @param entity[in] The entity name.
         */
        void ProcessOFST(CodeGenerator* codegen, const std::string& entity);
};
