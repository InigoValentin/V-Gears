#include "decompiler/field/ff7_field_engine.h"
#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include "../../../include/decompiler/field/FieldCodeGenerator.h"
#include "../../../include/decompiler/field/FieldDisassembler.h"

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

std::unique_ptr<Disassembler> FF7FieldEngine::getDisassembler(
  InstVec &insts, const std::vector<unsigned char>& raw_script_data
){
    auto ret = std::make_unique<FieldDisassembler>(mFormatter, this, insts, raw_script_data);
    scale_factor_ = ret->GetScaleFactor();
    return std::move(ret);
}

std::unique_ptr<Disassembler> FF7FieldEngine::getDisassembler(InstVec &insts){
    auto ret = std::make_unique<FieldDisassembler>(mFormatter, this, insts);
    scale_factor_ = ret->GetScaleFactor();
    return std::move(ret);
}

std::unique_ptr<CodeGenerator> FF7FieldEngine::getCodeGenerator(
  const InstVec& insts, std::ostream &output
){
    // The broken version:
    //return std::make_unique<FF7CodeGenerator>(this, insts, output);
    // The not-as-nice-but-at-least-it-works version:
    return std::make_unique<FieldCodeGenerator>(this, insts, output, mFormatter);
}

void FF7FieldEngine::postCFG(InstVec& insts, Graph g){
    /*
    // In FF7 some scripts ends with an infinite loop to "keep it alive"
    // in VGears this isn't required so we can remove them
    RemoveTrailingInfiniteLoops(insts, g);

    // This could generate bad code, but it always seems to follow that pattern that if the last
    // instruction is an uncond jump back into the script then it simply nests all of those blocks
    // in an infinite loop
    MarkInfiniteLoopGroups(insts, g);

    // Scripts end with a "return" this isn't required so strip them out
    RemoveExtraneousReturnStatements(insts, g);
    */
}

std::map<std::string, int> FF7FieldEngine::GetEntities() const{
    std::map<std::string, int> r;
    for (auto& f : _functions){
        const Function& func = f.second;
        FunctionMetaData meta(func._metadata);
        auto it = r.find(meta.GetEntityName());
        if (it != std::end(r)){
            // Try to find a function in this entity has that has a char id
            // don't overwrite a valid char id with a "blank" one
            if (it->second == -1) it->second = meta.GetCharacterId();
        }
        // TODO: Don't add lines here:
        else r[meta.GetEntityName()] = meta.GetCharacterId();
    }
    return r;
}

std::vector<SUDM::Field::FieldEntity> FF7FieldEngine::GetEntityList() const{
    std::vector<SUDM::Field::FieldEntity> entities;
    for (auto entity: mEntityIndexMap){
        if (entity.second.IsLine() == false){
            SUDM::Field::FieldEntity ent;
            ent.name = entity.second.Name();
            ent.index = entity.second.GetIndex();

            // Get character ID.
            ent.char_id = -1;
            std::map<std::string, int> r;
            //if (ent.name == "Cloud") std::cout << "CHAR_ID FOR CLOUD" << std::endl;
            for (auto& f : _functions){

                const Function& func = f.second;
                FunctionMetaData meta(func._metadata);
                if (meta.GetEntityName() == ent.name){
                    ent.char_id = meta.GetCharacterId();
                    break;
                }
            }
            entities.push_back(ent);
        }
    }
    return entities;
}

std::vector<SUDM::Field::Line> FF7FieldEngine::GetLineList() const{
    std::vector<SUDM::Field::Line> lines;
    for (auto entity: mEntityIndexMap){
        if (entity.second.IsLine() == true){
            SUDM::Field::Line line;
            line.name = entity.second.Name();
            line.point_a = entity.second.GetLinePointA();
            line.point_b = entity.second.GetLinePointB();
            lines.push_back(line);
        }
    }
    return lines;
}

void FF7FieldEngine::AddEntityFunction(
  const std::string& entity_name, size_t entity_index,
  const std::string& func_name, size_t func_index
){
    auto it = mEntityIndexMap.find(entity_index);
    if (it != std::end(mEntityIndexMap)) (*it).second.AddFunction(func_name, func_index);
    else{
        Entity e(entity_name, entity_index);
        e.AddFunction(func_name, func_index);
        mEntityIndexMap.insert(std::make_pair(entity_index, e));
    }
}

void FF7FieldEngine::MarkEntityAsLine(
  size_t entity_index, bool line, std::vector<float> point_a, std::vector<float> point_b
){
    auto it = mEntityIndexMap.find(entity_index);
    if (it != std::end(mEntityIndexMap)){
        (*it).second.MarkAsLine(line, point_a, point_b);
    }
}

bool FF7FieldEngine::EntityIsLine(size_t entity_index){
    auto it = mEntityIndexMap.find(entity_index);
    if (it != std::end(mEntityIndexMap)) return (*it).second.IsLine();
    return false;
}

void FF7FieldEngine::RemoveExtraneousReturnStatements(InstVec& insts, Graph g){
    for (auto& f : _functions){
        Function& func = f.second;
        for (auto it = insts.begin(); it != insts.end(); it ++){
            // Is it the last instruction in the function, and is it a return statement?
            if ((*it)->_address == func.mEndAddr){
                if ((*it)->_opcode == OPCODE::RET){
                    // Set new end address to be before the NOP.
                    func.mEndAddr = (*(it - 1))->_address;
                    func.mNumInstructions --;
                    Instruction* nop = new FF7NoOperationInstruction();
                    nop->_opcode = OPCODE::NOP;
                    nop->_address = (*it)->_address;
                    (*it).reset(nop);
                    break;
                }
            }
        }
    }
}

void FF7FieldEngine::RemoveTrailingInfiniteLoops(InstVec& insts, Graph g){
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
                    nop->_opcode = OPCODE::NOP;
                    nop->_address = (*it)->_address;
                    (*it).reset(nop);
                    break;
                }
            }
        }
    }
}

void FF7FieldEngine::MarkInfiniteLoopGroups(InstVec& insts, Graph g){
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

void FF7CondJumpInstruction::processInst(
  Function& function, ValueStack &stack, Engine* engine, CodeGenerator* code_gen
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);

    std::string funcName;
    if (_opcode == OPCODE::IFKEYON) funcName = "entity_manager:is_key_on";
    else if (_opcode == OPCODE::IFKEYOFF) funcName = "entity_manager:is_key_off";
    else if (_opcode == OPCODE::IFKEY) funcName = "Key";
    else if (_opcode == OPCODE::IFMEMBQ) funcName = "IFMEMBQ";
    else if (_opcode == OPCODE::IFPRTYQ) funcName = "IFPRTYQ";

    // If condition is a function, add and stop.
    if (!funcName.empty()){

        uint32 param = _params[0]->getUnsigned();

        // Special cases. The first parameter of IFKEY, IFKEYON and IFKEYOFF
        // can be ORed to get the individual keys, but there are two invalid
        // ones: 512 and 1024. They must be XORed.
        if (
          _opcode == OPCODE::IFKEY
          || _opcode == OPCODE::IFKEYON
          || _opcode == OPCODE::IFKEYOFF
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
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
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

uint32 FF7CondJumpInstruction::getDestAddress() const{
    uint32 params_size = 0;
    uint32 jump_param_index = 5;
    switch (_opcode){
        case OPCODE::IFUB: params_size = 5; break;
        case OPCODE::IFUBL: params_size = 5; break;
        case OPCODE::IFSW: params_size = 7; break;
        case OPCODE::IFSWL: params_size = 7; break;
        case OPCODE::IFUW: params_size = 7; break;
        case OPCODE::IFUWL: params_size = 7; break;
        case OPCODE::IFKEYON:
        case OPCODE::IFKEYOFF:
        case OPCODE::IFKEY:
            params_size = 3;
            jump_param_index = 1;
            break;
        case OPCODE::IFPRTYQ:
        case OPCODE::IFMEMBQ:
            params_size = 2;
            jump_param_index = 1;
            break;
        default: throw UnknownJumpTypeException(_address, _opcode);
    }
    return _address + _params[jump_param_index]->getUnsigned() + params_size;
}

std::ostream& FF7CondJumpInstruction::print(std::ostream &output) const{
    Instruction::print(output);
    output << " (False target address: 0x" << std::hex << getDestAddress() << std::dec << ")";
    return output;
}

bool FF7UncondJumpInstruction::isFuncCall() const{return _isCall;}

bool FF7UncondJumpInstruction::isUncondJump() const{return !_isCall;}

uint32 FF7UncondJumpInstruction::getDestAddress() const{
    if (
      static_cast<OPCODE>(_opcode) == OPCODE::JMPF
      || static_cast<OPCODE>(_opcode) == OPCODE::JMPFL){
        // Short or long forward jump.
        return _address + _params[0]->getUnsigned() + 1;
    }
    // Backwards jump,  OPCODE::JMPB/L.
    return _address - _params[0]->getUnsigned();
}

std::ostream& FF7UncondJumpInstruction::print(std::ostream &output) const{
    Instruction::print(output);
    output << " (Jump target address: 0x" << std::hex << getDestAddress() << std::dec << ")";
    return output;
}


void FF7UncondJumpInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator* code_gen
){
    switch (_opcode){
        case OPCODE::JMPB:
        case OPCODE::JMPBL:
            // HACK: Hard loop will hang the game, insert a wait to yield control
            code_gen->AddOutputLine("-- Hack, yield control for possible inf loop");
            code_gen->AddOutputLine("script:wait(0)");
            break;
    }
}

static void WriteTodo(CodeGenerator *code_gen, std::string entity_name, std::string opCode){
    code_gen->AddOutputLine("-- Todo(\"" + opCode + "\")");
}

void FF7ControlFlowInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FF7FieldEngine& eng = static_cast<FF7FieldEngine&>(*engine);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::RET:
            // A few notes in RET.
            // - Lua requires all functions to end with a return.
            // - Lua doesn's like returns if not followed by an end.
            // - Don't add a return statement to on_start functions.
            //     Game 'init' and 'main' code is concatenated in V-Gears. If the
            //     'main' return is kept, the code of 'main' is never executed.
            //     There are safeguards forfunctions without a return in the end,
            //     so it's OK not to include it here.
            if (func._name != "on_start") code_gen->AddOutputLine("do return 0 end");
            break;
        case OPCODE::REQ: processREQ(code_gen, eng); break;
        case OPCODE::REQSW: processREQSW(code_gen, eng); break;
        case OPCODE::REQEW: processREQEW(code_gen, eng); break;
        case OPCODE::PREQ: WriteTodo(code_gen, md.GetEntityName(), "PREQ"); break;
        case OPCODE::PRQSW: WriteTodo(code_gen, md.GetEntityName(), "PRQSW"); break;
        case OPCODE::PRQEW: WriteTodo(code_gen, md.GetEntityName(), "PRQEW"); break;
        case OPCODE::RETTO: processRETTO(code_gen); break;
        case OPCODE::WAIT: processWAIT(code_gen); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7ControlFlowInstruction::processREQ(
  CodeGenerator* code_gen, const FF7FieldEngine& engine
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& entity = engine.EntityByIndex(_params[0]->getSigned());
    const auto& script_name = cg->GetFormatter().FunctionName(
      entity.Name(), entity.FunctionByIndex(_params[2]->getUnsigned())
    );
    auto priority = _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("script:request(Script.ENTITY, \"%1%\", \"%2%\", %3%)")
      % entity.Name() % script_name % priority
    ).str());
}

void FF7ControlFlowInstruction::processREQSW(
  CodeGenerator* code_gen, const FF7FieldEngine& engine
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& entity = engine.EntityByIndex(_params[0]->getSigned());
    const auto& script_name = cg->GetFormatter().FunctionName(
      entity.Name(), entity.FunctionByIndex(_params[2]->getUnsigned())
    );
    auto priority = _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("script:request_start_sync(Script.ENTITY, \"%1%\", \"%2%\", %3%)")
      % entity.Name() % script_name % priority
    ).str());
}

void FF7ControlFlowInstruction::processREQEW(
  CodeGenerator* code_gen, const FF7FieldEngine& engine
){
    try{
        FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
        const auto& entity = engine.EntityByIndex(_params[0]->getSigned());
        const auto& script_name = cg->GetFormatter().FunctionName(
          entity.Name(), entity.FunctionByIndex(_params[2]->getUnsigned())
        );
        auto priority = _params[1]->getUnsigned();
        code_gen->AddOutputLine((
          boost::format("script:request_end_sync(Script.ENTITY, \"%1%\", \"%2%\", %3%)")
          % entity.Name() % script_name % priority
        ).str());
    }
    catch (const InternalDecompilerError&){
        code_gen->AddOutputLine((
          boost::format("-- ERROR call to non existing function index %1%")
          % _params[2]->getUnsigned()
        ).str());
    }
}

void FF7ControlFlowInstruction::processRETTO(CodeGenerator* code_gen){
    auto entity_index = _params[0]->getUnsigned();
    auto priority = _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- return_to(script_id_in_current_entity=%2%, priority=%1%)")
      % entity_index % priority
    ).str());
}

void FF7ControlFlowInstruction::processWAIT(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("script:wait(%1%)") % (_params[0]->getUnsigned() / 30.0f)
    ).str());
}

void FF7ModuleInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::DSKCG: WriteTodo(code_gen, md.GetEntityName(), "DSKCG"); break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::ARROW:
            code_gen->AddOutputLine((
              boost::format("game:pointer_enable(%1%)")
              % (_params[0]->getUnsigned() ? "true" : "false")
            ).str());
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::PNAME:
            WriteTodo(code_gen, md.GetEntityName(), "PNAME");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::GMSPD:
            WriteTodo(code_gen, md.GetEntityName(), "GMSPD");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::SMSPD:
            WriteTodo(code_gen, md.GetEntityName(), "SMSPD");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::FLMAT:
            code_gen->AddOutputLine("game:fill_materia()");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::FLITM:
            WriteTodo(code_gen, md.GetEntityName(), "FLITM");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::BTLCK:
            code_gen->AddOutputLine((
              boost::format("game:battle_enable(%1%)")
              % (_params[0]->getUnsigned() ? "true" : "false")
            ).str());
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::MVLCK:
            code_gen->AddOutputLine((
              boost::format("game:movie_enable(%1%)")
              % (_params[0]->getUnsigned() ? "true" : "false")
            ).str());
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::SPCNM:
            WriteTodo(code_gen, md.GetEntityName(), "SPCNM");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::RSGLB:
            code_gen->AddOutputLine("game:global_reset()");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::CLITM:
            WriteTodo(code_gen, md.GetEntityName(), "CLITM");
            break;
        case OPCODE::MINIGAME: WriteTodo(code_gen, md.GetEntityName(), "MINIGAME"); break;
        case OPCODE::BTMD2: WriteTodo(code_gen, md.GetEntityName(), "BTMD2"); break;
        case OPCODE::BTRLD: WriteTodo(code_gen, md.GetEntityName(), "BTRLD"); break;
        case OPCODE::BTLTB: WriteTodo(code_gen, md.GetEntityName(), "BTLTB"); break;
        case OPCODE::MAPJUMP: processMAPJUMP(code_gen, func); break;
        case OPCODE::LSTMP: WriteTodo(code_gen, md.GetEntityName(), "LSTMP"); break;
        case OPCODE::BATTLE: processBATTLE(code_gen); break;
        case OPCODE::BTLON: processBTLON(code_gen); break;
        case OPCODE::BTLMD: WriteTodo(code_gen, md.GetEntityName(), "BTLMD"); break;
        case OPCODE::MPJPO:
            // Gateway function will do nothing if this is set to true
            code_gen->AddOutputLine(
              std::string("FFVII.Data.DisableGateways=")
              + (_params[0]->getUnsigned() ? "true" : "false")
            );
            break;
        // Prepare to change map, don't need to output anything for this.
        case OPCODE::PMJMP: code_gen->AddOutputLine("-- Prepare map change"); break;
        // Prepare to change map, don't need to output anything for this,
        // seems to be the same thing as PMJMP
        case OPCODE::PMJMP2: code_gen->AddOutputLine("-- Prepare map change 2"); break;
        case OPCODE::GAMEOVER: WriteTodo(code_gen, md.GetEntityName(), "GAMEOVER"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7ModuleInstruction::processBATTLE(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& battle_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    code_gen->AddOutputLine((boost::format("entity_manager:battle_run(%1%)") % battle_id).str());
}

void FF7ModuleInstruction::processBTLON(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("entity_manager:random_encounters_on(%1%)")
      % FieldCodeGenerator::FormatInvertedBool(_params[0]->getUnsigned())
    ).str());
}

void FF7ModuleInstruction::processMAPJUMP(CodeGenerator* code_gen, Function& func){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto target_map_id = _params[0]->getUnsigned();
    FunctionMetaData md(func._metadata);
    const std::string source_spawn_point_name
      = cg->GetFormatter().SpawnPointName(target_map_id, md.GetEntityName(), func._name, _address);

    cg->GetFormatter().AddSpawnPoint(
      target_map_id, md.GetEntityName(), func._name, _address,
      _params[1]->getSigned(), // X
      _params[2]->getSigned(), // Y
      _params[3]->getSigned(), // Walk mesh triangle ID
      _params[4]->getSigned()  // Angle
    );

    const std::string target_map_name = cg->GetFormatter().MapName(target_map_id);
    code_gen->AddOutputLine(
      "load_field_map_request(\"" + target_map_name + "\", \"" + source_spawn_point_name + "\")"
    );
}

void FF7MathInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::PLUS_: processSaturatedPLUS(code_gen); break;
        case OPCODE::PLUS2_: processSaturatedPLUS2(code_gen); break;
        case OPCODE::MINUS_: processSaturatedMINUS(code_gen); break;
        case OPCODE::MINUS2_: processSaturatedMINUS2(code_gen); break;
        case OPCODE::INC_: processSaturatedINC(code_gen); break;
        case OPCODE::INC2_: processSaturatedINC2(code_gen); break;
        case OPCODE::DEC_: processSaturatedDEC(code_gen); break;
        case OPCODE::DEC2_: processSaturatedDEC2(code_gen); break;
        case OPCODE::RDMSD: processRDMSD(code_gen); break;
        case OPCODE::SETBYTE:
        case OPCODE::SETWORD:
            processSETBYTE_SETWORD(code_gen);
            break;
        case OPCODE::BITON: processBITON(code_gen); break;
        case OPCODE::BITOFF: processBITOFF(code_gen); break;
        case OPCODE::BITXOR: WriteTodo(code_gen, md.GetEntityName(), "BITXOR"); break;
        case OPCODE::PLUS:
        case OPCODE::PLUS2:
            processPLUSx_MINUSx(code_gen, "+");
            break;
        case OPCODE::MINUS:
        case OPCODE::MINUS2:
            processPLUSx_MINUSx(code_gen, "-");
            break;
        case OPCODE::MUL: WriteTodo(code_gen, md.GetEntityName(), "MUL"); break;
        case OPCODE::MUL2: WriteTodo(code_gen, md.GetEntityName(), "MUL2"); break;
        case OPCODE::DIV: WriteTodo(code_gen, md.GetEntityName(), "DIV"); break;
        case OPCODE::DIV2: WriteTodo(code_gen, md.GetEntityName(), "DIV2"); break;
        case OPCODE::MOD:
            {
                const uint32 source_bank = _params[0]->getUnsigned();
                const uint32 source_address_or_value = _params[2]->getUnsigned();
                auto source = FieldCodeGenerator::FormatValueOrVariable(
                  cg->GetFormatter(), source_bank, source_address_or_value
                );
                const uint32 dest_bank = _params[1]->getUnsigned();
                const uint32 dest_address = _params[3]->getUnsigned();
                auto dest = FieldCodeGenerator::FormatValueOrVariable(
                  cg->GetFormatter(), dest_bank, dest_address
                );
                code_gen->AddOutputLine(
                  source + " = " + source + " % " + dest + code_gen->TargetLang().LineTerminator()
                );
            }
        break;
        case OPCODE::MOD2: WriteTodo(code_gen, md.GetEntityName(), "MOD2"); break;
        case OPCODE::AND: WriteTodo(code_gen, md.GetEntityName(), "AND"); break;
        case OPCODE::AND2: WriteTodo(code_gen, md.GetEntityName(), "AND2"); break;
        case OPCODE::OR: WriteTodo(code_gen, md.GetEntityName(), "OR"); break;
        case OPCODE::OR2: WriteTodo(code_gen, md.GetEntityName(), "OR2"); break;
        case OPCODE::XOR: WriteTodo(code_gen, md.GetEntityName(), "XOR"); break;
        case OPCODE::XOR2: WriteTodo(code_gen, md.GetEntityName(), "XOR2"); break;
        case OPCODE::INC:
        case OPCODE::INC2:
            processINCx_DECx(code_gen, "+");
            break;
        case OPCODE::DEC:
        case OPCODE::DEC2:
            processINCx_DECx(code_gen, "-");
            break;
        case OPCODE::RANDOM: processRANDOM(code_gen); break;
        case OPCODE::LBYTE: WriteTodo(code_gen, md.GetEntityName(), "LBYTE"); break;
        case OPCODE::HBYTE: WriteTodo(code_gen, md.GetEntityName(), "HBYTE"); break;
        case OPCODE::TWOBYTE: WriteTodo(code_gen, md.GetEntityName(), "2BYTE"); break;
        case OPCODE::SIN: WriteTodo(code_gen, md.GetEntityName(), "SIN"); break;
        case OPCODE::COS: WriteTodo(code_gen, md.GetEntityName(), "COS"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7MathInstruction::processSaturatedPLUS(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 255); %1% = 255; end") % lhs).str());
}

void FF7MathInstruction::processSaturatedPLUS2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 32767); %1% = 32767; end") % lhs).str());
}

void FF7MathInstruction::processSaturatedMINUS(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % lhs).str());
}

void FF7MathInstruction::processSaturatedMINUS2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % lhs).str());
}

void FF7MathInstruction::processSaturatedINC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 255); %1% = 255; end") % dest).str());
}

void FF7MathInstruction::processSaturatedINC2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 32767); %1% = 32767; end") % dest).str());
}

void FF7MathInstruction::processSaturatedDEC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % dest).str());
}

void FF7MathInstruction::processSaturatedDEC2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % dest).str());
}

void FF7MathInstruction::processRDMSD(CodeGenerator* code_gen){
    // TODO: we don't have os.time...
    // TODO: RNG emulation?
    code_gen->AddOutputLine("math.randomseed(os.time())");
}

void FF7MathInstruction::processSETBYTE_SETWORD(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: respect destination bank sizes (16-bit writes only affect low byte)
    code_gen->AddOutputLine((boost::format("%1% = %2%") % destination % source).str());
}

void FF7MathInstruction::processBITON(CodeGenerator* code_gen){
    code_gen->AddOutputLine((boost::format("bit_on(%1%, %2%, %3%)")
      % _params[0]->getUnsigned() % _params[2]->getUnsigned() % _params[3]->getUnsigned()
    ).str());
}

void FF7MathInstruction::processBITOFF(CodeGenerator* code_gen){
    code_gen->AddOutputLine((boost::format("bit_off(%1%, %2%, %3%)")
      % _params[0]->getUnsigned() % _params[2]->getUnsigned() % _params[3]->getUnsigned()
    ).str());
}

void FF7MathInstruction::processPLUSx_MINUSx(CodeGenerator* code_gen, const std::string& op){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: repect destination bank sizes and wraparound
    code_gen->AddOutputLine((boost::format("%1% = %1% %2% %3%") % lhs % op % rhs).str());
}

void FF7MathInstruction::processINCx_DECx(CodeGenerator* code_gen, const std::string& op){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: repect destination bank sizes and wraparound
    code_gen->AddOutputLine((boost::format("%1% = %1% %2% 1") % destination % op).str());
}

void FF7MathInstruction::processRANDOM(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes (16-bit writes only affect low byte).
    // TODO: RNG emulation?
    code_gen->AddOutputLine((boost::format("%1% = math.random(0, 255)") % destination).str());
}

void FF7WindowInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FF7FieldEngine& eng = static_cast<FF7FieldEngine&>(*engine);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::TUTOR: WriteTodo(code_gen, md.GetEntityName(), "TUTOR"); break;
        case OPCODE::WCLS: WriteTodo(code_gen, md.GetEntityName(), "WCLS"); break;
        case OPCODE::WSIZW: WriteTodo(code_gen, md.GetEntityName(), "WSIZW"); break;
        case OPCODE::WSPCL: WriteTodo(code_gen, md.GetEntityName(), "WSPCL"); break;
        case OPCODE::WNUMB: WriteTodo(code_gen, md.GetEntityName(), "WNUMB"); break;
        case OPCODE::STTIM: WriteTodo(code_gen, md.GetEntityName(), "STTIM"); break;
        case OPCODE::MESSAGE: processMESSAGE(code_gen, eng.ScriptName()); break;
        case OPCODE::MPARA: WriteTodo(code_gen, md.GetEntityName(), "MPARA"); break;
        case OPCODE::MPRA2: WriteTodo(code_gen, md.GetEntityName(), "MPRA2"); break;
        case OPCODE::MPNAM: processMPNAM(code_gen); break;
        case OPCODE::ASK: WriteTodo(code_gen, md.GetEntityName(), "ASK"); break;
        case OPCODE::MENU: WriteTodo(code_gen, md.GetEntityName(), "MENU"); break;
        case OPCODE::MENU2: processMENU2(code_gen); break;
        case OPCODE::WINDOW: processWINDOW(code_gen); break;
        case OPCODE::WMOVE: WriteTodo(code_gen, md.GetEntityName(), "WMOVE"); break;
        case OPCODE::WMODE: WriteTodo(code_gen, md.GetEntityName(), "WMODE"); break;
        case OPCODE::WREST: WriteTodo(code_gen, md.GetEntityName(), "WREST"); break;
        case OPCODE::WCLSE: processWCLSE(code_gen); break;
        case OPCODE::WROW: WriteTodo(code_gen, md.GetEntityName(), "WROW"); break;
        case OPCODE::GWCOL: WriteTodo(code_gen, md.GetEntityName(), "GWCOL"); break;
        case OPCODE::SWCOL: WriteTodo(code_gen, md.GetEntityName(), "SWCOL"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7WindowInstruction::processWINDOW(CodeGenerator* code_gen){
    // Init a new window. It won't be displayed until MESSAGE is used.
    auto windowId = _params[0]->getUnsigned();
    auto x = _params[1]->getUnsigned();
    auto y = _params[2]->getUnsigned();
    auto width = _params[3]->getUnsigned();
    auto height = _params[4]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("dialog:dialog_open(\"%1%\", %2%, %3%, %4%, %5%)")
      % windowId % x % y % width % height
    ).str());
}

void FF7WindowInstruction::processMESSAGE(
  CodeGenerator* code_gen, const std::string& script_name
){
    // Displays a dialog in the WINDOW that has previously been initialized to display this dialog.
    auto window_id = _params[0]->getUnsigned();
    auto dialog_id = _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("dialog:dialog_set_text(\"%1%\", \"%2%_%3%\")")
      % window_id % script_name % dialog_id
    ).str());
    code_gen->AddOutputLine(
      (boost::format("dialog:dialog_wait_for_close(\"%1%\")") % window_id).str()
    );

}

void FF7WindowInstruction::processWCLSE(CodeGenerator* code_gen){
    // Close a dialog.
    auto windowId = _params[0]->getUnsigned();
    code_gen->AddOutputLine((boost::format("dialog:dialog_close(\"%1%\")") % windowId).str());
}

void FF7WindowInstruction::processMPNAM(CodeGenerator* code_gen){
    code_gen->AddOutputLine(
      (boost::format("-- field:map_name(%1%)") % _params[0]->getUnsigned()).str()
    );
}

void FF7WindowInstruction::processMENU2(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- field:menu_lock(%1%)")
      % FieldCodeGenerator::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FF7PartyInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::SPTYE: WriteTodo(code_gen, md.GetEntityName(), "SPTYE"); break;
        case OPCODE::GTPYE: WriteTodo(code_gen, md.GetEntityName(), "GTPYE"); break;
        case OPCODE::GOLDU: WriteTodo(code_gen, md.GetEntityName(), "GOLDU"); break;
        case OPCODE::GOLDD: WriteTodo(code_gen, md.GetEntityName(), "GOLDD"); break;
        case OPCODE::HMPMAX1: WriteTodo(code_gen, md.GetEntityName(), "HMPMAX1"); break;
        case OPCODE::HMPMAX2: WriteTodo(code_gen, md.GetEntityName(), "HMPMAX2"); break;
        case OPCODE::MHMMX: WriteTodo(code_gen, md.GetEntityName(), "MHMMX"); break;
        case OPCODE::HMPMAX3: WriteTodo(code_gen, md.GetEntityName(), "HMPMAX3"); break;
        case OPCODE::MPU: WriteTodo(code_gen, md.GetEntityName(), "MPU"); break;
        case OPCODE::MPD: WriteTodo(code_gen, md.GetEntityName(), "MPD"); break;
        case OPCODE::HPU: WriteTodo(code_gen, md.GetEntityName(), "HPU"); break;
        case OPCODE::HPD: WriteTodo(code_gen, md.GetEntityName(), "HPD"); break;
        case OPCODE::STITM: processSTITM(code_gen); break;
        case OPCODE::DLITM: WriteTodo(code_gen, md.GetEntityName(), "DLITM"); break;
        case OPCODE::CKITM: WriteTodo(code_gen, md.GetEntityName(), "CKITM"); break;
        case OPCODE::SMTRA: WriteTodo(code_gen, md.GetEntityName(), "SMTRA"); break;
        case OPCODE::DMTRA: WriteTodo(code_gen, md.GetEntityName(), "DMTRA"); break;
        case OPCODE::CMTRA: WriteTodo(code_gen, md.GetEntityName(), "CMTRA"); break;
        case OPCODE::GETPC: WriteTodo(code_gen, md.GetEntityName(), "GETPC"); break;
        case OPCODE::PRTYP: WriteTodo(code_gen, md.GetEntityName(), "PRTYP"); break;
        case OPCODE::PRTYM: WriteTodo(code_gen, md.GetEntityName(), "PRTYM"); break;
        case OPCODE::PRTYE: processPRTYE(code_gen); break;
        case OPCODE::MMBUD: WriteTodo(code_gen, md.GetEntityName(), "MMBUD"); break;
        case OPCODE::MMBLK: WriteTodo(code_gen, md.GetEntityName(), "MMBLK"); break;
        case OPCODE::MMBUK: WriteTodo(code_gen, md.GetEntityName(), "MMBUK"); break;
        case OPCODE::CHGLD: WriteTodo(code_gen, md.GetEntityName(), "CHGLD"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7PartyInstruction::processSTITM(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& item_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& amount = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine((boost::format("FFVII.add_item(%1%, %2%)") % item_id % amount).str());
}

void FF7PartyInstruction::processPRTYE(CodeGenerator* code_gen){
    FieldCodeGenerator* gc = static_cast<FieldCodeGenerator*>(code_gen);
    auto char_id_1 = gc->GetFormatter().CharName(_params[0]->getUnsigned());
    char_id_1 = (char_id_1 == "") ? "nil" : ("\"" + char_id_1 + "\"");
    auto char_id_2 = gc->GetFormatter().CharName(_params[1]->getUnsigned());
    char_id_2 = (char_id_2 == "") ? "nil" : ("\"" + char_id_2 + "\"");
    auto char_id_3 = gc->GetFormatter().CharName(_params[2]->getUnsigned());
    char_id_3 = (char_id_3 == "") ? "nil" : ("\"" + char_id_3 + "\"");
    code_gen->AddOutputLine((
      boost::format("FFVII.set_party(%1%, %2%, %3%)") % char_id_1 % char_id_2 % char_id_3
    ).str());
}

void FF7ModelInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FF7FieldEngine& eng = static_cast<FF7FieldEngine&>(*engine);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::JOIN: processJOIN(code_gen); break;
        case OPCODE::SPLIT: processSPLIT(code_gen); break;
        case OPCODE::BLINK: WriteTodo(code_gen, md.GetEntityName(), "BLINK"); break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::EYETX:
            WriteTodo(code_gen, md.GetEntityName(), "EYETX");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::TRNSP:
            WriteTodo(code_gen, md.GetEntityName(), "TRNSP");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::AMBNT:
            WriteTodo(code_gen, md.GetEntityName(), "AMBNT");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown03:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown03");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown04:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown04");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown05:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown05");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::LIGHT:
            WriteTodo(code_gen, md.GetEntityName(), "LIGHT");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown07:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown07");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown08:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown08");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown09:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown09");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::SBOBJ:
            WriteTodo(code_gen, md.GetEntityName(), "SBOBJ");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown0B:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown0B");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown0C:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown0C");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::SHINE:
            WriteTodo(code_gen, md.GetEntityName(), "SHINE");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::RESET:
            WriteTodo(code_gen, md.GetEntityName(), "RESET");
            break;
        case OPCODE::KAWIW: WriteTodo(code_gen, md.GetEntityName(), "KAWIW"); break;
        case OPCODE::PMOVA: WriteTodo(code_gen, md.GetEntityName(), "PMOVA"); break;
        case OPCODE::PDIRA: WriteTodo(code_gen, md.GetEntityName(), "PDIRA"); break;
        case OPCODE::PTURA: WriteTodo(code_gen, md.GetEntityName(), "PTURA"); break;
        case OPCODE::PGTDR: WriteTodo(code_gen, md.GetEntityName(), "PGTDR"); break;
        case OPCODE::PXYZI: WriteTodo(code_gen, md.GetEntityName(), "PXYZI"); break;
        case OPCODE::TLKON: processTLKON(code_gen, md.GetEntityName()); break;
        case OPCODE::PC: processPC(code_gen, md.GetEntityName()); break;
        case OPCODE::opCodeCHAR: processCHAR(code_gen, md.GetEntityName()); break;
        case OPCODE::DFANM: processDFANM(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::ANIME1: processANIME1(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::VISI: processVISI(code_gen, md.GetEntityName()); break;
        case OPCODE::XYZI: processXYZI(code_gen, md.GetEntityName()); break;
        case OPCODE::XYI: WriteTodo(code_gen, md.GetEntityName(), "XYI"); break;
        case OPCODE::XYZ: WriteTodo(code_gen, md.GetEntityName(), "XYZ"); break;
        case OPCODE::MOVE: processMOVE(code_gen, md.GetEntityName()); break;
        case OPCODE::CMOVE: WriteTodo(code_gen, md.GetEntityName(), "CMOVE"); break;
        case OPCODE::MOVA: WriteTodo(code_gen, md.GetEntityName(), "MOVA"); break;
        case OPCODE::TURA: WriteTodo(code_gen, md.GetEntityName(), "TURA"); break;
        case OPCODE::ANIMW: WriteTodo(code_gen, md.GetEntityName(), "ANIMW"); break;
        case OPCODE::FMOVE: WriteTodo(code_gen, md.GetEntityName(), "FMOVE"); break;
        case OPCODE::ANIME2: WriteTodo(code_gen, md.GetEntityName(), "ANIME2"); break;
        case OPCODE::ANIM_1: WriteTodo(code_gen, md.GetEntityName(), "ANIM_1"); break;
        case OPCODE::CANIM1: WriteTodo(code_gen, md.GetEntityName(), "CANIM1"); break;
        case OPCODE::CANM_1: WriteTodo(code_gen, md.GetEntityName(), "CANM_1"); break;
        case OPCODE::MSPED: processMSPED(code_gen, md.GetEntityName()); break;
        case OPCODE::DIR: processDIR(code_gen, md.GetEntityName()); break;
        case OPCODE::TURNGEN: processTURNGEN(code_gen, md.GetEntityName()); break;
        case OPCODE::TURN: WriteTodo(code_gen, md.GetEntityName(), "TURN"); break;
        case OPCODE::DIRA: WriteTodo(code_gen, md.GetEntityName(), "DIRA"); break;
        case OPCODE::GETDIR: WriteTodo(code_gen, md.GetEntityName(), "GETDIR"); break;
        case OPCODE::GETAXY: WriteTodo(code_gen, md.GetEntityName(), "GETAXY"); break;
        case OPCODE::GETAI: processGETAI(code_gen, eng); break;
        case OPCODE::ANIM_2: processANIM_2(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::CANIM2: processCANIM2(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::CANM_2: processCANM_2(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::ASPED: WriteTodo(code_gen, md.GetEntityName(), "ASPED"); break;
        case OPCODE::CC: processCC(code_gen, eng); break;
        case OPCODE::JUMP: processJUMP(code_gen, md.GetEntityName()); break;
        case OPCODE::AXYZI: processAXYZI(code_gen); break;
        case OPCODE::LADER: processLADER(code_gen, md.GetEntityName()); break;
        case OPCODE::OFST: processOFST(code_gen, md.GetEntityName()); break;
        case OPCODE::OFSTW:
            code_gen->AddOutputLine("self." + md.GetEntityName() + ":offset_sync()");
            break;
        case OPCODE::TALKR: WriteTodo(code_gen, md.GetEntityName(), "TALKR"); break;
        case OPCODE::SLIDR: WriteTodo(code_gen, md.GetEntityName(), "SLIDR"); break;
        case OPCODE::SOLID: processSOLID(code_gen, md.GetEntityName()); break;
        case OPCODE::TLKR2: WriteTodo(code_gen, md.GetEntityName(), "TLKR2"); break;
        case OPCODE::SLDR2: WriteTodo(code_gen, md.GetEntityName(), "SLDR2"); break;
        case OPCODE::CCANM: WriteTodo(code_gen, md.GetEntityName(), "CCANM"); break;
        case OPCODE::FCFIX: WriteTodo(code_gen, md.GetEntityName(), "FCFIX"); break;
        case OPCODE::ANIMB: WriteTodo(code_gen, md.GetEntityName(), "ANIMB"); break;
        case OPCODE::TURNW: WriteTodo(code_gen, md.GetEntityName(), "TURNW"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7ModelInstruction::processJOIN(CodeGenerator* code_gen){
    code_gen->AddOutputLine("join_party(" + std::to_string(_params[0]->getUnsigned()) + ")");
}

void FF7ModelInstruction::processSPLIT(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& ax = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[6]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& ay = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[7]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& ar = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[8]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& bx = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[9]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& by = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[4]->getUnsigned(), _params[10]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& br = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[5]->getUnsigned(), _params[11]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& speed = _params[12]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("split_party(%1%, %2%, %3%, %4%, %5%, %6%, %7%)")
      % ax % ay % ar % bx % by % br % speed
    ).str());
}

void FF7ModelInstruction::processTLKON(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_talkable(%2%)")
      % entity % FieldCodeGenerator::FormatInvertedBool(_params[0]->getUnsigned())
    ).str());
}

void FF7ModelInstruction::processPC(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine(
      (boost::format("set_entity_to_character(\"%1%\", \"%1%\")") % entity).str()
    );
}

void FF7ModelInstruction::processCHAR(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine(
      (boost::format("self.%1% = entity_manager:get_entity(\"%1%\")") % entity).str()
    );
}

void FF7ModelInstruction::processDFANM(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_default_animation(\"%2%\") -- speed %3%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\")")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id)
    ).str());
}

void FF7ModelInstruction::processANIME1(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\") -- speed %3%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FF7ModelInstruction::processVISI(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_visible(%2%)")
      % entity % FieldCodeGenerator::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FF7ModelInstruction::processXYZI(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& z = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[6]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& triangle_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[7]->getUnsigned()
    );
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_position(%2%, %3%, %4%) -- triangle ID %5%")
      % entity % x % y % z % triangle_id
    ).str());
}

void FF7ModelInstruction::processMOVE(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    code_gen->AddOutputLine(
      (boost::format("self.%1%:move_to_position(%2%, %3%)") % entity % x % y).str()
    );
    code_gen->AddOutputLine((boost::format("self.%1%:move_sync()") % entity).str());
}

void FF7ModelInstruction::processMSPED(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& speed = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[2]->getUnsigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f * scale / 30.0f
    );
    code_gen->AddOutputLine(
      (boost::format("self.%1%:set_move_auto_speed(%2%)") % entity % speed).str()
    );
}

void FF7ModelInstruction::processDIR(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& degrees = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f / 360.0f
    );
    code_gen->AddOutputLine((boost::format("self.%1%:set_rotation(%2%)") % entity % degrees).str());
}

void FF7ModelInstruction::processTURNGEN(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& degrees = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[2]->getUnsigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f / 360.0f
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
    code_gen->AddOutputLine((
      boost::format("self.%1%:turn_to_direction(%2%, %3%, %4%, %5%)")
      % entity % degrees % direction % step_type % scaled_steps
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:turn_sync()") % entity).str());
}

void FF7ModelInstruction::processGETAI(CodeGenerator* code_gen, const FF7FieldEngine& engine){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: check for assignment to literal.
    const auto& variable = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    const auto& entity = engine.EntityByIndex(_params[2]->getUnsigned());
    code_gen->AddOutputLine(
      (boost::format("%1% = entity_manager:get_entity(\"%2%\"):get_move_triangle_id()")
      % variable % entity.Name()
    ).str());
}

void FF7ModelInstruction::processANIM_2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation_stop(\"%2%\") -- speed %3%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FF7ModelInstruction::processCANIM2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    auto start_frame = _params[1]->getUnsigned() / 30.0f;
    auto end_frame = _params[2]->getUnsigned() / 30.0f;
    // TODO: check for zero.
    auto speed = 1.0f / _params[3]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\", %3%, %4%) -- speed %5%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id)
      % start_frame % end_frame % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FF7ModelInstruction::processCANM_2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // ID will be fixed-up downstream.
    auto animation_id = _params[0]->getUnsigned();
    auto start_frame = _params[1]->getUnsigned() / 30.0f;
    auto end_frame = _params[2]->getUnsigned() / 30.0f;
    // TODO: check for zero.
    auto speed = 1.0f / _params[3]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation_stop(\"%2%\", %3%, %4%) -- speed %5%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id)
      % start_frame % end_frame % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FF7ModelInstruction::processCC(CodeGenerator* code_gen, const FF7FieldEngine& engine){
    const auto& entity = engine.EntityByIndex(_params[0]->getUnsigned());
    code_gen->AddOutputLine(
      (boost::format("entity_manager:set_player_entity(\"%1%\")") % entity.Name()).str()
    );
}

void FF7ModelInstruction::processJUMP(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    float x = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float y = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    int i = atoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[6]->getSigned()
    ).c_str());
    int steps = atoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[7]->getSigned()
    ).c_str());
    //x *= 0.00781250273224;
    //y *= 0.00781250273224;
    // Always force reduce steps and bound it between 1 and 5.
    steps /= 6;
    if (steps < 1) steps = 1;
    if (steps > 5) steps = 5;
    // TODO: Z hardcoded as -1, handled in Entity::ScriptJumpToPosition.
    // TODO: Hardcoded 0.5 in seconds. Calculate using distance.
    code_gen->AddOutputLine((
      boost::format("self.%1%:jump_to_position(%2%, %3%, -1, 0.5, %4%) -- %5% steps.")
      % entity % x % y % i % steps
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:jump_sync()") % entity).str());
}

void FF7ModelInstruction::processAXYZI(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    code_gen->AddOutputLine((
      boost::format("axyzi(%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%)")
      % _params[0]->getSigned() % _params[1]->getSigned() % _params[2]->getSigned()
      % _params[3]->getSigned() % _params[4]->getSigned() % _params[5]->getSigned()
      % _params[6]->getSigned() % _params[7]->getSigned() % _params[8]->getSigned()
      % scale
    ).str());
}

void FF7ModelInstruction::processLADER(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& z = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[6]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    uint end_triangle = atoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[7]->getUnsigned()
    ).c_str());
    uint keys = _params[8]->getUnsigned();
    uint animation = _params[9]->getUnsigned();
    //float orientation = _params[10]->getUnsigned() / (256.0f / 360.0f);
    const auto& orientation = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), 0, _params[10]->getSigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f / 360.0f
    );
    uint speed = _params[11]->getUnsigned();
    // TODO: Animation hardcoded as "btce".
    // TODO: Orientation and speed not set.
    code_gen->AddOutputLine((
      boost::format(
        "self.%1%:linear_to_position(%2%, %3%, %4%, %5%, \"Climb\", %6%, %7%) "
        "-- Animation %8% -- Speed %9%"
      ) % entity % x % y % z % keys % orientation % end_triangle % animation % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:linear_sync()") % entity).str());
}

void FF7ModelInstruction::processSOLID(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_solid(%2%)")
      % entity % FieldCodeGenerator::FormatInvertedBool(_params[0]->getUnsigned())
    ).str());
}

void FF7ModelInstruction::processOFST(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    float x = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[5]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float y = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(),_params[6]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float z = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[7]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float speed = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[8]->getUnsigned()
    ));
    // Spatial coordinates need to be scaled down.
    // TODO: This number is empirically deducted. Why this number?
    //x *= 0.00390f;
    //y *= 0.00390f;
    //z *= 0.00390f;
    // Speed needs to be scaled down by the frame rate.
    speed /= 30.0f;
    code_gen->AddOutputLine((
      boost::format("self.%1%:offset_to_position(%2%, %3%, %4%, %5%, %6%)")
      % entity % x % y % z
      % (_params[4]->getUnsigned() ? "Entity.SMOOTH" : "Entity.LINEAR")% speed
    ).str());
}

void FF7WalkmeshInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::SLIP: WriteTodo(code_gen, md.GetEntityName(), "SLIP"); break;
        case OPCODE::UC: processUC(code_gen); break;
        case OPCODE::IDLCK:
            // Triangle id, on or off
            code_gen->AddOutputLine(
                (boost::format("walkmesh:lock_walkmesh(%1%, %2%)")
                % _params[0]->getUnsigned()
                % FieldCodeGenerator::FormatBool(_params[1]->getUnsigned())).str());
            break;
        case OPCODE::LINE: processLINE(code_gen, md.GetEntityName()); break;
        case OPCODE::LINON: WriteTodo(code_gen, md.GetEntityName(), "LINON"); break;
        case OPCODE::SLINE: WriteTodo(code_gen, md.GetEntityName(), "SLINE"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7WalkmeshInstruction::processUC(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("entity_manager:player_lock(%1%)")
      % FieldCodeGenerator::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FF7WalkmeshInstruction::processLINE(CodeGenerator* code_gen, const std::string& entity){
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
    code_gen->AddOutputLine(
      "-- LINE (" + std::to_string(xa) + ", " + std::to_string(ya) + ", " + std::to_string(za)
      + ")-(" + std::to_string(xb) + ", " +std::to_string(yb) + ", " + std::to_string(zb) + ")"
    );
}

void FF7BackgroundInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::BGPDH: WriteTodo(code_gen, md.GetEntityName(), "BGPDH");break;
        case OPCODE::BGSCR: WriteTodo(code_gen, md.GetEntityName(), "BGSCR"); break;
        case OPCODE::MPPAL: WriteTodo(code_gen, md.GetEntityName(), "MPPAL"); break;
        case OPCODE::BGON: processBGON(code_gen); break;
        case OPCODE::BGOFF: processBGOFF(code_gen); break;
        case OPCODE::BGROL: WriteTodo(code_gen, md.GetEntityName(), "BGROL"); break;
        case OPCODE::BGROL2: WriteTodo(code_gen, md.GetEntityName(), "BGROL2"); break;
        case OPCODE::BGCLR: processBGCLR(code_gen); break;
        case OPCODE::STPAL: processSTPAL(code_gen); break;
        case OPCODE::LDPAL: processLDPAL(code_gen); break;
        case OPCODE::CPPAL: processCPPAL(code_gen); break;
        case OPCODE::RTPAL: WriteTodo(code_gen, md.GetEntityName(), "RTPAL"); break;
        case OPCODE::ADPAL: processADPAL(code_gen); break;
        case OPCODE::MPPAL2: processMPPAL2(code_gen); break;
        case OPCODE::STPLS: processSTPLS(code_gen); break;
        case OPCODE::LDPLS: processLDPLS(code_gen); break;
        case OPCODE::CPPAL2: WriteTodo(code_gen, md.GetEntityName(), "CPPAL2"); break;
        case OPCODE::ADPAL2: WriteTodo(code_gen, md.GetEntityName(), "ADPAL2"); break;
        case OPCODE::RTPAL2: WriteTodo(code_gen, md.GetEntityName(), "RTPAL2"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7BackgroundInstruction::processBGON(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& layer_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_on(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FF7BackgroundInstruction::processBGOFF(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& layer_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_off(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FF7BackgroundInstruction::processBGCLR(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[2]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_clear(%1%)") % background_id).str()
    );
}

void FF7BackgroundInstruction::processSTPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FF7BackgroundInstruction::processLDPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FF7BackgroundInstruction::processCPPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- copy palette %1% to palette %2%, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FF7BackgroundInstruction::processADPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[4]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[8]->getUnsigned()
    );
    auto num_entries = _params[11]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format(
        "-- add RGB(%3%, %4%, %5%) to %6% entries of palette stored at position %1%, "
        "storing result in position %2%"
      ) % source % destination % r % g % b % num_entries
    ).str());
}

void FF7BackgroundInstruction::processMPPAL2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[4]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[8]->getUnsigned()
    );
    auto num_entries = _params[11]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format(
        "-- multiply RGB(%3%, %4%, %5%) by %6% entries of palette stored at position %1%, "
        "storing result in position %2%"
      ) % source % destination % r % g % b % num_entries).str());
}

void FF7BackgroundInstruction::processSTPLS(CodeGenerator* code_gen){
    auto source = _params[0]->getUnsigned();
    auto destination = _params[1]->getUnsigned();
    auto start_clut = _params[2]->getUnsigned();
    auto num_entries = _params[3]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index %3%, %4% entries")
      % source % destination % start_clut % num_entries
    ).str());
}

void FF7BackgroundInstruction::processLDPLS(CodeGenerator* code_gen){
    auto source = _params[0]->getUnsigned();
    auto destination = _params[1]->getUnsigned();
    auto startClut = _params[2]->getUnsigned();
    auto num_entries = _params[3]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index %3%, %4% entries")
      % source % destination % startClut % num_entries
    ).str());
}

void FF7CameraInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
    case OPCODE::NFADE: processNFADE(code_gen); break;
    case OPCODE::SHAKE: WriteTodo(code_gen, md.GetEntityName(), "SHAKE"); break;
    case OPCODE::SCRLO: WriteTodo(code_gen, md.GetEntityName(), "SCRLO"); break;
    case OPCODE::SCRLC: WriteTodo(code_gen, md.GetEntityName(), "SCRLC"); break;
    case OPCODE::SCR2D: processSCR2D(code_gen); break;
    case OPCODE::SCRCC: WriteTodo(code_gen, md.GetEntityName(), "SCRCC"); break;
    case OPCODE::SCR2DC: processSCR2DC(code_gen); break;
    case OPCODE::SCRLW: WriteTodo(code_gen, md.GetEntityName(), "SCRLW"); break;
    case OPCODE::SCR2DL: WriteTodo(code_gen, md.GetEntityName(), "SCR2DL"); break;
    case OPCODE::VWOFT: WriteTodo(code_gen, md.GetEntityName(), "VWOFT"); break;
    case OPCODE::FADE: processFADE(code_gen); break;
    case OPCODE::FADEW: WriteTodo(code_gen, md.GetEntityName(), "FADEW"); break;
    case OPCODE::SCRLP: WriteTodo(code_gen, md.GetEntityName(), "SCRLP"); break;
    case OPCODE::MVCAM: WriteTodo(code_gen, md.GetEntityName(), "MVCAM"); break;
    case OPCODE::SCRLA: WriteTodo(code_gen, md.GetEntityName(), "SCRLA"); break;
    default:
        code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
          md.GetEntityName(), _address, _opcode
        ));
    }
}

void FF7CameraInstruction::processNFADE(CodeGenerator* code_gen){
    // TODO: Not fully reversed.
    auto raw_type = _params[4]->getUnsigned();
    if (raw_type == 0){
        code_gen->AddOutputLine("-- fade:clear()");
        return;
    }
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const std::string type = raw_type == 12 ? "Fade.SUBTRACT" : "Fade.ADD";
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[5]->getUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& unknown = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[8]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%)") % type % r % g % b % unknown).str()
    );
}

void FF7CameraInstruction::processSCR2D(CodeGenerator* code_gen){
    // kUpScaler.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getSigned()
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getSigned()
    );
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.NONE, 0)")
      % x % y
    ).str());
}

void FF7CameraInstruction::processSCR2DC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getSigned()
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getSigned()
    );
    const auto& speed = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[6]->getUnsigned(),
      FieldCodeGenerator::ValueType::Float, 30.0f
    );
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.SMOOTH, %3%)")
      % x % y % speed
    ).str());
}

void FF7CameraInstruction::processFADE(CodeGenerator* code_gen){
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
        case 4: code_gen->AddOutputLine("-- fade:black()"); return;
        default: type = "Fade.ADD"; break;
    }

    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[6]->getUnsigned()
    );
    // TODO: needs to be divided by 30.0f?
    auto speed = _params[7]->getUnsigned();
    auto start = _params[9]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%, %6%)") % type % r % g % b % speed % start
    ).str());
}

void FF7AudioVideoInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::BGMOVIE: WriteTodo(code_gen, md.GetEntityName(), "BGMOVIE"); break;
        case OPCODE::AKAO2: processAKAO2(code_gen); break;
        case OPCODE::MUSIC: processMUSIC(code_gen); break;
        case OPCODE::SOUND: processSOUND(code_gen); break;
        case OPCODE::AKAO: processAKAO(code_gen); break;
        case OPCODE::MUSVT: WriteTodo(code_gen, md.GetEntityName(), "MUSVT"); break;
        case OPCODE::MUSVM: WriteTodo(code_gen, md.GetEntityName(), "MUSVM"); break;
        case OPCODE::MULCK: processMULCK(code_gen); break;
        case OPCODE::BMUSC: WriteTodo(code_gen, md.GetEntityName(), "BMUSC"); break;
        case OPCODE::CHMPH: WriteTodo(code_gen, md.GetEntityName(), "CHMPH"); break;
        case OPCODE::PMVIE: processPMVIE(code_gen); break;
        case OPCODE::MOVIE: processMOVIE(code_gen); break;
        case OPCODE::MVIEF: processMVIEF(code_gen); break;
        case OPCODE::FMUSC: WriteTodo(code_gen, md.GetEntityName(), "FMUSC"); break;
        case OPCODE::CMUSC: WriteTodo(code_gen, md.GetEntityName(), "CMUSC"); break;
        case OPCODE::CHMST: WriteTodo(code_gen, md.GetEntityName(), "CHMST"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7AudioVideoInstruction::processAKAO2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& param1 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& param2 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[8]->getUnsigned()
    );
    const auto& param3 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& param4 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& param5 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[5]->getUnsigned(), _params[11]->getUnsigned()
    );
    auto op = _params[6]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
      % param1 % param2 % param3 % param4 % param5 % op
    ).str());
}

void FF7AudioVideoInstruction::processMUSIC(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x10, pointer_to_field_AKAO_%1%)")
      % _params[0]->getUnsigned()
    ).str());
}

void FF7AudioVideoInstruction::processSOUND(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& soundId = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& panning = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- music:execute_akao(0x20, %1%, %2%)") % soundId % panning).str()
    );
}

void FF7AudioVideoInstruction::processAKAO(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& param1 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& param2 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[8]->getUnsigned()
    );
    const auto& param3 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& param4 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& param5 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[5]->getUnsigned(), _params[11]->getUnsigned()
    );
    auto op = _params[6]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
      % param1 % param2 % param3 % param4 % param5 % op
    ).str());
}

void FF7AudioVideoInstruction::processMULCK(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- music:lock(%1%)")
      % FieldCodeGenerator::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FF7AudioVideoInstruction::processPMVIE(CodeGenerator* code_gen){
    code_gen->AddOutputLine(
      (boost::format("-- field:movie_set(%1%)") % _params[0]->getUnsigned()
    ).str());
}

void FF7AudioVideoInstruction::processMOVIE(CodeGenerator* code_gen){
    code_gen->AddOutputLine("-- field:play_movie()");
}

void FF7AudioVideoInstruction::processMVIEF(CodeGenerator* code_gen){
    // TODO: Check for assignment to value.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[2]->getUnsigned());
    code_gen->AddOutputLine(
      (boost::format("-- %1% = field:get_movie_frame()") % destination).str()
    );
}

void FF7UncategorizedInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::MPDSP: WriteTodo(code_gen, md.GetEntityName(), "MPDSP"); break;
        case OPCODE::SETX: WriteTodo(code_gen, md.GetEntityName(), "SETX"); break;
        case OPCODE::GETX: WriteTodo(code_gen, md.GetEntityName(), "GETX"); break;
        case OPCODE::SEARCHX: WriteTodo(code_gen, md.GetEntityName(), "SEARCHX"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode)
            );
    }
}

void FF7NoOperationInstruction::processInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){}
