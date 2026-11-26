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

#include <algorithm>
#include <iostream>
#include <set>
#include <stdexcept>
#include <boost/format.hpp>
#include "decompiler/ControlFlow.h"

#include "decompiler/Stack.h"

/**
 * Adds a vertex to a group.
 *
 * @param[in] vertex Vertex to add.
 * @param[in] group Group to add the vertex to.
 */
#define PUT(vertex, group) boost::put(boost::vertex_name, graph_, vertex, group);

/**
 * Adds an edge to the graph.
 *
 * @param[in] edge The edge to add.
 * @param[in] is_jump Indicates if the edge is a jump.
 */
#define PUT_EDGE(edge, is_jump) boost::put(boost::edge_attribute, graph_, edge, is_jump);

/**
 * Adds a vertex to the graph.
 *
 * @param[in] vertex The vertex to add.
 * @param[in] id The vertext index.
 */
#define PUT_ID(vertex, id) boost::put(boost::vertex_index, graph_, vertex, id);

/**
 * Retrieves a vertex.
 *
 * @param[in] vertex The vertex.
 * @return The retrieved vertex.
 */
#define GET(vertex) (boost::get(boost::vertex_name, graph_, vertex))

/**
 * Retrieves an edge.
 *
 * @param[in] edge The edge.
 * @return The retrieved edge.
 */
#define GET_EDGE(edge) (boost::get(boost::edge_attribute, graph_, edge))

ControlFlow::ControlFlow(InstVec& insts, Engine& engine): insts_(insts),engine_(engine){
    if (insts_.empty()) return;

    // Automatically add a function if we're not supposed to look for more functions
    // and no functions are defined.
    // This avoids a special case for when no real functions exist in the script.
    if (engine_.functions.empty()){
        engine_.functions[(*insts.begin())->GetAddress()]= Function(
          (*insts.begin())->GetAddress(), (insts.back())->GetAddress()
        );
    }
    GroupPtr prev = NULL;
    int id = 0;
    // Create vertices.
    for (InstIterator it = insts.begin(); it != insts.end(); ++ it){
        GraphVertex cur = boost::add_vertex(graph_);
        addr_map_[(*it)->GetAddress()] = cur;
        PUT(cur, new Group(cur, it, it, prev));
        PUT_ID(cur, id);
        id ++;
        // Add reference to vertex if function starts here.
        if (engine_.functions.find((*it)->GetAddress()) != engine_.functions.end()){
            engine_.functions[(*it)->GetAddress()].vertex = cur;
        }
        prev = GET(cur);
    }
    // Add regular edges.
    GraphVertex last ={};
    bool add_edge = false;
    prev = NULL;
    for (InstIterator it = insts.begin(); it != insts.end(); ++it){
        if (engine_.functions.find((*it)->GetAddress()) != engine_.functions.end()) add_edge = false;
        GraphVertex cur = Find(it);
        if (add_edge){
            GraphEdge e = boost::add_edge(last, cur, graph_).first;
            PUT_EDGE(e, false);
        }
        last = cur;
        add_edge = !((*it)->IsUncondJump() || (*it)->IsReturn());
        prev = GET(cur);

    }
    // Add jump edges.
    for (InstIterator it = insts.begin(); it != insts.end(); ++ it){
        if ((*it)->IsJump()){
            const uint32 src_addr = (*it)->GetAddress();
            const uint32 dest_addr = (*it)->GetDestAddress();
            auto src_it = addr_map_.find(src_addr);
            auto dest_it = addr_map_.find(dest_addr);
            if (src_it == addr_map_.end() || dest_it == addr_map_.end()){
                std::cerr << "Invalid jump edge in control-flow graph: source " << boost::format("0x%08x") % src_addr
                  << " -> destination " << boost::format("0x%08x") % dest_addr << " (opcode "
                  << boost::format("0x%02x") % (*it)->GetOpcode() << ")" << std::endl;
                continue;
            }
            GraphEdge e = boost::add_edge(src_it->second, dest_it->second, graph_).first;
            PUT_EDGE(e, true);
        }
    }
}

const Graph& ControlFlow::GetGraph() const{return graph_;}

GraphVertex ControlFlow::Find(const InstPtr inst){return addr_map_[inst->GetAddress()];}

GraphVertex ControlFlow::Find(ConstInstIterator it){return addr_map_[(*it)->GetAddress()];}

GraphVertex ControlFlow::Find(uint32 address){
    std::map<uint32, GraphVertex>::iterator it = addr_map_.find(address);
    if (it == addr_map_.end())
        throw std::runtime_error((boost::format("Request for instruction at unknown address 0x%08x") % address).str());
    return it->second;
}

void ControlFlow::Merge(GraphVertex graph_1, GraphVertex graph_2){
    // Update property.
    GroupPtr gr1 = GET(graph_1);
    GroupPtr gr2 = GET(graph_2);
    gr1->end = gr2->end;
    PUT(graph_1, gr1);
    // Update address map.
    ConstInstIterator it = gr2->start;
    do{
        addr_map_[(*it)->GetAddress()] = graph_1;
        ++ it;
    } while (gr2->start != gr2->end && it != gr2->end);
    // Add outgoing edges from graph_2.
    OutEdgeRange r = boost::out_edges(graph_2, graph_);
    for (OutEdgeIterator e = r.first; e != r.second; ++e){
        GraphEdge newE = boost::add_edge(graph_1, boost::target(*e, graph_), graph_).first;
        PUT_EDGE(newE, GET_EDGE(*e));
    }
    // Update _next pointer.
    gr1->next = gr2->next;
    if (gr2->next != NULL) gr2->next->prev = gr2->prev;
    // Remove edges to/from graph_2
    boost::clear_vertex(graph_2, graph_);
    // Remove vertex.
    boost::remove_vertex(graph_2, graph_);
}

typedef std::pair<GraphVertex, int> LevelEntry;

void ControlFlow::SetStackLevel(GraphVertex graph, int level){
    Stack<LevelEntry> level_stack;
    std::set<GraphVertex> seen;
    level_stack.Push(LevelEntry(graph, level));
    seen.insert(graph);
    while (!level_stack.IsEmpty()){
        LevelEntry e = level_stack.Pop();
        GroupPtr gr = GET(e.first);
        if (gr->stack_level != -1){
            if (gr->stack_level != e.second)
                std::cerr << boost::format(
                  "WARNING: Inconsistency in expected stack level for instruction "
                  "at address 0x%08x (current: %d, requested: %d)\n"
                ) % (*gr->start)->GetAddress() % gr->stack_level % e.second;
            continue;
        }
        gr->stack_level = e.second;
        OutEdgeRange r = boost::out_edges(e.first, graph_);
        for (OutEdgeIterator oe = r.first; oe != r.second; ++ oe){
            GraphVertex target = boost::target(*oe, graph_);
            if (seen.find(target) == seen.end()){
                level_stack.Push(LevelEntry(target, e.second + (*gr->start)->GetStackChange()));
                seen.insert(target);
            }
        }
    }
}

void ControlFlow::CreateGroups(){
    if (
      !engine_.functions.empty()
      //&& GET(engine_.functions.begin()->second.GetVertex())->_stackLevel != -1
      //&& GET(engine_.functions.begin()->second.vertex_)->_stackLevel != -1
      && GET(engine_.functions.begin()->second.vertex)->stack_level != -1
    ) return;

    for (FuncMap::iterator fn = engine_.functions.begin(); fn != engine_.functions.end(); ++ fn){
        SetStackLevel(fn->second.vertex, 0);
    }
    ConstInstIterator cur_inst, next_inst;
    next_inst = insts_.begin();
    next_inst ++;
    int stack_level = 0;
    int expected_stack_level = 0;
    for (cur_inst = insts_.begin(); next_inst != insts_.end(); ++ cur_inst, ++ next_inst){
        GraphVertex cur = Find(cur_inst);
        GraphVertex next = Find(next_inst);
        GroupPtr group_cur = GET(cur);
        GroupPtr group_next = GET(next);
        // Don't process unreachable code.
        if (group_cur->stack_level < 0){
            stack_level = group_next->stack_level;
            continue;
        }
        expected_stack_level = group_cur->stack_level;
        // If expected stack level decreases in next vertex, then
        // use next vertex level as expected level.
        if (expected_stack_level > group_next->stack_level && group_next->stack_level >= 0){
            expected_stack_level = group_next->stack_level;
            // Also set the stack level of the current group
            // to remember that we expect it to be lower.
            group_cur->stack_level = expected_stack_level;
        }
        stack_level += (*cur_inst)->GetStackChange();
        // For stack operations, the new stack level becomes the
        // expected stack level starting from the next group.
        if ((*cur_inst)->IsStackOp()){
            expected_stack_level = stack_level;
            group_next->stack_level = stack_level;
        }
        // Group ends after a jump.
        if ((*cur_inst)->IsJump()){
            stack_level = group_next->stack_level;
            continue;
        }
        // Group ends with a return.
        if ((*cur_inst)->IsReturn()){
            stack_level = group_next->stack_level;
            continue;
        }
        // Group ends before target of a jump.
        if (in_degree(next, graph_) != 1){
            stack_level = group_next->stack_level;
            continue;
        }
        // This part is only relevant if we use the stack level.
        if (!engine_.UsePureGrouping()){
            // If group has no instructions with stack effect >= 0, don't merge on balanced stack.
            bool forceMerge = true;
            ConstInstIterator it = group_cur->start;
            do{
                if ((*it)->GetStackChange() >= 0) forceMerge = false;
                ++ it;
            } while (group_cur->start != group_cur->end && it != group_cur->end);
            // Group ends when stack is balanced, unless just before conditional jump.
            if (stack_level == expected_stack_level && !forceMerge && !(*next_inst)->IsCondJump())
                continue;
        }
        // All checks passed, merge groups
        Merge(cur, next);
    }

    // FIXME: The short-circuit detection is disabled because short-circuited
    // groups require some special handling in the code generation. It's not
    // entirely clear how to handle it properly, though: It has to be deduced
    // which effect is created by the conditional jumps in the middle of a
    // block, which seems to get fairly complex when there are multiple groups
    // that are merged by the short-circuit detection.
    //detectShortCircuit();
}
