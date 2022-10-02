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

#include <string>
#include <vector>

#include "../Engine.h"
#include "decompiler/sudm.h"

namespace FF7{

    /**
     * Represents the FF7 Field engine.
     */
    class FieldEngine : public Engine{

        public:

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
                    Entity(const std::string& name, size_t index);

                    /**
                     * Retrieves the entity name.
                     *
                     * @return The entity name.
                     */
                    std::string GetName() const;

                    /**
                     * Retrieves the entity index.
                     *
                     * The index is the one at which appears in the original
                     * game script.
                     *
                     * @return The entity index.
                     */
                    size_t GetIndex() const;

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
                    std::string FunctionByIndex(size_t index) const;

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
                    void AddFunction(const std::string& name, size_t index);

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
                    );

                    /**
                     * Checks if the entity is a line.
                     *
                     * Note that an entity is not considered to be a line
                     * until a function has been found containing the opcode
                     * LINE and {@see MarkAsLine} has been called.
                     *
                     * @return true if the entity is a line.
                     */
                    bool IsLine();

                    /**
                     * Retrieves the first point of the line entity.
                     *
                     * If the entity is not a line, the behavior is undefined.
                     *
                     * @return The first point of the line entity.
                     */
                    std::vector<float> GetLinePointA();

                    /**
                     * Retrieves the second point of the line entity.
                     *
                     * If the entity is not a line, the behavior is undefined.
                     *
                     * @return The second point of the line entity.
                     */
                    std::vector<float> GetLinePointB();



                private:

                    /**
                     * Entity name.
                     */
                    std::string name_;

                    /**
                     * Entity index.
                     */
                    size_t index_;

                    /**
                     * Entity function (script) list.
                     */
                    std::map< size_t, std::string > functions_;

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
             * Constructor.
             *
             * @param formatter[in] The formatter to be used by the engine.
             * @param script_name[in] The script name.
             */
            FieldEngine(SUDM::IScriptFormatter& formatter, std::string script_name);

            /**
             * Copy constructor, disabled.
             *
             * @param engine[in] The engine to copy.
             */
            FieldEngine(const FieldEngine& engine) = delete;

            /**
             * Copy constructor, disabled.
             *
             * @param engine[in] The engine to copy.
             */
            FieldEngine& operator = (const FieldEngine& engine) = delete;

            /**
             * Retrieves the disasembler.
             *
             * @param insts[in] List of instructions.
             * @param raw_script_data[in] Script data, raw format.
             * @return Pointer to the disasembler.
             */
            virtual std::unique_ptr<Disassembler> GetDisassembler(
              InstVec &insts, const std::vector<unsigned char>& raw_script_data
            ) override;

            /**
             * Retrieves the dissasembler.
             *
             * @param insts[in] List of instructions.
             * @return Pointer to the dissasembler.
             */
            virtual std::unique_ptr<Disassembler> GetDisassembler(InstVec &insts) override;

            /**
             * Retrieves the code generator.
             *
             * @param insts[in] List of instructions.
             * @param output[in] Pointer to the output (file, stream...).
             * @return Pointer to the generator.
             */
            virtual std::unique_ptr<CodeGenerator> GetCodeGenerator(
              const InstVec& insts, std::ostream &output
            ) override;

            /**
             * Post-processing actions to apply to the scripts.
             *
             * It actually does nothing. CFG stands for control flow group.
             *
             * @param insts[in] Instruction list.
             * @param graph[in] Code graph.
             */
            virtual void PostCFG(InstVec &insts, Graph graph) override;

            /**
             * Indicates if instructions are purely grouped.
             *
             * @return True if instructions are purely grouped. Always false.
             * @todo What is pure grouping?
             */
            virtual bool UsePureGrouping() const override;

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
            std::map<size_t, Entity> GetEntityIndexMap() const{return entity_index_map_;}

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
            const Entity& EntityByIndex(size_t index) const;

            /**
             * Retrieves the scale factor for the map.
             *
             * @return Map scale factor.
             */
            float GetScaleFactor() const;

            /**
             * Retrieves the script name.
             *
             * @return The script name.
             */
            const std::string& GetScriptName() const;

        private:

            /**
             * Removes extraneous return statements.
             *
             * Useful for scripts that only contain one one return statement.
             *
             * @param insts[in|out] List of instructions to process.
             * Extraneous return statements will be deleted from the
             * instructions.
             * @param graph[in] Code graph. Unused.
             */
            void RemoveExtraneousReturnStatements(InstVec& insts, Graph graph);

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
            void RemoveTrailingInfiniteLoops(InstVec& insts, Graph graph);

            /**
             * Tries to detect scripts with trailing infinite loops.
             *
             * In FF7 some scripts ends with an infinite loop to keep it
             * alive. in VGears this isn't required, and can cause infinite
             * loops, so they can be removed. This function marks them, so
             * they can be deleted with
             * @{see FieldEngine::RemoveTrailingInfiniteLoops}.
             */
            void MarkInfiniteLoopGroups(InstVec& insts, Graph graph);

            /**
             * The script formatter.
             */
            SUDM::IScriptFormatter& formatter_;

            /**
             * The entity index map for the field.
             */
            std::map<size_t, Entity> entity_index_map_;

            /**
             * The map scale factor.
             */
            float scale_factor_;

            /**
             * The script name.
             */
            std::string script_name_;
    };

}
