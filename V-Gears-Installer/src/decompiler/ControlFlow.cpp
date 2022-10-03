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
#include <boost/format.hpp>
#include "decompiler/ControlFlow.h"
#include "decompiler/stack.h"

/**
 * Adds a vertex to a group.
 *
 * @param vertex[in] Vertex to add.
 * @param group[in] Group to add the vertex to.
 */
#define PUT(vertex, group) boost::put(boost::vertex_name, graph_, vertex, group);

/**
 * Adds an edge to the graph.
 *
 * @param edge[in] The edge to add.
 * @param is_jump[in] Indicates if the edge is a jump.
 */
#define PUT_EDGE(edge, is_jump) boost::put(boost::edge_attribute, graph_, edge, is_jump);

/**
 * Adds a vertex to the graph.
 *
 * @param vertex[in] The vertex to add.
 * @param id[in] The vertext index.
 */
#define PUT_ID(vertex, id) boost::put(boost::vertex_index, graph_, vertex, id);

/**
 * Retrieves a vertex.
 *
 * @param vertex[in] The vertex.
 * @return The retrieved vertex.
 */
#define GET(vertex) (boost::get(boost::vertex_name, graph_, vertex))

/**
 * Retrieves an edge.
 *
 * @param edge[in] The edge.
 * @return The retrieved edge.
 */
#define GET_EDGE(edge) (boost::get(boost::edge_attribute, graph_, edge))

ControlFlow::ControlFlow(InstVec& insts, Engine& engine): insts_(insts),engine_(engine){

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
    FuncMap::iterator fn;
    GraphVertex last ={};
    bool add_edge = false;
    prev = NULL;
    for (InstIterator it = insts.begin(); it != insts.end(); ++it){
        if (engine_.functions.find((*it)->GetAddress()) != engine_.functions.end())
            add_edge = false;
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
            GraphEdge e = boost::add_edge(Find(it), Find((*it)->GetDestAddress()), graph_).first;
            PUT_EDGE(e, true);
        }
    }
}

const Graph& ControlFlow::GetGraph() const{return graph_;}

GraphVertex ControlFlow::Find(const InstPtr inst){return addr_map_[inst->GetAddress()];}

GraphVertex ControlFlow::Find(ConstInstIterator it){return addr_map_[(*it)->GetAddress()];}

GraphVertex ControlFlow::Find(uint32 address){
    std::map<uint32, GraphVertex>::iterator it = addr_map_.find(address);
    if (it == addr_map_.end()){
        std::cerr << "Request for instruction at unknown address "
          << boost::format("0x%08x") % address << std::endl;
    }
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
    level_stack.push(LevelEntry(graph, level));
    seen.insert(graph);
    while (!level_stack.empty()){
        LevelEntry e = level_stack.pop();
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
                level_stack.push(LevelEntry(target, e.second + (*gr->start)->GetStackChange()));
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
    ){
        return;
    }

    for (
      FuncMap::iterator fn = engine_.functions.begin();
      fn != engine_.functions.end();
      ++ fn
    ){
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

void ControlFlow::DetectShortCircuit(){
    ConstInstIterator last_inst = insts_.end();
    -- last_inst;
    GraphVertex cur = Find(last_inst);
    GroupPtr gr = GET(cur);
    while (gr->prev != NULL){
        bool do_merge = false;
        cur = Find(gr->start);
        GraphVertex prev = Find(gr->prev->start);
        // Block is candidate for short-circuit merging if it and the
        // preceding block both end with conditional jumps.
        if (out_degree(cur, graph_) == 2 && out_degree(prev, graph_) == 2){
            do_merge = true;
            OutEdgeRange range_cur = boost::out_edges(cur, graph_);
            std::vector<GraphVertex> succs;
            // Find possible target vertices.
            for (OutEdgeIterator it = range_cur.first; it != range_cur.second; ++ it)
                succs.push_back(boost::target(*it, graph_));
            // Check if vertex would add new targets - if yes, don't merge.
            OutEdgeRange range_prev = boost::out_edges(prev, graph_);
            for (OutEdgeIterator it = range_prev.first; it != range_prev.second; ++ it){
                GraphVertex target = boost::target(*it, graph_);
                do_merge &= (
                  std::find(succs.begin(), succs.end(), target) != succs.end() || target == cur
                );
            }
            if (do_merge){
                gr = gr->prev;
                Merge(prev, cur);
                continue;
            }
        }
        gr = gr->prev;
    }
}

const Graph &ControlFlow::Analyze(){
    DetectDoWhile();
    DetectWhile();
    DetectBreak();
    DetectContinue();
    DetectIf();
    DetectElse();
    return graph_;
}

void ControlFlow::DetectWhile(){
    VertexRange vertex_range = boost::vertices(graph_);
    for (VertexIterator v = vertex_range.first; v != vertex_range.second; ++ v){
        GroupPtr gr = GET(*v);
        // Undetermined block that ends with conditional jump.
        if (out_degree(*v, graph_) == 2 && gr->type == GROUP_TYPE_NORMAL){
            InEdgeRange ier = boost::in_edges(*v, graph_);
            bool is_while = false;
            for (InEdgeIterator e = ier.first; e != ier.second; ++ e){
                GroupPtr source_gr = GET(boost::source(*e, graph_));
                // Block has ingoing edge from block later in the
                // code that isn't a do-while condition.
                if (
                  (*source_gr->start)->GetAddress() > (*gr->start)->GetAddress()
                  && source_gr->type != GROUP_TYPE_DO_WHILE
                ){
                    is_while = true;
                }
            }
            if (is_while) gr->type = GROUP_TYPE_WHILE;
        }
    }
}

void ControlFlow::DetectDoWhile(){
    VertexRange vertex_range = boost::vertices(graph_);
    for (VertexIterator v = vertex_range.first; v != vertex_range.second; ++ v){
        GroupPtr gr = GET(*v);
        // Undetermined block that ends with conditional jump...
        if (out_degree(*v, graph_) == 2 && gr->type == GROUP_TYPE_NORMAL){
            OutEdgeRange oer = boost::out_edges(*v, graph_);
            for (OutEdgeIterator e = oer.first; e != oer.second; ++e){
                GroupPtr target_gr = GET(boost::target(*e, graph_));
                // ...to earlier in code.
                if ((*target_gr->start)->GetAddress() < (*gr->start)->GetAddress())
                    gr->type = GROUP_TYPE_DO_WHILE;
            }
        }
    }
}

void ControlFlow::DetectBreak(){
    VertexRange vertex_range = boost::vertices(graph_);
    for (VertexIterator v = vertex_range.first; v != vertex_range.second; ++ v){
        GroupPtr gr = GET(*v);
        // Undetermined block with unconditional jump...
        if (
          gr->type == GROUP_TYPE_NORMAL
          && ((*gr->end)->IsUncondJump())
          && out_degree(*v, graph_) == 1
        ){
            OutEdgeIterator oe = boost::out_edges(*v, graph_).first;
            GraphVertex target = boost::target(*oe, graph_);
            GroupPtr target_gr = GET(target);
            // ...to somewhere later in the code...
            if ((*gr->start)->GetAddress() >= (*target_gr->start)->GetAddress()) continue;
            InEdgeRange ier = boost::in_edges(target, graph_);
            for (InEdgeIterator ie = ier.first; ie != ier.second; ++ ie){
                GroupPtr source_gr = GET(boost::source(*ie, graph_));
                // ...to block immediately after a do-while condition,
                // or to jump target of a while condition.
                if (
                  (target_gr->prev == source_gr && source_gr->type == GROUP_TYPE_DO_WHILE)
                  || source_gr->type == GROUP_TYPE_WHILE
                ){
                    if (ValidateBreakOrContinue(gr, source_gr)) gr->type = GROUP_TYPE_BREAK;
                }
            }
        }
    }
}

void ControlFlow::DetectContinue(){
    VertexRange vertex_range = boost::vertices(graph_);
    for (VertexIterator v = vertex_range.first; v != vertex_range.second; ++ v){
        GroupPtr gr = GET(*v);
        // Undetermined block with unconditional jump...
        if (
          gr->type == GROUP_TYPE_NORMAL
          && ((*gr->end)->IsUncondJump())
          && out_degree(*v, graph_) == 1
        ){
            OutEdgeIterator oe = boost::out_edges(*v, graph_).first;
            GraphVertex target = boost::target(*oe, graph_);
            GroupPtr target_gr = GET(target);
            // ...to a while or do-while condition...
            if (
              target_gr->type == GROUP_TYPE_WHILE || target_gr->type == GROUP_TYPE_DO_WHILE
            ){
                bool is_continue = true;
                // ...unless...
                OutEdgeRange toer = boost::out_edges(target, graph_);
                bool after_jump_jargets = true;
                for (OutEdgeIterator toe = toer.first; toe != toer.second; ++ toe){
                    // ...it is targeting a while condition which jumps to the next sequential group
                    if (
                      target_gr->type == GROUP_TYPE_WHILE
                      && GET(boost::target(*toe, graph_)) == gr->next
                    ){
                        is_continue = false;
                    }
                    // ...or the instruction is placed after all jump targets from condition.
                    if (
                      (*GET(boost::target(*toe, graph_))->start)->GetAddress()
                        > (*gr->start)->GetAddress()
                    ){
                        after_jump_jargets = false;
                    }
                }
                if (after_jump_jargets) is_continue = false;

                if (is_continue && ValidateBreakOrContinue(gr, target_gr))
                    gr->type = GROUP_TYPE_CONTINUE;
            }
        }
    }
}

bool ControlFlow::ValidateBreakOrContinue(GroupPtr group, GroupPtr condition_group){
    GroupPtr from;
    GroupPtr to;
    GroupPtr cursor;
    if (condition_group->type == GROUP_TYPE_DO_WHILE){
        to = condition_group;
        from = group;
    }
    else{
        to = group;
        from = condition_group->next;
    }
    GROUP_TYPE ogt = (
      condition_group->type == GROUP_TYPE_DO_WHILE ? GROUP_TYPE_WHILE : GROUP_TYPE_DO_WHILE
    );
    // Verify that destination deals with innermost while/do-while.
    for (cursor = from; cursor->next != NULL && cursor != to; cursor = cursor->next){
        if (cursor->type == condition_group->type){
            OutEdgeRange oer_validate = boost::out_edges(Find(cursor->start), graph_);
            for (
              OutEdgeIterator oe_validate = oer_validate.first;
              oe_validate != oer_validate.second;
              ++ oe_validate
            ){
                GraphVertex v_validate = boost::target(*oe_validate, graph_);
                GroupPtr g_validate = GET(v_validate);
                // For all other loops of same type found in range,
                // all targets must fall within that range.
                if (
                  (*g_validate->start)->GetAddress() < (*from->start)->GetAddress()
                  || (*g_validate->start)->GetAddress() > (*to->start)->GetAddress()
                ){
                    return false;
                }
                InEdgeRange ier_validate = boost::in_edges(v_validate, graph_);
                for (
                  InEdgeIterator ie_validate = ier_validate.first;
                  ie_validate != ier_validate.second;
                  ++ ie_validate
                ){
                    GroupPtr ig_validate = GET(boost::source(*ie_validate, graph_));
                    // All loops of other type going into range must be placed within range.
                    if (
                      ig_validate->type == ogt
                      && (
                        (*ig_validate->start)->GetAddress() < (*from->start)->GetAddress()
                        || (*ig_validate->start)->GetAddress() > (*to->start)->GetAddress()
                      )
                    ){
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

void ControlFlow::DetectIf(){
    VertexRange vr = boost::vertices(graph_);
    for (VertexIterator v = vr.first; v != vr.second; ++v){
        GroupPtr gr = GET(*v);
        // If: Undetermined block with conditional jump.
        if (gr->type == GROUP_TYPE_NORMAL && ((*gr->end)->IsCondJump()))
            gr->type = GROUP_TYPE_IF;
    }
}

void ControlFlow::DetectElse(){
    VertexRange vr = boost::vertices(graph_);
    for (VertexIterator v = vr.first; v != vr.second; ++v){
        GroupPtr gr = GET(*v);
        if (gr->type == GROUP_TYPE_IF){
            OutEdgeRange oer = boost::out_edges(*v, graph_);
            GraphVertex target;
            uint32 max_address = 0;
            GroupPtr target_gr;
            // Find jump target.
            for (OutEdgeIterator oe = oer.first; oe != oer.second; ++ oe){
                target_gr = GET(boost::target(*oe, graph_));
                if ((*target_gr->start)->GetAddress() > max_address){
                    target = boost::target(*oe, graph_);
                    max_address = (*target_gr->start)->GetAddress();
                }
            }
            target_gr = GET(target);
            // Else: Jump target of if immediately preceded by an unconditional jump...
            if (!(*target_gr->prev->end)->IsUncondJump()) continue;
            // ...which is not a break or a continue...
            if (
              target_gr->prev->type == GROUP_TYPE_CONTINUE
              || target_gr->prev->type == GROUP_TYPE_BREAK
            ){
                continue;
            }
            // ...to later in the code.
            OutEdgeIterator toe = boost::out_edges(
              Find((*target_gr->prev->start)->GetAddress()), graph_
            ).first;
            GroupPtr target_target_gr = GET(boost::target(*toe, graph_));
            if ((*target_target_gr->start)->GetAddress() > (*target_gr->end)->GetAddress()){
                if (ValidateElseBlock(gr, target_gr, target_target_gr)){
                    target_gr->start_else = true;
                    target_target_gr->prev->end_else.push_back(target_gr.get());
                }
            }
        }
    }
}

bool ControlFlow::ValidateElseBlock(GroupPtr if_group, GroupPtr start, GroupPtr end){
    for (GroupPtr cursor = start; cursor != end; cursor = cursor->next){
        if (
          cursor->type == GROUP_TYPE_IF
          || cursor->type == GROUP_TYPE_WHILE
          || cursor->type == GROUP_TYPE_DO_WHILE
        ){
            // Validate outgoing edges of conditions.
            OutEdgeRange oer = boost::out_edges(Find(cursor->start), graph_);
            for (OutEdgeIterator oe = oer.first; oe != oer.second; ++ oe){
                GraphVertex target = boost::target(*oe, graph_);
                GroupPtr target_gr = GET(target);
                // Each edge from condition must not leave the range [start, end].
                if (
                  (*start->start)->GetAddress() > (*target_gr->start)->GetAddress()
                  || (*target_gr->start)->GetAddress() > (*end->start)->GetAddress()
                ){
                    return false;
                }
            }
        }
        // If previous group ends an else, that else must start inside the range.
        for (
          ElseEndIterator it = cursor->prev->end_else.begin();
          it != cursor->prev->end_else.end();
          ++ it
        ){
            if ((*(*it)->start)->GetAddress() < (*start->start)->GetAddress()) return false;
        }
        // Unless group is a simple unconditional jump...
        if ((*cursor->start)->IsUncondJump()) continue;
        // ...validate ingoing edges
        InEdgeRange ier = boost::in_edges(Find(cursor->start), graph_);
        for (InEdgeIterator ie = ier.first; ie != ier.second; ++ie){
            GraphVertex source = boost::source(*ie, graph_);
            GroupPtr source_gr = GET(source);
            // Edges going to conditions...
            if (
              source_gr->type == GROUP_TYPE_IF
              || source_gr->type == GROUP_TYPE_WHILE
              || source_gr->type == GROUP_TYPE_DO_WHILE
            ){
                // ...must not come from outside the range [start, end]...
                if (
                  (*start->start)->GetAddress() > (*source_gr->start)->GetAddress()
                  || (*source_gr->start)->GetAddress() > (*end->start)->GetAddress()
                ){
                    // ...unless source is simple unconditional jump...
                    if ((*source_gr->start)->IsUncondJump()) continue;
                    // ...or the edge is from the if condition associated with this else.
                    if (if_group == source_gr) continue;
                    return false;
                }
            }
        }
    }
    return true;
}
