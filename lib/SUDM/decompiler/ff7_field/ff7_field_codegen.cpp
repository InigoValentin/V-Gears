#include "ff7_field_codegen.h"
#include "ff7_field_engine.h"
#include <boost/algorithm/string/predicate.hpp>

void FF7::FF7CodeGenerator::onBeforeStartFunction(const Function& func)
{
    // Start class
    FunctionMetaData metaData(func._metadata);
    if (metaData.IsStart())
    {
        addOutputLine("EntityContainer[\"" + metaData.EntityName() + "\"] = {", false, true);
        if (metaData.CharacterId() != -1)
        {
            addOutputLine(metaData.EntityName() + " = nil,");
        }
        addOutputLine("");
    }
}

void FF7::FF7CodeGenerator::onStartFunction(const Function& func)
{
    addOutputLine("--[[");
    for (const auto& inst : mInsts)
    {
        if (inst->_address >= func.mStartAddr && inst->_address <= func.mEndAddr)
        {
            std::stringstream output;
            output << inst;
            addOutputLine(output.str());
        }
    }
    addOutputLine("]]\n");
}

void FF7::FF7CodeGenerator::onEndFunction(const Function& func)
{
    // End function
    addOutputLine("end,", true, false);
    
    // End class
    FunctionMetaData metaData(func._metadata);
    if (metaData.IsEnd())
    {
        addOutputLine("}", true, false);
    }
}

std::string FF7::FF7CodeGenerator::constructFuncSignature(const Function &func)
{
    // Generate name
    return func._name + " = function(self)";
}

void FF7::FF7SimpleCodeGenerator::generate(InstVec& insts, const Graph& /*g*/){
    // TODO: Break into parts

    auto instruction = insts.begin();
    
    std::vector<std::pair<Function&, InstVec>> functions_with_bodies;
    for (
      auto function = _engine->_functions.begin();
      function != _engine->_functions.end();
      ++ function
    ){
        InstVec body;
        for (size_t i = 0; i < function->second.mNumInstructions; ++i, ++instruction)
            body.push_back(*instruction);
        functions_with_bodies.push_back(std::pair<Function&, InstVec> {function->second, body});
    }

    for (
      auto function = functions_with_bodies.begin();
      function != functions_with_bodies.end();
      ++ function
    ){
        onBeforeStartFunction(function->first);
        auto signature = constructFuncSignature(function->first);
        addOutputLine(signature, false, true);
        onStartFunction(function->first);
        
        // Comment with original instructions.
        std::unordered_map<uint32, InstVec> labels;
        for (
          auto instruction = function->second.begin();
          instruction != function->second.end();
          ++ instruction
        ){
            if ((*instruction)->isCondJump() || (*instruction)->isUncondJump()){
                auto targetAddr = (*instruction)->getDestAddress();
                auto label = labels.find(targetAddr);
                if (label == labels.end()) labels.insert({targetAddr, InstVec()});
                labels[targetAddr].push_back(*instruction);
            }
        }

        // Implemented instructions.
        for (
          auto instruction = function->second.begin();
          instruction != function->second.end();
          ++instruction
        ){
            auto label = labels.find((*instruction)->_address);
            if (label != labels.end()){
                bool needs_label = false;
                bool needs_new_line = false;
                for (auto origin = label->second.begin(); origin != label->second.end(); ++ origin){
                    if ((*origin)->isCondJump()){
                        addOutputLine("end", true, false);
                        needs_new_line = true;
                    }
                    else needs_label = true;
                }

                if (needs_new_line) addOutputLine("");
                if (needs_label)
                    addOutputLine((boost::format("::label_0x%1$X::") % label->first).str());
            }

            ValueStack stack;
            (*instruction)->processInst(function->first, stack, _engine, this);

            if ((*instruction)->isCondJump()){
                addOutputLine(
                  (boost::format("if (%s) then") % stack.pop()->getString()).str(), false, true
                );

                // If there are no more instructions then ensure end is outputted.
                if (instruction + 1 == std::end(function->second)){
                    addOutputLine("end", true, false);
                }
            }
            else if ((*instruction)->isUncondJump()){
                // If destination address is outside the functions, turn goto into a return.
                if ((*instruction)->getDestAddress() > function->first.mEndAddr){
                    addOutputLine(
                      "-- Overflowed jump to "
                      + (boost::format("0x%1$X") % (*instruction)->getDestAddress()).str()
                      + " (last address in function is "
                      + (boost::format("0x%1$X)") % function->first.mEndAddr).str()
                    );
                    addOutputLine("do return 0 end");
                }
                else{
                    addOutputLine(
                      (boost::format("goto label_0x%1$X") % (*instruction)->getDestAddress()).str()
                    );
                }
            }
            // Else, already output'd.
        }

        if (
          "return 0" != mLines.at(mLines.size() - 1)._line
          && "do return 0 end" != mLines.at(mLines.size() - 1)._line
        ){
            /*if (function->first._name == "Init"){
                addOutputLine("--do return 0 end -- INIT return, omit.", false, false);
            }
            addOutputLine("-- " + function->first._name + ": Missing original RET", false, false);
            addOutputLine("--LAST --" + mLines.at(mLines.size() - 1)._line + "--", false, false);*/
            addOutputLine("do return 0 end", false, false);
        }
        onEndFunction(function->first);
    }

    for (auto i = mLines.begin(); i != mLines.end(); ++i){
        if (i->_unindentBefore){
            assert(_indentLevel > 0);
            _indentLevel --;
        }
        _output << indentString(i->_line) << std::endl;
        if (i->_indentAfter) _indentLevel++;
    }
}

void FF7::FF7SimpleCodeGenerator::addOutputLine(std::string s, bool unindentBefore, bool indentAfter)
{
    mLines.push_back(CodeLine(s, unindentBefore, indentAfter));
}

float FF7::FF7SimpleCodeGenerator::ScaleFactor() const
{
    return static_cast<FF7::FF7FieldEngine*>(_engine)->ScaleFactor();
}

void FF7::FF7SimpleCodeGenerator::onBeforeStartFunction(const Function& func)
{
    // Start class
    FunctionMetaData metaData(func._metadata);
    if (metaData.IsStart())
    {
        addOutputLine("EntityContainer[\"" + metaData.EntityName() + "\"] = {", false, true);
        if (metaData.CharacterId() != -1)
        {
            addOutputLine(metaData.EntityName() + " = nil,\n");
        }
    }

    const auto comment = mFormatter.FunctionComment(metaData.EntityName(), func._name);
    if (!comment.empty())
    {
        addOutputLine("-- " + comment);
    }
}

void FF7::FF7SimpleCodeGenerator::onStartFunction(const Function& func)
{
    addOutputLine("--[[");
    for (const auto& inst : mInsts)
    {
        if (inst->_address >= func.mStartAddr && inst->_address <= func.mEndAddr)
        {
            std::stringstream output;
            output << inst;
            addOutputLine(output.str());
        }
    }
    addOutputLine("]]\n");

    if (func._name == "on_start" || func._name == "init")
    {
        addOutputLine("-- HACK ensure camera follows cloud, fix in engine properly later");
        addOutputLine("background2d:autoscroll_to_entity(entity_manager:get_entity(\"Cloud\"))");
    }
  
}

void FF7::FF7SimpleCodeGenerator::onEndFunction(const Function& func)
{
    // End function
    addOutputLine("end,", true, false);

    // End class
    FunctionMetaData metaData(func._metadata);
    if (metaData.IsEnd())
    {
        addOutputLine("}\n\n\n", true, false);
    }
    else
    {
        addOutputLine("\n");
    }
}

std::string FF7::FF7SimpleCodeGenerator::constructFuncSignature(const Function &func)
{
    // Generate name
    FunctionMetaData metaData(func._metadata);
    return mFormatter.FunctionName(metaData.EntityName(), func._name) + " = function(self)";
}

const std::string FF7::FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(const std::string& entity, uint32 address, uint32 opcode)
{
    return (boost::format("-- log:log(\"In entity \\\"%1%\\\", address 0x%2$08x: instruction 0x%3$04x not implemented\")") % entity % address % opcode).str();
}

const std::string FF7::FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(const std::string& entity, uint32 address, const Instruction& instruction)
{
    std::stringstream parameterList;
    for (auto i = instruction._params.begin(); i != instruction._params.end(); ++i)
    {
        if (i != instruction._params.begin())
        {
            parameterList << ", ";
        }
        parameterList << *i;
    }
    return (boost::format("-- log:log(\"In entity \\\"%1%\\\", address 0x%2$08x: instruction %3%(%4%) not implemented\")") % entity % address % instruction._name % parameterList.str()).str();
}

const std::string FF7::FF7CodeGeneratorHelpers::FormatBool(uint32 value)
{
    return value == 0 ? "false" : "true";
}

const std::string FF7::FF7CodeGeneratorHelpers::FormatInvertedBool(uint32 value)
{
    return value == 0 ? "true" : "false";
}
