/*
 * InstLookUp.cpp
 *
 *  Created on: 2016/03/09
 *      Author: LittleBird
 */

#include "InstLookUp.h"

namespace lb {

const std::string InstLookUp::opCodeLookUpTable[] = {
        "R-Type", // 0x00
        "undef",
        "j",
        "jal",
        "beq",
        "bne",    // 0x05
        "undef",
        "bgtz",
        "addi",
        "addiu",
        "slti",   // 0x0A
        "undef",
        "andi",
        "ori",
        "nori",
        "lui",    // 0x0F
        "undef",  // 0x10
        "undef",
        "undef",
        "undef",
        "undef",
        "undef",  // 0x15
        "undef",
        "undef",
        "undef",
        "undef",
        "undef",  // 0x1A
        "undef",
        "undef",
        "undef",
        "undef",
        "undef",  // 0x1F
        "lb",     // 0x20
        "lh",
        "undef",
        "lw",
        "lbu",
        "lhu",    // 0x25
        "undef",
        "undef",
        "sb",
        "sh",
        "undef",  // 0x2A
        "sw"
};

const std::string InstLookUp::functLookUpTable[] = {
        "sll",    // 0x00
        "undef",
        "srl",
        "sra",
        "undef",
        "undef",  // 0x05
        "undef",
        "undef",
        "jr",
        "undef",
        "undef",  // 0x0A
        "undef",
        "undef",
        "undef",
        "undef",
        "undef",  // 0x0F
        "undef",  // 0x10
        "undef",
        "undef",
        "undef",
        "undef",
        "undef",  // 0x15
        "undef",
        "undef",
        "undef",
        "undef",
        "undef",  // 0x1A
        "undef",
        "undef",
        "undef",
        "undef",
        "undef",  // 0x1F
        "add",    // 0x20
        "addu",
        "sub",
        "undef",
        "and",
        "or",     // 0x25
        "xor",
        "nor",
        "nand",
        "undef",
        "slt"     // 0x30
};

std::string InstLookUp::opCodeLookUp(const unsigned& src) {
    if (src == 0x3Fu) {
        return "halt";
    }
    if (src > 0x2Bu) {
        return "undef";
    }
    return InstLookUp::opCodeLookUpTable[src];
}

std::string InstLookUp::functLookUp(const unsigned& src) {
    if (src > 0x30u) {
        return "undef";
    }
    return InstLookUp::functLookUpTable[src];
}

std::string InstLookUp::registerLookUpNumber(const unsigned& src) {
    if (src > 0x1Fu) {
        return "undef";
    }
    return toString(src);
}

} /* namespace lb */
