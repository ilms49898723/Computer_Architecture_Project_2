/*
 * InstEnum.h
 *
 *  Created on: 2016/03/25
 *      Author: LittleBird
 */

#ifndef INSTENUM_H_
#define INSTENUM_H_

namespace lb {

// enum class for instruction elements
// OPCODE, RS, RT, RD, C, FUNCT
enum class InstElement : unsigned {
    OPCODE, RS, RT, RD, C, FUNCT
};

// enum class for pipeline stages
// ID, IF, EM, DM, WB, NONE, STALL
enum class InstStage : unsigned {
    ID, IF, EX, DM, WB, NONE, STALL
};

// enum class for basic operation type
// add, sub, mul, div, mod
enum class InstOpType : unsigned {
    ADD, SUB
};

// enum class for simulate operation
// continue, halt
enum class InstAction : unsigned {
    CONTINUE, HALT
};

// enum class for instruction type
// R-type, I-type, J-type, Specialized, Undefined
enum class InstType : unsigned {
    R, I, J, S, Undef
};

// enum class for memory size type
// WORD: 4 bytes
// HALFWORD: 2 bytes
// BYTE: 1 byte
enum class InstMemLen : unsigned {
    WORD, HALF, BYTE
};

} /* namespace lb */

#endif /* INSTENUM_H_ */
