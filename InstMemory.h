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

/**
 * memory, 1024 bytes and 32 registers
 */
class InstMemory {

public:
    InstMemory();

    virtual ~InstMemory();

    /**
     * initialize
     */
    void init();

    /**
     * get register value at addr
     * @param addr register number to get
     * @param type load type(WORD, HALF WORD, BYTE)
     */
    unsigned getRegister(const unsigned& addr, const InstSize& type = InstSize::WORD) const;

    /**
     * set register value at addr
     * @param addr register number to set
     * @param val value to set
     * @param type set type(WORD, HALF WORD, BYTE)
     */
    void setRegister(const unsigned& addr, const unsigned& val, const InstSize& type = InstSize::WORD);

    /**
     * get memory value at addr
     * @param addr memory address to get
     * @param type load type(WORD, HALF WORD, BYTE)
     */
    unsigned getMemory(const unsigned& addr, const InstSize& type) const;

    /**
     * set memory value at addr
     * @param addr memory address to set
     * @param val value to set
     * @param type set type(WORD, HALF WORD, BYTE)
     */
    void setMemory(const unsigned& addr, const unsigned& val, const InstSize& type);

private:
    unsigned char mem[1024];
    unsigned reg[32];
};

} /* namespace lb */

#endif /* INSTMEMORY_H_ */
