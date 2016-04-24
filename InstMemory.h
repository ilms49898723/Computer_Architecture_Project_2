/*
 * InstMemory.h
 *
 *  Created on: 2016/03/10
 *      Author: LittleBird
 */

#ifndef INSTMEMORY_H_
#define INSTMEMORY_H_

#include <cstring>
#include <string>
#include "InstUtility.h"
#include "InstEnum.h"

namespace lb {

// 1024 bytes memory + 32 registers
class InstMemory {
public:
    InstMemory();

    virtual ~InstMemory();

    // initialize
    void init();

    // get reg value at addr
    unsigned getRegValue(const unsigned& addr, const InstMemLen& type = InstMemLen::WORD) const;

    // set reg value to addr
    void setRegValue(const unsigned& addr, const unsigned& val, const InstMemLen& type = InstMemLen::WORD);

    // get memory value at addr, return as unsigned
    unsigned getMemValue(const unsigned& addr, const InstMemLen& type) const;

    // set memory value at addr, parameter all passed as unsigned
    void setMemValue(const unsigned& addr, const unsigned& val, const InstMemLen& type);

private:
    unsigned char mem[1024];
    unsigned reg[32];
};

} /* namespace lb */

#endif /* INSTMEMORY_H_ */
