#include "ff7_field_engine.h"
#include "ff7_field_disassembler.h"
#include "ff7_field_codegen.h" 

#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include "make_unique.h"

#define GET(vertex) (boost::get(boost::vertex_name, g, vertex))

/*
OpCodes which need implementing (already done in DAT dumper)

"BLINK"
"XYI"
"CMOVE"
"MOVA"
"TURA"
"ANIMW"
"FMOVE"
"ANIME2"
"ANIM_1"
"CANIM1" ?
"CANM_1"
"TURN"
"DIRA"
"GETDIR"
"GETAXY"
"JUMP"
"AXYZI"
"LADER"
"OFST"
"TALKR"
"ANIMB"
"TURNW"
*/

std::unique_ptr<Disassembler> FF7::FF7FieldEngine::getDisassembler(
  InstVec &insts, const std::vector<unsigned char>& raw_script_data
){
    auto ret = std::make_unique<FF7Disassembler>(mFormatter, this, insts, raw_script_data);
    mScaleFactor = ret->ScaleFactor();
    return std::move(ret);
}

std::unique_ptr<Disassembler> FF7::FF7FieldEngine::getDisassembler(InstVec &insts){
    auto ret = std::make_unique<FF7Disassembler>(mFormatter, this, insts);
    mScaleFactor = ret->ScaleFactor();
    return std::move(ret);
}

std::unique_ptr<CodeGenerator> FF7::FF7FieldEngine::getCodeGenerator(
  const InstVec& insts, std::ostream &output
){
    // The broken version:
    //return std::make_unique<FF7CodeGenerator>(this, insts, output);
    // The not-as-nice-but-at-least-it-works version:
    return std::make_unique<FF7SimpleCodeGenerator>(this, insts, output, mFormatter);
}

void FF7::FF7FieldEngine::postCFG(InstVec& insts, Graph g){
    /*
    // In FF7 some scripts ends with an infinite loop to "keep it alive"
    // in QGears this isn't required so we can remove them
    RemoveTrailingInfiniteLoops(insts, g);

    // This could generate bad code, but it always seems to follow that pattern that if the last
    // instruction is an uncond jump back into the script then it simply nests all of those blocks
    // in an infinite loop
    MarkInfiniteLoopGroups(insts, g);

    // Scripts end with a "return" this isn't required so strip them out
    RemoveExtraneousReturnStatements(insts, g);
    */
}

std::map<std::string, int> FF7::FF7FieldEngine::GetEntities() const{
    std::map<std::string, int> r;
    for (auto& f : _functions){
        const Function& func = f.second;
        FF7::FunctionMetaData meta(func._metadata);
        auto it = r.find(meta.EntityName());
        if (it != std::end(r)){
            // Try to find a function in this entity has that has a char id
            // don't overwrite a valid char id with a "blank" one
            if (it->second == -1) it->second = meta.CharacterId();
        }
        // TODO: Don't add lines here:
        else r[meta.EntityName()] = meta.CharacterId();
    }
    return r;
}

std::vector<SUDM::FF7::Field::Line> FF7::FF7FieldEngine::GetLineList() const{
    std::vector<SUDM::FF7::Field::Line> lines;
    for (auto entity: mEntityIndexMap){
        if (entity.second.IsLine() == true){
            SUDM::FF7::Field::Line line;
            line.name = entity.second.Name();
            line.point_a = entity.second.GetLinePointA();
            line.point_b = entity.second.GetLinePointB();
            lines.push_back(line);
        }
    }
    return lines;
}

void FF7::FF7FieldEngine::AddEntityFunction(
  const std::string& entity_name, size_t entity_index,
  const std::string& func_name, size_t func_index
){
    auto it = mEntityIndexMap.find(entity_index);
    if (it != std::end(mEntityIndexMap)) (*it).second.AddFunction(func_name, func_index);
    else{
        Entity e(entity_name);
        e.AddFunction(func_name, func_index);
        mEntityIndexMap.insert(std::make_pair(entity_index, e));
    }
}

void FF7::FF7FieldEngine::MarkEntityAsLine(
  size_t entity_index, bool line, std::vector<float> point_a, std::vector<float> point_b
){
    auto it = mEntityIndexMap.find(entity_index);
    if (it != std::end(mEntityIndexMap)){
        (*it).second.MarkAsLine(line, point_a, point_b);
    }
}

bool FF7::FF7FieldEngine::EntityIsLine(size_t entity_index){
    auto it = mEntityIndexMap.find(entity_index);
    if (it != std::end(mEntityIndexMap)) return (*it).second.IsLine();
    return false;
}

void FF7::FF7FieldEngine::RemoveExtraneousReturnStatements(InstVec& insts, Graph g){
    for (auto& f : _functions){
        Function& func = f.second;
        for (auto it = insts.begin(); it != insts.end(); it ++){
            // Is it the last instruction in the function, and is it a return statement?
            if ((*it)->_address == func.mEndAddr){
                if ((*it)->_opcode == eOpcodes::RET){
                    // Set new end address to be before the NOP.
                    func.mEndAddr = (*(it - 1))->_address;
                    func.mNumInstructions --;
                    Instruction* nop = new FF7NoOperationInstruction();
                    nop->_opcode = eOpcodes::NOP;
                    nop->_address = (*it)->_address;
                    (*it).reset(nop);
                    break;
                }
            }
        }
    }
}

void FF7::FF7FieldEngine::RemoveTrailingInfiniteLoops(InstVec& insts, Graph g){
    for (auto& f : _functions){
        Function& func = f.second;
        for (auto it = insts.begin(); it != insts.end(); it ++){
            // Is it the last instruction in the function, a jump, and a jumping to itself?
            if ((*it)->_address == func.mEndAddr){
                if ((*it)->isJump() && (*it)->getDestAddress() == (*it)->_address){
                    // Set new end address to be before the NOP
                    func.mEndAddr = (*(it - 1))->_address;
                    func.mNumInstructions--;
                    Instruction* nop = new FF7NoOperationInstruction();
                    nop->_opcode = eOpcodes::NOP;
                    nop->_address = (*it)->_address;
                    (*it).reset(nop);
                    break;
                }
            }
        }
    }
}

void FF7::FF7FieldEngine::MarkInfiniteLoopGroups(InstVec& insts, Graph g){
    for (auto& f : _functions){
        Function& func = f.second;
        for (auto it = insts.begin(); it != insts.end(); it ++){
            if ((*it)->_address == func.mEndAddr){
                // Note: This is a "best effort heuristic", so quite a few loops will
                // still end up as goto's. This could potentially generate invalid code too.
                if ((*it)->isUncondJump()){
                    // Then assume its an infinite do { } while(true)
                    // loop that wraps part of the script.
                    VertexRange vr = boost::vertices(g);
                    for (VertexIterator v = vr.first; v != vr.second; ++ v){
                        GroupPtr gr = GET(*v);
                        if ((*gr->_start)->_address == func.mEndAddr){
                            // Then assume its an infinite do { } while(true) loop
                            // that wraps part of the script.
                            gr->_type = kDoWhileCondGroupType;
                        }
                    }
                }
                break;
            }
        }
    }
}

void FF7::FF7CondJumpInstruction::processInst(
  Function& function, ValueStack &stack, Engine* engine, CodeGenerator* code_gen
){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);

    std::string funcName;
    if (_opcode == eOpcodes::IFKEYON) funcName = "entity_manager:is_key_on";
    else if (_opcode == eOpcodes::IFKEYOFF) funcName = "entity_manager:is_key_off";
    else if (_opcode == eOpcodes::IFKEY) funcName = "Key";
    else if (_opcode == eOpcodes::IFMEMBQ) funcName = "IFMEMBQ";
    else if (_opcode == eOpcodes::IFPRTYQ) funcName = "IFPRTYQ";

    // If condition is a function, add and stop.
    if (!funcName.empty()){

        uint32 param = _params[0]->getUnsigned();

        // Special cases. The first parameter of IFKEY, IFKEYON and IFKEYOFF
        // can be ORed to get the individual keys, but there are two invalid
        // ones: 512 and 1024. They must be XORed.
        if (
          _opcode == eOpcodes::IFKEY
          || _opcode == eOpcodes::IFKEYON
          || _opcode == eOpcodes::IFKEYOFF
        ){
            if (param >= 1024) param = param ^ 1024;
            if (param >= 512) param = param ^ 512;
        }

        ValuePtr v = new UnqotedStringValue(funcName + "(" + std::to_string(param) + ")");
        stack.push(v);
        return;
    }

    std::string op;
    uint32 type = _params[4]->getUnsigned();
    const auto& source = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );

    switch (type){
        case 0: op = "=="; break;
        case 1: op = "~="; break;
        case 2: op = ">"; break;
        case 3: op = "<"; break;
        case 4: op = ">="; break;
        case 5: op = "<="; break;
        case 6: op = "&"; break;
        case 7: op = "^"; break;
        case 8: op = "|"; break;
        case 9:
            {
                op = "bit(" + _params[0]->getString() + ", " + _params[2]->getString()
                  + ", " + destination + ") == 1";
                ValuePtr v = new UnqotedStringValue(op);
                stack.push(v);
            }
            return;
        case 0xA:
            {
                op = "bit(" + _params[0]->getString() + ", " + _params[2]->getString()
                  + ", " + destination + ") == 0";
                ValuePtr v = new UnqotedStringValue(op);
                stack.push(v);
            }
            return;
        default: throw UnknownConditionalOperatorException(_address, type);
    }

    ValuePtr v = new BinaryOpValue(new VarValue(source), new VarValue(destination), op);
    stack.push(v); 
}

uint32 FF7::FF7CondJumpInstruction::getDestAddress() const{
    uint32 params_size = 0;
    uint32 jump_param_index = 5;
    switch (_opcode){
        case eOpcodes::IFUB: params_size = 5; break;
        case eOpcodes::IFUBL: params_size = 5; break;
        case eOpcodes::IFSW: params_size = 7; break;
        case eOpcodes::IFSWL: params_size = 7; break;
        case eOpcodes::IFUW: params_size = 7; break;
        case eOpcodes::IFUWL: params_size = 7; break;
        case eOpcodes::IFKEYON:
        case eOpcodes::IFKEYOFF:
        case eOpcodes::IFKEY:
            params_size = 3;
            jump_param_index = 1;
            break;
        case eOpcodes::IFPRTYQ:
        case eOpcodes::IFMEMBQ:
            params_size = 2;
            jump_param_index = 1;
            break;
        default: throw UnknownJumpTypeException(_address, _opcode);
    }
    return _address + _params[jump_param_index]->getUnsigned() + params_size;
}

std::ostream& FF7::FF7CondJumpInstruction::print(std::ostream &output) const{
    Instruction::print(output);
    output << " (False target address: 0x" << std::hex << getDestAddress() << std::dec << ")";
    return output;
}

bool FF7::FF7UncondJumpInstruction::isFuncCall() const{return _isCall;}

bool FF7::FF7UncondJumpInstruction::isUncondJump() const{return !_isCall;}

uint32 FF7::FF7UncondJumpInstruction::getDestAddress() const{
    if (
      static_cast<eOpcodes>(_opcode) == eOpcodes::JMPF
      || static_cast<eOpcodes>(_opcode) == eOpcodes::JMPFL){
        // Short or long forward jump.
        return _address + _params[0]->getUnsigned() + 1;
    }
    // Backwards jump,  eOpcodes::JMPB/L.
    return _address - _params[0]->getUnsigned();
}

std::ostream& FF7::FF7UncondJumpInstruction::print(std::ostream &output) const{
    Instruction::print(output);
    output << " (Jump target address: 0x" << std::hex << getDestAddress() << std::dec << ")";
    return output;
}


void FF7::FF7UncondJumpInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator* code_gen
){
    switch (_opcode){
        case eOpcodes::JMPB:
        case eOpcodes::JMPBL:
            // HACK: Hard loop will hang the game, insert a wait to yield control
            code_gen->addOutputLine("-- Hack, yield control for possible inf loop");
            code_gen->addOutputLine("script:wait(0)");
            break;
    }
}

static void WriteTodo(CodeGenerator *code_gen, std::string entity_name, std::string opCode){
    code_gen->addOutputLine("-- Todo(\"" + opCode + "\")");
}

void FF7::FF7ControlFlowInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FF7::FF7FieldEngine& eng = static_cast<FF7::FF7FieldEngine&>(*engine);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::RET:
            // A few notes in RET.
            // - Lua requires all functions to end with a return.
            // - Lua doesn's like returns if not followed by an end.
            // - Don't add a return statement to on_start functions.
            //     Game 'init' and 'main' code is concatenated in V-Gears. If the
            //     'main' return is kept, the code of 'main' is never executed.
            //     There are safeguards forfunctions without a return in the end,
            //     so it's OK not to include it here.
            if (func._name != "on_start") code_gen->addOutputLine("do return 0 end");
            break;
        case eOpcodes::REQ: processREQ(code_gen, eng); break;
        case eOpcodes::REQSW: processREQSW(code_gen, eng); break;
        case eOpcodes::REQEW: processREQEW(code_gen, eng); break;
        case eOpcodes::PREQ: WriteTodo(code_gen, md.EntityName(), "PREQ"); break;
        case eOpcodes::PRQSW: WriteTodo(code_gen, md.EntityName(), "PRQSW"); break;
        case eOpcodes::PRQEW: WriteTodo(code_gen, md.EntityName(), "PRQEW"); break;
        case eOpcodes::RETTO: processRETTO(code_gen); break;
        case eOpcodes::WAIT: processWAIT(code_gen); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode
            ));
    }
}

void FF7::FF7ControlFlowInstruction::processREQ(
  CodeGenerator* code_gen, const FF7FieldEngine& engine
){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& entity = engine.EntityByIndex(_params[0]->getSigned());
    const auto& script_name = cg->mFormatter.FunctionName(
      entity.Name(), entity.FunctionByIndex(_params[2]->getUnsigned())
    );
    auto priority = _params[1]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("script:request(Script.ENTITY, \"%1%\", \"%2%\", %3%)")
      % entity.Name() % script_name % priority
    ).str());
}

void FF7::FF7ControlFlowInstruction::processREQSW(
  CodeGenerator* code_gen, const FF7FieldEngine& engine
){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& entity = engine.EntityByIndex(_params[0]->getSigned());
    const auto& script_name = cg->mFormatter.FunctionName(
      entity.Name(), entity.FunctionByIndex(_params[2]->getUnsigned())
    );
    auto priority = _params[1]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("script:request_start_sync(Script.ENTITY, \"%1%\", \"%2%\", %3%)")
      % entity.Name() % script_name % priority
    ).str());
}

void FF7::FF7ControlFlowInstruction::processREQEW(
  CodeGenerator* code_gen, const FF7FieldEngine& engine
){
    try{
        FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
        const auto& entity = engine.EntityByIndex(_params[0]->getSigned());
        const auto& script_name = cg->mFormatter.FunctionName(
          entity.Name(), entity.FunctionByIndex(_params[2]->getUnsigned())
        );
        auto priority = _params[1]->getUnsigned();
        code_gen->addOutputLine((
          boost::format("script:request_end_sync(Script.ENTITY, \"%1%\", \"%2%\", %3%)")
          % entity.Name() % script_name % priority
        ).str());
    }
    catch (const InternalDecompilerError&){
        code_gen->addOutputLine((
          boost::format("-- ERROR call to non existing function index %1%")
          % _params[2]->getUnsigned()
        ).str());
    }
}

void FF7::FF7ControlFlowInstruction::processRETTO(CodeGenerator* code_gen){
    auto entity_index = _params[0]->getUnsigned();
    auto priority = _params[1]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("-- return_to(script_id_in_current_entity=%2%, priority=%1%)")
      % entity_index % priority
    ).str());
}

void FF7::FF7ControlFlowInstruction::processWAIT(CodeGenerator* code_gen){
    code_gen->addOutputLine((
      boost::format("script:wait(%1%)") % (_params[0]->getUnsigned() / 30.0f)
    ).str());
}

void FF7::FF7ModuleInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::DSKCG: WriteTodo(code_gen, md.EntityName(), "DSKCG"); break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::ARROW:
            code_gen->addOutputLine((
              boost::format("game:pointer_enable(%1%)")
              % (_params[0]->getUnsigned() ? "true" : "false")
            ).str());
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::PNAME:
            WriteTodo(code_gen, md.EntityName(), "PNAME");
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::GMSPD:
            WriteTodo(code_gen, md.EntityName(), "GMSPD");
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::SMSPD:
            WriteTodo(code_gen, md.EntityName(), "SMSPD");
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::FLMAT:
            code_gen->addOutputLine("game:fill_materia()");
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::FLITM:
            WriteTodo(code_gen, md.EntityName(), "FLITM");
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::BTLCK:
            code_gen->addOutputLine((
              boost::format("game:battle_enable(%1%)")
              % (_params[0]->getUnsigned() ? "true" : "false")
            ).str());
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::MVLCK:
            code_gen->addOutputLine((
              boost::format("game:movie_enable(%1%)")
              % (_params[0]->getUnsigned() ? "true" : "false")
            ).str());
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::SPCNM:
            WriteTodo(code_gen, md.EntityName(), "SPCNM");
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::RSGLB:
            code_gen->addOutputLine("game:global_reset()");
            break;
        case (eOpcodes::SPECIAL << 8) | eSpecialOpcodes::CLITM:
            WriteTodo(code_gen, md.EntityName(), "CLITM");
            break;
        case eOpcodes::MINIGAME: WriteTodo(code_gen, md.EntityName(), "MINIGAME"); break;
        case eOpcodes::BTMD2: WriteTodo(code_gen, md.EntityName(), "BTMD2"); break;
        case eOpcodes::BTRLD: WriteTodo(code_gen, md.EntityName(), "BTRLD"); break;
        case eOpcodes::BTLTB: WriteTodo(code_gen, md.EntityName(), "BTLTB"); break;
        case eOpcodes::MAPJUMP: processMAPJUMP(code_gen, func); break;
        case eOpcodes::LSTMP: WriteTodo(code_gen, md.EntityName(), "LSTMP"); break;
        case eOpcodes::BATTLE: processBATTLE(code_gen); break;
        case eOpcodes::BTLON: processBTLON(code_gen); break;
        case eOpcodes::BTLMD: WriteTodo(code_gen, md.EntityName(), "BTLMD"); break;
        case eOpcodes::MPJPO:
            // Gateway function will do nothing if this is set to true
            code_gen->addOutputLine(
              std::string("FFVII.Data.DisableGateways=")
              + (_params[0]->getUnsigned() ? "true" : "false")
            );
            break;
        // Prepare to change map, don't need to output anything for this.
        case eOpcodes::PMJMP: code_gen->addOutputLine("-- Prepare map change"); break;
        // Prepare to change map, don't need to output anything for this,
        // seems to be the same thing as PMJMP
        case eOpcodes::PMJMP2: code_gen->addOutputLine("-- Prepare map change 2"); break;
        case eOpcodes::GAMEOVER: WriteTodo(code_gen, md.EntityName(), "GAMEOVER"); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode
            ));
    }
}

void FF7::FF7ModuleInstruction::processBATTLE(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& battle_id = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    code_gen->addOutputLine((boost::format("entity_manager:battle_run(%1%)") % battle_id).str());
}

void FF7::FF7ModuleInstruction::processBTLON(CodeGenerator* code_gen){
    code_gen->addOutputLine((
      boost::format("entity_manager:random_encounters_on(%1%)")
      % FF7CodeGeneratorHelpers::FormatInvertedBool(_params[0]->getUnsigned())
    ).str());
}

void FF7::FF7ModuleInstruction::processMAPJUMP(CodeGenerator* code_gen, Function& func){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto target_map_id = _params[0]->getUnsigned();
    FunctionMetaData md(func._metadata);
    const std::string source_spawn_point_name
      = cg->mFormatter.SpawnPointName(target_map_id, md.EntityName(), func._name, _address);

    cg->mFormatter.AddSpawnPoint(
      target_map_id, md.EntityName(), func._name, _address,
      _params[1]->getSigned(), // X
      _params[2]->getSigned(), // Y
      _params[3]->getSigned(), // Walk mesh triangle ID
      _params[4]->getSigned()  // Angle
    );

    const std::string target_map_name = cg->mFormatter.MapName(target_map_id);
    code_gen->addOutputLine(
      "load_field_map_request(\"" + target_map_name + "\", \"" + source_spawn_point_name + "\")"
    );
}

void FF7::FF7MathInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::PLUS_: processSaturatedPLUS(code_gen); break;
        case eOpcodes::PLUS2_: processSaturatedPLUS2(code_gen); break;
        case eOpcodes::MINUS_: processSaturatedMINUS(code_gen); break;
        case eOpcodes::MINUS2_: processSaturatedMINUS2(code_gen); break;
        case eOpcodes::INC_: processSaturatedINC(code_gen); break;
        case eOpcodes::INC2_: processSaturatedINC2(code_gen); break;
        case eOpcodes::DEC_: processSaturatedDEC(code_gen); break;
        case eOpcodes::DEC2_: processSaturatedDEC2(code_gen); break;
        case eOpcodes::RDMSD: processRDMSD(code_gen); break;
        case eOpcodes::SETBYTE:
        case eOpcodes::SETWORD:
            processSETBYTE_SETWORD(code_gen);
            break;
        case eOpcodes::BITON: processBITON(code_gen); break;
        case eOpcodes::BITOFF: processBITOFF(code_gen); break;
        case eOpcodes::BITXOR: WriteTodo(code_gen, md.EntityName(), "BITXOR"); break;
        case eOpcodes::PLUS:
        case eOpcodes::PLUS2:
            processPLUSx_MINUSx(code_gen, "+");
            break;
        case eOpcodes::MINUS:
        case eOpcodes::MINUS2:
            processPLUSx_MINUSx(code_gen, "-");
            break;
        case eOpcodes::MUL: WriteTodo(code_gen, md.EntityName(), "MUL"); break;
        case eOpcodes::MUL2: WriteTodo(code_gen, md.EntityName(), "MUL2"); break;
        case eOpcodes::DIV: WriteTodo(code_gen, md.EntityName(), "DIV"); break;
        case eOpcodes::DIV2: WriteTodo(code_gen, md.EntityName(), "DIV2"); break;
        case eOpcodes::MOD:
            {
                const uint32 source_bank = _params[0]->getUnsigned();
                const uint32 source_address_or_value = _params[2]->getUnsigned();
                auto source = FF7::FF7CodeGeneratorHelpers::FormatValueOrVariable(
                  cg->mFormatter, source_bank, source_address_or_value
                );
                const uint32 dest_bank = _params[1]->getUnsigned();
                const uint32 dest_address = _params[3]->getUnsigned();
                auto dest = FF7::FF7CodeGeneratorHelpers::FormatValueOrVariable(
                  cg->mFormatter, dest_bank, dest_address
                );
                code_gen->addOutputLine(
                  source + " = " + source + " % " + dest + code_gen->TargetLang().LineTerminator()
                );
            }
        break;
        case eOpcodes::MOD2: WriteTodo(code_gen, md.EntityName(), "MOD2"); break;
        case eOpcodes::AND: WriteTodo(code_gen, md.EntityName(), "AND"); break;
        case eOpcodes::AND2: WriteTodo(code_gen, md.EntityName(), "AND2"); break;
        case eOpcodes::OR: WriteTodo(code_gen, md.EntityName(), "OR"); break;
        case eOpcodes::OR2: WriteTodo(code_gen, md.EntityName(), "OR2"); break;
        case eOpcodes::XOR: WriteTodo(code_gen, md.EntityName(), "XOR"); break;
        case eOpcodes::XOR2: WriteTodo(code_gen, md.EntityName(), "XOR2"); break;
        case eOpcodes::INC:
        case eOpcodes::INC2:
            processINCx_DECx(code_gen, "+");
            break;
        case eOpcodes::DEC:
        case eOpcodes::DEC2:
            processINCx_DECx(code_gen, "-");
            break;
        case eOpcodes::RANDOM: processRANDOM(code_gen); break;
        case eOpcodes::LBYTE: WriteTodo(code_gen, md.EntityName(), "LBYTE"); break;
        case eOpcodes::HBYTE: WriteTodo(code_gen, md.EntityName(), "HBYTE"); break;
        case eOpcodes::TWOBYTE: WriteTodo(code_gen, md.EntityName(), "2BYTE"); break;
        case eOpcodes::SIN: WriteTodo(code_gen, md.EntityName(), "SIN"); break;
        case eOpcodes::COS: WriteTodo(code_gen, md.EntityName(), "COS"); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode
            ));
    }
}

void FF7::FF7MathInstruction::processSaturatedPLUS(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->addOutputLine((boost::format("%1% = %1% + %2%") % lhs % rhs).str());
    code_gen->addOutputLine((boost::format("--if (%1% > 255); %1% = 255; end") % lhs).str());
}

void FF7::FF7MathInstruction::processSaturatedPLUS2(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->addOutputLine((boost::format("%1% = %1% + %2%") % lhs % rhs).str());
    code_gen->addOutputLine((boost::format("--if (%1% > 32767); %1% = 32767; end") % lhs).str());
}

void FF7::FF7MathInstruction::processSaturatedMINUS(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->addOutputLine((boost::format("%1% = %1% - %2%") % lhs % rhs).str());
    code_gen->addOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % lhs).str());
}

void FF7::FF7MathInstruction::processSaturatedMINUS2(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->addOutputLine((boost::format("%1% = %1% - %2%") % lhs % rhs).str());
    code_gen->addOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % lhs).str());
}

void FF7::FF7MathInstruction::processSaturatedINC(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->addOutputLine((boost::format("%1% = %1% + 1") % dest).str());
    code_gen->addOutputLine((boost::format("--if (%1% > 255); %1% = 255; end") % dest).str());
}

void FF7::FF7MathInstruction::processSaturatedINC2(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->addOutputLine((boost::format("%1% = %1% + 1") % dest).str());
    code_gen->addOutputLine((boost::format("--if (%1% > 32767); %1% = 32767; end") % dest).str());
}

void FF7::FF7MathInstruction::processSaturatedDEC(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->addOutputLine((boost::format("%1% = %1% - 1") % dest).str());
    code_gen->addOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % dest).str());
}

void FF7::FF7MathInstruction::processSaturatedDEC2(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->addOutputLine((boost::format("%1% = %1% - 1") % dest).str());
    code_gen->addOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % dest).str());
}

void FF7::FF7MathInstruction::processRDMSD(CodeGenerator* code_gen){
    // TODO: we don't have os.time...
    // TODO: RNG emulation?
    code_gen->addOutputLine("math.randomseed(os.time())");
}

void FF7::FF7MathInstruction::processSETBYTE_SETWORD(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& source = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: respect destination bank sizes (16-bit writes only affect low byte)
    code_gen->addOutputLine((boost::format("%1% = %2%") % destination % source).str());
}

void FF7::FF7MathInstruction::processBITON(CodeGenerator* code_gen){
    code_gen->addOutputLine((boost::format("bit_on(%1%, %2%, %3%)")
      % _params[0]->getUnsigned() % _params[2]->getUnsigned() % _params[3]->getUnsigned()
    ).str());
}

void FF7::FF7MathInstruction::processBITOFF(CodeGenerator* code_gen){
    code_gen->addOutputLine((boost::format("bit_on(%1%, %2%, %3%)")
      % _params[0]->getUnsigned() % _params[2]->getUnsigned() % _params[3]->getUnsigned()
    ).str());
}

void FF7::FF7MathInstruction::processPLUSx_MINUSx(CodeGenerator* code_gen, const std::string& op){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: repect destination bank sizes and wraparound
    code_gen->addOutputLine((boost::format("%1% = %1% %2% %3%") % lhs % op % rhs).str());
}

void FF7::FF7MathInstruction::processINCx_DECx(CodeGenerator* code_gen, const std::string& op){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: repect destination bank sizes and wraparound
    code_gen->addOutputLine((boost::format("%1% = %1% %2% 1") % destination % op).str());
}

void FF7::FF7MathInstruction::processRANDOM(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes (16-bit writes only affect low byte).
    // TODO: RNG emulation?
    code_gen->addOutputLine((boost::format("%1% = math.random(0, 255)") % destination).str());
}

void FF7::FF7WindowInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FF7::FF7FieldEngine& eng = static_cast<FF7::FF7FieldEngine&>(*engine);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::TUTOR: WriteTodo(code_gen, md.EntityName(), "TUTOR"); break;
        case eOpcodes::WCLS: WriteTodo(code_gen, md.EntityName(), "WCLS"); break;
        case eOpcodes::WSIZW: WriteTodo(code_gen, md.EntityName(), "WSIZW"); break;
        case eOpcodes::WSPCL: WriteTodo(code_gen, md.EntityName(), "WSPCL"); break;
        case eOpcodes::WNUMB: WriteTodo(code_gen, md.EntityName(), "WNUMB"); break;
        case eOpcodes::STTIM: WriteTodo(code_gen, md.EntityName(), "STTIM"); break;
        case eOpcodes::MESSAGE: processMESSAGE(code_gen, eng.ScriptName()); break;
        case eOpcodes::MPARA: WriteTodo(code_gen, md.EntityName(), "MPARA"); break;
        case eOpcodes::MPRA2: WriteTodo(code_gen, md.EntityName(), "MPRA2"); break;
        case eOpcodes::MPNAM: processMPNAM(code_gen); break;
        case eOpcodes::ASK: WriteTodo(code_gen, md.EntityName(), "ASK"); break;
        case eOpcodes::MENU: WriteTodo(code_gen, md.EntityName(), "MENU"); break;
        case eOpcodes::MENU2: processMENU2(code_gen); break;
        case eOpcodes::WINDOW: processWINDOW(code_gen); break;
        case eOpcodes::WMOVE: WriteTodo(code_gen, md.EntityName(), "WMOVE"); break;
        case eOpcodes::WMODE: WriteTodo(code_gen, md.EntityName(), "WMODE"); break;
        case eOpcodes::WREST: WriteTodo(code_gen, md.EntityName(), "WREST"); break;
        case eOpcodes::WCLSE: processWCLSE(code_gen); break;
        case eOpcodes::WROW: WriteTodo(code_gen, md.EntityName(), "WROW"); break;
        case eOpcodes::GWCOL: WriteTodo(code_gen, md.EntityName(), "GWCOL"); break;
        case eOpcodes::SWCOL: WriteTodo(code_gen, md.EntityName(), "SWCOL"); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode
            ));
    }
}

void FF7::FF7WindowInstruction::processWINDOW(CodeGenerator* code_gen){
    // Init a new window. It won't be displayed until MESSAGE is used.
    auto windowId = _params[0]->getUnsigned();
    auto x = _params[1]->getUnsigned();
    auto y = _params[2]->getUnsigned();
    auto width = _params[3]->getUnsigned();
    auto height = _params[4]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("dialog:dialog_open(\"%1%\", %2%, %3%, %4%, %5%)")
      % windowId % x % y % width % height
    ).str());
}

void FF7::FF7WindowInstruction::processMESSAGE(
  CodeGenerator* code_gen, const std::string& script_name
){
    // Displays a dialog in the WINDOW that has previously been initialized to display this dialog.
    auto window_id = _params[0]->getUnsigned();
    auto dialog_id = _params[1]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("dialog:dialog_set_text(\"%1%\", \"%2%_%3%\")")
      % window_id % script_name % dialog_id
    ).str());
    code_gen->addOutputLine(
      (boost::format("dialog:dialog_wait_for_close(\"%1%\")") % window_id).str()
    );

}

void FF7::FF7WindowInstruction::processWCLSE(CodeGenerator* code_gen){
    // Close a dialog.
    auto windowId = _params[0]->getUnsigned();
    code_gen->addOutputLine((boost::format("dialog:dialog_close(\"%1%\")") % windowId).str());
}

void FF7::FF7WindowInstruction::processMPNAM(CodeGenerator* code_gen){
    code_gen->addOutputLine(
      (boost::format("-- field:map_name(%1%)") % _params[0]->getUnsigned()).str()
    );
}

void FF7::FF7WindowInstruction::processMENU2(CodeGenerator* code_gen){
    code_gen->addOutputLine((
      boost::format("-- field:menu_lock(%1%)")
      % FF7CodeGeneratorHelpers::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FF7::FF7PartyInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::SPTYE: WriteTodo(code_gen, md.EntityName(), "SPTYE"); break;
        case eOpcodes::GTPYE: WriteTodo(code_gen, md.EntityName(), "GTPYE"); break;
        case eOpcodes::GOLDU: WriteTodo(code_gen, md.EntityName(), "GOLDU"); break;
        case eOpcodes::GOLDD: WriteTodo(code_gen, md.EntityName(), "GOLDD"); break;
        case eOpcodes::HMPMAX1: WriteTodo(code_gen, md.EntityName(), "HMPMAX1"); break;
        case eOpcodes::HMPMAX2: WriteTodo(code_gen, md.EntityName(), "HMPMAX2"); break;
        case eOpcodes::MHMMX: WriteTodo(code_gen, md.EntityName(), "MHMMX"); break;
        case eOpcodes::HMPMAX3: WriteTodo(code_gen, md.EntityName(), "HMPMAX3"); break;
        case eOpcodes::MPU: WriteTodo(code_gen, md.EntityName(), "MPU"); break;
        case eOpcodes::MPD: WriteTodo(code_gen, md.EntityName(), "MPD"); break;
        case eOpcodes::HPU: WriteTodo(code_gen, md.EntityName(), "HPU"); break;
        case eOpcodes::HPD: WriteTodo(code_gen, md.EntityName(), "HPD"); break;
        case eOpcodes::STITM: processSTITM(code_gen); break;
        case eOpcodes::DLITM: WriteTodo(code_gen, md.EntityName(), "DLITM"); break;
        case eOpcodes::CKITM: WriteTodo(code_gen, md.EntityName(), "CKITM"); break;
        case eOpcodes::SMTRA: WriteTodo(code_gen, md.EntityName(), "SMTRA"); break;
        case eOpcodes::DMTRA: WriteTodo(code_gen, md.EntityName(), "DMTRA"); break;
        case eOpcodes::CMTRA: WriteTodo(code_gen, md.EntityName(), "CMTRA"); break;
        case eOpcodes::GETPC: WriteTodo(code_gen, md.EntityName(), "GETPC"); break;
        case eOpcodes::PRTYP: WriteTodo(code_gen, md.EntityName(), "PRTYP"); break;
        case eOpcodes::PRTYM: WriteTodo(code_gen, md.EntityName(), "PRTYM"); break;
        case eOpcodes::PRTYE: processPRTYE(code_gen); break;
        case eOpcodes::MMBUD: WriteTodo(code_gen, md.EntityName(), "MMBUD"); break;
        case eOpcodes::MMBLK: WriteTodo(code_gen, md.EntityName(), "MMBLK"); break;
        case eOpcodes::MMBUK: WriteTodo(code_gen, md.EntityName(), "MMBUK"); break;
        case eOpcodes::CHGLD: WriteTodo(code_gen, md.EntityName(), "CHGLD"); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode
            ));
    }
}

void FF7::FF7PartyInstruction::processSTITM(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& item_id = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& amount = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->addOutputLine((boost::format("FFVII.add_item(%1%, %2%)") % item_id % amount).str());
}

void FF7::FF7PartyInstruction::processPRTYE(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* gc = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    auto char_id_1 = gc->mFormatter.CharName(_params[0]->getUnsigned());
    char_id_1 = (char_id_1 == "") ? "nil" : ("\"" + char_id_1 + "\"");
    auto char_id_2 = gc->mFormatter.CharName(_params[1]->getUnsigned());
    char_id_2 = (char_id_2 == "") ? "nil" : ("\"" + char_id_2 + "\"");
    auto char_id_3 = gc->mFormatter.CharName(_params[2]->getUnsigned());
    char_id_3 = (char_id_3 == "") ? "nil" : ("\"" + char_id_3 + "\"");
    code_gen->addOutputLine((
      boost::format("FFVII.set_party(%1%, %2%, %3%)") % char_id_1 % char_id_2 % char_id_3
    ).str());
}

void FF7::FF7ModelInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FF7::FF7FieldEngine& eng = static_cast<FF7::FF7FieldEngine&>(*engine);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::JOIN: processJOIN(code_gen); break;
        case eOpcodes::SPLIT: processSPLIT(code_gen); break;
        case eOpcodes::BLINK: WriteTodo(code_gen, md.EntityName(), "BLINK"); break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::EYETX:
            WriteTodo(code_gen, md.EntityName(), "EYETX");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::TRNSP:
            WriteTodo(code_gen, md.EntityName(), "TRNSP");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::AMBNT:
            WriteTodo(code_gen, md.EntityName(), "AMBNT");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::Unknown03:
            WriteTodo(code_gen, md.EntityName(), "Unknown03");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::Unknown04:
            WriteTodo(code_gen, md.EntityName(), "Unknown04");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::Unknown05:
            WriteTodo(code_gen, md.EntityName(), "Unknown05");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::LIGHT:
            WriteTodo(code_gen, md.EntityName(), "LIGHT");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::Unknown07:
            WriteTodo(code_gen, md.EntityName(), "Unknown07");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::Unknown08:
            WriteTodo(code_gen, md.EntityName(), "Unknown08");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::Unknown09:
            WriteTodo(code_gen, md.EntityName(), "Unknown09");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::SBOBJ:
            WriteTodo(code_gen, md.EntityName(), "SBOBJ");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::Unknown0B:
            WriteTodo(code_gen, md.EntityName(), "Unknown0B");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::Unknown0C:
            WriteTodo(code_gen, md.EntityName(), "Unknown0C");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::SHINE:
            WriteTodo(code_gen, md.EntityName(), "SHINE");
            break;
        case (eOpcodes::KAWAI << 8) | eKawaiOpcodes::RESET:
            WriteTodo(code_gen, md.EntityName(), "RESET");
            break;
        case eOpcodes::KAWIW: WriteTodo(code_gen, md.EntityName(), "KAWIW"); break;
        case eOpcodes::PMOVA: WriteTodo(code_gen, md.EntityName(), "PMOVA"); break;
        case eOpcodes::PDIRA: WriteTodo(code_gen, md.EntityName(), "PDIRA"); break;
        case eOpcodes::PTURA: WriteTodo(code_gen, md.EntityName(), "PTURA"); break;
        case eOpcodes::PGTDR: WriteTodo(code_gen, md.EntityName(), "PGTDR"); break;
        case eOpcodes::PXYZI: WriteTodo(code_gen, md.EntityName(), "PXYZI"); break;
        case eOpcodes::TLKON: processTLKON(code_gen, md.EntityName()); break;
        case eOpcodes::PC: processPC(code_gen, md.EntityName()); break;
        case eOpcodes::opCodeCHAR: processCHAR(code_gen, md.EntityName()); break;
        case eOpcodes::DFANM: processDFANM(code_gen, md.EntityName(), md.CharacterId()); break;
        case eOpcodes::ANIME1: processANIME1(code_gen, md.EntityName(), md.CharacterId()); break;
        case eOpcodes::VISI: processVISI(code_gen, md.EntityName()); break;
        case eOpcodes::XYZI: processXYZI(code_gen, md.EntityName()); break;
        case eOpcodes::XYI: WriteTodo(code_gen, md.EntityName(), "XYI"); break;
        case eOpcodes::XYZ: WriteTodo(code_gen, md.EntityName(), "XYZ"); break;
        case eOpcodes::MOVE: processMOVE(code_gen, md.EntityName()); break;
        case eOpcodes::CMOVE: WriteTodo(code_gen, md.EntityName(), "CMOVE"); break;
        case eOpcodes::MOVA: WriteTodo(code_gen, md.EntityName(), "MOVA"); break;
        case eOpcodes::TURA: WriteTodo(code_gen, md.EntityName(), "TURA"); break;
        case eOpcodes::ANIMW: WriteTodo(code_gen, md.EntityName(), "ANIMW"); break;
        case eOpcodes::FMOVE: WriteTodo(code_gen, md.EntityName(), "FMOVE"); break;
        case eOpcodes::ANIME2: WriteTodo(code_gen, md.EntityName(), "ANIME2"); break;
        case eOpcodes::ANIM_1: WriteTodo(code_gen, md.EntityName(), "ANIM_1"); break;
        case eOpcodes::CANIM1: WriteTodo(code_gen, md.EntityName(), "CANIM1"); break;
        case eOpcodes::CANM_1: WriteTodo(code_gen, md.EntityName(), "CANM_1"); break;
        case eOpcodes::MSPED: processMSPED(code_gen, md.EntityName()); break;
        case eOpcodes::DIR: processDIR(code_gen, md.EntityName()); break;
        case eOpcodes::TURNGEN: processTURNGEN(code_gen, md.EntityName()); break;
        case eOpcodes::TURN: WriteTodo(code_gen, md.EntityName(), "TURN"); break;
        case eOpcodes::DIRA: WriteTodo(code_gen, md.EntityName(), "DIRA"); break;
        case eOpcodes::GETDIR: WriteTodo(code_gen, md.EntityName(), "GETDIR"); break;
        case eOpcodes::GETAXY: WriteTodo(code_gen, md.EntityName(), "GETAXY"); break;
        case eOpcodes::GETAI: processGETAI(code_gen, eng); break;
        case eOpcodes::ANIM_2: processANIM_2(code_gen, md.EntityName(), md.CharacterId()); break;
        case eOpcodes::CANIM2: processCANIM2(code_gen, md.EntityName(), md.CharacterId()); break;
        case eOpcodes::CANM_2: processCANM_2(code_gen, md.EntityName(), md.CharacterId()); break;
        case eOpcodes::ASPED: WriteTodo(code_gen, md.EntityName(), "ASPED"); break;
        case eOpcodes::CC: processCC(code_gen, eng); break;
        case eOpcodes::JUMP: WriteTodo(code_gen, md.EntityName(), "JUMP"); break;
        case eOpcodes::AXYZI: WriteTodo(code_gen, md.EntityName(), "AXYZI"); break;
        case eOpcodes::LADER: WriteTodo(code_gen, md.EntityName(), "LADER"); break;
        case eOpcodes::OFST: processOFST(code_gen, md.EntityName()); break;
        case eOpcodes::OFSTW:
            code_gen->addOutputLine("self." + md.EntityName() + ":offset_sync()");
            break;
        case eOpcodes::TALKR: WriteTodo(code_gen, md.EntityName(), "TALKR"); break;
        case eOpcodes::SLIDR: WriteTodo(code_gen, md.EntityName(), "SLIDR"); break;
        case eOpcodes::SOLID: processSOLID(code_gen, md.EntityName()); break;
        case eOpcodes::TLKR2: WriteTodo(code_gen, md.EntityName(), "TLKR2"); break;
        case eOpcodes::SLDR2: WriteTodo(code_gen, md.EntityName(), "SLDR2"); break;
        case eOpcodes::CCANM: WriteTodo(code_gen, md.EntityName(), "CCANM"); break;
        case eOpcodes::FCFIX: WriteTodo(code_gen, md.EntityName(), "FCFIX"); break;
        case eOpcodes::ANIMB: WriteTodo(code_gen, md.EntityName(), "ANIMB"); break;
        case eOpcodes::TURNW: WriteTodo(code_gen, md.EntityName(), "TURNW"); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode
            ));
    }
}

void FF7::FF7ModelInstruction::processJOIN(CodeGenerator* code_gen){
    code_gen->addOutputLine("join_party(" + std::to_string(_params[0]->getUnsigned()) + ")");
}

void FF7::FF7ModelInstruction::processSPLIT(CodeGenerator* code_gen){
    code_gen->addOutputLine(
      "split_party("
      + std::to_string(_params[0]->getUnsigned()) + ", " // ax_addr
      + std::to_string(_params[1]->getUnsigned()) + ", " // ay_addr
      + std::to_string(_params[2]->getUnsigned()) + ", " // ar_addr
      + std::to_string(_params[3]->getUnsigned()) + ", " // bx_addr
      + std::to_string(_params[4]->getUnsigned()) + ", " // by_addr
      + std::to_string(_params[5]->getUnsigned()) + ", " // br_addr
      + std::to_string(_params[6]->getSigned()) + ", " // ax
      + std::to_string(_params[7]->getSigned()) + ", " // ay
      + std::to_string(_params[8]->getSigned()) + ", " // ar
      + std::to_string(_params[9]->getSigned()) + ", " // bx
      + std::to_string(_params[10]->getSigned()) + ", " // by
      + std::to_string(_params[11]->getSigned()) + ", " // br
      + std::to_string(_params[12]->getUnsigned()) + ", " // speed
      + ")");
}

void FF7::FF7ModelInstruction::processTLKON(CodeGenerator* code_gen, const std::string& entity){
    code_gen->addOutputLine((
      boost::format("self.%1%:set_talkable(%2%)")
      % entity % FF7CodeGeneratorHelpers::FormatInvertedBool(_params[0]->getUnsigned())
    ).str());
}

void FF7::FF7ModelInstruction::processPC(CodeGenerator* code_gen, const std::string& entity){
    code_gen->addOutputLine(
      (boost::format("set_entity_to_character(\"%1%\", \"%1%\")") % entity).str()
    );
}

void FF7::FF7ModelInstruction::processCHAR(CodeGenerator* code_gen, const std::string& entity){
    code_gen->addOutputLine(
      (boost::format("self.%1% = entity_manager:get_entity(\"%1%\")") % entity).str()
    );
}

void FF7::FF7ModelInstruction::processDFANM(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / _params[1]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("self.%1%:set_default_animation(\"%2%\") -- speed %3%")
      % entity % cg->mFormatter.AnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->addOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\")")
      % entity % cg->mFormatter.AnimationName(char_id, animation_id)
    ).str());
}

void FF7::FF7ModelInstruction::processANIME1(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / _params[1]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\") -- speed %3%")
      % entity % cg->mFormatter.AnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->addOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FF7::FF7ModelInstruction::processVISI(CodeGenerator* code_gen, const std::string& entity){
    code_gen->addOutputLine((
      boost::format("self.%1%:set_visible(%2%)")
      % entity % FF7CodeGeneratorHelpers::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FF7::FF7ModelInstruction::processXYZI(CodeGenerator* code_gen, const std::string& entity){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->ScaleFactor();
    const auto& x = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[4]->getSigned(),
      FF7CodeGeneratorHelpers::ValueType::Float, scale
    );
    const auto& y = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[5]->getSigned(),
      FF7CodeGeneratorHelpers::ValueType::Float, scale
    );
    const auto& z = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[2]->getUnsigned(), _params[6]->getSigned(),
      FF7CodeGeneratorHelpers::ValueType::Float, scale
    );
    const auto& triangle_id = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[3]->getUnsigned(), _params[7]->getUnsigned()
    );
    code_gen->addOutputLine((
      boost::format("self.%1%:set_position(%2%, %3%, %4%) -- triangle ID %5%")
      % entity % x % y % z % triangle_id
    ).str());
}

void FF7::FF7ModelInstruction::processMOVE(CodeGenerator* code_gen, const std::string& entity){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->ScaleFactor();
    const auto& x = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getSigned(),
      FF7CodeGeneratorHelpers::ValueType::Float, scale
    );
    const auto& y = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getSigned(),
      FF7CodeGeneratorHelpers::ValueType::Float, scale
    );
    code_gen->addOutputLine(
      (boost::format("self.%1%:move_to_position(%2%, %3%)") % entity % x % y).str()
    );
    code_gen->addOutputLine((boost::format("self.%1%:move_sync()") % entity).str());
}

void FF7::FF7ModelInstruction::processMSPED(CodeGenerator* code_gen, const std::string& entity){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->ScaleFactor();
    const auto& speed = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[2]->getUnsigned(),
      FF7CodeGeneratorHelpers::ValueType::Float, 256.0f * scale / 30.0f
    );
    code_gen->addOutputLine(
      (boost::format("self.%1%:set_move_auto_speed(%2%)") % entity % speed).str()
    );
}

void FF7::FF7ModelInstruction::processDIR(CodeGenerator* code_gen, const std::string& entity){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& degrees = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[1]->getUnsigned(),
      FF7CodeGeneratorHelpers::ValueType::Float, 256.0f / 360.0f
    );
    code_gen->addOutputLine((boost::format("self.%1%:set_rotation(%2%)") % entity % degrees).str());
}

void FF7::FF7ModelInstruction::processTURNGEN(CodeGenerator* code_gen, const std::string& entity){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& degrees = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[2]->getUnsigned(),
      FF7CodeGeneratorHelpers::ValueType::Float, 256.0f / 360.0f
    );
    std::string direction;
    switch (_params[3]->getUnsigned()){
        case 0: direction = "Entity.CLOCKWISE"; break;
        case 1: direction = "Entity.ANTICLOCKWISE"; break;
        case 2: direction = "Entity.CLOSEST"; break;
        // Default to closest:
        default: direction = "Entity.CLOSEST"; break;
    }
    auto steps = _params[4]->getUnsigned();
    std::string step_type;
    switch (_params[5]->getUnsigned()){
        case 1: step_type = "Entity.LINEAR"; break;
        case 2: step_type = "Entity.SMOOTH"; break;
        // Default to smooth
        default: step_type = "Entity.SMOOTH"; break;
    }
    const float scaled_steps = static_cast<float>(steps) / 30.0f;
    code_gen->addOutputLine((
      boost::format("self.%1%:turn_to_direction(%2%, %3%, %4%, %5%)")
      % entity % degrees % direction % step_type % scaled_steps
    ).str());
    code_gen->addOutputLine((boost::format("self.%1%:turn_sync()") % entity).str());
}

void FF7::FF7ModelInstruction::processGETAI(CodeGenerator* code_gen, const FF7FieldEngine& engine){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // TODO: check for assignment to literal.
    const auto& variable = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    const auto& entity = engine.EntityByIndex(_params[2]->getUnsigned());
    code_gen->addOutputLine(
      (boost::format("%1% = entity_manager:get_entity(\"%2%\"):get_move_triangle_id()")
      % variable % entity.Name()
    ).str());
}

void FF7::FF7ModelInstruction::processANIM_2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / _params[1]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("self.%1%:play_animation_stop(\"%2%\") -- speed %3%")
      % entity % cg->mFormatter.AnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->addOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FF7::FF7ModelInstruction::processCANIM2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    auto start_frame = _params[1]->getUnsigned() / 30.0f;
    auto end_frame = _params[2]->getUnsigned() / 30.0f;
    // TODO: check for zero.
    auto speed = 1.0f / _params[3]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\", %3%, %4%) -- speed %5%")
      % entity % cg->mFormatter.AnimationName(char_id, animation_id)
      % start_frame % end_frame % speed
    ).str());
    code_gen->addOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FF7::FF7ModelInstruction::processCANM_2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    // ID will be fixed-up downstream.
    auto animation_id = _params[0]->getUnsigned();
    auto start_frame = _params[1]->getUnsigned() / 30.0f;
    auto end_frame = _params[2]->getUnsigned() / 30.0f;
    // TODO: check for zero.
    auto speed = 1.0f / _params[3]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("self.%1%:play_animation_stop(\"%2%\", %3%, %4%) -- speed %5%")
      % entity % cg->mFormatter.AnimationName(char_id, animation_id)
      % start_frame % end_frame % speed
    ).str());
    code_gen->addOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FF7::FF7ModelInstruction::processCC(CodeGenerator* code_gen, const FF7FieldEngine& engine){
    const auto& entity = engine.EntityByIndex(_params[0]->getUnsigned());
    code_gen->addOutputLine(
      (boost::format("entity_manager:set_player_entity(\"%1%\")") % entity.Name()).str()
    );
}

void FF7::FF7ModelInstruction::processSOLID(CodeGenerator* code_gen, const std::string& entity){
    code_gen->addOutputLine((
      boost::format("self.%1%:set_solid(%2%)")
      % entity % FF7CodeGeneratorHelpers::FormatInvertedBool(_params[0]->getUnsigned())
    ).str());
}

void FF7::FF7ModelInstruction::processOFST(CodeGenerator* code_gen, const std::string& entity){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    float x = atoi(FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[5]->getSigned()
    ).c_str());
    float y = atoi(FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(),_params[6]->getSigned()
    ).c_str());
    float z = atoi(FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[2]->getUnsigned(), _params[7]->getSigned()
    ).c_str());
    float speed = atoi(FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[3]->getUnsigned(), _params[8]->getUnsigned()
    ).c_str());
    // Spatial coordinates need to be scaled down.
    // TODO: This number is empirically deducted. Why this number?
    x *= 0.00390f;
    y *= 0.00390f;
    z *= 0.00390f;
    // Speed needs to be scaled down by the frame rate.
    speed /= 30.0f;
    code_gen->addOutputLine((
      boost::format("self.%1%:offset_to_position(%2%, %3%, %4%, %5%, %6%)")
      % entity % x % y % z
      % (_params[4]->getUnsigned() ? "Entity.SMOOTH" : "Entity.LINEAR")% speed
    ).str());
}

void FF7::FF7WalkmeshInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::SLIP: WriteTodo(code_gen, md.EntityName(), "SLIP"); break;
        case eOpcodes::UC: processUC(code_gen); break;
        case eOpcodes::IDLCK:
            // Triangle id, on or off
            code_gen->addOutputLine(
                (boost::format("walkmesh:lock_walkmesh(%1%, %2%)")
                % _params[0]->getUnsigned()
                % FF7CodeGeneratorHelpers::FormatBool(_params[1]->getUnsigned())).str());
            break;
        case eOpcodes::LINE: processLINE(code_gen, md.EntityName()); break;
        case eOpcodes::LINON: WriteTodo(code_gen, md.EntityName(), "LINON"); break;
        case eOpcodes::SLINE: WriteTodo(code_gen, md.EntityName(), "SLINE"); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode
            ));
    }
}

void FF7::FF7WalkmeshInstruction::processUC(CodeGenerator* code_gen){
    code_gen->addOutputLine((
      boost::format("entity_manager:player_lock(%1%)")
      % FF7CodeGeneratorHelpers::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FF7::FF7WalkmeshInstruction::processLINE(CodeGenerator* code_gen, const std::string& entity){
    float xa = _params[0]->getSigned();
    float ya = _params[1]->getSigned();
    float za = _params[2]->getSigned();
    float xb = _params[3]->getSigned();
    float yb = _params[4]->getSigned();
    float zb = _params[5]->getSigned();
    // Scale down. TODO: Why this number?
    xa *= 0.00781249709639;
    ya *= 0.00781249709639;
    za *= 0.00781249709639;
    xb *= 0.00781249709639;
    yb *= 0.00781249709639;
    zb *= 0.00781249709639;
    // Just print as a comment.
    code_gen->addOutputLine(
      "-- LINE (" + std::to_string(xa) + ", " + std::to_string(ya) + ", " + std::to_string(za)
      + ")-(" + std::to_string(xb) + ", " +std::to_string(yb) + ", " + std::to_string(zb) + ")"
    );
}

void FF7::FF7BackgroundInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::BGPDH: WriteTodo(code_gen, md.EntityName(), "BGPDH");break;
        case eOpcodes::BGSCR: WriteTodo(code_gen, md.EntityName(), "BGSCR"); break;
        case eOpcodes::MPPAL: WriteTodo(code_gen, md.EntityName(), "MPPAL"); break;
        case eOpcodes::BGON: processBGON(code_gen); break;
        case eOpcodes::BGOFF: processBGOFF(code_gen); break;
        case eOpcodes::BGROL: WriteTodo(code_gen, md.EntityName(), "BGROL"); break;
        case eOpcodes::BGROL2: WriteTodo(code_gen, md.EntityName(), "BGROL2"); break;
        case eOpcodes::BGCLR: processBGCLR(code_gen); break;
        case eOpcodes::STPAL: processSTPAL(code_gen); break;
        case eOpcodes::LDPAL: processLDPAL(code_gen); break;
        case eOpcodes::CPPAL: processCPPAL(code_gen); break;
        case eOpcodes::RTPAL: WriteTodo(code_gen, md.EntityName(), "RTPAL"); break;
        case eOpcodes::ADPAL: processADPAL(code_gen); break;
        case eOpcodes::MPPAL2: processMPPAL2(code_gen); break;
        case eOpcodes::STPLS: processSTPLS(code_gen); break;
        case eOpcodes::LDPLS: processLDPLS(code_gen); break;
        case eOpcodes::CPPAL2: WriteTodo(code_gen, md.EntityName(), "CPPAL2"); break;
        case eOpcodes::ADPAL2: WriteTodo(code_gen, md.EntityName(), "ADPAL2"); break;
        case eOpcodes::RTPAL2: WriteTodo(code_gen, md.EntityName(), "RTPAL2"); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode
            ));
    }
}

void FF7::FF7BackgroundInstruction::processBGON(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& background_id = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& layer_id = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->addOutputLine(
      (boost::format("-- field:background_on(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FF7::FF7BackgroundInstruction::processBGOFF(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& background_id = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& layer_id = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->addOutputLine(
      (boost::format("-- field:background_off(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FF7::FF7BackgroundInstruction::processBGCLR(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& background_id = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[2]->getUnsigned()
    );
    code_gen->addOutputLine(
      (boost::format("-- field:background_clear(%1%)") % background_id).str()
    );
}

void FF7::FF7BackgroundInstruction::processSTPAL(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& source = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->addOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FF7::FF7BackgroundInstruction::processLDPAL(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& source = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->addOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FF7::FF7BackgroundInstruction::processCPPAL(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& source = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->addOutputLine((
      boost::format("-- copy palette %1% to palette %2%, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FF7::FF7BackgroundInstruction::processADPAL(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& source = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& r = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[4]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& g = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[3]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& b = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[2]->getUnsigned(), _params[8]->getUnsigned()
    );
    auto num_entries = _params[11]->getUnsigned() + 1;
    code_gen->addOutputLine((
      boost::format(
        "-- add RGB(%3%, %4%, %5%) to %6% entries of palette stored at position %1%, "
        "storing result in position %2%"
      ) % source % destination % r % g % b % num_entries
    ).str());
}

void FF7::FF7BackgroundInstruction::processMPPAL2(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& source = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& r = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[4]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& g = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[3]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& b = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[2]->getUnsigned(), _params[8]->getUnsigned()
    );
    auto num_entries = _params[11]->getUnsigned() + 1;
    code_gen->addOutputLine((
      boost::format(
        "-- multiply RGB(%3%, %4%, %5%) by %6% entries of palette stored at position %1%, "
        "storing result in position %2%"
      ) % source % destination % r % g % b % num_entries).str());
}

void FF7::FF7BackgroundInstruction::processSTPLS(CodeGenerator* code_gen){
    auto source = _params[0]->getUnsigned();
    auto destination = _params[1]->getUnsigned();
    auto start_clut = _params[2]->getUnsigned();
    auto num_entries = _params[3]->getUnsigned() + 1;
    code_gen->addOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index %3%, %4% entries")
      % source % destination % start_clut % num_entries
    ).str());
}

void FF7::FF7BackgroundInstruction::processLDPLS(CodeGenerator* code_gen){
    auto source = _params[0]->getUnsigned();
    auto destination = _params[1]->getUnsigned();
    auto startClut = _params[2]->getUnsigned();
    auto num_entries = _params[3]->getUnsigned() + 1;
    code_gen->addOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index %3%, %4% entries")
      % source % destination % startClut % num_entries
    ).str());
}

void FF7::FF7CameraInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
    case eOpcodes::NFADE: processNFADE(code_gen); break;
    case eOpcodes::SHAKE: WriteTodo(code_gen, md.EntityName(), "SHAKE"); break;
    case eOpcodes::SCRLO: WriteTodo(code_gen, md.EntityName(), "SCRLO"); break;
    case eOpcodes::SCRLC: WriteTodo(code_gen, md.EntityName(), "SCRLC"); break;
    case eOpcodes::SCR2D: processSCR2D(code_gen); break;
    case eOpcodes::SCRCC: WriteTodo(code_gen, md.EntityName(), "SCRCC"); break;
    case eOpcodes::SCR2DC: processSCR2DC(code_gen); break;
    case eOpcodes::SCRLW: WriteTodo(code_gen, md.EntityName(), "SCRLW"); break;
    case eOpcodes::SCR2DL: WriteTodo(code_gen, md.EntityName(), "SCR2DL"); break;
    case eOpcodes::VWOFT: WriteTodo(code_gen, md.EntityName(), "VWOFT"); break;
    case eOpcodes::FADE: processFADE(code_gen); break;
    case eOpcodes::FADEW: WriteTodo(code_gen, md.EntityName(), "FADEW"); break;
    case eOpcodes::SCRLP: WriteTodo(code_gen, md.EntityName(), "SCRLP"); break;
    case eOpcodes::MVCAM: WriteTodo(code_gen, md.EntityName(), "MVCAM"); break;
    case eOpcodes::SCRLA: WriteTodo(code_gen, md.EntityName(), "SCRLA"); break;
    default:
        code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
          md.EntityName(), _address, _opcode
        ));
    }
}

void FF7::FF7CameraInstruction::processNFADE(CodeGenerator* code_gen){
    // TODO: Not fully reversed.
    auto raw_type = _params[4]->getUnsigned();
    if (raw_type == 0){
        code_gen->addOutputLine("-- fade:clear()");
        return;
    }
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const std::string type = raw_type == 12 ? "Fade.SUBTRACT" : "Fade.ADD";
    const auto& r = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[5]->getUnsigned()
    );
    const auto& g = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& b = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[2]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& unknown = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[3]->getUnsigned(), _params[8]->getUnsigned()
    );
    code_gen->addOutputLine(
      (boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%)") % type % r % g % b % unknown).str()
    );
}

void FF7::FF7CameraInstruction::processSCR2D(CodeGenerator* code_gen){
    // kUpScaler.
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& x = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getSigned()
    );
    const auto& y = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getSigned()
    );
    code_gen->addOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.NONE, 0)")
      % x % y
    ).str());
}

void FF7::FF7CameraInstruction::processSCR2DC(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& x = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[4]->getSigned()
    );
    const auto& y = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[5]->getSigned()
    );
    const auto& speed = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[3]->getUnsigned(), _params[6]->getUnsigned(),
      FF7CodeGeneratorHelpers::ValueType::Float, 30.0f
    );
    code_gen->addOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.SMOOTH, %3%)")
      % x % y % speed
    ).str());
}

void FF7::FF7CameraInstruction::processFADE(CodeGenerator* code_gen){
    // TODO: not fully reversed
    auto raw_type = _params[8]->getUnsigned();
    std::string type;
    switch (raw_type) {
        case 1:
        case 2:
        case 7:
        case 8:
            type = "Fade.SUBTRACT";
            break;
        case 4: code_gen->addOutputLine("-- fade:black()"); return;
        default: type = "Fade.ADD"; break;
    }

    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& r = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[4]->getUnsigned()
    );
    const auto& g = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[5]->getUnsigned()
    );
    const auto& b = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[2]->getUnsigned(), _params[6]->getUnsigned()
    );
    // TODO: needs to be divided by 30.0f?
    auto speed = _params[7]->getUnsigned();
    auto start = _params[9]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%, %6%)") % type % r % g % b % speed % start
    ).str());
}

void FF7::FF7AudioVideoInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::BGMOVIE: WriteTodo(code_gen, md.EntityName(), "BGMOVIE"); break;
        case eOpcodes::AKAO2: processAKAO2(code_gen); break;
        case eOpcodes::MUSIC: processMUSIC(code_gen); break;
        case eOpcodes::SOUND: processSOUND(code_gen); break;
        case eOpcodes::AKAO: processAKAO(code_gen); break;
        case eOpcodes::MUSVT: WriteTodo(code_gen, md.EntityName(), "MUSVT"); break;
        case eOpcodes::MUSVM: WriteTodo(code_gen, md.EntityName(), "MUSVM"); break;
        case eOpcodes::MULCK: processMULCK(code_gen); break;
        case eOpcodes::BMUSC: WriteTodo(code_gen, md.EntityName(), "BMUSC"); break;
        case eOpcodes::CHMPH: WriteTodo(code_gen, md.EntityName(), "CHMPH"); break;
        case eOpcodes::PMVIE: processPMVIE(code_gen); break;
        case eOpcodes::MOVIE: processMOVIE(code_gen); break;
        case eOpcodes::MVIEF: processMVIEF(code_gen); break;
        case eOpcodes::FMUSC: WriteTodo(code_gen, md.EntityName(), "FMUSC"); break;
        case eOpcodes::CMUSC: WriteTodo(code_gen, md.EntityName(), "CMUSC"); break;
        case eOpcodes::CHMST: WriteTodo(code_gen, md.EntityName(), "CHMST"); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode
            ));
    }
}

void FF7::FF7AudioVideoInstruction::processAKAO2(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& param1 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& param2 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[8]->getUnsigned()
    );
    const auto& param3 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[2]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& param4 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[3]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& param5 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[5]->getUnsigned(), _params[11]->getUnsigned()
    );
    auto op = _params[6]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
      % param1 % param2 % param3 % param4 % param5 % op
    ).str());
}

void FF7::FF7AudioVideoInstruction::processMUSIC(CodeGenerator* code_gen){
    code_gen->addOutputLine((
      boost::format("-- music:execute_akao(0x10, pointer_to_field_AKAO_%1%)")
      % _params[0]->getUnsigned()
    ).str());
}

void FF7::FF7AudioVideoInstruction::processSOUND(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& soundId = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& panning = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->addOutputLine(
      (boost::format("-- music:execute_akao(0x20, %1%, %2%)") % soundId % panning).str()
    );
}

void FF7::FF7AudioVideoInstruction::processAKAO(CodeGenerator* code_gen){
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& param1 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[0]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& param2 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[8]->getUnsigned()
    );
    const auto& param3 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[2]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& param4 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[3]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& param5 = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[5]->getUnsigned(), _params[11]->getUnsigned()
    );
    auto op = _params[6]->getUnsigned();
    code_gen->addOutputLine((
      boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
      % param1 % param2 % param3 % param4 % param5 % op
    ).str());
}

void FF7::FF7AudioVideoInstruction::processMULCK(CodeGenerator* code_gen){
    code_gen->addOutputLine((
      boost::format("-- music:lock(%1%)")
      % FF7CodeGeneratorHelpers::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FF7::FF7AudioVideoInstruction::processPMVIE(CodeGenerator* code_gen){
    code_gen->addOutputLine(
      (boost::format("-- field:movie_set(%1%)") % _params[0]->getUnsigned()
    ).str());
}

void FF7::FF7AudioVideoInstruction::processMOVIE(CodeGenerator* code_gen){
    code_gen->addOutputLine("-- field:play_movie()");
}

void FF7::FF7AudioVideoInstruction::processMVIEF(CodeGenerator* code_gen){
    // TODO: Check for assignment to value.
    FF7SimpleCodeGenerator* cg = static_cast<FF7SimpleCodeGenerator*>(code_gen);
    const auto& destination = FF7CodeGeneratorHelpers::FormatValueOrVariable(
      cg->mFormatter, _params[1]->getUnsigned(), _params[2]->getUnsigned());
    code_gen->addOutputLine(
      (boost::format("-- %1% = field:get_movie_frame()") % destination).str()
    );
}

void FF7::FF7UncategorizedInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case eOpcodes::MPDSP: WriteTodo(code_gen, md.EntityName(), "MPDSP"); break;
        case eOpcodes::SETX: WriteTodo(code_gen, md.EntityName(), "SETX"); break;
        case eOpcodes::GETX: WriteTodo(code_gen, md.EntityName(), "GETX"); break;
        case eOpcodes::SEARCHX: WriteTodo(code_gen, md.EntityName(), "SEARCHX"); break;
        default:
            code_gen->addOutputLine(FF7CodeGeneratorHelpers::FormatInstructionNotImplemented(
              md.EntityName(), _address, _opcode)
            );
    }
}

void FF7::FF7NoOperationInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){}
