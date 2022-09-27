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
#include <string>
#include <vector>
#include "sudm.h"

namespace FF7{

    /**
     * Represents the FF7 Field engine.
     */
    class FF7FieldEngine : public Engine{

        public:

            /**
             * FF7FieldEngine constructor.
             *
             * Generates a FF7FieldEngine.
             *
             * @param formatter[in] The formatter to be used by the engine.
             * @param scriptName The script name.
             */
            FF7FieldEngine(SUDM::IScriptFormatter& formatter, std::string scriptName) :
              mFormatter(formatter), mScriptName(scriptName)
            {setOutputStackEffect(false);}

            /**
             * Destructor.
             */
            FF7FieldEngine(const FF7FieldEngine&) = delete;

            /**
             * Destructor.
             */
            FF7FieldEngine& operator = (const FF7FieldEngine&) = delete;

            /**
             * Represents an entity.
             *
             * An entity can be almost anything in a field map: the playable
             * character, an NPC, an item, a line...
             */
            class Entity{
                public:

                    /**
                     * Entity constructor.
                     *
                     * It doesn't initialize any of the fields.
                     */
                    Entity() = default;

                    /**
                     * Entity constructor.
                     *
                     * Instantiates an entity with a name.
                     *
                     * @param name[in] Entity name.
                     */
                    Entity(const std::string& name, size_t index):
                      mName(name), index_(index), is_line_(false)
                    {}

                    /**
                     * Retrieves the entity name.
                     *
                     * @return The entity name.
                     */
                    std::string Name() const{return mName;}

                    /**
                     * Retrieves the entity index.
                     *
                     * The index is the one at which appears in the original
                     * game script.
                     *
                     * @return The entity index.
                     */
                    size_t GetIndex() const{return index_;}

                    /**
                     * Retrieves a function.
                     *
                     * Retrieves the name of a function from it's index.
                     *
                     * @param index[in] Function index.
                     * @return Function name.
                     * @throws InternalDecompilerError if there is no function
                     * with the specified index.
                     * @todo What is a function here? An Opcode?
                     */
                    std::string FunctionByIndex(size_t index) const{
                        auto it = mFunctions.find(index);
                        if (it == std::end(mFunctions)) throw InternalDecompilerError();
                        return it->second;
                    }

                    /**
                     * Adds a function to the engine.
                     *
                     * Must be added by name and index.
                     *
                     * @param name[in] Function name. If the entity is a line,
                     * the name will be overridden.
                     * @param index[in] Function index.
                     * @todo What is a function here? An Opcode?
                     */
                    void AddFunction(const std::string& name, size_t index){
                        mFunctions[index] = name;
                    }

                    /**
                     * Marks the entity as a line.
                     *
                     * @param line[in] True to mark the entity as a line,
                     * false to unmark it.
                     * @param point_a[in] First point of the line. Can be null
                     * if line is false.
                     * @param point_b[in] Second point of the line. Can be
                     * null if line is false.
                     */
                    void MarkAsLine(
                      bool line, std::vector<float> point_a, std::vector<float> point_b
                    ){
                        is_line_ = line;
                        point_a_.clear();
                        point_b_.clear();
                        if (line){
                            if (point_a.size() >= 3 && point_b.size() >= 3){
                                point_a_.push_back(point_a[0]);
                                point_a_.push_back(point_a[1]);
                                point_a_.push_back(point_a[2]);
                                point_b_.push_back(point_b[0]);
                                point_b_.push_back(point_b[1]);
                                point_b_.push_back(point_b[2]);
                            }
                            // TODO: Notify on else.
                        }
                        // TODO: These are not getting to the final script.
                        // Maybe this can be removed?
                        AddFunction("on_enter_line", 1);
                        AddFunction("on_move_to_line", 2);
                        AddFunction("on_cross_line", 3);
                        AddFunction("on_leave_line", 4);
                    }

                    /**
                     * Checks if the entity is a line.
                     *
                     * Note that an entity is not considered to be a line
                     * until a function has been found containing the opcode
                     * LINE and {@see MarkAsLine} has been called.
                     *
                     * @return true if the entity is a line.
                     */
                    bool IsLine(){return is_line_;}

                    /**
                     * Retrieves the first point of the line entity.
                     *
                     * If the entity is not a line, the behavior is undefined.
                     *
                     * @return The first point of the line entity.
                     */
                    std::vector<float> GetLinePointA(){return point_a_;}

                    /**
                     * Retrieves the second point of the line entity.
                     *
                     * If the entity is not a line, the behavior is undefined.
                     *
                     * @return The second point of the line entity.
                     */
                    std::vector<float> GetLinePointB(){return point_b_;}



                private:

                    /**
                     * Entity name.
                     */
                    std::string mName;

                    /**
                     * Entity index.
                     */
                    size_t index_;

                    /**
                     * Function list.
                     * @todo What is a function here? An Opcode?
                     */
                    std::map< size_t, std::string > mFunctions;

                    /**
                     * Indicates if the entity is a line.
                     */
                    bool is_line_;

                    /**
                     * The first point of a line entity.
                     *
                     * If the entity is not a line, it may not be initializer.
                     */
                    std::vector<float> point_a_;

                    /**
                     * The second point of a line entity.
                     *
                     * If the entity is not a line, it may not be initializer.
                     */
                    std::vector<float> point_b_;
            };

            /**
             * Retrieves the dissasembler.
             *
             * @param insts[in] List of instructions.
             * @param raw_script_data[in] Script data, raw format.
             * @return Pointer to the dissasembler.
             * @todo Understand and document properly.
             */
            virtual std::unique_ptr<Disassembler> getDisassembler(
              InstVec &insts, const std::vector<unsigned char>& raw_script_data
            ) override;

            /**
             * Retrieves the dissasembler.
             *
             * @param insts[in] List of instructions.
             * @return Pointer to the dissasembler.
             * @todo Understand and document properly.
             */
            virtual std::unique_ptr<Disassembler> getDisassembler(InstVec &insts) override;

            /**
             * Retrieves the code generator.
             *
             * @param insts[in] List of instructions.
             * @param output[in] Pointer to the output (file, stream...).
             * @return Pointer to the generator.
             * @todo Understand and document properly.
             */
            virtual std::unique_ptr<CodeGenerator> getCodeGenerator(
              const InstVec& insts, std::ostream &output
            ) override;

            /**
             * Postprocessing actions to apply to the scripts.
             *
             * @param insts[in] Instruction list.
             * @param g[in] Code graph.
             * @todo Understand and document properly.
             * @todo What is the graph used to?
             */
            virtual void postCFG(InstVec &insts, Graph g) override;

            /**
             * Indicates if instructions are purely grouped.
             *
             * @return True if instructions are purely grouped.
             * @todo What is pure grouping?
             */
            virtual bool usePureGrouping() const override{return false;}

            /**
             * Retrieves all entities in the map.
             *
             * @return A map of entities, with the name and index.
             */
            std::map<std::string, int> GetEntities() const;

            /**
             * Retrieves all non-line entities in the map.
             *
             * @return A list of non-line entities.
             */
            std::vector<SUDM::FF7::Field::FieldEntity> GetEntityList() const;

            /**
             * Retrieves all line entities in the map.
             *
             * @param A list of line entities.
             */
            std::vector<SUDM::FF7::Field::Line> GetLineList() const;

            /**
             * Retrieves all entities in the map.
             *
             * @return A map of entities, with the name and index.
             */
            std::map<size_t, Entity> GetEntityIndexMap() const{return mEntityIndexMap;}

            /**
             * Adds a function to an entity.
             *
             * @param entity_name Name of the entity.
             * @param entity_index Index of the entity.
             * @param func_name Name of the function.
             * @param func_index Index of the function.
             */
            void AddEntityFunction(
              const std::string& entity_name, size_t entity_index,
              const std::string& func_name, size_t func_index
            );

            /**
             * Marks an entity as a line.
             *
             * @param entity_index Index of the entity.
             * @param line[in] True to mark the entity as a line, false to
             * unmark it.
             * @param point_a[in] First point of the line. Can be null if line
             * is false.
             * @param point_b[in] Second point of the line. Can be null if
             * line is false.
             */
            void MarkEntityAsLine(
                size_t entity_index, bool line,
                std::vector<float> point_a, std::vector<float> point_b
            );

            /**
             * Checks if an entity has been marked as a line.
             *
             * @param entity_index[in] Index of the entity to check.
             * @return True if the entity is a line. False if it isn't, or if
             * there is no such entity.
             */
            bool EntityIsLine(size_t entity_index);

            /**
             * Retrieves an entity.
             *
             * @param index[in] Index of the entity to retrieve.
             * @throws InternalDecompilerError if there is no entity at the
             * specified index.
             */
            const Entity& EntityByIndex(size_t index) const{
                auto it = mEntityIndexMap.find(index);
                if (it == std::end(mEntityIndexMap)) throw InternalDecompilerError();
                return it->second;
            }

            /**
             * Retrieves the scale factor for the map.
             *
             * @return Map scale factor.
             */
            float ScaleFactor() const {return mScaleFactor;}

            /**
             * Retrieves the script name.
             *
             * @return The script name.
             */
            const std::string& ScriptName() const {return mScriptName;}

        private:

            /**
             * Removes extraneous return statements.
             *
             * Usefull for scripts that only contain one one return
             * statement.
             *
             * @param insts[in|out] List of instructions to proccess.
             * Extraneous return statements will be deleted from the
             * instructions.
             * @param g[in] Code graph. Unused.
             */
            void RemoveExtraneousReturnStatements(InstVec& insts, Graph g);

            /**
             * Removes trailing infinite loops.
             *
             * In FF7 some scripts ends with an infinite loop to keep it
             * alive. in VGears this isn't required, and can cause infinite
             * loops, so they can be removed.
             *
             * @param insts[in|out] List of instructions to proccess. Trailing
             * infinite loops will be deleted from the instructions.
             * @param g[in] Code graph.
             * @todo What is the graph used to?
             */
            void RemoveTrailingInfiniteLoops(InstVec& insts, Graph g);

            /**
             * Tries to detect scripts with trailing infinite loops.
             *
             * In FF7 some scripts ends with an infinite loop to keep it
             * alive. in VGears this isn't required, and can cause infinite
             * loops, so they can be removed. This function marks them, so
             * they can be deleted with
             * @{see FF7FieldEngine::RemoveTrailingInfiniteLoops}.
             */
            void MarkInfiniteLoopGroups(InstVec& insts, Graph g);

            /**
             * The script formatter.
             */
            SUDM::IScriptFormatter& mFormatter;

            /**
             * The entity index map for the field.
             */
            std::map<size_t, Entity> mEntityIndexMap;

            /**
             * The map scale factor.
             */
            float mScaleFactor = 1.0f;

            /**
             * The script name.
             */
            std::string mScriptName;
    };

    /**
     * An unconditional map jump instruction.
     */
    class FF7UncondJumpInstruction : public UncondJumpInstruction{

        public:

            // TODO: Make private and add accessors.
            /**
             * Whether or not this is really a call to a script function.
             */
            bool _isCall;

            /**
             * Constructor.
             */
            FF7UncondJumpInstruction() : _isCall(false) {}

            /**
             * Indicates if the instruction is a function call.
             *
             * @return true if the instruction is a function call, false if not.
             */
            virtual bool isFuncCall() const;

            /**
             * Indicates if the instruction is an unconditional jump.
             *
             * @return true if the instruction is an unconditional jump, false
             * if it's not.
             */
            virtual bool isUncondJump() const;

            /**
             * Retrieves the destination address of the jump.
             *
             * @return The offset (number of bytes) to jump from the beginning
             * of the instruction.
             */
            virtual uint32 getDestAddress() const;

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine.
             * @param code_gen[in] Code generator.
             * @todo Func and engine are unused?
             * @todo Understand and document properly.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

            /**
             * Prints the instruction
             *
             * @param output The stram to print the instruction to.
             * @todo Understand and document properly.
             */
            virtual std::ostream& print(std::ostream &output) const override;
    };

    /**
     * A conditional map jump instruction.
     */
    class FF7CondJumpInstruction : public CondJumpInstruction{

        public:

            /**
             * Processes a conditional jump instruction.
             *
             * Checks if the condition is a function or a comparison, and
             * adds the function to the stack.
             *
             * @param function[in] Function to process. Unused.
             * @param stack[out] Function stack. The Instruction will be added
             * here.
             * @param engine[in] Engine. Unused.
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

            /**
             * Retrieves the destination address of the jump.
             *
             * @return The offset (number of bytes) to jump from the beginning
             * of the instruction.
             */
            virtual uint32 getDestAddress() const override;

            /**
             * Prints the instruction
             *
             * @param output The stram to print the instruction to.
             * @todo Understand and document properly.
             */
            virtual std::ostream& print(std::ostream &output) const override;
        };

    /**
     * A script flow control instruction.
     */
    class FF7ControlFlowInstruction : public KernelCallInstruction{

        public:

            /**
             * Create a FF7ControlFlowInstruction.
             *
             * @return Pointer to the newly created instruction.
             */
            static InstPtr Create(){return new FF7ControlFlowInstruction();}

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine.
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            /**
             * Processes a REQ command.
             *
             * Opcode: 0x01
             * Short name: REQ
             * Long name: Request remote execution (asynchronous,
             *            non-guaranteed)
             *
             * Memory layout
             *   0x01
             *   E
             *   P/F
             *
             * Arguments
             *
             * const UByte E: The ID of the target entity.
             * const Bit[3] P: The priority at which we want to execute the
             *                 remote script (high 3 bits of byte).
             * const Bit[5] F: The ID of the specific member function of E to
             *                 be executed (low 5 bits of byte).
             *
             * Requests that a remote entity executes one of its member
             * functions at a specified priority. The request is asynchronous
             * and returns immediately without waiting for the remote
             * execution to start or finish. If the specified priority is
             * already busy executing, the request will fail silently.
             *
             * @param codegen[in|out] Code generator. Output lines are
             * appended to it.
             * @param engine[in] The engine instance to fetch entities.
             */
            void processREQ(CodeGenerator* code_gen, const FF7FieldEngine& engine);

            /**
             * Processes a REQSW command.
             *
             * Opcode: 0x02
             * Short name: REQSW
             * Long name: Request remote execution (asynchronous execution,
             *            guaranteed)
             *
             * Memory layout
             *   0x02
             *   E
             *   P/F
             *
             * Arguments
             *
             * const UByte E: The ID of the target entity.
             * const Bit[3] P: The priority at which we want to execute the
             *                 remote script (high 3 bits of byte).
             * const Bit[5] F: The ID of the specific member function of E to
             *                 be executed (low 5 bits of byte).
             *
             * Requests that a remote entity executes one of its member
             * functions at a specified priority. If the specified priority is
             * already busy executing, the request will block until it becomes
             * available and only then return. The remote execution is still
             * carried out asynchronously, with no notification of completion.
             *
             * @param codegen[in|out] Code generator. Output lines are
             * appended to it.
             * @param engine[in] The engine instance to fetch entities.
             */
            void processREQSW(CodeGenerator* code_gen, const FF7FieldEngine& engine);

            /**
             * Processes a REQEW command.
             *
             * Opcode: 0x03
             * Short name: REQEW
             * Long name: Request remote execution (synchronous, guaranteed)
             *
             * Memory layout
             *   0x03
             *   E
             *   P/F
             *
             * Arguments
             *
             * const UByte E: The ID of the target entity.
             * const Bit[3] P: The priority at which we want to execute the
             *                 remote script (high 3 bits of byte).
             * const Bit[5] F: The ID of the specific member function of E to
             *                 be executed (low 5 bits of byte).
             *
             * Requests that a remote entity executes one of its member
             * functions at a specified priority. The request will block until
             * remote execution has finished before returning.
             *
             * @param codegen[in|out] Code generator. Output lines are
             * appended to it.
             * @param engine[in] The engine instance to fetch entities.
             */
            void processREQEW(CodeGenerator* code_gen, const FF7FieldEngine& engine);

            /**
             * Processes a RETTO command.
             *
             * Opcode: 0x07
             * Short name: RETTO
             * Long name: Return To
             *
             * Memory layout
             *   0x07
             *   P/F
             *
             * Arguments
             *   const Bit[3] P: The priority at which we want to execute the
             *                   remote script (high 3 bits of byte).
             *   const Bit[5] F: The ID of the specific member function of the
             *                   current entity to be executed to (low 5 bits
             *                   of byte).
             *
             * Stops the active script loop for this entity and also any
             * script loops (except the main) that are queuing to be executed
             * after the current script. This is essentially the same as
             * adding a RET onto each of the active / queued scripts next
             * execution position and returning the current op index to index
             * for each script. Then the script control is passed to the
             * script F within the current entity with the priority P.
             *
             * @param codegen[in|out] Code generator. Output lines are
             * appended to it.
             */
            void processRETTO(CodeGenerator* code_gen);

            /**
             * Processes a WAIT command.
             *
             * Opcode: 0x24
             * Short name: WAIT
             * Long name: Wait
             *
             * Memory layout
             *   0x24
             *   A
             *
             * Arguments
             *   const UShort A: Amount (number of frames) to wait.
             *
             * Pauses current script execution for a specific amount of time.
             * Rather than a specific time value in milliseconds/seconds,
             * the amount specifies the number of frames that must be drawn
             * before execution resumes. Since the game runs at 30fps,
             * WAIT(0x1E) (or WAIT(30) in decimal) will pause script execution
             * for 1 second, WAIT(0x96) will pause for 5 seconds, and so on.
             *
             * @param codegen[in|out] Code generator. Output lines are
             * appended to it.
             */
            void processWAIT(CodeGenerator* code_gen);
    };

    /**
     * A module instruction.
     */
    class FF7ModuleInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine. Unused.
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            /**
             * Processes a BATTLE opcode.
             *
             * Opcode: 0x70
             * Short name: BATTLE
             * Long name: Start battle
             *
             * Memory layout
             *   0x70
             *   B
             *   N
             *   N
             *
             * Arguments
             *   const UByte B: Bank (16-bit) to retrieve the address of the
             *                  battle ID, or zero if it is given as a literal
             *                  value.
             *   const UWord N: Battle ID, or address to find ID if B is
             *                  non-zero.
             *
             * This launches the battle module with whatever battle number is
             * used in the argument, or the value retrieved from memory location
             * N if B is non-zero. Battle 1, 2, and 999 (0x03E7) are debug
             * battles.
             *
             * @param codegen[in|out] Code generator. Output lines are appended
             * to it.
             */
            void processBATTLE(CodeGenerator* code_gen);

            /**
             * Processes a BTLON opcode.
             *
             * Opcode: 0x71
             * Short name: BTLON
             * Long name: Battle switch
             *
             * Memory layout
             *   0x71
             *   S
             *
             * Arguments
             *   const UByte S: Switch battles on/off (0/1, respectively).
             *
             * Turns random encounters on or off for this field. Note that if
             * a field does not have any Encounter Data set in its field file,
             * battles will not occur regardless of the argument passed with
             * this opcode.
             *
             * @param codegen[in|out] Code generator. Output lines are
             * appended to it.
             */
            void processBTLON(CodeGenerator* code_gen);

            /**
             * Processes a MAPJUMP opcode.
             *
             * Opcode: 0x60
             * Short name: BTLON
             * Long name: Change Field
             *
             * Memory layout
             *   0x60
             *   I
             *   I
             *   X
             *   X
             *   Y
             *   Y
             *   Z
             *   Z
             *   D
             *
             * Arguments
             *   const UShort I: Field ID of the map to jump to.
             *   const Short X: X-coordinate of the player on the next field.
             *   const Short Y: Y-coordinate of the player on the next field.
             *   const Short Z: Z-coordinate of the player on the next field.
             *   const UByte D: Direction the character will be facing on the
             *                  next field, in the standard game format.
             *
             * Switches fields to the one indicated by I, and places the
             * character at the coordinates and direction specified. This is
             * an alternative to using a gateway, and can complement their
             * usage as it allows for more than 12 gateways by simulating
             * their behavior through a LINE which, when crossed, executes a
             * MAPJUMP.
             *
             * @param codegen[in|out] Code generator. Output lines are
             * appended to it.
             * @param func[in] Function
             * @todo What is func for?
             */
            void processMAPJUMP(CodeGenerator* code_gen, Function& func);
    };

    /**
     * A math instruction.
     */
    class FF7MathInstruction : public StoreInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine. Unused.
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            void processSaturatedPLUS(CodeGenerator* code_gen);
            void processSaturatedPLUS2(CodeGenerator* code_gen);
            void processSaturatedMINUS(CodeGenerator* code_gen);
            void processSaturatedMINUS2(CodeGenerator* code_gen);
            void processSaturatedINC(CodeGenerator* code_gen);
            void processSaturatedINC2(CodeGenerator* code_gen);
            void processSaturatedDEC(CodeGenerator* code_gen);
            void processSaturatedDEC2(CodeGenerator* code_gen);
            void processRDMSD(CodeGenerator* code_gen);
            void processSETBYTE_SETWORD(CodeGenerator* code_gen);

            /**
             * Processes a BITON opcode.
             *
             * Opcode: 0x82
             * Short name: BITON
             * Long name: Set Bit
             *
             * Memory layout (4 bytes)
             * |0x82|D/S|A|B|
             *
             * Arguments
             * - const Bit[4] D: Destination bank.
             * - const Bit[4] S: Source bank.
             * - const UByte A: Destination address.
             * - const UByte Bit: The number of the bit to turn on.
             *
             * Sets the nth bit in the "A" location, where n is a number
             * between 0-7 supplied in B. A value of zero in B will set the
             * least significant bit. If the Source Bank is 0 then the bit to
             * be set is taken from "Bit". If the Source Bank is an 8 bit
             * bank, then the bit is the address in that bank where the
             * operand is.
             *
             * @param code_gen[in|out] Code generator. Output lines are
             * appended to it.
             */
            void processBITON(CodeGenerator* code_gen);

            /**
             * Processes a BITON opcode.
             *
             * Opcode: 0x83
             * Short name: BITOFF
             * Long name: Reset Bit
             *
             * Memory layout (4 bytes)
             * |0x83|D/S|A|B|
             *
             * Arguments
             * - const Bit[4] D: Destination bank.
             * - const Bit[4] S: Source bank.
             * - const UByte A: Destination address.
             * - const UByte Bit: The number of the bit to turn off.
             *
             * Sets the nth bit in the "A" location, where n is a number
             * between 0-7 supplied in B. A value of zero in B will reset the
             * least significant bit. If the Source Bank is 0 then the bit to
             * be set is taken from "Bit". If the Source Bank is an 8 bit
             * bank, then the bit is the address in that bank where the
             * operand is.
             *
             * @param code_gen[in|out] Code generator. Output lines are
             * appended to it.
             */
            void processBITOFF(CodeGenerator* code_gen);
            void processPLUSx_MINUSx(CodeGenerator* code_gen, const std::string& op);
            void processINCx_DECx(CodeGenerator* code_gen, const std::string& op);
            void processRANDOM(CodeGenerator* code_gen);
    };

    /**
     * A window instruction.
     */
    class FF7WindowInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine.
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            void processMESSAGE(CodeGenerator* code_gen, const std::string& script_name);
            void processMPNAM(CodeGenerator* code_gen);
            void processMENU2(CodeGenerator* code_gen);
            void processWINDOW(CodeGenerator* code_gen);
            void processWCLSE(CodeGenerator* code_gen);
    };

    /**
     * A party instruction
     */
    class FF7PartyInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine. Unused
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            void processSTITM(CodeGenerator* code_gen);
            void processPRTYE(CodeGenerator* code_gen);
    };

    /**
     * A model instruction.
     */
    class FF7ModelInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine.
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
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
             * Causes seperated party characters that have previously been
             * SPLIT onto the field, to be joined back together again; that
             * is, only the party leader becomes visible on the field. This
             * should be called if a previous SPLIT has completed (the party
             * members have finished speaking, or performing their actions,
             * for example). As with SPLIT, the speed of the join is
             * specified, from a scale of 1 (almost instant) to FF (very slow
             * walk), and must be non-zero. In contrast to most MOVE related
             * op codes, the speed is this setting is actually the total
             * number of frames required. Depending on the distance from the
             * player character and the number of frames required, the entity
             * plays a run or walk animation. Also, all characters take the
             * same time irrespective of distance. Calling JOIN without having
             * previously SPLIT the characters will cause the party members to
             * appear at the walkmesh origin and attempt to JOIN from there.
             * This is not normally the required behaviour and should be
             * avoided.
             *
             * @param code_gen The code generator.
             */
            void processJOIN(CodeGenerator* code_gen);

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
             * - const Short XA: X-coordinate of the second character in the
             * party after the split, or address for the value if B1 is
             * non-zero.
             * - const Short YA: Y-coordinate of the second character in the
             * party after the split, or address for the value if B2 is
             * non-zero.
             * - const UByte DA: Direction the second character faces after the
             * split, or address for the value if B3 is non-zero.
             * - const Short XB: X-coordinate of the third character in the
             * party after the split, or address for the value if B4 is
             * non-zero.
             * - const Short YB: Y-coordinate of the third character in the
             * party after the split, or address for the value if B5 is
             * non-zero.
             * - const UByte DB: Direction the third character faces after the
             * split, or address for the value if B6 is non-zero.
             * - const UByte S: Speed that the characters split.
             *
             * Causes the common 'split effect' whereby the second and third
             * characters in the current party 'come out' from the party
             * leader. That is, they become visible in the field, starting
             * from the center of the party leader, and move out to the
             * coordinates specified in the argument list. This is commonly
             * used when the other characters in the current party have an
             * action or dialog to perform and must be individually visible in
             * the field. As well as specifying final coordinates for the two
             * other party characters, the directions each character faces
             * after the split are specified as a byte, using the common
             * direction values found throughout the game. Speed is also given
             * and is used to specify the rate at which the characters leave
             * the party leader, using a scale from 1 (almost instant) to FF
             * (extremely slow walk); this must be non-zero. In contrast to
             * most MOVE related op codes, the speed is this setting is
             * actually the total number of frames required. Depending on the
             * distance from the player character and the number of frames
             * required, the entity plays a run or walk animation. Also, all
             * characters take the same time irrespective of distance.
             *
             * @param code_gen The code generator.
             */
            void processSPLIT(CodeGenerator* code_gen);
            void processTLKON(CodeGenerator* code_gen, const std::string& entity);
            void processPC(CodeGenerator* code_gen, const std::string& entity);
            void processCHAR(CodeGenerator* code_gen, const std::string& entity);
            void processDFANM(CodeGenerator* code_gen, const std::string& entity, int char_id);
            void processANIME1(CodeGenerator* code_gen, const std::string& entity, int char_id);
            void processVISI(CodeGenerator* code_gen, const std::string& entity);
            void processXYZI(CodeGenerator* code_gen, const std::string& entity);
            void processMOVE(CodeGenerator* code_gen, const std::string& entity);
            void processMSPED(CodeGenerator* code_gen, const std::string& entity);
            void processDIR(CodeGenerator* code_gen, const std::string& entity);
            void processTURNGEN(CodeGenerator* code_gen, const std::string& entity);
            void processGETAI(CodeGenerator* code_gen, const FF7FieldEngine& engine);
            void processANIM_2(CodeGenerator* code_gen, const std::string& entity, int char_id);
            void processCANIM2(CodeGenerator* code_gen, const std::string& entity, int char_id);
            void processCANM_2(CodeGenerator* code_gen, const std::string& entity, int char_id);
            void processCC(CodeGenerator* code_gen, const FF7FieldEngine& engine);

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
             * - const Short X: X-coordinate of the target to jump to, or
             * lower byte specifying address if B1 is non-zero.
             * - const Short Y: Y-coordinate of the target to jump to, or
             * lower byte specifying address if B2 is non-zero.
             * - const Short I: Triangle ID of the target to jump to, or
             * lower byte specifying address if B3 is non-zero.
             * - const UShort Steps: Steps in jump. Must be non-zero if a
             * literal value. Alternatively, lower byte specifies address if
             * B4 is non-zero.
             *
             * Causes the character to jump to the specified point and
             * triangle ID, with the jump curve peaking at a height which is
             * increased by using a larger value for the H argument. In
             * addition, the larger the number, the longer the jump will take
             * to complete. A "normal" value is around 0x15, 0x01 is fast and
             * instantaneous; the argument must not be zero or the game will
             * crash. Whilst this is an unsigned two-byte number, a large
             * value (beyond around 0x60) will not only cause a vast jump
             * height, but also cause the screen to scroll erratically (the
             * larger the number, the more erratic).
             * Main update function go through all entity with JUMP state and
             * if stage is 0 it calculates final Z point according to triangle
             * id. It sets current coords as start coords. The main thing this
             * function does is set B coefficient for later calculation. It
             * defines as follows:
             *   B = (Z_final - Z_start) / steps - steps * 1.45;
             * Then it set current step to 0 and stage to 1. On next update
             * other part of function works. It's calculate real position.
             * First it increment current step number. Then it calculate X and
             * Y. They change linear so nothing interesting here. The Z
             * calculation is as follows:
             *   Z_current = - step^2 * 1.45 + step * B + Z_start;
             * If current substep equal number of steps then we set current
             * triangle to final triangle and set stage to 2. Which finalizes
             * the routine on next opcode call. Neither animation nor sound is
             * specified in this opcode. An animation is played by using an
             * animation opcode such as DFANM, and a SOUND played, before the
             * jump.
             *
             * @param code_gen[in] The code generator.
             * @param entity[in] The name of the entity.
             */
            void processJUMP(CodeGenerator* code_gen, const std::string& entity);

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
             * - const UByte I: Address to store the ID of the walkmesh
             * triangle the object is standing on.
             *
             * Retrieves the coordinates of the field object that the entity,
             * whose ID specified in A, is associated with. This opcode uses
             * an entity ID, not a field object offset; as such, if an entity
             * ID is given that does not have a field object, this opcode will
             * store zero in each of the four address specified.
             */
            void processAXYZI(CodeGenerator* code_gen);

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
             * - const Bit[4] B1: Bank to retrieve X-coordinate, or zero if X
             * is specified as a literal value.
             * - const Bit[4] B2: Bank to retrieve Y-coordinate, or zero if Y
             * is specified as a literal value.
             * - const Bit[4] B3: Bank to retrieve Z-coordinate, or zero if Z
             * is specified as a literal value.
             * - const Bit[4] B4: Bank to retrieve ID, or zero if I is
             * specified as a literal value.
             * - const Short X: X-coordinate of the end of the ladder, or
             * address to find X-coordinate if B1 is non-zero.
             * - const Short Y: Y-coordinate of the end of the ladder, or
             * address to find Y-coordinate if B2 is non-zero.
             * - const Short Z: Z-coordinate of the end of the ladder, or
             * address to find Z-coordinate if B3 is non-zero.
             * - const UShort I: ID of the walkmesh triangle found at the end
             * of the ladder, or address to find ID if B4 is non-zero.
             * - const UByte K: The keys used to move the character on the
             * ladder.
             * - const UByte A: Animation ID for the field object's movement
             * animation.
             * - const UByte D: Direction the character faces when climbing
             * the ladder.
             * - const UByte S: Speed of the animation whilst climbing the
             * ladder.
             *
             * Causes the character to climb a ladder; that is, switching from
             * standard walkmesh movement, to climbing along a line connecting
             * two points on the walkmesh. If B1, B2, B3 or B4 is non-zero,
             * then the value for that particular component is taken from
             * memory using the corresponding bank and address specified,
             * rather than as a literal value. Both retrieved values and
             * literals can be used for different components. If using X, Y, Z
             * or I as addresses, the lower byte should hold the address
             * whilst the higher byte should be zero. The coordinates specify
             * the end-point of the ladder; the current position of the
             * character is used as the start point. The ID of the walkmesh
             * triangle must be specified; this is the triangle the character
             * will step onto after reaching the end point of the ladder. The
             * K value specifies the keys used to move the character across
             * the ladder; keys outside the range found in the table will
             * cause unpredictable behaviour. The animation ID specifies an
             * offset into the field object's animation list; this animation
             * is played at the speed specified by S whilst the character
             * climbs. Finally, the D argument is a direction value in the
             * game's standard direction format, which orients the character
             * on the ladder. This opcode is used as part of the character's
             * entity, rather than in a seperate entity, as with a LINE. A
             * LINE is used to set the start point of the ladder on the
             * walkmesh. When this LINE is crossed by the player, a script in
             * the LINE then uses a PREQ (or one of its variants), calling the
             * script in the party leader that defines the LADER, causing the
             * character to switch to 'climbing mode'. To set up a two-way
             * ladder, two LINEs are used at either end, with different values
             * for the LADER arguments, such as differing end points. If this
             * opcode is used as part of a non-playable character entity, the
             * NPC object will automatically climb from the start to the end
             * point without need for player interaction.
             *
             * @param code_gen[in] The code generator.
             * @param entity[in] The name of the entity.
             */
            void processLADER(CodeGenerator* code_gen, const std::string& entity);

            void processSOLID(CodeGenerator* code_gen, const std::string& entity);

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
             * - const Short X: X offset amount, relative to current position,
             * or address to find X offset, if B1 is non-zero.
             * - const Short Y: Y offset amount, relative to current position,
             * or address to find Y offset, if B2 is non-zero.
             * - const Short Z: Z offset amount, relative to current position,
             * or address to find Z offset, if B3 is non-zero.
             * - const UShort S: Speed of the offset movement, if type is
             * non-zero, or address to find speed, if B4 is non-zero.
             *
             * Offsets the field object, belonging to the entity whose script
             * this opcode resides in, by a certain amount. After being
             * offset, the character continues to be constrained in movement
             * as defined by the walkmesh's shape, but at a certain distance
             * away from the normal walkmesh position. Other field objects are
             * unaffected, and their position or movements are maintained on
             * the walkmesh's original position. If B1, B2, B3 or B4 is
             * non-zero, then the value for that particular component is taken
             * from memory using the corresponding bank and address specified,
             * rather than as a literal value. Both retrieved values and
             * literals can be used for different components. If using T, X, Y
             * or S as addresses, the lower byte should hold the address
             * whilst the higher byte should be zero. The amount to offset is
             * specified relative to the current position. If Type is
             * specified, the object moves gradually from its current point to
             * the offset position; this can be used to simulate movements
             * such as elevators. Any type outside the range in the table will
             * cause the offset not to occur. If the object is set to move
             * gradually, then the speed of offset can be set; the greater the
             * number, the slower the object moves to its target offset.
             * Script execution may also be halted until the gradual offset
             * has been completed. For this, see OFSTW.
             *
             * @param codegen[in] The code generator.
             * @param entity[in] The entity name.
             */
            void processOFST(CodeGenerator* codegen, const std::string& entity);
    };

    /**
     * A walkmesh instruction.
     */
    class FF7WalkmeshInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine. Unused.
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            void processUC(CodeGenerator* code_gen);

            /**
             * Processes a LINE opcode.
             *
             * Opcode: 0xD0
             * Short name: LINE
             * Long name: Line definition
             *
             * Memory layout (7 bytes)
             * |0xD0|XA|YA|ZA|XB|YB|ZB|
             *
             * Arguments:
             * - const Short XA: X-coordinate of the first point of the line.
             * - const Short YA: Y-coordinate of the first point of the line.
             * - const Short ZA: Z-coordinate of the first point of the line.
             * - const Short XB: X-coordinate of the second point of the line.
             * - const Short YB: Y-coordinate of the second point of the line.
             * - const Short ZB: Z-coordinate of the second point of the line.
             *
             * Defines a line on the walkmesh that, when crossed by a playable
             * character, causes one of the entity's scripts to be executed.
             * These are similar to the triggers in Section 8. All the lines
             * in the current field can be turned on or off by using the LINON
             * opcode.
             *
             * There are generally 6 scripts (other than the init and main) if
             * the entity is a LINE.
             * - script index 2 -> S1 - [OK].
             * - script index 3 -> S2 - Move.
             * - script index 4 -> S3 - Move.
             * - script index 5 -> S4 - Go.
             * - script index 6 -> S5 - Go 1x.
             * - script index 7 -> S6 - Go away.
             *
             * @param code_gen The code generator.
             * @param entity[in] The entity name.
             */
            void processLINE(CodeGenerator* code_gen, const std::string& entity);
    };

    /**
     * A background instruction.
     */
    class FF7BackgroundInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine. Unused
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            void processBGON(CodeGenerator* code_gen);
            void processBGOFF(CodeGenerator* code_gen);
            void processBGCLR(CodeGenerator* code_gen);
            void processSTPAL(CodeGenerator* code_gen);
            void processLDPAL(CodeGenerator* code_gen);
            void processCPPAL(CodeGenerator* code_gen);
            void processADPAL(CodeGenerator* code_gen);
            void processMPPAL2(CodeGenerator* code_gen);
            void processSTPLS(CodeGenerator* code_gen);
            void processLDPLS(CodeGenerator* code_gen);
    };

    /**
     * A camera instruction.
     */
    class FF7CameraInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine. Unused
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            void processNFADE(CodeGenerator* code_gen);
            void processSCR2D(CodeGenerator* code_gen);
            void processSCR2DC(CodeGenerator* code_gen);
            void processFADE(CodeGenerator* code_gen);
    };

    /**
     * An audio or video (or both) instruction.
     */
    class FF7AudioVideoInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine. Unused
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            void processAKAO2(CodeGenerator* code_gen);
            void processMUSIC(CodeGenerator* code_gen);
            void processSOUND(CodeGenerator* code_gen);
            void processAKAO(CodeGenerator* code_gen);
            void processMULCK(CodeGenerator* code_gen);
            void processPMVIE(CodeGenerator* code_gen);
            void processMOVIE(CodeGenerator* code_gen);
            void processMVIEF(CodeGenerator* code_gen);
    };

    /**
     * An instructions that doesn't fall in any other category.
     */
    class FF7UncategorizedInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine. Unused.
             * @param code_gen[in] Code generator.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;
    };

    /**
     * An instruction that does nothing.
     */
    class FF7NoOperationInstruction : public Instruction{

        public:

            /**
             * Generates a instruction that does nothing.
             *
             * @return The generated instruction.
             */
            static InstPtr Create(){return new FF7NoOperationInstruction();}

            /**
             * Processes the instruction.
             *
             * It doesn't do anything.
             *
             * @param func[in] Function to process. Unused.
             * @param stack[out] Function stack. Unused.
             * @param engine[in] Engine. Unused.
             * @param code_gen[in] Code generator. Unused.
             */
            virtual void processInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;
    };
}
