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
enum class InstElementType : unsigned {
    OPCODE, RS, RT, RD, C, FUNCT, UNDEF
};

// enum class for pipeline stages
// NONE, STALL, FORWARD
enum class InstState : unsigned {
    NONE, STALL, FORWARD
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
    R, I, J, S, UNDEF
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
