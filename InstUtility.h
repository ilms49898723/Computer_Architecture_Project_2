/*
 * InstUtility.h
 *
 *  Created on: 2016/03/10
 *      Author: LittleBird
 */

#ifndef INSTUTILITY_H_
#define INSTUTILITY_H_

#include <sstream>
#include <string>
#include "InstEnum.h"

namespace lb {

template<typename Tp>
std::string toString(const Tp& val) {
    std::basic_stringstream<char> oss;
    oss << val;
    return oss.str();
}

// if has number, use decimal
template<typename Tp>
std::string toDecString(const Tp& val) {
    std::basic_stringstream<char> oss;
    oss << val;
    return oss.str();
}

// if has number, use hex-decimal
template<typename Tp>
std::string toHexString(const Tp& val) {
    std::basic_stringstream<char> oss;
    oss << std::showbase << std::hex << val;
    return oss.str();
}

// for convenience, only use static_cast<int>
int toSigned(const unsigned& src);

// extend sign bits
// ex. 1 byte "0x80" to 4 bytes "0xFFFFFF80"
int toSigned(const unsigned& src, const lb::InstMemLen& type);

// extend sign bits
// similar to int toSigned(const unsigned& src, const LB::InstMemLen& type);
// argument type pass by bits number instead
// bits: src bits
int toSigned(const unsigned& src, const int& bits);

// for convenience, only use static_cast<unsigned>
unsigned toUnsigned(const int& src);

// get bits from range [l, r)
// ex. getBitsInRange(0xC, 0, 3) returns 0x4
// 0xC = 0b1100 returns 0b100
// zero based
unsigned getBitsInRange(const unsigned& src, const int& l, const int& r);

} /* namespace lb */

#endif /* INSTUTILITY_H_ */
