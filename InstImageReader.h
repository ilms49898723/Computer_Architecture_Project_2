/*
 * InstImageReader.h
 *
 *  Created on: 2016/03/24
 *      Author: LittleBird
 */

#ifndef INSTIMAGEREADER_H_
#define INSTIMAGEREADER_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>

namespace lb {

/**
 * a reader to read iimage.bin, dimage.bin by using C I/O
 */
class InstImageReader {
public:
    static unsigned readImageI(const std::string& filePath, unsigned* dst, unsigned* pc);

    static unsigned readImageI(FILE* iimage, unsigned* dst, unsigned* pc);

    static unsigned readImageD(const std::string& filePath, unsigned* dst, unsigned* sp);

    static unsigned readImageD(FILE* dimage, unsigned* dst, unsigned* sp);

public:
    static unsigned readWordFromBin(FILE* fin);
};

} /* namespace lb */

#endif /* INSTIMAGEREADER_H_ */
