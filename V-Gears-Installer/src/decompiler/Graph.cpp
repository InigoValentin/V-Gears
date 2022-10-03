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


#include "decompiler/Graph.h"
#include "decompiler/Engine.h"

IsJump::IsJump(){is_jump = false;};

IsJump::IsJump(bool jump): is_jump(jump){};

CodeLine::CodeLine(const std::string& line, bool unindent_before, bool indent_after):
  line(line), unindent_before(unindent_before), indent_after(indent_after){}

Group::Group(): stack_level(-1), type(GROUP_TYPE_NORMAL){}

Group::Group(GraphVertex vertex, InstIterator start, InstIterator end, GroupPtr prev){
    this->vertex = vertex;
    this->start = start;
    this->end = end;
    stack_level = -1;
    type = GROUP_TYPE_NORMAL;
    this->prev = prev.get();
    start_else = false;
    if (prev != NULL) prev->next = this;
    next = NULL;
    coalesced_else = false;
}

GraphProperties::GraphProperties(Engine *engine, const Graph &graph): engine(engine), graph(&graph)
{}

GraphProperties::GraphProperties(const GraphProperties& rhs){*this = rhs;}

GraphProperties& GraphProperties::operator = (const GraphProperties& rhs){
    if (this != &rhs){
        engine = rhs.engine;
        graph = rhs.graph;
    }
    return *this;
}

void GraphProperties::operator()(std::ostream& out) const{
    out << "node [shape=record]" << std::endl;
    for (
      FuncMap::iterator fn = engine->functions.begin();
      fn != engine->functions.end();
      ++ fn
    ){
        int index = (boost::get(boost::vertex_index, *graph, fn->second.vertex));
        out << "XXX" << index << " [shape=none, label=\"\", height=0]" << std::endl;
        out << "XXX" << index << " -> " << index << std::endl;
    }
}
