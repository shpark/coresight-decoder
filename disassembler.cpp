#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "disassembler.hpp"

// https://www.mztn.org/dragon/arm6408cond.html
static const uint16_t branch_opcode[] = {
    // conditional branch
    ARM64_INS_CBZ,
    ARM64_INS_CBNZ,
    ARM64_INS_TBZ,
    ARM64_INS_TBNZ,

    // unconditional direct branch
    ARM64_INS_B,
    ARM64_INS_BL,

    // indirect branch
    ARM64_INS_BR,
    ARM64_INS_BLR,
    ARM64_INS_RET,
    ARM64_INS_ERET,
};

void disassembleInit(csh* handle)
{
    // Initialize capstone
    cs_err err = cs_open(CS_ARCH_ARM64, CS_MODE_ARM, handle);
    if (err != CS_ERR_OK) {
        std::cerr << "Failed on cs_open() with error returned: " << err << std::endl;
        std::exit(1);
    }
}

void disassembleDelete(csh* handle)
{
    cs_close(handle);
}

// https://www.capstone-engine.org/iteration.html
cs_insn* disassembleNextBranchInsn(const csh* handle, const std::vector<uint8_t> code, const uint64_t offset)
{
    const uint8_t *code_ptr = &code[0] + offset;
    size_t code_size = code.size() - offset;
    uint64_t address = offset; // address of first instruction to be disassembled

    // allocate memory cache for 1 instruction, to be used by cs_disasm_iter later.
    cs_insn *insn = cs_malloc(*handle);

    // disassemble one instruction a time & store the result into @insn variable above
    while(cs_disasm_iter(*handle, &code_ptr, &code_size, &address, insn)) {
        std::cout << "MNEMONIC: " << insn->mnemonic << " ADDRESS: " << std::hex << insn->address << std::endl;
        // analyze disassembled instruction in @insn variable
        // NOTE: @code_ptr, @code_size & @address variables are all updated
        // to point to the next instruction after each iteration.
        for (size_t i = 0; i < sizeof(branch_opcode) / sizeof(uint16_t); ++i) {
            if (insn->id == branch_opcode[i]) {
                std::cout << "OP_STR: " << insn->op_str << std::endl;
                return insn;
            }
        }
    }

    // release the cache memory when done
    cs_free(insn, 1);

    std::cerr << "Cannot find branch instruction" << std::endl;
    std::exit(1);
}


uint64_t getAddressFromInsn(const cs_insn *insn)
{
    // insn->op_str is #Addr format. ex) 0x72c -> #72c
    uint64_t address = std::stol(insn->op_str + 1, nullptr, 16);
    return address;
}