/*
 * InstErrorDetector.cpp
 *
 *  Created on: 2016/03/23
 *      Author: LittleBird
 */

#include "InstErrorDetector.h"

namespace lb {

bool InstErrorDetector::isRegWritable(const unsigned& reg) {
    return reg != 0u;
}

bool InstErrorDetector::isOverflowed(const int& src0, const int& src1, const InstOpType& op) {
    int a = src0;
    int b = src1;
    if (op == InstOpType::SUB) {
        b *= (-1);
    }
    int result = a + b;
    int signA = (a >= 0) ? 1 : -1;
    int signB = (b >= 0) ? 1 : -1;
    int signR = (result >= 0) ? 1 : -1;
    return signA * signB >= 0 && signA * signR < 0;
}

bool InstErrorDetector::isValidMemoryAddr(const unsigned& addr, const InstSize& type) {
    if (type == InstSize::WORD) {
        return addr < 1024u && (addr + 4 - 1) < 1024u;
    }
    else if (type == InstSize::HALF) {
        return addr < 1024u && (addr + 2 - 1) < 1024u;
    }
    else {
        return addr < 1024u;
    }
}

bool InstErrorDetector::isAlignedAddr(const unsigned& addr, const InstSize& type) {
    if (type == InstSize::WORD) {
        return (addr % 4) == 0;
    }
    else if (type == InstSize::HALF) {
        return (addr & 1) == 0;
    }
    else {
        return true;
    }
}

} /* namespace lb */
