/*
 * InstUtility.cpp
 *
 *  Created on: 2016/03/10
 *      Author: LittleBird
 */

#include "InstUtility.h"

namespace lb {
std::string toUpperString(std::string val) {
    for (unsigned long long i = 0; i < val.length(); ++i) {
        val[i] = static_cast<char>(toupper(static_cast<int>(val[i])));
    }
    return val;
}

int toSigned(const unsigned& src) {
    return static_cast<int>(src);
}

int toSigned(const unsigned& src, const InstSize& type) {
    if (type == InstSize::WORD) {
        return static_cast<int>(src);
    }
    else if (type == InstSize::HALF) {
        int var = static_cast<int>(src << 16);
        int ret = var >> 16;
        return ret;
    }
    else {
        int var = static_cast<int>(src << 24);
        int ret = var >> 24;
        return ret;
    }
}

int toSigned(const unsigned& src, const int& bits) {
    int var = static_cast<int>(src << (32 - bits));
    int ret = var >> (32 - bits);
    return ret;
}

unsigned toUnsigned(const int& src) {
    return static_cast<unsigned>(src);
}

unsigned getBitsInRange(const unsigned& src, const int& l, const int& r) {
    return (src << (32 - r)) >> (l + (32 - r));
}

} /* namespace lb */
