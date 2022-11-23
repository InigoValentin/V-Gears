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

#include <ostream>
#include <utility>
#include <vector>
#include <boost/format.hpp>
#include <boost/version.hpp>
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning(disable:4512)
#pragma warning(disable:4100)
#endif
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#ifdef _MSC_VER
#pragma warning (pop)
#endif
#include <boost/intrusive_ptr.hpp>
#include "instruction/Instruction.h"
#include "RefCounted.h"

/**
 * Enumeration representing the different kinds of groups.
 */
enum GROUP_TYPE {

    /**
     * Normal group.
     */
    GROUP_TYPE_NORMAL,

    /**
     * Group is the condition check for a while-loop.
     */
    GROUP_TYPE_WHILE,

    /**
     * Group is the condition check for a do-while-loop.
     */
    GROUP_TYPE_DO_WHILE,

    /**
     * Group is the condition check for an if.
     */
    GROUP_TYPE_IF,

    /**
     * Group is a break.
     */
    GROUP_TYPE_BREAK,

    /**
     * Group is a continue.
     */
    GROUP_TYPE_CONTINUE
};

struct Group;

typedef boost::intrusive_ptr<Group> GroupPtr;

typedef boost::property<boost::vertex_name_t, GroupPtr> GroupProperty;

typedef boost::property<boost::vertex_index_t, int, GroupProperty> GraphProperty;

/**
 * Structure representing whether or not an edge is a jump.
 */
struct IsJump {

    /**
     * Whether or not the edge is a jump.
     */
    bool is_jump = false;

    /**
     * Parameterless constructor.
     *
     * Required for use with STL and Boost, should not be called manually.
     */
    IsJump();

    /**
     * Constructor.
     *
     * @param[in] jump Whether or not the edge is a jump.
     */
    IsJump(bool jump);

    /**
     * Output edge information as a graphviz edge property.
     *
     * @param[out] output The stream to output to.
     * @param[in] is_jump The IsJump to output.
     * @return The stream used for output.
     */
    friend std::ostream &operator<<(std::ostream &output, IsJump is_jump){
        if (is_jump.is_jump) output << "empty";
        else output << "normal";
        return output;
    }
};

namespace boost {

    /**
     * Property writer for the IsJump property.
     */
    template <class Name> class ArrowheadWriter{

        public:

            /**
             * Constructor.
             *
             * @param[in] name The name of the attribute to use.
             */
            explicit ArrowheadWriter(Name name): name(name){}

            /**
             * Outputs the arrowhead edge property.
             *
             * @param[out] out The stream to output to.
             * @param[in] v The vertex or edge to output the attribute for.
             */
            template <class VertexOrEdge> void operator()(
              std::ostream& out, const VertexOrEdge& v
            ) const {
                out << "[arrowhead=\"" << get(name, v) << "\"]";
            }

        private:

            /**
             * The name of the attribute to use.
             */
            Name name;
    };

    /**
     * Creates an arrowhead property writer.
     *
     * @param[in] n The name of the attribute to use.
     */
    template <class Name> inline ArrowheadWriter<Name> MakeArrowheadWriter(Name n) {
        return ArrowheadWriter<Name>(n);
    }

}

typedef boost::property<boost::edge_attribute_t, IsJump> EdgeProperty;

typedef boost::adjacency_list<boost::setS, boost::listS, boost::bidirectionalS, GraphProperty, EdgeProperty> Graph;

typedef Graph::vertex_descriptor GraphVertex;

typedef Graph::vertex_iterator VertexIterator;

typedef Graph::edge_descriptor GraphEdge;

typedef Graph::out_edge_iterator OutEdgeIterator;

typedef Graph::in_edge_iterator InEdgeIterator;

typedef std::pair<VertexIterator, VertexIterator> VertexRange;

typedef std::pair<OutEdgeIterator, OutEdgeIterator> OutEdgeRange;

typedef std::pair<InEdgeIterator, InEdgeIterator> InEdgeRange;

/**
 * Structure representing a line of code.
 */
struct CodeLine {

    /**
     * The line of code.
     */
    std::string line = "0";

    /**
     * Whether or not to add an indentation level before outputting the line.
     */
    bool unindent_before = false;

    /**
     * Whether to remove an indentation level after outputting the line.
     */
    bool indent_after = false;

    /**
     * Constructor for CodeLine.
     *
     * @param[in] line The line of code.
     * @param[in] unindent_before Whether or not to remove an indentation
     * level before the line. Defaults to false.
     * @param[in] indent_after Whether or not to add an indentation level
     * after the line. Defaults to false.
     */
    CodeLine(const std::string& line, bool unindent_before, bool indent_after);
};

typedef std::vector<Group *> ElseEnds; // Weak references to groups ending else blocks.

typedef ElseEnds::iterator ElseEndIterator;

/**
 * Structure representing a group of instructions.
 */
struct Group : public RefCounted {

        /**
         * Vertex the group belongs to.
         */
        GraphVertex vertex;

        /**
         * First instruction in the group.
         */
        InstIterator start;

        /**
         * Last instruction in the group.
         */
        InstIterator end;

        /**
         * Level of the stack upon entry.
         */
        int stack_level;

        /**
         * Type of the group.
         */
        GROUP_TYPE type;

        /**
         * Group is start of an else block.
         */
        bool start_else;

        /**
         * Group is end of an else block.
         */
        ElseEnds end_else;

        /**
         * Pointer to the previous group, when ordered by address.
         *
         * Used for short-circuit analysis.
         */
        Group *prev;

        /**
         * Pointer to the next group, when ordered by address.
         */
        Group *next;

        /**
         * Decompiled lines of code.
         */
        std::vector<CodeLine> code;

        /**
         * Indicates if an starting else coalesces with another block.
         *
         * True if an else starting has been coalesced with another block
         * (e.g. "else if"). If true, an else starting here should not be
         * closed explicitly, but left to the other block.
         */
        bool coalesced_else;

        /**
         * Parameterless constructor.
         *
         * Required for use with STL and Boost, should not be called manually.
         */
        Group();

        /**
         * Constructor.
         *
         * @param[in] vertex The vertex the group belongs to.
         * @param[in] start First instruction in the group.
         * @param[in] end Last instruction in the group.
         * @param[in] prev Pointer to the previous group, when ordered by
         * address.
         */
        Group(GraphVertex vertex, InstIterator start, InstIterator end, GroupPtr prev);

        /**
         * Output a group to an stream as a graphviz label.
         *
         * @param[out] output The stream to output to.
         * @param[in] group The Group to output.
         * @return The stream used for output.
         */
        friend std::ostream &operator<<(std::ostream &output, GroupPtr group){
            output << "{Block type: ";
            switch(group->type) {
                case GROUP_TYPE_NORMAL: output << "Normal"; break;
                case GROUP_TYPE_WHILE: output << "While condition"; break;
                case GROUP_TYPE_DO_WHILE: output << "Do-while condition"; break;
                case GROUP_TYPE_IF: output << "If condition"; break;
                case GROUP_TYPE_BREAK: output << "Break"; break;
                case GROUP_TYPE_CONTINUE: output << "Continue"; break;
            }
            output << "\\n";
            output << "Expected stack level: " << group->stack_level << "\\n";
            if (group->start_else) output << "Start of else\\n";
            for (ElseEndIterator it = group->end_else.begin(); it != group->end_else.end(); ++ it)
                output << boost::format("End of else at %08x\\n") % (*(*it)->start)->GetAddress();
            output << "|";
            ConstInstIterator inst = group->start;
            do {
                std::stringstream stream;
                stream << *inst;
#if (BOOST_VERSION >= 104500)
                output << stream.str();
#else
                std::string s = stream.str();
                for (std::string::iterator it = s.begin(); it != s.end(); ++it)
                    if (*it == '"') output << "\\\"";
                    else if (*it == '|') output << "\\|";
                    else if (*it == '{') output << "\\{";
                    else if (*it == '}') output << "\\}";
                    else output << *it;
#endif
                output << "\\n";
            } while (inst ++ != group->end);
            output << "}";
            return output;
        }
};

class Engine;

/**
 * Type used to set properties for dot output.
 */
struct GraphProperties {

    public:

        /**
         * Constructor.
         *
         * @param[in] engine The engine with function information for the
         * script.
         * @param[in] graph he graph for the script.
         */
        GraphProperties(Engine *engine, const Graph &graph);

        /**
         * Copy constructor.
         *
         * @param[in] rhs Graph properties to copy.
         */
        GraphProperties(const GraphProperties& rhs);

        /**
         * Copy constructor.
         *
         * @param[in] rhs Graph properties to copy.
         */
        GraphProperties& operator = (const GraphProperties& rhs);

        /**
         * Print properties of the graph.
         *
         * Called by write_graphviz from Boost.Graph
         *
         * @param[out] out The stream write_graphviz is writing to.
         */
        void operator()(std::ostream& out) const;

    private:

        /**
         * Engine containing function information for the script.
         */
        Engine *engine;

        /**
         * Pointer to the to the graph for the script.
         */
        const Graph* graph;
};

