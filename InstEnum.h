/*
 * InstEnum.h
 *
 *  Created on: 2016/03/25
 *      Author: LittleBird
 */

#ifndef INSTENUM_H_
#define INSTENUM_H_

namespace lb {

// enum class for basic operation type
// add, sub, mul, div, mod
enum class InstOpType : unsigned {
    ADD, SUB, MUL, DIV, MOD
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
