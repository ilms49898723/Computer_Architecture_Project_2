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
#include "InstType.h"

namespace lb {

// 1024 bytes memory + 32 registers
class InstMemory {
public:
    InstMemory();

    virtual ~InstMemory();

    // initialize
    void init();

    // get reg value at addr
    unsigned getRegister(const unsigned& addr, const InstSize& type = InstSize::WORD) const;

    // set reg value to addr
    void setRegister(const unsigned& addr, const unsigned& val, const InstSize& type = InstSize::WORD);

    // get memory value at addr, return as unsigned
    unsigned getMemory(const unsigned& addr, const InstSize& type) const;

    // set memory value at addr, parameter all passed as unsigned
    void setMemory(const unsigned& addr, const unsigned& val, const InstSize& type);

private:
    unsigned char mem[1024];
    unsigned reg[32];
};

} /* namespace lb */

#endif /* INSTMEMORY_H_ */
