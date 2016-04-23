/*
 * InstDecoder.h
 *
 *  Created on: 2016/03/09
 *      Author: LittleBird
 */

#ifndef INSTDECODER_H_
#define INSTDECODER_H_

#include <string>
#include "InstDataBin.h"
#include "InstDataStr.h"
#include "InstLookUp.h"
#include "InstUtility.h"
#include "InstEnum.h"

namespace lb {

class InstDecoder {
public:
    // Decode hex-decimal format instruction set to readable string
    // All static functions
    // return type InstData defined in "InstData.h"

    // decode instruction set, saved by string
    static InstDataStr decodeInstStr(const unsigned& src);

    // decode instruction set, save by unsigned
    static InstDataBin decodeInstBin(const unsigned& src);
};

} /* namespace lb */

#endif /* INSTDECODER_H_ */
