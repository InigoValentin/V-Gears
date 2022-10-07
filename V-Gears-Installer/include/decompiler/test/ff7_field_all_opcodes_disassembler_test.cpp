#include <gmock/gmock.h>

#include "../ControlFlow.h"
#include "decompiler/ff7_field/ff7_field_disassembler.h"
#include "decompiler/ff7_field/ff7_field_engine.h"
#include "decompiler/ff7_field/ff7_field_codegen.h"
#include "util.h"
#include "ff7_field_dummy_formatter.h"

#define MAKE_SUBOPCODE(high, low) ((uint16)(((uint8)(low)) | ((uint16)((uint8)(high))) << 8))

TEST(FF7Field, AllOpcodesDisassembler)
{
    InstVec insts;
    DummyFormatter formatter;
    FieldEngine engine(formatter);

    auto d = engine.GetDisassembler(insts);
    d->open("decompiler/test/ff7_all_opcodes.dat");
    d->disassemble();
    d->dumpDisassembly(std::cout);
    std::cout << std::endl;


    ASSERT_EQ(insts[0]->_opcode, eOpcodes::RET);
    ASSERT_EQ(insts[0]->_params.size(), 0);

	ASSERT_EQ(insts[1]->_opcode, eOpcodes::REQ);
	ASSERT_EQ(insts[1]->_params.size(), 3);
	ASSERT_EQ(insts[1]->_params[0]->getSigned(), 0);
	ASSERT_EQ(insts[1]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[1]->_params[2]->getSigned(), 1);

	ASSERT_EQ(insts[2]->_opcode, eOpcodes::REQSW);
	ASSERT_EQ(insts[2]->_params.size(), 3);
	ASSERT_EQ(insts[2]->_params[0]->getSigned(), 0);
	ASSERT_EQ(insts[2]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[2]->_params[2]->getSigned(), 1);

	ASSERT_EQ(insts[3]->_opcode, eOpcodes::REQEW);
	ASSERT_EQ(insts[3]->_params.size(), 3);
	ASSERT_EQ(insts[3]->_params[0]->getSigned(), 0);
	ASSERT_EQ(insts[3]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[3]->_params[2]->getSigned(), 1);

	ASSERT_EQ(insts[4]->_opcode, eOpcodes::PREQ);
	ASSERT_EQ(insts[4]->_params.size(), 3);
	ASSERT_EQ(insts[4]->_params[0]->getSigned(), 0);
	ASSERT_EQ(insts[4]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[4]->_params[2]->getSigned(), 1);

	ASSERT_EQ(insts[5]->_opcode, eOpcodes::PREQSW);
	ASSERT_EQ(insts[5]->_params.size(), 3);
	ASSERT_EQ(insts[5]->_params[0]->getSigned(), 0);
	ASSERT_EQ(insts[5]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[5]->_params[2]->getSigned(), 1);

	ASSERT_EQ(insts[6]->_opcode, eOpcodes::PREQEW);
	ASSERT_EQ(insts[6]->_params.size(), 3);
	ASSERT_EQ(insts[6]->_params[0]->getSigned(), 0);
	ASSERT_EQ(insts[6]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[6]->_params[2]->getSigned(), 1);

	ASSERT_EQ(insts[7]->_opcode, eOpcodes::JOIN);
	ASSERT_EQ(insts[7]->_params.size(), 1);
	ASSERT_EQ(insts[7]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[8]->_opcode, eOpcodes::SPLIT);
	ASSERT_EQ(insts[8]->_params.size(), 13);
	ASSERT_EQ(insts[8]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[8]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[8]->_params[2]->getSigned(), 3);
	ASSERT_EQ(insts[8]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[8]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[8]->_params[5]->getSigned(), 6);
	ASSERT_EQ(insts[8]->_params[6]->getSigned(), 7);
	ASSERT_EQ(insts[8]->_params[7]->getSigned(), 8);
	ASSERT_EQ(insts[8]->_params[8]->getSigned(), 9);
	ASSERT_EQ(insts[8]->_params[9]->getSigned(), 10);
	ASSERT_EQ(insts[8]->_params[10]->getSigned(), 11);
	ASSERT_EQ(insts[8]->_params[11]->getSigned(), 12);
	ASSERT_EQ(insts[8]->_params[12]->getSigned(), 13);

	ASSERT_EQ(insts[9]->_opcode, eOpcodes::SPTYE);
	ASSERT_EQ(insts[9]->_params.size(), 7);
	ASSERT_EQ(insts[9]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[9]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[9]->_params[2]->getSigned(), 3);
	ASSERT_EQ(insts[9]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[9]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[9]->_params[5]->getSigned(), 6);
	ASSERT_EQ(insts[9]->_params[6]->getSigned(), 7);

	ASSERT_EQ(insts[10]->_opcode, eOpcodes::GTPYE);
	ASSERT_EQ(insts[10]->_params.size(), 7);
	ASSERT_EQ(insts[10]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[10]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[10]->_params[2]->getSigned(), 3);
	ASSERT_EQ(insts[10]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[10]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[10]->_params[5]->getSigned(), 6);
	ASSERT_EQ(insts[10]->_params[6]->getSigned(), 7);

	ASSERT_EQ(insts[11]->_opcode, eOpcodes::DSKCG);
	ASSERT_EQ(insts[11]->_params.size(), 1);
	ASSERT_EQ(insts[11]->_params[0]->getSigned(), 1);

	
	ASSERT_EQ(insts[12]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::ARROW));
	ASSERT_EQ(insts[12]->_params.size(), 1);
	ASSERT_EQ(insts[12]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[13]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::PNAME));
	ASSERT_EQ(insts[13]->_params.size(), 1);
	ASSERT_EQ(insts[13]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[14]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::GMSPD));
	ASSERT_EQ(insts[14]->_params.size(), 1);
	ASSERT_EQ(insts[14]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[15]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::SMSPD));
	ASSERT_EQ(insts[15]->_params.size(), 2);
	ASSERT_EQ(insts[15]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[15]->_params[1]->getSigned(), 2);

	ASSERT_EQ(insts[16]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::FLMAT));
	ASSERT_EQ(insts[16]->_params.size(), 0);

	ASSERT_EQ(insts[17]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::FLITM));
	ASSERT_EQ(insts[17]->_params.size(), 0);

	ASSERT_EQ(insts[18]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::BTLCK));
	ASSERT_EQ(insts[18]->_params.size(), 1);
	ASSERT_EQ(insts[18]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[19]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::MVLCK));
	ASSERT_EQ(insts[19]->_params.size(), 1);
	ASSERT_EQ(insts[19]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[20]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::SPCNM));
	ASSERT_EQ(insts[20]->_params.size(), 2);
	ASSERT_EQ(insts[20]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[20]->_params[1]->getSigned(), 2);

	ASSERT_EQ(insts[21]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::RSGLB));
	ASSERT_EQ(insts[21]->_params.size(), 0);

	ASSERT_EQ(insts[22]->_opcode, MAKE_SUBOPCODE(eOpcodes::SPECIAL, eSpecialOpcodes::CLITM));
	ASSERT_EQ(insts[22]->_params.size(), 0);

	ASSERT_EQ(insts[23]->_opcode, eOpcodes::JMPF);
	ASSERT_EQ(insts[23]->_params.size(), 1);

	ASSERT_EQ(insts[24]->_opcode, eOpcodes::JMPFL);
	ASSERT_EQ(insts[24]->_params.size(), 1);

	ASSERT_EQ(insts[25]->_opcode, eOpcodes::JMPB);
	ASSERT_EQ(insts[25]->_params.size(), 1);

	ASSERT_EQ(insts[26]->_opcode, eOpcodes::JMPBL);
	ASSERT_EQ(insts[26]->_params.size(), 1);

	ASSERT_EQ(insts[27]->_opcode, eOpcodes::IFUB);
	ASSERT_EQ(insts[27]->_params.size(), 6);
	ASSERT_EQ(insts[27]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[27]->_params[1]->getSigned(), 3);
	ASSERT_EQ(insts[27]->_params[2]->getSigned(), 2);
	ASSERT_EQ(insts[27]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[27]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[27]->_params[5]->getSigned(), 1);

	ASSERT_EQ(insts[28]->_opcode, eOpcodes::IFUBL);
	ASSERT_EQ(insts[28]->_params.size(), 6);
	ASSERT_EQ(insts[28]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[28]->_params[1]->getSigned(), 3);
	ASSERT_EQ(insts[28]->_params[2]->getSigned(), 2);
	ASSERT_EQ(insts[28]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[28]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[28]->_params[5]->getSigned(), 2); // For some reason this one has to be 2

	ASSERT_EQ(insts[29]->_opcode, eOpcodes::IFSW);
	ASSERT_EQ(insts[29]->_params.size(), 6);
	ASSERT_EQ(insts[29]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[29]->_params[1]->getSigned(), 3);
	ASSERT_EQ(insts[29]->_params[2]->getSigned(), 2);
	ASSERT_EQ(insts[29]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[29]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[29]->_params[5]->getSigned(), 1);

	ASSERT_EQ(insts[30]->_opcode, eOpcodes::IFSWL);
	ASSERT_EQ(insts[30]->_params.size(), 6);
	ASSERT_EQ(insts[30]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[30]->_params[1]->getSigned(), 3);
	ASSERT_EQ(insts[30]->_params[2]->getSigned(), 2);
	ASSERT_EQ(insts[30]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[30]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[30]->_params[5]->getSigned(), 2); // For some reason this one has to be 2

	ASSERT_EQ(insts[31]->_opcode, eOpcodes::IFUW);
	ASSERT_EQ(insts[31]->_params.size(), 6);
	ASSERT_EQ(insts[31]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[31]->_params[1]->getSigned(), 3);
	ASSERT_EQ(insts[31]->_params[2]->getSigned(), 2);
	ASSERT_EQ(insts[31]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[31]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[31]->_params[5]->getSigned(), 1);

	ASSERT_EQ(insts[32]->_opcode, eOpcodes::IFUWL);
	ASSERT_EQ(insts[32]->_params.size(), 6);
	ASSERT_EQ(insts[32]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[32]->_params[1]->getSigned(), 3);
	ASSERT_EQ(insts[32]->_params[2]->getSigned(), 2);
	ASSERT_EQ(insts[32]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[32]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[32]->_params[5]->getSigned(), 2); // For some reason this one has to be 2

	ASSERT_EQ(insts[33]->_opcode, eOpcodes::MINIGAME);
	ASSERT_EQ(insts[33]->_params.size(), 6);
	ASSERT_EQ(insts[33]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[33]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[33]->_params[2]->getSigned(), 3);
	ASSERT_EQ(insts[33]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[33]->_params[4]->getSigned(), 5);
	ASSERT_EQ(insts[33]->_params[5]->getSigned(), 6);

	ASSERT_EQ(insts[34]->_opcode, eOpcodes::TUTOR);
	ASSERT_EQ(insts[34]->_params.size(), 1);
	ASSERT_EQ(insts[34]->_params[0]->getSigned(), 1);

	// BATTLE MODE STATES START

	ASSERT_EQ(insts[35]->_opcode, eOpcodes::BTMD2);
	ASSERT_EQ(insts[35]->_params.size(), 4);
	ASSERT_EQ(insts[35]->_params[0]->getSigned(), 0x80);

	ASSERT_EQ(insts[36]->_opcode, eOpcodes::BTMD2);
	ASSERT_EQ(insts[36]->_params.size(), 4);
	ASSERT_EQ(insts[36]->_params[0]->getSigned(), 0x40);

	ASSERT_EQ(insts[37]->_opcode, eOpcodes::BTMD2);
	ASSERT_EQ(insts[37]->_params.size(), 4);
	ASSERT_EQ(insts[37]->_params[0]->getSigned(), 0x20);

	ASSERT_EQ(insts[38]->_opcode, eOpcodes::BTMD2);
	ASSERT_EQ(insts[38]->_params.size(), 4);
	ASSERT_EQ(insts[38]->_params[0]->getSigned(), 0x10);

	ASSERT_EQ(insts[39]->_opcode, eOpcodes::BTMD2);
	ASSERT_EQ(insts[39]->_params.size(), 4);
	ASSERT_EQ(insts[39]->_params[0]->getSigned(), 0x08);

	ASSERT_EQ(insts[40]->_opcode, eOpcodes::BTMD2);
	ASSERT_EQ(insts[40]->_params.size(), 4);
	ASSERT_EQ(insts[40]->_params[0]->getSigned(), 0x04);

	ASSERT_EQ(insts[41]->_opcode, eOpcodes::BTMD2);
	ASSERT_EQ(insts[41]->_params.size(), 4);
	ASSERT_EQ(insts[41]->_params[0]->getSigned(), 0x02);

	//ASSERT_EQ(insts[42]->_opcode, eOpcodes::BTMD2);
	//ASSERT_EQ(insts[42]->_params.size(), 4);
	//ASSERT_EQ(insts[42]->_params[1]->getSigned(), 0x80);

	ASSERT_EQ(insts[67]->_opcode, eOpcodes::BTRLD);
	ASSERT_EQ(insts[67]->_params.size(), 2);
	ASSERT_EQ(insts[67]->_params[0]->getSigned(), 1);
	//ASSERT_EQ(insts[67]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[68]->_opcode, eOpcodes::WAIT);
	ASSERT_EQ(insts[68]->_params.size(), 1);
	ASSERT_EQ(insts[68]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[69]->_opcode, eOpcodes::NFADE);
	ASSERT_EQ(insts[69]->_params.size(), 8);
	//ASSERT_EQ(insts[69]->_params[0]->getSigned(), 1); // TODO:These values are what they are set to in Makou Reactor however SUDM reads them as something else Reads:(18)
	//ASSERT_EQ(insts[69]->_params[1]->getSigned(), 2); // TODO:These values are what they are set to in Makou Reactor however SUDM reads them as something else Reads:(52)
	//ASSERT_EQ(insts[69]->_params[2]->getSigned(), 3); // TODO:These values are what they are set to in Makou Reactor however SUDM reads them as something else Reads:(5)
	//ASSERT_EQ(insts[69]->_params[3]->getSigned(), 4); // TODO:These values are what they are set to in Makou Reactor however SUDM reads them as something else Reads:(6)
	//ASSERT_EQ(insts[69]->_params[4]->getSigned(), 5); // TODO:These values are what they are set to in Makou Reactor however SUDM reads them as something else Reads:(0)
	//ASSERT_EQ(insts[69]->_params[5]->getSigned(), 6); // TODO:These values are what they are set to in Makou Reactor however SUDM reads them as something else Reads:(0)
	ASSERT_EQ(insts[69]->_params[6]->getSigned(), 7);
	ASSERT_EQ(insts[69]->_params[7]->getSigned(), 8);

	ASSERT_EQ(insts[70]->_opcode, eOpcodes::BLINK);
	ASSERT_EQ(insts[70]->_params.size(), 1);
	ASSERT_EQ(insts[70]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[71]->_opcode, eOpcodes::BGMOVIE);
	ASSERT_EQ(insts[71]->_params.size(), 1);
	ASSERT_EQ(insts[71]->_params[0]->getSigned(), 1);

	//KAWAI is normally here

	ASSERT_EQ(insts[72]->_opcode, eOpcodes::KAWIW);
	ASSERT_EQ(insts[72]->_params.size(), 0);

	ASSERT_EQ(insts[73]->_opcode, eOpcodes::PMOVA);
	ASSERT_EQ(insts[73]->_params.size(), 1);
	ASSERT_EQ(insts[73]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[74]->_opcode, eOpcodes::SLIP);
	ASSERT_EQ(insts[74]->_params.size(), 1);
	ASSERT_EQ(insts[74]->_params[0]->getSigned(), 1);

	ASSERT_EQ(insts[75]->_opcode, eOpcodes::BGPDH);
	ASSERT_EQ(insts[75]->_params.size(), 4);
	ASSERT_EQ(insts[75]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[75]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[75]->_params[2]->getSigned(), 3);
	ASSERT_EQ(insts[75]->_params[3]->getSigned(), 4);

	//ASSERT_EQ(insts[76]->_opcode, eOpcodes::BGSCR);
	//ASSERT_EQ(insts[76]->_params.size(), 5);
	//ASSERT_EQ(insts[76]->_params[0]->getSigned(), 1);
	//ASSERT_EQ(insts[76]->_params[1]->getSigned(), 2);
	//ASSERT_EQ(insts[76]->_params[2]->getSigned(), 3);
	//ASSERT_EQ(insts[76]->_params[3]->getSigned(), 4);
	//ASSERT_EQ(insts[76]->_params[4]->getSigned(), 5);

	ASSERT_EQ(insts[78]->_opcode, eOpcodes::WSIZW);
	ASSERT_EQ(insts[78]->_params.size(), 5);
	ASSERT_EQ(insts[78]->_params[0]->getSigned(), 1);
	ASSERT_EQ(insts[78]->_params[1]->getSigned(), 2);
	ASSERT_EQ(insts[78]->_params[2]->getSigned(), 3);
	ASSERT_EQ(insts[78]->_params[3]->getSigned(), 4);
	ASSERT_EQ(insts[78]->_params[4]->getSigned(), 5);

	//ASSERT_EQ(insts[79]->_opcode, eOpcodes::IFKEY); // TODO
	//ASSERT_EQ(insts[79]->_params.size(), 2);
	//ASSERT_EQ(0x0001, insts[79]->_params[0]->getSigned());

	//ASSERT_EQ(insts[80]->_opcode, eOpcodes::IFKEY); // TODO
	//ASSERT_EQ(insts[80]->_params.size(), 2);
	//ASSERT_EQ(0x8000, insts[78]->_params[0]->getSigned());

	//ASSERT_EQ(insts[81]->_opcode, eOpcodes::IFKEYON); // TODO
	//ASSERT_EQ(insts[81]->_params.size(), 2);
	//ASSERT_EQ(0x0001, insts[81]->_params[0]->getSigned());

	//ASSERT_EQ(insts[82]->_opcode, eOpcodes::IFKEYON); // TODO
	//ASSERT_EQ(insts[82]->_params.size(), 2);
	//ASSERT_EQ(0x8000, insts[82]->_params[0]->getSigned());

	//ASSERT_EQ(insts[83]->_opcode, eOpcodes::IFKEYOFF); // TODO
	//ASSERT_EQ(insts[83]->_params.size(), 2);
	//ASSERT_EQ(0x0001, insts[83]->_params[0]->getSigned());

	//ASSERT_EQ(insts[84]->_opcode, eOpcodes::IFKEYOFF); // TODO
	//ASSERT_EQ(insts[84]->_params.size(), 2);
	//ASSERT_EQ(0x8000, insts[84]->_params[0]->getSigned());

	ASSERT_EQ(insts[85]->_opcode, eOpcodes::UC);
	ASSERT_EQ(1, insts[85]->_params.size());
	ASSERT_EQ(1, insts[85]->_params[0]->getSigned());

	ASSERT_EQ(insts[86]->_opcode, eOpcodes::PDIRA);
	ASSERT_EQ(1, insts[86]->_params.size());
	ASSERT_EQ(1, insts[86]->_params[0]->getSigned());

	ASSERT_EQ(insts[87]->_opcode, eOpcodes::PTURA);
	ASSERT_EQ(3, insts[87]->_params.size());
	ASSERT_EQ(1, insts[87]->_params[0]->getSigned());
	ASSERT_EQ(2, insts[87]->_params[1]->getSigned());
	ASSERT_EQ(3, insts[87]->_params[2]->getSigned());

	ASSERT_EQ(insts[88]->_opcode, eOpcodes::WSPCL);
	ASSERT_EQ(4, insts[88]->_params.size());
	ASSERT_EQ(1, insts[88]->_params[0]->getSigned());
	ASSERT_EQ(2, insts[88]->_params[1]->getSigned());
	ASSERT_EQ(3, insts[88]->_params[2]->getSigned());
	ASSERT_EQ(4, insts[88]->_params[3]->getSigned());

	ASSERT_EQ(insts[89]->_opcode, eOpcodes::STTIM);
	ASSERT_EQ(7, insts[89]->_params.size());
	ASSERT_EQ(1, insts[89]->_params[0]->getSigned());
	ASSERT_EQ(2, insts[89]->_params[1]->getSigned());
	ASSERT_EQ(3, insts[89]->_params[2]->getSigned());
	ASSERT_EQ(4, insts[89]->_params[3]->getSigned());
	ASSERT_EQ(5, insts[89]->_params[4]->getSigned());
	ASSERT_EQ(6, insts[89]->_params[5]->getSigned());
	ASSERT_EQ(7, insts[89]->_params[6]->getSigned());

    ASSERT_EQ(insts[90]->_opcode, eOpcodes::GOLDU);
    ASSERT_EQ(3, insts[90]->_params.size());
    ASSERT_EQ(0, insts[90]->_params[0]->getSigned());
    ASSERT_EQ(0, insts[90]->_params[1]->getSigned());
    ASSERT_EQ(1, insts[90]->_params[2]->getSigned());

    ASSERT_EQ(insts[91]->_opcode, eOpcodes::GOLDU);
    ASSERT_EQ(3, insts[91]->_params.size());
    ASSERT_EQ(1, insts[91]->_params[0]->getSigned());
    ASSERT_EQ(2, insts[91]->_params[1]->getSigned());
    ASSERT_EQ(0, insts[91]->_params[2]->getSigned());

    ASSERT_EQ(insts[92]->_opcode, eOpcodes::GOLDD);
    ASSERT_EQ(3, insts[92]->_params.size());
    ASSERT_EQ(0, insts[92]->_params[0]->getSigned());
    ASSERT_EQ(0, insts[92]->_params[1]->getSigned());
    ASSERT_EQ(1, insts[92]->_params[2]->getSigned());

    ASSERT_EQ(insts[93]->_opcode, eOpcodes::GOLDD);
    ASSERT_EQ(3, insts[93]->_params.size());
    ASSERT_EQ(1, insts[93]->_params[0]->getSigned());
    ASSERT_EQ(2, insts[93]->_params[1]->getSigned());
    ASSERT_EQ(0, insts[93]->_params[2]->getSigned());

    ASSERT_EQ(insts[94]->_opcode, eOpcodes::CHGLD);
    ASSERT_EQ(4, insts[94]->_params.size());
    ASSERT_EQ(1, insts[94]->_params[0]->getSigned());
    ASSERT_EQ(2, insts[94]->_params[1]->getSigned());
    ASSERT_EQ(3, insts[94]->_params[2]->getSigned());
    ASSERT_EQ(4, insts[94]->_params[3]->getSigned());



    // If it had a value then check the values using:
    //ASSERT_EQ(insts[0]->_params[0]->getSigned(), 1);

	// To check amount of parameters use:
	//ASSERT_EQ(insts[1]->_params.size(), 1);

}
