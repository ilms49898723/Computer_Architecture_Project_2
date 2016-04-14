/*
 * InstImageReader.cpp
 *
 *  Created on: 2016/03/24
 *      Author: LittleBird
 */

#include "InstImageReader.h"

namespace lb {

unsigned InstImageReader::readImageI(std::string filePath, unsigned* dst, unsigned* pc) {
    FILE* iimage = fopen(filePath.c_str(), "rb");
    if (!iimage) {
        *pc = 0u;
        return 0u;
    }
    unsigned ret = readImageI(iimage, dst, pc);
    fclose(iimage);
    return ret;
}

unsigned InstImageReader::readImageI(FILE* iimage, unsigned* dst, unsigned* pc) {
    unsigned readPc = readWordFromBin(iimage);
    unsigned len = readWordFromBin(iimage);
    unsigned wordRead = 0;
    *pc = readPc;
    for (wordRead = 0; wordRead < len; ++wordRead) {
        dst[wordRead] = readWordFromBin(iimage);
    }
    return wordRead;
}

unsigned InstImageReader::readImageD(std::string filePath, unsigned* dst, unsigned* sp) {
    FILE* dimage = fopen(filePath.c_str(), "rb");
    if (!dimage) {
        *sp = 0u;
        return 0u;
    }
    unsigned ret = readImageD(dimage, dst, sp);
    fclose(dimage);
    return ret;
}

unsigned InstImageReader::readImageD(FILE* dimage, unsigned* dst, unsigned* sp) {
    unsigned readSp = readWordFromBin(dimage);
    unsigned len = readWordFromBin(dimage);
    unsigned wordRead = 0;
    *sp = readSp;
    for (wordRead = 0; wordRead < len; ++wordRead) {
        dst[wordRead] = readWordFromBin(dimage);
    }
    return wordRead;
}

unsigned char InstImageReader::readByteFromBin(FILE* fin) {
    unsigned char input;
    if (fread(&input, sizeof(unsigned char), 1, fin) != 1u) {
        return 0u;
    }
    return input;
}

unsigned InstImageReader::readWordFromBin(FILE* fin) {
    unsigned char input[4];
    if (fread(input, sizeof(unsigned char), 4, fin) != 4u) {
        return 0u;
    }
    return (input[0] << 24) | (input[1] << 16) | (input[2] << 8) | (input[3]);
}

} /* namespace lb */
