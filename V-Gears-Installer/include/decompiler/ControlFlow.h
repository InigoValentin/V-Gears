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

#include "Engine.h"
#include "Graph.h"

/**
 * Class for doing code flow analysis.
 */
class ControlFlow {

    public:

        /**
         * Constructor for the control flow graph.
         *
         * @param[in] insts The instructions to analyze control flow for.
         * @param[in] engine Pointer to the Engine used for the script.
         */
        ControlFlow(InstVec& insts, Engine& engine);

        /**
         * Copy constructor disabled.
         *
         * @param[in] control_flow The control flow to copy.
         */
        ControlFlow(const ControlFlow& control_flow) = delete;

        /**
         * Copy constructor disabled.
         *
         * @param[in] control_flow The control flow to copy.
         */
        ControlFlow& operator = (const ControlFlow& control_flow) = delete;

        /**
         * Retrieves the current control flow graph.
         *
         * @returns The current control flow graph.
         */
        const Graph& GetGraph() const;

        /**
         * Creates groups suitable for a stack-based machine.
         *
         * Before group creation, the expected stack level for each
         * instruction is determined. After group creation, short-circuit
         * detection is applied to the groups.
         */
        void CreateGroups();

        /**
         * Performs control flow analysis.
         *
         * The constructs are detected in the following order: do-while,
         * while, break, continue, if/else.
         *
         * @returns The control flow graph after analysis.
         */
        const Graph& Analyze();

    private:

        /**
         * The control flow graph.
         */
        Graph graph_;

        /**
         * The engine used for the script.
         */
        Engine& engine_;

        /**
         * The instructions being analyzed.
         */
        InstVec &insts_;

        /**
         * Map of addresses and vertices.
         */
        std::map<uint32, GraphVertex> addr_map_;

        /**
         * Finds a graph vertex through an instruction.
         *
         * @param[in] inst The instruction to find the vertex for.
         */
        GraphVertex Find(const InstPtr inst);

        /**
         * Finds a graph vertex through an instruction iterator.
         *
         * @param[in] it The iterator to find the vertex for.
         */
        GraphVertex Find(ConstInstIterator it);

        /**
         * Finds a graph vertex through an address.
         *
         * @param[in] address The address to find the vertex for.
         */
        GraphVertex Find(uint32 address);

        /**
         * Merges two graph vertices.
         *
         * graph_2 will be merged into graph_1.
         *
         * @param[in,out] graph_1 The first vertex to merge.
         * @param[in] graph_2 The second vertex to merge.
         */
        void Merge(GraphVertex graph_1, GraphVertex graph_2);

        /**
         * Sets the stack level for all instructions, using depth-first search.
         *
         * @param[in] graph The GraphVertex to search from.
         * @param[in] level The stack level when g is reached.
         */
        void SetStackLevel(GraphVertex graph, int level);

        /**
         * Merged groups that are part of the same short-circuited condition.
         */
        void DetectShortCircuit();

        /**
         * Detects while blocks.
         *
         * Do-while detection must be completed before running this method.
         */
        void DetectWhile();

        /**
         * Detects do-while blocks.
         */
        void DetectDoWhile();

        /**
         * Detects break statements.
         *
         * Do-while and while detection must be completed before running this
         * method.
         */
        void DetectBreak();

        /**
         * Detects continue statements.
         *
         * Do-while and while detection must be completed before running this
         * method.
         */
        void DetectContinue();

        /**
         * Checks if a candidate break/continue goes to the closest loop.
         *
         * @param[in] group The group containing the candidate break/continue.
         * @param[in] condition_group The group containing the respective loop
         * condition.
         * @returns True if the validation succeeded, false if it did not.
         */
        bool ValidateBreakOrContinue(GroupPtr group, GroupPtr condition_group);

        /**
         * Detects if blocks.
         *
         * Must be performed after break and continue detection.
         */
        void DetectIf();

        /**
         * Detects else blocks.
         *
         * Must be performed after if detection.
         */
        void DetectElse();

        /**
         * Checks if a candidate else block will cross block boundaries.
         *
         * @param[in] if_group The group containing the if this else candidate
         * is associated with.
         * @param[in] start The group containing the start of the else.
         * @param[in] end The group immediately after the group ending the
         * else.
         * @returns True if the validation succeeded, false if it did not.
         */
        bool ValidateElseBlock(GroupPtr if_group, GroupPtr start, GroupPtr end);

};

