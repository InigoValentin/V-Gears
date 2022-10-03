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
#include "decompiler/CodeGenerator.h"

#include "../../include/decompiler/Engine.h"
#include "decompiler/LuaLanguage.h"

#define GET(vertex)    (boost::get(boost::vertex_name, graph_, vertex))
#define GET_EDGE(edge) (boost::get(boost::edge_attribute, graph_, edge))

CodeGenerator::CodeGenerator(
  Engine *engine, std::ostream &output, ARGUMENT_ORDER bin_order, ARGUMENT_ORDER call_order
): output_(output), bin_order_(bin_order), call_order_(call_order)
{
    engine_ = engine;
    indent_level_ = 0;
    target_lang_ = std::make_unique<LuaLanguage>();
}

CodeGenerator::~CodeGenerator(){}

LuaLanguage& CodeGenerator::GetLanguage(){
    assert(target_lang_);
    return *target_lang_;
}

void CodeGenerator::WriteFunctionCall(
  std::string function_name, std::string param_format, const std::vector<ValuePtr>& params
){
    std::string func_call = function_name + target_lang_->FunctionCallBegin();
    const char* format = param_format.c_str();
    int param_index = 0;
    while (*format){
        bool skip_argument = false;
        switch (*format){
            case 'b': func_call += params[param_index]->getUnsigned() ? "true" : "false"; break;
            case 'n': func_call += std::to_string(params[param_index]->getUnsigned()); break;
            case 'f':
                func_call += std::to_string(
                  static_cast<float>(params[param_index]->getUnsigned()) / 30.0f
                );
                break;
            case '_': skip_argument = true; break;// Ignore param
            default: throw std::runtime_error("Unknown param type");
        }
        param_index ++;
        format ++;
        if (*format) {
            // There is another param.
            if (!skip_argument) func_call += target_lang_->FunctionCallArgumentSeperator() + " ";
        }
    }
    func_call += target_lang_->FunctionCallEnd();
    AddOutputLine(func_call);
}

void CodeGenerator::Generate(InstVec& insts, const Graph &graph){
    if (OutputOnlyRequiredLabels()){
        // Call twice, once where no output is generated but instructions are
        // marked as "needs label", then a 2nd time to actually output the code
        is_label_pass_ = true;
        GeneratePass(insts, graph);
    }
    is_label_pass_ = false;
    GeneratePass(insts, graph);
}

void CodeGenerator::AddOutputLine(std::string line, bool unindent_before, bool indent_after){
    // Don't generate output in the labels pass, just find
    // instructions that require a label to be outputted.
    if (!is_label_pass_) cur_group_->code.push_back(CodeLine(line, unindent_before, indent_after));
}

void CodeGenerator::WriteTodo(std::string class_name, std::string instruction){
    AddOutputLine("-- UNIMPLMENTED INSTRUCTION: \"" + instruction + "\")");
}

void CodeGenerator::WriteAssignment(ValuePtr dst, ValuePtr src) {
    std::stringstream s;
    s << dst << " = " << src << target_lang_->LineTerminator();
    AddOutputLine(s.str());
}

void CodeGenerator::AddArg(ValuePtr arg) {
    if (call_order_ == FIFO_ARGUMENT_ORDER) arg_list_.push_front(arg);
    else if (call_order_ == LIFO_ARGUMENT_ORDER) arg_list_.push_back(arg);
}

void CodeGenerator::ProcessSpecialMetadata(const InstPtr inst, char c, int) {
    switch (c){
    case 'p': AddArg(stack_.Pop()); break;
    default:
        std::cerr << boost::format("WARNING: Unknown character in metadata: %c\n") % c;
        break;
    }
}

ValueList CodeGenerator::GetArgList(){return arg_list_;}

ARGUMENT_ORDER CodeGenerator::GetBinaryOrder(){return bin_order_;}

void CodeGenerator::ProcessInst(Function& function, InstVec& insts, const InstPtr inst){
    inst->ProcessInst(function, stack_, engine_, this);
    if (inst->IsCondJump()) ProcessCondJumpInst(inst);
    else if (inst->IsUncondJump()) ProcessUncondJumpInst(function, insts, inst);
}

void CodeGenerator::ProcessUncondJumpInst(Function& function, InstVec& insts, const InstPtr inst){
    switch (cur_group_->type){
        case GROUP_TYPE_BREAK: AddOutputLine(target_lang_->LoopBreak()); break;
        case GROUP_TYPE_CONTINUE: AddOutputLine(target_lang_->LoopContinue()); break;
        default: // Might be a goto.
            {
                bool print_jump = true;
                OutEdgeRange jump_targets = boost::out_edges(cur_vertex_, graph_);
                for (
                  OutEdgeIterator target = jump_targets.first;
                  target != jump_targets.second && print_jump;
                  ++ target
                ){
                    Group* next = cur_group_->next;
                    if (next){
                        // Don't output jump to next vertex.
                        if (boost::target(*target, graph_) == next->vertex){
                            print_jump = false;
                            break;
                        }
                        // Don't output jump if next vertex starts an else block.
                        if (next->start_else){
                            print_jump = false;
                            break;
                        }
                        OutEdgeRange target_range = boost::out_edges(
                          boost::target(*target, graph_), graph_
                        );
                        for (
                          OutEdgeIterator target_it = target_range.first;
                          target_it != target_range.second;
                          ++ target_it
                        ){
                            // Don't output jump to while loop that has jump to next vertex.
                            if (boost::target(*target_it, graph_) == next->vertex)
                                print_jump = false;
                        }
                        if (print_jump){
                            // Check if this instruction is the last instruction in the function
                            // and its an uncond jump.
                            if (
                              cur_group_->type == GROUP_TYPE_DO_WHILE
                              && inst->GetAddress() == function.end_addr
                              && inst->IsUncondJump()
                            ){
                                print_jump = false;
                                AddOutputLine(
                                  target_lang_->DoLoopFooter(true) + "true"
                                  + target_lang_->DoLoopFooter(false), true, false
                                );
                            }
                        }
                    }
                }
                if (print_jump){
                    const uint32 dst_addr = inst->GetDestAddress();
                    if (is_label_pass_){
                        // Mark the goto target.
                        for (auto& i : insts){
                            if (i->GetAddress() == dst_addr){
                                i->SetLabelRequired(true);
                                break;
                            }
                        }
                    }
                    AddOutputLine(target_lang_->Goto(dst_addr));
                }
            }
        break;
    }
}

void CodeGenerator::ProcessCondJumpInst(const InstPtr inst){
    std::stringstream s;
    switch (cur_group_->type){
        case GROUP_TYPE_IF:
            if (cur_group_->start_else && cur_group_->code.size() == 1){
                OutEdgeRange oer = boost::out_edges(cur_vertex_, graph_);
                bool coalesce_else = false;
                for (OutEdgeIterator oe = oer.first; oe != oer.second; ++ oe){
                    GroupPtr oGr = GET(boost::target(*oe, graph_))->prev;
                    if (
                      std::find(oGr->end_else.begin(), oGr->end_else.end(), cur_group_.get())
                      != oGr->end_else.end()
                    ){
                        coalesce_else = true;
                    }
                }
                if (coalesce_else){
                    cur_group_->code.clear();
                    cur_group_->coalesced_else = true;
                    s << target_lang_->EndBlock(LuaLanguage::TO_ELSE_BLOCK)
                      << " " << target_lang_->Else() << " ";
                }
            }
            s << target_lang_->If(true) << stack_.Pop()->negate() << target_lang_->If(false);
            AddOutputLine(s.str(), cur_group_->coalesced_else, true);
            break;
        case GROUP_TYPE_WHILE:
            s << target_lang_->WhileHeader(true) << stack_.Pop()->negate()
              << target_lang_->WhileHeader(false) << " "
              << target_lang_->StartBlock(LuaLanguage::BEGIN_WHILE);
            AddOutputLine(s.str(), false, true);
            break;
        case GROUP_TYPE_DO_WHILE:
            s << target_lang_->EndBlock(LuaLanguage::END_WHILE) <<  " "
              << target_lang_->WhileHeader(true) << stack_.Pop()
              << target_lang_->WhileHeader(false);
            AddOutputLine(s.str(), true, false);
            break;
        default:
            break;
    }
}

std::string CodeGenerator::IndentString(std::string s){
    std::stringstream stream;
    stream << std::string(INDENT_SPACES * indent_level_, ' ') << s;
    return stream.str();
}

std::string CodeGenerator::ConstructFuncSignature(const Function& function){return "";}

void CodeGenerator::OnBeforeStartFunction(const Function& function){}

void CodeGenerator::OnEndFunction(const Function& function){AddOutputLine("}", true, false);}

void CodeGenerator::OnStartFunction(const Function& function){}

bool CodeGenerator::OutputOnlyRequiredLabels() const{return false;}

typedef std::pair<GraphVertex, ValueStack> DFSEntry;

void CodeGenerator::GeneratePass(InstVec& insts, const Graph& graph){
    graph_ = graph;
    for (
      FuncMap::iterator fn = engine_->functions.begin();
      fn != engine_->functions.end();
      ++ fn
    ){
        while (!stack_.IsEmpty()) stack_.Pop();
        GraphVertex entry_point = fn->second.vertex;
        std::string func_signature = ConstructFuncSignature(fn->second);
        // Write the function start.
        bool print_func_signature = !func_signature.empty();
        if (print_func_signature){
            cur_group_ = GET(entry_point);
            if (!(fn == engine_->functions.begin())) AddOutputLine("");
            OnBeforeStartFunction(fn->second);
            AddOutputLine(func_signature, false, true);
            OnStartFunction(fn->second);
        }
        GroupPtr last_group = GET(entry_point);
        // DFS from entry point to process each vertex.
        Stack<DFSEntry> dfs_stack;
        std::set<GraphVertex> seen;
        dfs_stack.Push(DFSEntry(entry_point, ValueStack()));
        seen.insert(entry_point);
        while (!dfs_stack.IsEmpty()){
            DFSEntry e = dfs_stack.Pop();
            GroupPtr tmp = GET(e.first);
            if ((*tmp->start)->GetAddress() > (*last_group->start)->GetAddress()) last_group = tmp;
            stack_ = e.second;
            GraphVertex v = e.first;
            Process(fn->second, insts, v);
            OutEdgeRange r = boost::out_edges(v, graph_);
            for (OutEdgeIterator i = r.first; i != r.second; ++ i){
                GraphVertex target = boost::target(*i, graph_);
                if (seen.find(target) == seen.end()){
                    dfs_stack.Push(DFSEntry(target, stack_));
                    seen.insert(target);
                }
            }
        }
        // Write the function end
        if (print_func_signature){
            cur_group_ = last_group;
            OnEndFunction(fn->second);
        }
        // Print output.
        GroupPtr p = GET(entry_point);
        while (p != NULL){
            for (auto it = p->code.begin(); it != p->code.end(); ++ it){
                if (it->unindent_before){
                    assert(indent_level_ > 0);
                    indent_level_--;
                }
                if (OutputOnlyRequiredLabels()) output_ << IndentString(it->line) << std::endl;
                else{
                    output_ << (
                      boost::format("%08X: %s") % (*p->start)->GetAddress() % IndentString(it->line)
                    ) << std::endl;
                }
                if (it->indent_after) indent_level_++;
            }
            p = p->next;
        }
    }
}

void CodeGenerator::Process(Function& function, InstVec& insts, GraphVertex vertex){
    cur_vertex_ = vertex;
    cur_group_ = GET(vertex);

    // Check if we should add else start
    if (cur_group_->start_else){
        AddOutputLine(
          target_lang_->EndBlock(LuaLanguage::TO_ELSE_BLOCK) + " " + target_lang_->Else()
          + " " + target_lang_->StartBlock(LuaLanguage::BEGIN_ELSE), true, true
        );
    }
    // Check ingoing edges to see if we want to add any extra output
    InEdgeRange ier = boost::in_edges(vertex, graph_);
    for (InEdgeIterator ie = ier.first; ie != ier.second; ++ ie){
        GraphVertex in = boost::source(*ie, graph_);
        GroupPtr in_group = GET(in);
        if (!boost::get(boost::edge_attribute, graph_, *ie).is_jump || in_group->stack_level == -1)
            continue;
        switch (in_group->type){
            case GROUP_TYPE_DO_WHILE:
                AddOutputLine(target_lang_->DoLoopHeader(), false, true);
                break;
            case GROUP_TYPE_IF:
                if (!cur_group_->start_else)
                    AddOutputLine(target_lang_->EndBlock(LuaLanguage::END_OF_IF), true, false);
                break;
            case GROUP_TYPE_WHILE:
                AddOutputLine(target_lang_->EndBlock(LuaLanguage::END_OF_WHILE), true, false);
                break;
            default:
                break;
        }
    }
    ConstInstIterator it = cur_group_->start;
    do{
        // If we only want to write labels that targets of goto's then check
        // if this is the pass after we've setup mLabelRequired on each
        // instruction. If this is set then it needs a label so write one out.
        if (OutputOnlyRequiredLabels() && !is_label_pass_ && (*it)->LabelRequired())
            AddOutputLine(target_lang_->Label((*it)->GetAddress()));
        ProcessInst(function, insts, *it);
    } while (it++ != cur_group_->end);
    // Add else end if necessary
    for (
      ElseEndIterator else_it = cur_group_->end_else.begin();
      else_it != cur_group_->end_else.end();
      ++ else_it
    ){
        if (!(*else_it)->coalesced_else)
            AddOutputLine(target_lang_->EndBlock(LuaLanguage::END_OF_IF_ELSE_CHAIN), true, false);
    }
}
