/*
 * InstErrorDetector.h
 *
 *  Created on: 2016/03/23
 *      Author: LittleBird
 */

#ifndef INSTERRORDETECTOR_H_
#define INSTERRORDETECTOR_H_

#include "InstDataBin.h"
#include "InstDataStr.h"
#include "InstUtility.h"
#include "InstMemory.h"
#include "InstEnum.h"

namespace lb {

// detect error
// write to $0
// overflow
// memory addr overflow
// data misaligned
class InstErrorDetector {
public:
    // reg: addr to check
    static bool isRegWritable(const unsigned& reg);

    // check overflow
    // check src0 op src1, where op == ADD or SUB
    static bool isOverflowed(const int &src0, const int &src1, const InstOpType& op);

    // check memory address(should between 0 to 1023)
    static bool isValidMemoryAddr(const unsigned& addr, const InstMemLen& type);

    // check misaligned address
    static bool isAlignedAddr(const unsigned& addr, const InstMemLen& type);
};

} /* namespace lb */

#endif /* INSTERRORDETECTOR_H_ */
