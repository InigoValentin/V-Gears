/*
 * Q-Gears
 * Copyright (C) 2022 Q-Gears Team
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
            FF7FieldEngine(
              SUDM::IScriptFormatter& formatter, std::string scriptName
            ) : mFormatter(formatter), mScriptName(scriptName){
                setOutputStackEffect(false);
            }

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
                    Entity(const std::string& name): mName(name), is_line_(false){}

                    /**
                     * Retrieves the entity name.
                     *
                     * @return The entity name
                     */
                    std::string Name() const{
                        return mName;
                    }

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
                        if (it == std::end(mFunctions)){
                            throw InternalDecompilerError();
                        }
                        return it->second;
                    }

                    /**
                     * Adds a function to the engine.
                     *
                     * Must be added by name and index.
                     *
                     * @param name[in] Function name.
                     * @param index[in] Function index.
                     * @todo What is a function here? An Opcode?
                     */
                    void AddFunction(const std::string& name, size_t index){
                        mFunctions[index] = name;
                    }

                    /**
                     * Indicates if the entity is a line.
                     * @TODO: Make private.
                     */
                    bool is_line_;

                    /**
                     * The first point of the line.
                     * @TODO: Make private.
                     */
                    std::vector<float> point_a_;

                    /**
                     * The first point of the line.
                     * @TODO: Make private.
                     */
                    std::vector<float> point_b_;

                    float ax, ay, az, bx, by, bz;

                private:

                    /**
                     * Entity name.
                     */
                    std::string mName;

                    /**
                     * Function list.
                     * @todo What is a function here? An Opcode?
                     */
                    std::map< size_t, std::string > mFunctions;
            };

            /**
             * Retrieves the dissasembler.
             *
             * @param insts[in] List of instructions.
             * @param rawScriptData[in] Script data, raw format.
             * @return Pointer to the dissasembler.
             * @todo Understand and document properly.
             */
            virtual std::unique_ptr<Disassembler> getDisassembler(
              InstVec &insts, const std::vector<unsigned char>& rawScriptData
            ) override;

            /**
             * Retrieves the dissasembler.
             *
             * @param insts[in] List of instructions.
             * @return Pointer to the dissasembler.
             * @todo Understand and document properly.
             */
            virtual std::unique_ptr<Disassembler> getDisassembler(
              InstVec &insts
            ) override;

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
            std::map<size_t, Entity> GetEntityIndexMap() const{
                return mEntityIndexMap;
            }

            /**
             * Adds a function to an entity.
             *
             * @param entityName Name of the entity.
             * @param entityIndex Index of the entity.
             * @param functionName Name of the function.
             * @param functionIndex Index of the function.
             */
            void AddEntityFunction(
              const std::string& entityName, size_t entityIndex,
              const std::string& funcName, size_t funcIndex
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
             * Retrieves an entity.
             *
             * @param index[in] Index of the entity to retrieve.
             * @throws InternalDecompilerError if there is no entity at the
             * specified index.
             */
            const Entity& EntityByIndex(size_t index) const{
                auto it = mEntityIndexMap.find(index);
                if (it == std::end(mEntityIndexMap)){
                    throw InternalDecompilerError();
                }
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
            const std::string& ScriptName() const { return mScriptName; }

        private:

            /**
             * Removes extraneous return statements.
             *
             * Usefull for scripts that only contain one one return
             * statement.
             *
             * @param insts[in|out] List of instructions to proccess. Extraneous
             * return statements will be deleted from the instructions.
             * @param g[in] Code graph.
             * @todo What is the graph used to?
             */
            void RemoveExtraneousReturnStatements(InstVec& insts, Graph g);

            /**
             * Removes trailing infinite loops.
             *
             * In FF7 some scripts ends with an infinite loop to keep it alive.
             * in QGears this isn't required, and can cause infinite loops, so
             * they can be removed.
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
             * In FF7 some scripts ends with an infinite loop to keep it alive.
             * in QGears this isn't required, and can cause infinite loops, so
             * they can be removed. This function marks them, so they can be
             * deleted with @{see FF7FieldEngine::RemoveTrailingInfiniteLoops}
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
             * @param codeGen[in] Code generator.
             * @todo Func and engine are unused?
             * @todo Understand and document properly.
             */
            virtual void processInst(
              Function& func, ValueStack &stack,
              Engine *engine, CodeGenerator *codeGen
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
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine.
             * @param codeGen[in] Code generator.
             * @todo Func and engine are unused?
             * @todo Understand and document properly.
             */
            virtual void processInst(
              Function& func, ValueStack &stack,
              Engine *engine, CodeGenerator *codeGen
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
             * @param codeGen[in] Code generator.
             * @todo Func and engine are unused?
             * @todo Understand and document properly.
             */
            virtual void processInst(
              Function& func, ValueStack &stack,
              Engine *engine, CodeGenerator *codeGen
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
             * const Bit[5] F: The ID of the specific member function of E to be
             *                 executed (low 5 bits of byte).
             *
             * Requests that a remote entity executes one of its member
             * functions at a specified priority. The request is asynchronous
             * and returns immediately without waiting for the remote execution
             * to start or finish. If the specified priority is already busy
             * executing, the request will fail silently.
             *
             * @param codegen[in|out] Code generator. Output lines are appended
             * to it.
             * @param engine[in] The engine instance to fetch entities.
             */
            void processREQ(
              CodeGenerator* codeGen,
              const FF7FieldEngine& engine
            );

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
             * const Bit[5] F: The ID of the specific member function of E to be
             *                 executed (low 5 bits of byte).
             *
             * Requests that a remote entity executes one of its member
             * functions at a specified priority. If the specified priority is
             * already busy executing, the request will block until it becomes
             * available and only then return. The remote execution is still
             * carried out asynchronously, with no notification of completion.
             *
             * @param codegen[in|out] Code generator. Output lines are appended
             * to it.
             * @param engine[in] The engine instance to fetch entities.
             */
            void processREQSW(
              CodeGenerator* codeGen,
              const FF7FieldEngine& engine
            );

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
             * const Bit[5] F: The ID of the specific member function of E to be
             *                 executed (low 5 bits of byte).
             *
             * Requests that a remote entity executes one of its member
             * functions at a specified priority. The request will block until
             * remote execution has finished before returning.
             *
             * @param codegen[in|out] Code generator. Output lines are appended
             * to it.
             * @param engine[in] The engine instance to fetch entities.
             */
            void processREQEW(
              CodeGenerator* codeGen,
              const FF7FieldEngine& engine
            );

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
             * Stops the active script loop for this entity and also any script
             * loops (except the main) that are queuing to be executed after the
             * current script. This is essentially the same as adding a RET onto
             * each of the active / queued scripts next execution position and
             * returning the current op index to index for each script. Then the
             * script control is passed to the script F within the current
             * entity with the priority P.
             *
             * @param codegen[in|out] Code generator. Output lines are appended
             * to it.
             */
            void processRETTO(CodeGenerator* codeGen);

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
             * @param codegen[in|out] Code generator. Output lines are appended
             * to it.
             */
            void processWAIT(CodeGenerator* codeGen);
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
         * @param engine[in] Engine.
         * @param codeGen[in] Code generator.
         * @todo Func and engine are unused?
         * @todo Understand and document properly.
         */
            virtual void processInst(
              Function& func, ValueStack &stack,
              Engine *engine, CodeGenerator *codeGen
            ) override;

        private:

            /**
             * Processes a BATTLE command.
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
            void processBATTLE(CodeGenerator* codeGen);

            /**
             * Processes a BTLON command.
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
             * Turns random encounters on or off for this field. Note that if a
             * field does not have any Encounter Data set in its field file,
             * battles will not occur regardless of the argument passed with
             * this opcode.
             *
             * @param codegen[in|out] Code generator. Output lines are appended
             * to it.
             */
            void processBTLON(CodeGenerator* codeGen);

            /**
             * Processes a MAPJUMP command.
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
             * character at the coordinates and direction specified. This is an
             * alternative to using a gateway, and can complement their usage as
             * it allows for more than 12 gateways by simulating their behaviour
             * through a LINE which, when crossed, executes a MAPJUMP.
             *
             * @param codegen[in|out] Code generator. Output lines are appended
             * to it.
             * @param func[in] Function
             * @todo What is func for?
             */
            void processMAPJUMP(CodeGenerator* codeGen, Function& func);
    };

    class FF7MathInstruction : public StoreInstruction
    {
    public:
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;
    private:
        void processSaturatedPLUS(CodeGenerator* codeGen);
        void processSaturatedPLUS2(CodeGenerator* codeGen);
        void processSaturatedMINUS(CodeGenerator* codeGen);
        void processSaturatedMINUS2(CodeGenerator* codeGen);
        void processSaturatedINC(CodeGenerator* codeGen);
        void processSaturatedINC2(CodeGenerator* codeGen);
        void processSaturatedDEC(CodeGenerator* codeGen);
        void processSaturatedDEC2(CodeGenerator* codeGen);
        void processRDMSD(CodeGenerator* codeGen);
        void processSETBYTE_SETWORD(CodeGenerator* codeGen);
        void processBITON(CodeGenerator* codeGen);
        void processPLUSx_MINUSx(CodeGenerator* codeGen, const std::string& op);
        void processINCx_DECx(CodeGenerator* codeGen, const std::string& op);
        void processRANDOM(CodeGenerator* codeGen);
    };

    class FF7WindowInstruction : public KernelCallInstruction
    {
    public:
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;
    private:
        void processMESSAGE(CodeGenerator* codeGen, const std::string& scriptName);
        void processMPNAM(CodeGenerator* codeGen);
        void processMENU2(CodeGenerator* codeGen);
        void processWINDOW(CodeGenerator* codeGen);
        void processWCLSE(CodeGenerator* codeGen);
    };

    class FF7PartyInstruction : public KernelCallInstruction
    {
    public:
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;
    private:
        void processSTITM(CodeGenerator* codeGen);
        void processPRTYE(CodeGenerator* codeGen);
    };

    class FF7ModelInstruction : public KernelCallInstruction
    {
    public:
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;
    private:
        void processTLKON(CodeGenerator* codeGen, const std::string& entity);
        void processPC(CodeGenerator* codeGen, const std::string& entity);
        void processCHAR(CodeGenerator* codeGen, const std::string& entity);
        void processDFANM(CodeGenerator* codeGen, const std::string& entity, int charId);
        void processANIME1(CodeGenerator* codeGen, const std::string& entity, int charId);
        void processVISI(CodeGenerator* codeGen, const std::string& entity);
        void processXYZI(CodeGenerator* codeGen, const std::string& entity);
        void processMOVE(CodeGenerator* codeGen, const std::string& entity);
        void processMSPED(CodeGenerator* codeGen, const std::string& entity);
        void processDIR(CodeGenerator* codeGen, const std::string& entity);
        void processTURNGEN(CodeGenerator* codeGen, const std::string& entity);
        void processGETAI(CodeGenerator* codeGen, const FF7FieldEngine& engine);
        void processANIM_2(CodeGenerator* codeGen, const std::string& entity, int charId);
        void processCANIM2(CodeGenerator* codeGen, const std::string& entity, int charId);
        void processCANM_2(CodeGenerator* codeGen, const std::string& entity, int charId);
        void processCC(CodeGenerator* codeGen, const FF7FieldEngine& engine);
        void processSOLID(CodeGenerator* codeGen, const std::string& entity);

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
         * - const Bit[4] B4: Bank to retrieve speed, or zero if S is specified
         * as a literal.
         * - const UByte T: Type of movement.
         * - const Short X: X offset amount, relative to current position, or
         * address to find X offset, if B1 is non-zero.
         * - const Short Y: Y offset amount, relative to current position, or
         * address to find Y offset, if B2 is non-zero.
         * - const Short Z: Z offset amount, relative to current position, or
         * address to find Z offset, if B3 is non-zero.
         * - const UShort S: Speed of the offset movement, if type is non-zero,
         * or address to find speed, if B4 is non-zero.
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
         * is specified relative to the current position. If Type is specified,
         * the object moves gradually from its current point to the offset
         * position; this can be used to simulate movements such as elevators.
         * Any type outside the range in the table will cause the offset not to
         * occur. If the object is set to move gradually, then the speed of
         * offset can be set; the greater the number, the slower the object
         * moves to its target offset. Script execution may also be halted
         * until the gradual offset has been completed. For this, see OFSTW.
         *
         * @param codeGen The code generator.
         * @param entity[in] The entity name.
         */
        void processOFST(CodeGenerator* codegen, const std::string& entity);
    };

    class FF7WalkmeshInstruction : public KernelCallInstruction
    {
    public:
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;

    private:
        void processUC(CodeGenerator* codeGen);

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
         * character, causes one of the entity's scripts to be executed. These
         * are similar to the triggers in Section 8. All the lines in the
         * current field can be turned on or off by using the LINON opcode.
         *
         * There are generally 6 scripts (other than the init and main) if the entity is a LINE.
         * - script index 2 -> S1 - [OK].
         * - script index 3 -> S2 - Move.
         * - script index 4 -> S3 - Move.
         * - script index 5 -> S4 - Go.
         * - script index 6 -> S5 - Go 1x.
         * - script index 7 -> S6 - Go away.
         *
         * @param codeGen The code generator.
         * @param entity[in] The entity name.
         */
        void processLINE(CodeGenerator* codeGen, const std::string& entity);
    };

    class FF7BackgroundInstruction : public KernelCallInstruction
    {
    public:
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;
    private:
        void processBGON(CodeGenerator* codeGen);
        void processBGOFF(CodeGenerator* codeGen);
        void processBGCLR(CodeGenerator* codeGen);
        void processSTPAL(CodeGenerator* codeGen);
        void processLDPAL(CodeGenerator* codeGen);
        void processCPPAL(CodeGenerator* codeGen);
        void processADPAL(CodeGenerator* codeGen);
        void processMPPAL2(CodeGenerator* codeGen);
        void processSTPLS(CodeGenerator* codeGen);
        void processLDPLS(CodeGenerator* codeGen);
    };

    class FF7CameraInstruction : public KernelCallInstruction
    {
    public:
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;
    private:
        void processNFADE(CodeGenerator* codeGen);
        void processSCR2D(CodeGenerator* codeGen);
        void processSCR2DC(CodeGenerator* codeGen);
        void processFADE(CodeGenerator* codeGen);
    };

    class FF7AudioVideoInstruction : public KernelCallInstruction
    {
    public:
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;
    private:
        void processAKAO2(CodeGenerator* codeGen);
        void processMUSIC(CodeGenerator* codeGen);
        void processSOUND(CodeGenerator* codeGen);
        void processAKAO(CodeGenerator* codeGen);
        void processMULCK(CodeGenerator* codeGen);
        void processPMVIE(CodeGenerator* codeGen);
        void processMOVIE(CodeGenerator* codeGen);
        void processMVIEF(CodeGenerator* codeGen);
    };

    class FF7UncategorizedInstruction : public KernelCallInstruction
    {
    public:
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;
    };

    class FF7NoOperationInstruction : public Instruction
    {
    public:
        static InstPtr Create() { return new FF7NoOperationInstruction(); }
        virtual void processInst(Function& func, ValueStack &stack, Engine *engine, CodeGenerator *codeGen) override;
    };
}
