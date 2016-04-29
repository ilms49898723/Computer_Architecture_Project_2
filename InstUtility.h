/*
 * InstUtility.h
 *
 *  Created on: 2016/03/10
 *      Author: LittleBird
 */

#ifndef INSTUTILITY_H_
#define INSTUTILITY_H_

#include <cctype>
#include <sstream>
#include <string>
#include "InstType.h"

namespace lb {

/**
 * to std::string, if has number, use decimal
 * @param val value to change to string
 */
template<typename Tp>
std::string toString(const Tp& val) {
    std::basic_stringstream<char> oss;
    oss << val;
    return oss.str();
}

/**
 * to std::string, if has number, use hex-decimal
 * @param val value to change to string
 */
template<typename Tp>
std::string toHexString(const Tp& val) {
    std::basic_stringstream<char> oss;
    oss << std::showbase << std::hex << val;
    return oss.str();
}

/**
 * to upper string, "abc" -> "ABC"
 * @param val string to process
 */
std::string toUpperString(std::string val);

/**
 * extend number from unsigned to 32 bits signed,
 * no sign-extended
 * @param src number to change
 */
int toSigned(const unsigned& src);

/**
 * extend number from unsigned to 32 bits signed,
 * sign-extended
 * @param src number to change
 * @param type source number valid bits(WORD, HALFWORD, BYTE)
 */
int toSigned(const unsigned& src, const lb::InstSize& type);

/**
 * extend number from unsigned to 32 bits signed,
 * sign-extended
 * @param src number to change
 * @param type source number valid bits
 */
int toSigned(const unsigned& src, const int& bits);

/**
 * change signed number to unsigned number
 * @param src number to change
 */
unsigned toUnsigned(const int& src);

/**
 * get bits from range [l, r)
 * index starts from 0
 * @param src source number
 * @param l left bound(inclusive)
 * @param r right bound(exclusive)
 */
unsigned getBitsInRange(const unsigned& src, const int& l, const int& r);

} /* namespace lb */

#endif /* INSTUTILITY_H_ */
