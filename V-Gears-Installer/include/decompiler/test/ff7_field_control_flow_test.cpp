#include <gmock/gmock.h>

#include "../ControlFlow.h"
#include "decompiler/ff7_field/ff7_field_disassembler.h"
#include "decompiler/ff7_field/ff7_field_engine.h"
#include "decompiler/ff7_field/ff7_field_codegen.h"
#include "util.h"
#include "ff7_field_dummy_formatter.h"

// Verify next 3 instructions are NOP's and return address of final NOP
static uint32 Is3Nops(InstVec& insts, uint32& pos)
{
    for (int i = 0; i < 3; i++)
    {
        pos++;
        EXPECT_EQ(insts[pos]->opcode_, eOpcodes::NOP);
        EXPECT_EQ(insts[pos]->params_.size(), 0);
    }
    return insts[pos]->_address;
}

// Each forward jump type has its own script that starts with the
// jump opcode followed by 2 NOP's, the 3rd NOP is the end of the script
// and should be the target address of the jump.
static void CheckForwardJump(eOpcodes opCode, size_t paramsSize, InstVec& insts, uint32& pos)
{
    pos++;
    const InstPtr& jumpInst = insts[pos];
    ASSERT_EQ(jumpInst->opcode_, opCode);
    ASSERT_EQ(jumpInst->params_.size(), paramsSize);

    const uint32 lastNopAddr = Is3Nops(insts, pos);
    ASSERT_EQ(jumpInst->GetDestAddress(), lastNopAddr);
}

// Verify next instructions is a NOP and return address of final NOP
static uint32 IsNop(InstVec& insts, uint32& pos)
{
    pos++;
    EXPECT_EQ(insts[pos]->opcode_, eOpcodes::NOP);
    EXPECT_EQ(insts[pos]->params_.size(), 0);
    return insts[pos]->_address;
}


// For a backwards jump we start with 3 NOPS and the last instruction is the jump itself which
// should be targeting the second/middle NOP
static void CheckBackwardJump(eOpcodes opCode, size_t paramsSize, InstVec& insts, uint32& pos)
{
    IsNop(insts, pos);

    // Second is the target, so check is NOP & grab addr
    const uint32 firstNopAddr = IsNop(insts, pos);

    IsNop(insts, pos);

    // Grab the final instruction, the jump itself
    pos++;
    const InstPtr& jumpInst = insts[pos];

    ASSERT_EQ(jumpInst->opcode_, opCode);
    ASSERT_EQ(jumpInst->params_.size(), paramsSize);
    ASSERT_EQ(jumpInst->GetDestAddress(), firstNopAddr);
}



TEST(FF7Field, ControlFlow)
{
    InstVec insts;
    DummyFormatter formatter;
    FieldEngine engine(formatter, "test");

    auto d = engine.GetDisassembler(insts);
    d->open("decompiler/test/ff7_control_flow_test.dat");
    d->disassemble();
    d->dumpDisassembly(std::cout);
    std::cout << std::endl;

    uint32 pos = 0;
    ASSERT_EQ(insts[pos]->opcode_, eOpcodes::RET);
    ASSERT_EQ(insts[pos]->params_.size(), 0);

    CheckForwardJump(eOpcodes::IFUB, 6, insts, pos);
    CheckForwardJump(eOpcodes::IFSWL, 6, insts, pos);
    CheckForwardJump(eOpcodes::JMPF, 1, insts, pos);
    CheckForwardJump(eOpcodes::JMPFL, 1, insts, pos);
    CheckForwardJump(eOpcodes::IFKEY, 2, insts, pos);
    CheckForwardJump(eOpcodes::IFKEYON, 2, insts, pos);
    CheckForwardJump(eOpcodes::IFKEYOFF, 2, insts, pos);
    CheckForwardJump(eOpcodes::IFPRTYQ, 2, insts, pos);
    CheckForwardJump(eOpcodes::IFMEMBQ, 2, insts, pos);
    CheckBackwardJump(eOpcodes::JMPB, 1, insts, pos);
    CheckBackwardJump(eOpcodes::JMPBL, 1, insts, pos);
}
