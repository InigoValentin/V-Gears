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

#include "decompiler/decompiler_engine.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <boost/format.hpp>
#include "decompiler/CodeGenerator.h"
#include "decompiler/LuaLanguage.h"

#define GET(vertex)    (boost::get(boost::vertex_name, _g, vertex))
#define GET_EDGE(edge) (boost::get(boost::edge_attribute, _g, edge))

void CodeGenerator::OnBeforeStartFunction(const Function&)
{
}

void CodeGenerator::OnEndFunction(const Function &)
{
    AddOutputLine("}", true, false);
}

std::string CodeGenerator::ConstructFuncSignature(const Function &)
{
    return "";
}

std::string CodeGenerator::indentString(std::string s)
{
    std::stringstream stream;
    stream << std::string(INDENT_SPACES * _indentLevel, ' ') << s;
    return stream.str();
}

CodeGenerator::CodeGenerator(Engine *engine, std::ostream &output, ARGUMENT_ORDER binOrder, ARGUMENT_ORDER callOrder)
   : _output(output),
    _binOrder(binOrder),
    _callOrder(callOrder)
{
    _engine = engine;
    _indentLevel = 0;
    target_lang_ = std::make_unique<LuaLanguage>();
}

typedef std::pair<GraphVertex, ValueStack> DFSEntry;

void CodeGenerator::generatePass(InstVec& insts, const Graph& g)
{
    _g = g;
    for (FuncMap::iterator fn = _engine->_functions.begin(); fn != _engine->_functions.end(); ++fn)
    {
        while (!_stack.empty())
        {
            _stack.pop();
        }
        GraphVertex entryPoint = fn->second._v;
        std::string funcSignature = ConstructFuncSignature(fn->second);

        // Write the function start
        bool printFuncSignature = !funcSignature.empty();
        if (printFuncSignature)
        {
            cur_group_ = GET(entryPoint);
            if (!(fn == _engine->_functions.begin()))
            {
                 AddOutputLine("");
            }
            OnBeforeStartFunction(fn->second);

            AddOutputLine(funcSignature, false, true);

            OnStartFunction(fn->second);
        }

        GroupPtr lastGroup = GET(entryPoint);

        // DFS from entry point to process each vertex
        Stack<DFSEntry> dfsStack;
        std::set<GraphVertex> seen;
        dfsStack.push(DFSEntry(entryPoint, ValueStack()));
        seen.insert(entryPoint);
        while (!dfsStack.empty())
        {
            DFSEntry e = dfsStack.pop();
            GroupPtr tmp = GET(e.first);
            if ((*tmp->start_)->_address > (*lastGroup->start_)->_address)
            {
                lastGroup = tmp;
            }
            _stack = e.second;
            GraphVertex v = e.first;
            process(fn->second, insts, v);
            OutEdgeRange r = boost::out_edges(v, _g);
            for (OutEdgeIterator i = r.first; i != r.second; ++i)
            {
                GraphVertex target = boost::target(*i, _g);
                if (seen.find(target) == seen.end())
                {
                    dfsStack.push(DFSEntry(target, _stack));
                    seen.insert(target);
                }
            }
        }

        // Write the function end
        if (printFuncSignature)
        {
            cur_group_ = lastGroup;
            OnEndFunction(fn->second);
        }

        // Print output
        GroupPtr p = GET(entryPoint);
        while (p != NULL)
        {
            for (auto it = p->_code.begin(); it != p->_code.end(); ++it)
            {
                if (it->_unindentBefore)
                {
                    assert(_indentLevel > 0);
                    _indentLevel--;
                }

                if (OutputOnlyRequiredLabels())
                {
                    _output << indentString(it->_line) << std::endl;
                }
                else
                {
                    _output << boost::format("%08X: %s") % (*p->start_)->_address % indentString(it->_line) << std::endl;
                }

                if (it->_indentAfter)
                {
                    _indentLevel++;
                }
            }
            p = p->_next;
        }
    }
}

void CodeGenerator::Generate(InstVec& insts, const Graph &g)
{
    if (OutputOnlyRequiredLabels())
    {
        // Call twice, once where no output is generated but instructions are
        // marked as "needs label", then a 2nd time to actually output the code
        mIsLabelPass = true;
        generatePass(insts, g);
    }
    mIsLabelPass = false;
    generatePass(insts, g);
}

void CodeGenerator::AddOutputLine(std::string s, bool unindentBefore, bool indentAfter)
{
    // We don't generate output in the labels pass, we just find instructions that
    // require a label to be outputted
    if (!mIsLabelPass)
    {
        cur_group_->_code.push_back(CodeLine(s, unindentBefore, indentAfter));
    }
}

void CodeGenerator::writeAssignment(ValuePtr dst, ValuePtr src) 
{
    std::stringstream s;
    s << dst << " = " << src << target_lang_->LineTerminator();
    AddOutputLine(s.str());
}

void CodeGenerator::process(Function& func, InstVec& insts, GraphVertex v)
{
    _curVertex = v;
    cur_group_ = GET(v);

    // Check if we should add else start
    if (cur_group_->_startElse)
    {
        AddOutputLine(target_lang_->EndBlock(LuaLanguage::TO_ELSE_BLOCK) + " " + target_lang_->Else() + " " + target_lang_->StartBlock(LuaLanguage::BEGIN_ELSE), true, true);
    }

    // Check ingoing edges to see if we want to add any extra output
    InEdgeRange ier = boost::in_edges(v, _g);
    for (InEdgeIterator ie = ier.first; ie != ier.second; ++ie)
    {
        GraphVertex in = boost::source(*ie, _g);
        GroupPtr inGroup = GET(in);

        if (!boost::get(boost::edge_attribute, _g, *ie)._isJump || inGroup->_stackLevel == -1)
        {
            continue;
        }

        switch (inGroup->_type)
        {
        case kDoWhileCondGroupType:
            AddOutputLine(target_lang_->DoLoopHeader(), false, true);
            break;
        case kIfCondGroupType:
            if (!cur_group_->_startElse)
            {
                AddOutputLine(target_lang_->EndBlock(LuaLanguage::END_OF_IF), true, false);
            }
            break;
        case kWhileCondGroupType:
            AddOutputLine(target_lang_->EndBlock(LuaLanguage::END_OF_WHILE), true, false);
            break;
        default:
            break;
        }
    }

    ConstInstIterator it = cur_group_->start_;
    do 
    {
        // If we only want to write labels that targets of goto's then check if this is the pass
        // after we've setup mLabelRequired on each instruction. If this is set then it needs a label
        // so write one out.
        if (OutputOnlyRequiredLabels() && !mIsLabelPass && (*it)->mLabelRequired)
        {
            AddOutputLine(target_lang_->Label((*it)->_address));
        }
        ProcessInst(func, insts, *it);
    } while (it++ != cur_group_->end_);

    // Add else end if necessary
    for (ElseEndIterator elseIt = cur_group_->_endElse.begin(); elseIt != cur_group_->_endElse.end(); ++elseIt)
    {
        if (!(*elseIt)->_coalescedElse)
        {
            AddOutputLine(target_lang_->EndBlock(LuaLanguage::END_OF_IF_ELSE_CHAIN), true, false);
        }
    }
}

void CodeGenerator::ProcessUncondJumpInst(Function& func, InstVec& insts, const InstPtr inst)
{
    switch (cur_group_->_type)
    {
    case kBreakGroupType:
        AddOutputLine(target_lang_->LoopBreak());
        break;
    case kContinueGroupType:
        AddOutputLine(target_lang_->LoopContinue());
        break;
    default: // Might be a goto
    {
        bool printJump = true;
        OutEdgeRange jumpTargets = boost::out_edges(_curVertex, _g);
        for (OutEdgeIterator target = jumpTargets.first; target != jumpTargets.second && printJump; ++target)
        {
            Group* next = cur_group_->_next;
            if (next)
            {
                // Don't output jump to next vertex
                if (boost::target(*target, _g) == next->_vertex)
                {
                    printJump = false;
                    break;
                }

                // Don't output jump if next vertex starts an else block
                if (next->_startElse)
                {
                    printJump = false;
                    break;
                }

                OutEdgeRange targetR = boost::out_edges(boost::target(*target, _g), _g);
                for (OutEdgeIterator targetE = targetR.first; targetE != targetR.second; ++targetE)
                {
                    // Don't output jump to while loop that has jump to next vertex
                    if (boost::target(*targetE, _g) == next->_vertex)
                    {
                        printJump = false;
                    }
                }

                if (printJump)
                {
                    // Check if this instruction is the last instruction in the function
                    // and its an uncond jump
                    if (cur_group_->_type == kDoWhileCondGroupType && inst->_address == func.mEndAddr && inst->IsUncondJump())
                    {
                        printJump = false;
                        AddOutputLine(target_lang_->DoLoopFooter(true) + "true" + target_lang_->DoLoopFooter(false), true, false);
                    }
                }
            }
        }

        if (printJump)
        {
            const uint32 dstAddr = inst->GetDestAddress();
            if (mIsLabelPass)
            {
                // Mark the goto target
                for (auto& i : insts)
                {
                    if (i->_address == dstAddr)
                    {
                        i->mLabelRequired = true;
                        break;
                    }
                }
            }
            AddOutputLine(target_lang_->Goto(dstAddr));
        }
    }
        break;
    }
}

void CodeGenerator::writeFunctionCall(std::string functionName, std::string paramsFormat, const std::vector<ValuePtr>& params)
{
    std::string strFuncCall = functionName + target_lang_->FunctionCallBegin();
    const char* str = paramsFormat.c_str();
    int paramIndex = 0;
    while (*str)
    {
        bool skipArgument = false;
        switch (*str)
        {
        case 'b':
            strFuncCall += params[paramIndex]->getUnsigned() ? "true" : "false";
            break;

        case 'n':
            strFuncCall += std::to_string(params[paramIndex]->getUnsigned());
            break;

        case 'f':
            strFuncCall += std::to_string(static_cast<float>(params[paramIndex]->getUnsigned()) / 30.0f);
            break;

        case '_': // Ignore param
            skipArgument = true;
            break;

        default:
            throw std::runtime_error("Unknown param type");
            break;
        }
        paramIndex++;
        str++;
        if (*str)
        {
            // There is another param
            if (!skipArgument)
            {
                strFuncCall += target_lang_->FunctionCallArgumentSeperator() + " ";
            }
        }
    }
    strFuncCall += target_lang_->FunctionCallEnd();
    AddOutputLine(strFuncCall);
}

void CodeGenerator::ProcessCondJumpInst(const InstPtr inst)
{
    std::stringstream s;
    switch (cur_group_->_type)
    {
    case kIfCondGroupType:
        if (cur_group_->_startElse && cur_group_->_code.size() == 1)
        {
            OutEdgeRange oer = boost::out_edges(_curVertex, _g);
            bool coalesceElse = false;
            for (OutEdgeIterator oe = oer.first; oe != oer.second; ++oe)
            {
                GroupPtr oGr = GET(boost::target(*oe, _g))->_prev;
                if (std::find(oGr->_endElse.begin(), oGr->_endElse.end(), cur_group_.get()) != oGr->_endElse.end())
                {
                    coalesceElse = true;
                }
            }
            if (coalesceElse)
            {
                cur_group_->_code.clear();
                cur_group_->_coalescedElse = true;
                s << target_lang_->EndBlock(LuaLanguage::TO_ELSE_BLOCK) << " " << target_lang_->Else() << " ";
            }
        }
        s << target_lang_->If(true) << _stack.pop()->negate() << target_lang_->If(false);
        AddOutputLine(s.str(), cur_group_->_coalescedElse, true);
        break;
    case kWhileCondGroupType:
        s << target_lang_->WhileHeader(true) << _stack.pop()->negate() << target_lang_->WhileHeader(false) << " " << target_lang_->StartBlock(LuaLanguage::BEGIN_WHILE);
        AddOutputLine(s.str(), false, true);
        break;
    case kDoWhileCondGroupType:
        s << target_lang_->EndBlock(LuaLanguage::END_WHILE) <<  " " << target_lang_->WhileHeader(true) << _stack.pop() << target_lang_->WhileHeader(false);
        AddOutputLine(s.str(), true, false);
        break;
    default:
        break;
    }
}

void CodeGenerator::ProcessInst(Function& func, InstVec& insts, const InstPtr inst)
{
    inst->ProcessInst(func, _stack, _engine, this);
    if (inst->isCondJump())
    {
        ProcessCondJumpInst(inst);
    }
    else if (inst->IsUncondJump())
    {
        ProcessUncondJumpInst(func, insts, inst);
    }
}

void CodeGenerator::addArg(ValuePtr p) 
{
    if (_callOrder == FIFO_ARGUMENT_ORDER)
        _argList.push_front(p);
    else if (_callOrder == LIFO_ARGUMENT_ORDER)
        _argList.push_back(p);
}

void CodeGenerator::ProcessSpecialMetadata(const InstPtr inst, char c, int) 
{
    switch (c) 
    {
    case 'p':
        addArg(_stack.pop());
        break;
    default:
        std::cerr << boost::format("WARNING: Unknown character in metadata: %c\n") % c;
        break;
    }
}
