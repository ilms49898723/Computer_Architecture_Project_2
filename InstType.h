/*
 * InstType.h
 *
 *  Created on: 2016/03/25
 *      Author: LittleBird
 */

#ifndef INSTTYPE_H_
#define INSTTYPE_H_

namespace lb {

/**
 * enum class for instruction elements
 * OPCODE, RS, RT, RD, C, FUNCT
 */
enum class InstElementType : unsigned {
    OPCODE, RS, RT, RD, C, FUNCT, UNDEF
};

/**
 * enum class for pipeline states
 * NONE, STALL, FORWARD
 */
enum class InstState : unsigned {
    NONE, STALL, FORWARD
};

/**
 * enum class for basic operation type
 * add, sub
 */
enum class InstOpType : unsigned {
    ADD, SUB
};

/**
 * enum class for simulate operation
 * continue, halt
 */
enum class InstAction : unsigned {
    CONTINUE, HALT
};

/**
 * enum class for instruction type
 * R-type, I-type, J-type, Specialized, Undefined
 */
enum class InstType : unsigned {
    R, I, J, S, UNDEF
};

/**
 * enum class for memory size type
 * WORD: 4 bytes
 * HALFWORD: 2 bytes
 * BYTE: 1 byte
 */
enum class InstSize : unsigned {
    WORD, HALF, BYTE
};

/**
 * structure for record inst elements
 * rs, rt, rd, etc.
 */
struct InstElement {
    unsigned val;
    InstElementType type;

    InstElement(unsigned val = 0, InstElementType type = InstElementType::UNDEF) :
            val(val), type(type) { }
};

} /* namespace lb */

#endif /* INSTTYPE_H_ */
